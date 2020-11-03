// Raytracer.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <vector>
#include "math/Headers/mvector.h"
#include "BMPWriter.h"
#include "Object.h"

#define HEIGHT 1080
#define WIDTH 1920
#define FOV 90                 //fov in degrees
#define FOVR (FOV * PI / 180)     //fov in radians
#define BACKGROUND_COLOR vec3(0., 0.2, 0.65)

vec3 FinalColor(Ray ray, std::vector<Object*>& Scene)
{
    Material FinalMat;
    double MinDist = LLONG_MAX;
    bool bHit = false;

    for (Object* object : Scene)
    {
        Hit TempHit;
        if (object->Intersects(ray, TempHit))
        {
            bHit = true;
            if ((TempHit.Position - ray.Origin).getLength() < MinDist)
            {
                MinDist = (TempHit.Position - ray.Origin).getLength();
                FinalMat = TempHit.Mat;
            }
        }
    }

    if (bHit)
    {
        return FinalMat.Color;
    }
    else
    {
        return BACKGROUND_COLOR;
    }
}

void render(vec3* framebuffer, std::vector<Object*> &Scene)
{
    double AspectRatio = (double)WIDTH / HEIGHT;

    vec3 CameraPosition(0., 0., 0.);
    //vec3 CameraDirection(1., 0., 0.);

    for (size_t i = 0; i < HEIGHT; i++)
    {
        for (size_t j = 0; j < WIDTH; j++)
        {
            double SSX = (2. * (j + 0.5)) / (double)WIDTH - 1;  
            double SSY = (2. * (i + 0.5)) / (double)HEIGHT - 1; 
            SSX *= AspectRatio;

            double PixelCameraX = SSX * tan(FOVR / 2.);
            double PixelCameraY = SSY * tan(FOVR / 2.);

            Ray ray;
            ray.Origin = CameraPosition;
            ray.Direction = vec3(PixelCameraX, PixelCameraY, -1.).normalized();

            framebuffer[i * WIDTH + j] = FinalColor(ray, Scene);
        }
    }
}


int main()
{
    vec3* frame = new vec3[WIDTH * HEIGHT];

    std::vector<Object*> Scene;

    Sphere* S = new Sphere;
    S->Position = vec3(0., 0., -20.);
    S->Radius = 10.;
    S->Mat.Color = vec3(1., 0., 0.);
    Scene.push_back(S);


    render(frame, Scene);
    CreateImage(frame, HEIGHT, WIDTH);
}

