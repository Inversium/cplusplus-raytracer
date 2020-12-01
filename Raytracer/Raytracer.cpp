// Raytracer.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include <cmath>
#include "math/Headers/mvector.h"
#include "BMPWriter.h"
#include "Object.h"

#define HEIGHT 1080
#define WIDTH 1920
#define FOV 85                    //fov in degrees
#define RAY_SAMPLES 8
#define FOVR (FOV * PI / 180)     //fov in radians
#define BACKGROUND_COLOR vec3(0., 0.2, 0.65)
#define clamp(x, l, h) std::min(std::max(x, l), h)
#define AMBIENT vec3(0.02, 0.02, 0.02)

namespace BRDF
{
    static double G1(double dot, double k)
    {
        return dot / (dot * (1.0 - k) + k);
    }

    static double G(double NdotL, double NdotV, double Roughness)
    {
        double k = (Roughness + 1.0) * (Roughness + 1.0) / 8.0;
        return G1(NdotL, k) * G1(NdotV, k);
    }

    static double D(double m, double HdotN)
    {
        const double r = std::pow(m, 4);
        const double d = PI * std::pow(HdotN * HdotN * (r - 1.0) + 1.0, 2);
        return r / d;
    }

    static double F(double HdotV)
    {
        double F0 = std::pow((1.0 - 2.0) / (1.0 + 2.0), 2);
        return F0 + (1.0 - F0) * std::pow(1.0 - HdotV, 5.0);
    }

    static double BRDF(vec3 N, vec3 V, vec3 L, double roughness, double &Fresnel)
    {
        vec3 H = (L + V).normalized();
        roughness = clamp(roughness, 1e-2, 1.0);
        double HdotN = std::max(H | N, 0.0);
        double VdotN = std::max(V | N, 0.0);
        double LdotN = std::max(L | N, 0.0);
        double HdotV = std::max(V | H, 0.0);
        Fresnel = F(HdotV);
        return Fresnel * G(LdotN, VdotN, roughness) * D(roughness, HdotN) / std::max(VdotN * LdotN * 4.0, 1e-4);
    }
}

bool CheckShadow(Ray ray, std::vector<Object*>& Scene)
{
    for (Object* object : Scene)
    {
        Hit TempHit;
        if (object->Intersects(ray, TempHit))
        {
            return true;
        }
    }
    return false;
}


vec3 FinalColor(Ray ray, std::vector<Object*>& Scene, std::vector<Light>& Lights, int Depth = 0)
{
    if (Depth > RAY_SAMPLES) return BACKGROUND_COLOR;

    double MinDist = LONG_MAX;
    bool bHit = false;
    Hit HitInfo;

    for (Object* object : Scene)
    {
        Hit TempHit;
        if (object->Intersects(ray, TempHit))
        {
            bHit = true;
            if (TempHit.Depth < MinDist)
            {
                MinDist = TempHit.Depth;
                HitInfo = TempHit;
            }
        }
    }

    
    if (bHit)
    {   
        Material M = HitInfo.Mat;
        vec3 Color(0.0);    

        for (Light& LightSource : Lights)
        {
            Ray SRay;
            SRay.Origin = HitInfo.Position + HitInfo.Normal * 1e-5;
            SRay.Direction = (LightSource.Position - HitInfo.Position).normalized();
            if (CheckShadow(SRay, Scene)) continue;

            const vec3 LightDir = (LightSource.Position - HitInfo.Position).normalized();
            const double NdotL = clamp(HitInfo.Normal | LightDir, 0.0, 1.0);

            const double Distance = (LightSource.Position - HitInfo.Position).getLength();
            const double Attenuation = 1.0 / (Distance * Distance);
            const vec3 Radiance = LightSource.Color * Attenuation;

            double F = 0.0;
            const double rS = BRDF::BRDF(HitInfo.Normal, -ray.Direction, LightDir, HitInfo.Mat.Roughness, F);
            const double kS = F;
            const vec3 kD = (vec3(1.0) - vec3(kS)) / PI * NdotL;

            Ray RRay;
            RRay.Origin = HitInfo.Position;
            RRay.Direction = ray.Direction.MirrorByVector(HitInfo.Normal);
            const vec3 Reflected = FinalColor(RRay, Scene, Lights, Depth + 1) * rS;

            Color = Color + (kD * M.Color * (1.0 - M.Metallic) + vec3(rS)) * Radiance + Reflected;
        }

        

        return (AMBIENT + Color).Clamp(0.0, 1.0);
    }
    else
    {
        return BACKGROUND_COLOR;
    }
}

void render(vec3* framebuffer, std::vector<Object*> &Scene, std::vector<Light>& Lights)
{
    double AspectRatio = (double)WIDTH / HEIGHT;

    vec3 CameraPosition(0., 0., 0.);
    //vec3 CameraDirection(1., 0., 0.);

    #pragma omp parallel for
    for (int i = 0; i < HEIGHT; i++)
    {
        for (int j = 0; j < WIDTH; j++)
        {
            double SSX = 2. * (j + 0.5) / (double)WIDTH - 1;  
            double SSY = 2. * (i + 0.5) / (double)HEIGHT - 1; 
            SSX *= AspectRatio;

            double PixelCameraX = SSX * tan(FOVR / 2.);
            double PixelCameraY = SSY * tan(FOVR / 2.);

            Ray ray;
            ray.Origin = CameraPosition;
            ray.Direction = vec3(PixelCameraX, PixelCameraY, -1.).normalized();

            vec3 Color = FinalColor(ray, Scene, Lights);
            //Gamma Correction
            Color = Color / (Color + vec3(1.0));
            Color.x = std::pow(Color.x, 1.0 / 2.2);
            Color.y = std::pow(Color.y, 1.0 / 2.2);
            Color.z = std::pow(Color.z, 1.0 / 2.2);

            framebuffer[i * WIDTH + j] = Color;
        }
    }
}


int main()
{
    vec3* frame = new vec3[WIDTH * HEIGHT];

    std::vector<Object*> Scene;
    std::vector<Light> Lights;

    Sphere* S = new Sphere;
    S->Position = vec3(0.0, 0.0, -30.0);
    S->Radius = 10.0;
    S->Mat = Material::RedPlastic;
    Scene.push_back(S);

    Sphere* S1 = new Sphere;
    S1->Position = vec3(15.0, 5.0, -35.0);
    S1->Radius = 7.0;
    S1->Mat = Material::YellowRubber;
    Scene.push_back(S1);

    
    Sphere* S2 = new Sphere;
    S2->Position = vec3(-15.0, 7.0, -35.0);
    S2->Radius = 8.0;
    S2->Mat = Material::Metal;
    Scene.push_back(S2);
    

    Light L;
    L.Position = vec3(-20.0, -20.0, 0.0);
    L.Color = vec3(1.0, 1.0, 1.0) * 950.0;
    Lights.push_back(L);

    
    Light L1;
    L1.Position = vec3(100.0, 35.0, 20.0);
    L1.Color = vec3(1.0, 0.8, 0.4) * 1400.0;
    Lights.push_back(L1);
    

    render(frame, Scene, Lights);
    CreateImage(frame, HEIGHT, WIDTH);
}

