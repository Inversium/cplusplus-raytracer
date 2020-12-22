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
constexpr uint8_t RAY_SAMPLES = 5;


constexpr double FOVR = (FOV * PI / 180);     //fov in radians
#define BACKGROUND_COLOR vec3(0.4, 0.2, 0.65)
#define AMBIENT vec3(0.07, 0.07, 0.07)

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

    static double BRDF(const vec3 N, const vec3 V, const vec3 L, double Roughness, double &Fresnel)
    {
        const vec3 H = (L + V).normalized();
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

bool QueryShadow(const RRay Ray, std::vector<OObject*>& Scene, RLight LightSource)
{
    const double Distance = (Ray.Origin - LightSource.Position).getLength();
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


vec3 FinalColor(RRay Ray, std::vector<OObject*>& Scene, std::vector<RLight>& Lights, const int Depth = 0)
{
    if (Depth >= RAY_SAMPLES) return BACKGROUND_COLOR;
	
    RHit HitInfo;
    
    if (QueryScene(Ray, Scene, HitInfo))
    {   
        const auto M = HitInfo.Mat;
        vec3 Color(0.0);    

        for (auto const& LightSource : Lights)
        {
            const vec3 LightDir = (LightSource.Position - HitInfo.Position).normalized();
            const double NdotL = Clamp(HitInfo.Normal | LightDir, 0.0, 1.0);

#if SHADOW_ENABLED
            RRay ShadowRay;
            ShadowRay.Origin = HitInfo.Position + HitInfo.Normal * 1e-6;
            ShadowRay.Direction = (LightSource.Position - HitInfo.Position).normalized();
            if (QueryShadow(ShadowRay, Scene, LightSource)) continue;

#endif



            const double Distance = (LightSource.Position - HitInfo.Position).getLength();
            const double Attenuation = 1.0 / (Distance * Distance);
            const vec3 Radiance = LightSource.Color * Attenuation;

            double F = 0.0;
            const double rS = BRDF::BRDF(HitInfo.Normal, -Ray.Direction, LightDir, HitInfo.Mat.Roughness, F);
            const double kS = F;
            const vec3 kD = (vec3(1.0) - vec3(kS)) / PI * NdotL;

#if REFLECTIONS_ENABLED
            RRay ReflectionRay;
            ReflectionRay.Origin = HitInfo.Position + HitInfo.Normal * 1e-6;
            ReflectionRay.Direction = Ray.Direction.MirrorByVector(HitInfo.Normal);
            const vec3 ReflectedLight = FinalColor(ReflectionRay, Scene, Lights, Depth + 1);
#else
            const vec3 ReflectedLight = 0;
#endif
        	
            Color = Color + (kD * M.Color * (1.0 - M.Metallic) + vec3(rS)) * Radiance + ReflectedLight * kS;

            
        }

    	
        return (AMBIENT + Color).Clamp(0.0, 1.0);
        //return HitInfo.Normal.Clamp(0.0, 1.0);
        //return vec3(HitInfo.Depth / 500.0).Clamp(0.0, 1.0);
    }
    else
    {
        return BACKGROUND_COLOR;
    }
}

void Render(vec3* Framebuffer, std::vector<OObject*> &Scene, std::vector<RLight>& Lights)
{
    const double AspectRatio = (double)WIDTH / HEIGHT;

    const vec3 CameraPosition(0.0, 0.0, 0.0);
    //vec3 CameraDirection(1.0, 0.0, 0.0);

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
            Ray.Direction = vec3(PixelCameraX, PixelCameraY, -1.0).normalized();

            vec3 Color = FinalColor(Ray, Scene, Lights);
            //Gamma Correction
            Color = Color / (Color + vec3(1.0));
            Color.x = std::pow(Color.x, 1.0 / 2.2);
            Color.y = std::pow(Color.y, 1.0 / 2.2);
            Color.z = std::pow(Color.z, 1.0 / 2.2);

            Framebuffer[i * WIDTH + j] = Color;

            DrawPercent();
        }
    }
}


int main()
{
    auto* Frame = new vec3[WIDTH * HEIGHT];

    std::vector<OObject*> Scene;
    std::vector<RLight> Lights;

    auto* S = new OSphere;
    S->Position = vec3(0.0, 0.0, -30.0);
    S->Radius = 8.0;
    S->Mat = RMaterial::Metal;
    Scene.push_back(S);

    auto* S1 = new OSphere;
    S1->Position = vec3(20.0, 0.0, -30.0);
    S1->Radius = 8.0;
    S1->Mat = RMaterial::YellowRubber;
    Scene.push_back(S1);

    auto* S2 = new OSphere;
    S2->Position = vec3(-20.0, 0.0, -30.0);
    S2->Radius = 8.0;
    S2->Mat = RMaterial::Metal;
    Scene.push_back(S2);


    auto* P = new OPlane;
    P->Position = vec3(0.0, 30.0, 0.0);
    P->Mat = RMaterial::YellowRubber;
    P->Normal = vec3(0.0, -1.0, 0.0);
    Scene.push_back(P);

    auto* P1 = new OPlane;
    P1->Position = vec3(0.0, 0.0, -55.0);
    P1->Mat = RMaterial::BluePlastic;
    P1->Normal = vec3(0.0, 0.0, 1.0);
    Scene.push_back(P1);

    

    RLight L;
    L.Position = vec3(-20.0, -20.0, 0.0);
    L.Color = vec3(1.0, 1.0, 1.0) * 10950.0;
    Lights.push_back(L);

    
    RLight L1;
    L1.Position = vec3(30.0, -20.0, 20.0);
    L1.Color = vec3(1.0, 1.0, 1.0) * 10950.0;
    Lights.push_back(L1);
    

    Render(Frame, Scene, Lights);
    CreateImage(Frame, HEIGHT, WIDTH, "output.bmp");  
}

