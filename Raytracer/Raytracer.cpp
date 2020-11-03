// Raytracer.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include "math/Headers/mvector.h"
#include "BMPWriter.h"
#include "Object.h"

#define HEIGHT 1080
#define WIDTH 1920
#define FOV 85                 //fov in degrees
#define FOVR (FOV * PI / 180)     //fov in radians
#define BACKGROUND_COLOR vec3(0., 0.2, 0.65)
#define clamp(x, l, h) std::min(std::max(x, l), h)

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

    double k = 0;
    if (bHit)
    {      
        for (Light& LightSource : Lights)
        {
            vec3 LightDir = (LightSource.Position - HitInfo.Position).normalized();
            k += LightSource.Intensity * std::max(0., LightDir | HitInfo.Normal);
        }
    }

    if (bHit)
    {
        return (HitInfo.Mat.Color * k).Clamp(0., 1.);
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

    std::vector<Object*> Scene;
    std::vector<Light> Lights;

    Sphere* S = new Sphere;
    S->Position = vec3(0., 0., -20.);
    S->Radius = 10.;
    S->Mat.Color = vec3(1., 0., 0.);
    Scene.push_back(S);

    Sphere* S1 = new Sphere;
    S1->Position = vec3(-6., -7., -17.);
    S1->Radius = 5.;
    S1->Mat.Color = vec3(1., 1., 0.);
    Scene.push_back(S1);

    Light L;
    L.Position = vec3(-20., -20., 0.);
    L.Intensity = 1.;
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

