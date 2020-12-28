#include <iostream>
#include <vector>
#include <cmath>
#include "math/Headers/mvector.h"
#include "BMPWriter.h"
#include "OObject.h"

#define SHADOW_ENABLED 1
#define REFLECTIONS_ENABLED 1

constexpr uint32_t HEIGHT = 1080;
constexpr uint32_t WIDTH = 1920;
constexpr uint8_t FOV = 85;                    //fov in degrees;
constexpr uint8_t RAY_SAMPLES = 2;


constexpr double FOVR = (FOV * PI / 180);     //fov in radians
#define BACKGROUND_COLOR Vector3(0.4, 0.2, 0.65)
#define AMBIENT Vector3(0.0, 0.0, 0.0)

template<class T>
constexpr T Clamp(T X, T Low, T High)
{
    return std::min(std::max(X, Low), High);
}


namespace BRDF
{
    namespace
    {
        double G1(const double Dot, const double K)
        {
            return Dot / (Dot * (1.0 - K) + K);
        }

        double G(const double NdotL, const double NdotV, const double Roughness)
        {
            const double K = (Roughness + 1.0) * (Roughness + 1.0) / 8.0;
            return G1(NdotL, K) * G1(NdotV, K);
        }

        double D(const double Roughness, const double HdotN)
        {
            const double R = std::pow(Roughness, 4);
            const double D = PI * std::pow(HdotN * HdotN * (R - 1.0) + 1.0, 2);
            return R / D;
        }

        double F(double HdotV)
        {
            const double F0 = std::pow((1.0 - 2.0) / (1.0 + 2.0), 2);
            return F0 + (1.0 - F0) * std::pow(1.0 - HdotV, 5.0);
        }
    }

    static double BRDF(const Vector3 N, const Vector3 V, const Vector3 L, double Roughness, double &Fresnel)
    {
        const Vector3 H = (L + V).Normalized();
        Roughness = Clamp(Roughness, 1e-2, 1.0);
        const double HdotN = std::max(H | N, 0.0);
        const double VdotN = std::max(V | N, 0.0);
        const double LdotN = std::max(L | N, 0.0);
        const double HdotV = std::max(V | H, 0.0);
        Fresnel = F(HdotV);
        return Fresnel * G(LdotN, VdotN, Roughness) * D(Roughness, HdotN) / std::max(VdotN * LdotN * 4.0, 1e-4);
    }
}

void DrawPercent()
{
    static uint32_t Pixel = 0;
    Pixel++;
    
    if (Pixel % (HEIGHT * WIDTH / 20) == 0)
    {
        std::cout << "Processing... "<< ceil(Pixel * 100.0 / (double)(HEIGHT * WIDTH)) << "%\n";
    }
}

bool QueryScene(const RRay Ray, std::vector<OObject*>& Scene, RHit& OutHit)
{
    double MinDist = LONG_MAX;
    bool bHit = false;
	
    for (auto* Object : Scene)
    {
        RHit TempHit;
        if (Object->Intersects(Ray, TempHit))
        {
            bHit = true;
            if (TempHit.Depth < MinDist)
            {
                MinDist = TempHit.Depth;
                OutHit = TempHit;
            }
        }
    }
    return bHit;
}

bool QueryShadow(const RRay Ray, std::vector<OObject*>& Scene, const RLight LightSource)
{
    const double Distance = (Ray.Origin - LightSource.Position).Length();
    for (auto* Object : Scene)
    {
        RHit TempHit;
        if (Object->Intersects(Ray, TempHit))
        {
        	if(Distance > TempHit.Depth) 
                return true;
        }
    }
    return false;
}


Vector3 FinalColor(const RRay Ray, std::vector<OObject*>& Scene, std::vector<RLight>& Lights, const int Depth = 0)
{
    if (Depth >= RAY_SAMPLES) return BACKGROUND_COLOR;
	
    RHit HitInfo;
    
    if (QueryScene(Ray, Scene, HitInfo))
    {   
        const auto M = HitInfo.Mat;
        Vector3 Color(0.0);    

        for (auto const& LightSource : Lights)
        {
            const Vector3 LightDir = (LightSource.Position - HitInfo.Position).Normalized();
            const double NdotL = Clamp(HitInfo.Normal | LightDir, 0.0, 1.0);

#if SHADOW_ENABLED
            RRay ShadowRay;
            ShadowRay.Origin = HitInfo.Position + HitInfo.Normal * 1e-6;
            ShadowRay.Direction = (LightSource.Position - HitInfo.Position).Normalized();
            if (QueryShadow(ShadowRay, Scene, LightSource)) continue;

#endif



            const double Distance = (LightSource.Position - HitInfo.Position).Length();
            const double Attenuation = 1.0 / (Distance * Distance);
            const Vector3 Radiance = LightSource.Color * Attenuation;

            double F = 0.0;
            const double rS = BRDF::BRDF(HitInfo.Normal, -Ray.Direction, LightDir, HitInfo.Mat.Roughness, F);
            const double kS = F;
            const Vector3 kD = (Vector3(1.0) - Vector3(kS)) / PI * NdotL;

#if REFLECTIONS_ENABLED
            RRay ReflectionRay;
            ReflectionRay.Origin = HitInfo.Position + HitInfo.Normal * 1e-6;
            ReflectionRay.Direction = Ray.Direction.MirrorByVector(HitInfo.Normal);
            const Vector3 ReflectedLight = FinalColor(ReflectionRay, Scene, Lights, Depth + 1);
#else
            const Vector3 ReflectedLight = Vector3(0.0);
#endif
        	
            Color = Color + (kD * M.Color * (1.0 - M.Metallic) + Vector3(rS)) * Radiance + ReflectedLight * kS;

            
        }

    	
        return (AMBIENT + Color).Clamp(0.0, 1.0);
        //return HitInfo.Normal.Clamp(0.0, 1.0);
        //return Vector3(HitInfo.Depth / 500.0).Clamp(0.0, 1.0);
    }
    else
    {
        return BACKGROUND_COLOR;
    }
}

