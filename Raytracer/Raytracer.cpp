#include <iostream>
#include <future>
#include "Headers/math/Vector.h"
#include "Headers/ImageUtility.h"
#include "Headers/CoreUtilities.h"
#include "Headers/OObject.h"
#include "Headers/Scene.h"
#include "Headers/PostProcess.h"
#include "Headers/Texture.h"
#include "Headers/BlinnPhong.h"
#include "Headers/CookTorrance.h"
#include "Headers/Shader.h"
#include "ThirdParty/glfw3.h"
#include "ThirdParty/glfw3native.h"

constexpr uint32_t HEIGHT = 720;
constexpr uint32_t WIDTH = 1280;



int main()
{
    RScene MainScene(HEIGHT, WIDTH);
    MainScene.SetShader(new RShader);
    MainScene.SetBRDF(new CookTorrance);

    RTexture<Vector3> EnvMap(0, 0);
    EnvMap.Load("envmap.jpg");
    //MainScene.SetEnvironmentTexture(EnvMap);

    GLFWwindow* Window;
    glfwInit();
    Window = glfwCreateWindow(WIDTH, HEIGHT, "Raytracing", nullptr, nullptr);
    glfwMakeContextCurrent(Window);
    


    auto* L = new OLight;
    L->Transform.SetPosition(Vector3(-12.0, -5.0, 0.0));
    L->Color = Vector3(1.0, 1.0, 1.0) * 900.0;
    L->Radius = 5.0;
    MainScene.AddObject(L);

    
    auto* L1 = new OLight;
    L1->Transform.SetPosition(Vector3(10000.0, 7000.0, 10000.0));
    L1->Color = Vector3(1.0, 1.0, 1.0) * 400000000.0;
    L1->Radius = 5.0;
    //MainScene.AddObject(L1);

    auto* S1 = new OSphere;
    S1->Radius = 2.0;
    S1->Transform.SetPosition(Vector3(10.0, -5.0, -5.0));
    S1->Mat = new RMaterialPBR(RMaterialPBR::BluePlastic);
    MainScene.AddObject(S1);

    auto* S2 = new OSphere;
    S2->Radius = 3.0;
    S2->Transform.SetPosition(Vector3(10.0, 5.0, -4.0));
    S2->Mat = new RMaterialPBR(RMaterialPBR::YellowRubber);
    MainScene.AddObject(S2);

    /*
    auto* Duck = new OMesh("duck.obj");
    Duck->Transform.SetPosition(Vector3(10.0, -5.0, 0.0));
    Duck->Transform.SetScale(Vector3(1.0, 1.0, 1.0));
    Duck->Mat = new RMaterialBlinnPhong(Vector3(1.0, 0.0, 0.0), 5.0);
    //MainScene.AddObject(Duck);

    auto* Duck2 = new OMesh(*Duck);
    Duck2->Transform.SetPosition(Vector3(10.0, 5.0, 0.0));
    Duck2->Transform.SetScale(Vector3(1.0, 1.0, 1.0));
    Duck2->Transform.SetRotation(0.0, 0.0, 180.0);
    Duck2->Mat = new RMaterialBlinnPhong(Vector3(1.0, 0.0, 1.0), 50.0);
    //MainScene.AddObject(Duck2);
    */

    auto* B2 = new OBox;
    B2->Transform.SetPosition(Vector3(0.0, 0.0, 6.0));
    B2->Mat = new RMaterialPBR(Vector3(1.0), 0.5, 0.0, 1.0, 0.0, Vector3(0.0));
    B2->Extent = Vector3(15.0, 15.0, 13.0);
    MainScene.AddObject(B2);

    
    
    auto Render = std::async(std::launch::async, &RScene::Render, &MainScene);
    //MainScene.Render();

    
    auto data = new float[HEIGHT * WIDTH * 3];
    while (!glfwWindowShouldClose(Window))
    {
        RTexture<Vector3> HDR = MainScene.GetTexture3D(RScene::SceneType3D::HDR);
        ToneCompression(HDR);
        
        for (size_t i = 0; i < WIDTH * HEIGHT * 3; i += 3)
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
        
        glDeleteTextures(1, &tex);
        glfwSwapBuffers(Window);        
    }

    glfwTerminate();
    
    
    Render.wait();
    RTexture<Vector3> HDR = MainScene.GetTexture3D(RScene::SceneType3D::HDR);

    Bloom(HDR);
    ToneCompression(HDR, 2.0);
    GammaCorrection(HDR);
	
    ImageUtility::SaveImage(HDR, ".\\Result\\final");

    system("pause");
}

