// Raytracer.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "math/Headers/mvector.h"
#include "BMPWriter.h"

#define HEIGHT 1080
#define WIDTH 1920

void render(vec3* framebuffer)
{
    for (size_t i = 0; i < HEIGHT; i++)
    {
        for (size_t j = 0; j < WIDTH; j++)
        {
            framebuffer[i * WIDTH + j].x = (double) j / WIDTH;
            framebuffer[i * WIDTH + j].y = (double) i / HEIGHT;
            framebuffer[i * WIDTH + j].z = 0;
        }
    }
}


int main()
{
    vec3* frame = new vec3[WIDTH * HEIGHT];
    render(frame);
    CreateImage(frame, HEIGHT, WIDTH);
}

