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
#include "Headers/Light.h"
#include "ThirdParty/glfw3.h"
#include "ThirdParty/glfw3native.h"

constexpr uint32_t HEIGHT = 720;
constexpr uint32_t WIDTH = 1280;

void AddCornellBox(RScene* Scene, const Vector3& Position, const Vector3& Extent)
{
    auto Left = MakeShared<OPlane>();
    Left->Transform.SetPosition(Position + Vector3(0.0, -Extent.Y, 0.0));
    auto LeftMat = MakeShared<RMaterial>();
    LeftMat->InitializePBR(Vector3(1.0, 0.0, 0.0), Vector3(0.0), 0.8, 0.0, 1.0, 0.0);
    Left->SetMaterial(LeftMat);
    Left->Normal = Vector3(0.0, 1.0, 0.0);

    auto Right = MakeShared<OPlane>();
    Right->Transform.SetPosition(Position + Vector3(0.0, Extent.Y, 0.0));
    auto RightMat = MakeShared<RMaterial>();
    RightMat->InitializePBR(Vector3(0.0, 1.0, 0.0), Vector3(0.0), 0.8, 0.0, 1.0, 0.0);
    Right->SetMaterial(RightMat);
    Right->Normal = Vector3(0.0, -1.0, 0.0);

    auto Back = MakeShared<OPlane>();
    Back->Transform.SetPosition(Position + Vector3(Extent.X, 0.0, 0.0));
    auto BackMat = MakeShared<RMaterial>();
    BackMat->InitializePBR(Vector3(1.0), Vector3(0.0), 0.8, 0.0, 1.0, 0.0);
    Back->SetMaterial(BackMat);
    Back->Normal = Vector3(-1.0, 0.0, 0.0);

    auto Top = MakeShared<OPlane>();
    Top->Transform.SetPosition(Position + Vector3(0.0, 0.0, Extent.Z));
    auto TopMat = MakeShared<RMaterial>();
    TopMat->InitializePBR(Vector3(1.0), Vector3(0.0), 0.8, 0.0, 1.0, 0.0);
    Top->SetMaterial(TopMat);
    Top->Normal = Vector3(0.0, 0.0, -1.0);

    auto Bottom = MakeShared<OPlane>();
    Bottom->Transform.SetPosition(Position + Vector3(0.0, 0.0, -Extent.Z));
    auto BottomMat = MakeShared<RMaterial>();
    BottomMat->InitializePBR(Vector3(1.0), Vector3(0.0), 0.8, 0.0, 1.0, 0.0);
    Bottom->SetMaterial(BottomMat);
    Bottom->Normal = Vector3(0.0, 0.0, 1.0);

    auto Front = MakeShared<OPlane>();
    Front->Transform.SetPosition(Position + Vector3(-Extent.X, 0.0, 0.0));
    auto FrontMat = MakeShared<RMaterial>();
    FrontMat->InitializePBR(Vector3(1.0), Vector3(0.0), 0.8, 0.0, 1.0, 0.0);
    Front->SetMaterial(FrontMat);
    Front->Normal = Vector3(1.0, 0.0, 0.0);

    auto Light = MakeShared<RSphereLight>(Vector3(600.0), 0.2);
    Light->Transform.SetPosition(Position + Vector3(Extent.X / 2.0, 0.0, Extent.Z - 2.0));

    Scene->AddObject(Left);
    Scene->AddObject(Right);
    Scene->AddObject(Back);
    Scene->AddObject(Top);
    Scene->AddObject(Bottom);
    Scene->AddObject(Front);
    Scene->AddObject(Light);
}

void Resize(GLFWwindow* Window, const int Width, const int Height)
{
    glViewport(0, 0, Width, Height);
}

int main()
{
    RScene MainScene(HEIGHT, WIDTH);
    MainScene.SetShader(MakeUnique<RShader>());
    MainScene.SetBRDF(MakeUnique<CookTorrance>());

    auto EnvMap = MakeUnique<RTexture>(0, 0);
    EnvMap->Load("envmap.jpg");
    MainScene.SetEnvironmentTexture(EnvMap);

    GLFWwindow* Window;
    glfwInit();
    Window = glfwCreateWindow(WIDTH, HEIGHT, "Raytracing", nullptr, nullptr);
    glfwMakeContextCurrent(Window);
    glfwSetWindowSizeCallback(Window, &Resize);

    AddCornellBox(&MainScene, { 0.0, 0.0, 3.0 }, { 14.0, 10.0, 8.0 });
   

    auto S1 = MakeShared<OSphere>();
    S1->Radius = 2.0;
    S1->Transform.SetPosition(Vector3(10.0, -4.0, -3.0));
    S1->SetMaterial(MakeShared<RMaterial>(RMaterial::WhiteDielectric(1.0)));
    //MainScene.AddObject(S1);

    auto S2 = MakeShared<OSphere>();
    S2->Radius = 3.0;
    S2->Transform.SetPosition(Vector3(10.0, 4.0, -2.0));
    S2->SetMaterial(MakeShared<RMaterial>(RMaterial::Metal(0.25)));
    //MainScene.AddObject(S2);

    auto S3 = MakeShared<OSphere>();
    S3->Radius = 2.0;
    S3->Transform.SetPosition(Vector3(9.0, 0.0, -3.0));
    S3->SetMaterial(MakeShared<RMaterial>(RMaterial::Metal(0.2)));
    //MainScene.AddObject(S3);

    
    auto Teapot = MakeShared<OMesh>("dragon.obj");
    Teapot->Transform.SetPosition(Vector3(11.0, 0.0, -3.0));
    Teapot->Transform.SetRotation(Vector3(0.0, 0.0, 155.0));
    Teapot->Transform.SetScale(Vector3(3.8, 3.8, 3.8));
    auto ObjectMat = MakeShared<RMaterial>();
    ObjectMat->InitializePBR(Vector3(1.0, 1.0, 0.0), Vector3(0.0), 0.2, 1.0, 1.0, 0.0);
    Teapot->SetMaterial(ObjectMat);
    MainScene.AddObject(Teapot);
    

    auto Render = std::async(std::launch::async, &RScene::Render, &MainScene);
    //MainScene.Render();

    
    auto data = MakeUnique<float[]>(HEIGHT * WIDTH * 3);
    while (!glfwWindowShouldClose(Window))
    {
        auto HDR = MainScene.GetRenderTexture();
        
        for (size_t i = 0; i < WIDTH * HEIGHT; i++)
        {
            RColor Color = *(HDR->Data() + i);
            data[i * 3 + 0] = Color.R;
            data[i * 3 + 1] = Color.G;
            data[i * 3 + 2] = Color.B;
        }    

        glfwPollEvents();

        glClearColor(0.1, 0.1, 0.9, 1.0);

        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, WIDTH, HEIGHT, 0, GL_RGB, GL_FLOAT, data.get());
        glBindTexture(GL_TEXTURE_2D, 0);
       
        
        glMatrixMode(GL_PROJECTION);
        glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1);
        glMatrixMode(GL_MODELVIEW);

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
    auto HDR = MakeUnique<RTexture>(*MainScene.GetRenderTexture());

    Bloom(HDR, 10.0);
    ToneCompression(HDR, 2.0);
    GammaCorrection(HDR);
	
    ImageUtility::SaveImage(HDR.get(), ".\\Result\\final");

    system("pause");
}

