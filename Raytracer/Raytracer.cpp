#include <iostream>
#include <future>
#include "Headers/math/Vector.h"
#include "Headers/BMPWriter.h"
#include "Headers/CoreUtilities.h"
#include "Headers/OObject.h"
#include "Headers/Scene.h"
#include "Headers/PostProcess.h"
#include "Headers/Texture.h"
#include "Headers/BlinnPhong.h"
#include "ThirdParty/glfw3.h"
#include "ThirdParty/glfw3native.h"

constexpr uint32_t HEIGHT = 720;
constexpr uint32_t WIDTH = 1280;



int main()
{
    RScene MainScene(HEIGHT, WIDTH);
    MainScene.ShadowSamples = 512;
    MainScene.RayDepth = 3;
    MainScene.bShadows = false;
    MainScene.SetShader(new BlinnPhongModel);

    RTexture EnvMap(0, 0);
    EnvMap.Load("envmap.jpg");
    MainScene.SetEnvironmentTexture(EnvMap);

    GLFWwindow* Window;
    glfwInit();
    Window = glfwCreateWindow(WIDTH, HEIGHT, "Raytracing", nullptr, nullptr);
    glfwMakeContextCurrent(Window);
    
    
    
	/*
    auto* S = new OSphere;
    S->Transform.SetPosition(Vector3(0.0, 5.0, -30.0));
    S->Radius = 8.0;
    S->Mat = new RMaterial(RMaterialPBR::YellowRubber);
    //MainScene.AddObject(S);

    auto* Teapot = new OMesh("duck.obj");
    Teapot->Transform.SetPosition(Vector3(-15.0, 0.0, 0.0));
    Teapot->Transform.SetScale(Vector3(3.0, 3.0, 3.0));
    Teapot->Transform.SetRotation(Vector3(0.0, 0.0, 0.0));
    Teapot->Mat = new RMaterial(RMaterialPBR::Glass);
    MainScene.AddObject(Teapot);

    auto* S1 = new OSphere;
    S1->Transform.SetPosition(Vector3(0.0, 0.0, -30.0));
    S1->Transform.SetScale(Vector3(1.0, 0.7, 1.0));
    S1->Radius = 8.0;
    S1->Mat = new RMaterial(RMaterialPBR::Glass);
    //MainScene.AddObject(S1);

    auto* S2 = new OSphere;
    S2->Transform.SetPosition(Vector3(0.0, 0.0, -12.0));
    S2->Radius = 5;
    S2->Mat = new RMaterial(RMaterialPBR::Glass);
    //MainScene.AddObject(S2);

    

    auto* B1 = new OBox;
    B1->Transform.SetPosition(Vector3(40.0, 0.0, 0.0));
    B1->Mat = new RMaterial(RMaterialPBR::Mirror);
    B1->Extent = Vector3(1.0, 25.0, 25.0);
    MainScene.AddObject(B1);

    
    auto* B2 = new OBox;
    B2->Transform.SetPosition(Vector3(0.0, 0.0, 0.0));
    B2->Mat = new RMaterial(RMaterialPBR::YellowRubber);
    B2->Extent = Vector3(60.0, 26.0, 45.0);
    //MainScene.AddObject(B2);


    
    */

    auto* L = new OLight;
    L->Transform.SetPosition(Vector3(-20.0, -20.0, 0.0));
    L->Color = Vector3(1.0, 1.0, 1.0) * 6000.0;
    L->Radius = 5.0;
    MainScene.AddObject(L);

    
    auto* L1 = new OLight;
    L1->Transform.SetPosition(Vector3(30.0, -10000.0, 20.0));
    L1->Color = Vector3(1.0, 1.0, 1.0) * 40000000.0;
    L1->Radius = 5.0;
    MainScene.AddObject(L1);
    

    auto* Teapot = new OMesh("duck.obj");
    Teapot->Transform.SetPosition(Vector3(15.0, 0.0, 0.0));
    Teapot->Transform.SetScale(Vector3(3.0, 3.0, 3.0));
    Teapot->Transform.SetRotation(Vector3(0.0, 0.0, 0.0));
    Teapot->Mat = new RMaterialBlinnPhong(Vector3(1.0, 0.0, 0.0), 5.0);
    MainScene.AddObject(Teapot);
    
    auto Render = std::async(std::launch::async, &RScene::Render, &MainScene);
    //MainScene.Render();

    
    while (!glfwWindowShouldClose(Window))
    {
        RTexture<Vector3> HDR = MainScene.GetTexture3D(RScene::SceneType3D::HDR);
        ToneCompression(HDR);

        auto data = new float[HEIGHT * WIDTH * 3];
        
        for (size_t i = 0; i < WIDTH * HEIGHT * 3; i+=3)
        {
            Vector3 Color = *(HDR.Begin() + i / 3);
            data[i + 0] = Color.X;
            data[i + 1] = Color.Y;
            data[i + 2] = Color.Z;
        }
        

        glfwPollEvents();

        glClearColor(0.1, 0.1, 0.9, 1.0);

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, data);
        glBindTexture(GL_TEXTURE_2D, 0);

        //match projection to window resolution (could be in reshape callback)
        glMatrixMode(GL_PROJECTION);
        glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
        glMatrixMode(GL_MODELVIEW);

        //clear and draw quad with texture (could be in display callback)
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, tex);
        glEnable(GL_TEXTURE_2D);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(-1.0, -1.0);
        glTexCoord2f(0, 0); glVertex2f(-1.0, 1.0);
        glTexCoord2f(1, 0); glVertex2f(1.0, 1.0);
        glTexCoord2f(1, 1); glVertex2f(1.0, -1.0);
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, 0);
             
        glfwSwapBuffers(Window);        
    }

    glfwTerminate();
    

    
    Render.wait();
    RTexture<Vector3> HDR = MainScene.GetTexture3D(RScene::SceneType3D::HDR);
    RTexture<Vector3> Depth = Texture3DFrom1D(MainScene.GetTexture1D(RScene::SceneType1D::DEPTH), EColorChannels::R | EColorChannels::G | EColorChannels::B);
    RTexture<Vector3> NormalTex = MainScene.GetTexture3D(RScene::SceneType3D::NORMAL);

    std::transform(Depth.Begin(), Depth.End(), Depth.Begin(), [](const Vector3& Color)
        {
            return Color / 100.0;
        });

    Bloom(HDR);
    ToneCompression(HDR, 2.0);
    GammaCorrection(HDR);
	
    SaveImage(HDR, "final.bmp");
    SaveImage(Depth, "depth.bmp");
    SaveImage(NormalTex, "normals.bmp");
}

