// Raytracer.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include "math/Headers/mvector.h"
#include "BMPWriter.h"
#include "Object.h"

#define HEIGHT 1080
#define WIDTH 1920
#define FOV 85                    //fov in degrees
#define FOVR (FOV * PI / 180)     //fov in radians
#define BACKGROUND_COLOR vec3(0., 0.2, 0.65)
#define clamp(x, l, h) std::min(std::max(x, l), h)
#define AMBIENT vec3(0.01, 0.01, 0.01)

namespace BRDF
{
    static double G(vec3 N, vec3 H, vec3 V, vec3 L)
    {
        double k1 = 2 * (H | N) * (V | N) / (V | H);
        double k2 = 2 * (H | N) * (V | N) / (L | H);
        if (k1 < k2)
        {
            return std::min(1., k1);
        }
        else
        {
            return std::min(1., k1);
        }
    }

    static double D(double m, vec3 H, vec3 N)
    {
        double k = 1 / (4 * m * m * std::pow(H | N, 4));
        k *= std::exp((std::pow(H | N, 2) - 1) / (m * m * std::pow(H | N, 2)));
        return k;
    }

    static double F(vec3 V, vec3 N)
    {
        double Kr = std::pow((1.0 - 2.0) / (1.0 + 2.0), 2.0);
        return Kr + (1.0 - Kr) * pow((1.0 - (N | V)), 5.0);;
    }

    static double BRDF(vec3 N, vec3 V, vec3 L, double roughness)
    {
        L = L.normalized();
        vec3 H = (L + V).normalized();       
        return F(V, N) * G(N, H, V, L) * D(roughness, H, N) / ((V | N) * PI + 1e-8);
    }
}


vec3 FinalColor(Ray ray, std::vector<Object*>& Scene, std::vector<Light>& Lights)
{
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

    vec3 k(0, 0, 0);
    if (bHit)
    {      
        for (Light& LightSource : Lights)
        {
            vec3 LightDir = (LightSource.Position - HitInfo.Position).normalized();
            k = k + LightSource.Color * BRDF::BRDF(HitInfo.Normal, ray.Direction, LightSource.Position, HitInfo.Mat.Roughness) * ((LightSource.Position - HitInfo.Position).normalized() | HitInfo.Normal);
        }
    }

    if (bHit)
    {
        Material M = HitInfo.Mat;
        vec3 Kd = clamp(k, vec3(0, 0, 0), vec3(1, 1, 1));
        return ((AMBIENT + M.Color + k) * M.Albedo).Clamp(0., 1.) ;
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

            framebuffer[i * WIDTH + j] = FinalColor(ray, Scene, Lights);
        }
    }
}


int main()
{
    vec3* frame = new vec3[WIDTH * HEIGHT];

    Material RedGlass;
    RedGlass.Color = vec3(1, 0, 0);
    RedGlass.Albedo = 1.0;
    RedGlass.Roughness = 0.0;
    RedGlass.Specular = 2;

    Material YellowRubber;
    YellowRubber.Color = vec3(1, 1, 0);
    YellowRubber.Albedo = 1.0;
    YellowRubber.Roughness = 0.1;
    YellowRubber.Specular = 2;

    std::vector<Object*> Scene;
    std::vector<Light> Lights;

    Sphere* S = new Sphere;
    S->Position = vec3(0., 0., -20.);
    S->Radius = 10.;
    S->Mat = RedGlass;
    //Scene.push_back(S);

    Sphere* S1 = new Sphere;
    S1->Position = vec3(0, 0., -17.);
    S1->Radius = 7.;
    S1->Mat = YellowRubber;
    Scene.push_back(S1);

    Light L;
    L.Position = vec3(-20., -20., 0.);
    L.Intensity = 1.;
    L.Color = vec3(0.3, 0.3, 0.3);
    Lights.push_back(L);

    /*
    Light L1;
    L1.Position = vec3(-20., 15., 20.);
    L1.Intensity = 0.7;
    Lights.push_back(L1);
    */

    render(frame, Scene, Lights);
    CreateImage(frame, HEIGHT, WIDTH);
}

