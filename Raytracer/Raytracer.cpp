#include <iostream>
#include <vector>
#include <cmath>
#include "math/Headers/mvector.h"
#include "BMPWriter.h"
#include "OObject.h"

#define SHADOW_ENABLED 1
#define SSAA_ENABLED 0

constexpr uint32_t HEIGHT = 1080;
constexpr uint32_t WIDTH = 1920;
constexpr uint8_t FOV = 85;                    //fov in degrees;
constexpr uint8_t RAY_SAMPLES = 4;


constexpr double FOVR = (FOV * PI / 180);     //fov in radians
#define BACKGROUND_COLOR Vector3(0.0, 0.0, 0.0)
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

Vector3 Refract(const Vector3& I, const Vector3& N, const double Index)
{
    double cosi = -Clamp(N | I, -1.0, 1.0);
    double etai = 1.0, etat = Index;
    Vector3 Normal = N;

	if(cosi < 0.0) //ray inside object
	{
        cosi = -cosi;
        std::swap(etai, etat);
        Normal = -Normal;
	}
    double eta = etai / etat;
    double k = 1.0 - eta * eta * (1.0 - cosi * cosi);
    return k < 0.0 ? Vector3(0.0) : I * eta + Normal * (eta * cosi - sqrt(k));
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
            ShadowRay.Origin = HitInfo.Position + (NdotL > 0.0 ? HitInfo.Normal * 1e-6 : -HitInfo.Normal * 1e-6);
            ShadowRay.Direction = (LightSource.Position - HitInfo.Position).Normalized();

            double ShadowScale = 1.0;
            RHit ShadowHit;
            while (QueryScene(ShadowRay, Scene, ShadowHit))
            {
                if(ShadowHit.Mat.Transmission > 0.0)
                {
                    ShadowScale *= ShadowHit.Mat.Transmission;
                    ShadowRay.Origin = ShadowHit.Position + ShadowHit.Normal * ((ShadowHit.Normal | LightDir) > 0.0 ? 1e-6 : -1e-6);

                	/* With this line program goes into infinite loop somehow, also I'm not sure whether this is needed */
                    //ShadowRay.Direction = Refract(ShadowRay.Direction, ShadowHit.Normal, ShadowHit.Mat.RefractiveIndex);
                }
                else
                {
                    ShadowScale = 0.0;
                    break;
                }
            }

            if (ShadowScale < 1e-8) continue;

#endif



            const double Distance = (LightSource.Position - HitInfo.Position).Length();
            const double Attenuation = 1.0 / (Distance * Distance);
            const Vector3 Radiance = LightSource.Color * Attenuation;

            double F = 0.0;
            const double rS = BRDF::BRDF(HitInfo.Normal, -Ray.Direction, LightDir, M.Roughness, F);
            const double kS = F;
            const Vector3 kD = (Vector3(1.0) - Vector3(kS)) / PI * NdotL;

            RRay ReflectionRay;
            ReflectionRay.Origin = HitInfo.Position + (NdotL > 0.0 ? HitInfo.Normal * 1e-6 : -HitInfo.Normal * 1e-6);
            ReflectionRay.Direction = Ray.Direction.MirrorByVector(HitInfo.Normal);
            const Vector3 ReflectedLight = FinalColor(ReflectionRay, Scene, Lights, Depth + 1);

            Vector3 TransmittedLight;
            if (M.Transmission > 0.0)
            {
                RRay TransmittedRay;
                TransmittedRay.Origin = HitInfo.Position + HitInfo.Normal * -1e-6;
                TransmittedRay.Direction = Refract(Ray.Direction, HitInfo.Normal, M.RefractiveIndex);
                TransmittedLight = FinalColor(TransmittedRay, Scene, Lights, Depth + 1);
            }
            else
            {
                TransmittedLight = Vector3(0.0);
            }


            Color = Color + (kD * M.Color * (1.0 - M.Metallic) * (1.0 - M.Transmission) + Vector3(rS)) * Radiance; //Base light
            Color = Color + ReflectedLight * kS; // Reflected light
            Color = Color + TransmittedLight * M.Transmission; // Transmitted light
            Color = Color * ShadowScale;
        }

    	
        return (AMBIENT + Color + M.Emissive).Clamp(0.0, 1.0);
        //return HitInfo.Normal.Clamp(0.0, 1.0);
        //return Vector3(HitInfo.Depth / 500.0).Clamp(0.0, 1.0);
    }

    return BACKGROUND_COLOR;   
}