void Render(Vector3* Framebuffer, std::vector<OObject*> &Scene, std::vector<RLight>& Lights)
{
    const double AspectRatio = (double)WIDTH / HEIGHT;

    const Vector3 CameraPosition(0.0, 0.0, 0.0);
    //Vector3 CameraDirection(1.0, 0.0, 0.0);

    #pragma omp parallel for
    for (size_t i = 0; i < HEIGHT; i++)
    {
        for (size_t j = 0; j < WIDTH; j++)
        {
            double SSX = 2.0 * (j + 0.5) / (double)WIDTH - 1;  
            double SSY = 2.0 * (i + 0.5) / (double)HEIGHT - 1; 
            SSX *= AspectRatio;

            const double PixelCameraX = SSX * tan(FOVR / 2.);
            const double PixelCameraY = SSY * tan(FOVR / 2.);

            RRay Ray;
            Ray.Origin = CameraPosition;
            Ray.Direction = Vector3(PixelCameraX, PixelCameraY, -1.0).Normalized();

            Vector3 Color = FinalColor(Ray, Scene, Lights);
            //Gamma Correction
            Color = Color / (Color + Vector3(1.0));
            Color.X = std::pow(Color.X, 1.0 / 2.2);
            Color.Y = std::pow(Color.Y, 1.0 / 2.2);
            Color.Z = std::pow(Color.Z, 1.0 / 2.2);

            Framebuffer[i * WIDTH + j] = Color;

            DrawPercent();
        }
    }
}


int main()
{
    auto* Frame = new Vector3[WIDTH * HEIGHT];

    std::vector<OObject*> Scene;
    std::vector<RLight> Lights;

    auto* S = new OSphere;
    S->Position = Vector3(0.0, 0.0, -30.0);
    S->Radius = 8.0;
    S->Mat = RMaterial::Metal;
    Scene.push_back(S);

    auto* S1 = new OSphere;
    S1->Position = Vector3(20.0, 0.0, -30.0);
    S1->Radius = 8.0;
    S1->Mat = RMaterial::YellowRubber;
    Scene.push_back(S1);

    auto* S2 = new OSphere;
    S2->Position = Vector3(-20.0, 0.0, -30.0);
    S2->Radius = 8.0;
    S2->Mat = RMaterial::BluePlastic;
    Scene.push_back(S2);

    /*
    auto* P = new OPlane;
    P->Position = Vector3(0.0, 30.0, 0.0);
    P->Mat = RMaterial::YellowRubber;
    P->Normal = Vector3(0.0, -1.0, 0.0);
    Scene.push_back(P);

    auto* P1 = new OPlane;
    P1->Position = Vector3(0.0, 0.0, -55.0);
    P1->Mat = RMaterial::BluePlastic;
    P1->Normal = Vector3(0.0, 0.0, 1.0);
    Scene.push_back(P1);
    */

    auto* B1 = new OBox;
    B1->Position = Vector3(-25.0, -20.0, -30.0);
    B1->Mat = RMaterial::BluePlastic;
    B1->Extent = Vector3(5.0, 5.0, 5.0);
    Scene.push_back(B1);

    auto* B2 = new OBox;
    B2->Position = Vector3(0.0, 0.0, 0.0);
    B2->Mat = RMaterial::Mirror;
    B2->Extent = Vector3(40.0, 25.0, 40.0);
    Scene.push_back(B2);

    

    RLight L;
    L.Position = Vector3(-20.0, -20.0, 0.0);
    L.Color = Vector3(1.0, 1.0, 1.0) * 10000.0;
    Lights.push_back(L);

    
    RLight L1;
    L1.Position = Vector3(30.0, -20.0, 20.0);
    L1.Color = Vector3(1.0, 1.0, 1.0) * 10000.0;
    Lights.push_back(L1);
    

    Render(Frame, Scene, Lights);
    CreateImage(Frame, HEIGHT, WIDTH, "output.bmp");  
}