void Render(Vector3* Framebuffer, std::vector<OObject*> &Scene, std::vector<RLight>& Lights)
{
    const double AspectRatio = (double)WIDTH / HEIGHT;

#if SSAA_ENABLED
    double JitterMatrix[4 * 2] = {
        -1.0 / 4.0,  3.0 / 4.0,
		3.0 / 4.0,  1.0 / 3.0,
		-3.0 / 4.0, -1.0 / 4.0,
		1.0 / 4.0, -3.0 / 4.0
    };
#endif

    const Vector3 CameraPosition(0.0, 0.0, 0.0);
    //Vector3 CameraDirection(1.0, 0.0, 0.0);

    #pragma omp parallel for
    for (size_t i = 0; i < HEIGHT; i++)
    {
        for (size_t j = 0; j < WIDTH; j++)
        {
#if SSAA_ENABLED
            Vector3 Color(0.0);
            for (size_t sample = 0; sample < 4; sample++)
            {
                double SSX = 2.0 * (j + JitterMatrix[2 * sample]) / (double)WIDTH - 1;
                double SSY = 2.0 * (i + JitterMatrix[2 * sample + 1]) / (double)HEIGHT - 1;
                SSX *= AspectRatio;

                const double PixelCameraX = SSX * tan(FOVR / 2.0);
                const double PixelCameraY = SSY * tan(FOVR / 2.0);

                RRay Ray;
                Ray.Origin = CameraPosition;
                Ray.Direction = Vector3(PixelCameraX, PixelCameraY, -1.0).Normalized();

                Color = Color + FinalColor(Ray, Scene, Lights);
            }
            Color = Color / 4.0;
#else
            double SSX = 2.0 * (j + 0.5) / (double)WIDTH - 1;  
            double SSY = 2.0 * (i + 0.5) / (double)HEIGHT - 1;
            SSX *= AspectRatio;

            const double PixelCameraX = SSX * tan(FOVR / 2.0);
            const double PixelCameraY = SSY * tan(FOVR / 2.0);

            RRay Ray;
            Ray.Origin = CameraPosition;
            Ray.Direction = Vector3(PixelCameraX, PixelCameraY, -1.0).Normalized();

            Vector3 Color = FinalColor(Ray, Scene, Lights);
#endif
        	
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
    S->Mat = RMaterial::YellowRubber;
    Scene.push_back(S);

    auto* S1 = new OSphere;
    S1->Position = Vector3(20.0, 0.0, -30.0);
    S1->Radius = 8.0;
    S1->Mat = RMaterial::BluePlastic;
    Scene.push_back(S1);

    auto* S2 = new OSphere;
    S2->Position = Vector3(-2.0, -1.0, -11.0);
    S2->Radius = 2.3;
    S2->Mat = RMaterial::Glass;
    Scene.push_back(S2);

    
    auto* P = new OPlane;
    P->Position = Vector3(0.0, 30.0, 0.0);
    P->Mat = RMaterial::RedPlastic;
    P->Normal = Vector3(0.0, -1.0, 0.0);
    Scene.push_back(P);

	/*
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

    /*
    auto* B2 = new OBox;
    B2->Position = Vector3(0.0, 0.0, 0.0);
    B2->Mat = RMaterial::BluePlastic;
    B2->Extent = Vector3(40.0, 25.0, 40.0);
    Scene.push_back(B2);
    */
    

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

