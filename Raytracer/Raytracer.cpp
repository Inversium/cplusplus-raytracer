#include <iostream>
#include "Headers/math/mvector.h"
#include "Headers/BMPWriter.h"
#include "Headers/CoreUtilities.h"
#include "Headers/OObject.h"
#include "Headers/Scene.h"
#include "Headers/PostProcess.h"


constexpr uint32_t HEIGHT = 380;
constexpr uint32_t WIDTH = 640;


int main()
{
    Scene MainScene(HEIGHT, WIDTH);
    MainScene.ShadowSamples = 16;
    MainScene.RayDepth = 1;
	
    auto* S = new OSphere;
    S->Position = Vector3(0.0, 5.0, -30.0);
    S->Radius = 8.0;
    S->Mat = RMaterial::YellowRubber;
    MainScene.AddObject(S);

    auto* S1 = new OSphere;
    S1->Position = Vector3(20.0, 0.0, -30.0);
    S1->Radius = 8.0;
    S1->Mat = RMaterial::BluePlastic;
    MainScene.AddObject(S1);

    auto* S2 = new OSphere;
    S2->Position = Vector3(-5.0, 3.0, -7.0);
    S2->Radius = 2.3;
    S2->Mat = RMaterial::Glass;
    //MainScene.AddObject(S2);

    
    auto* P = new OPlane;
    P->Position = Vector3(0.0, 30.0, 0.0);
    P->Mat = RMaterial::RedPlastic;
    P->Normal = Vector3(0.0, -1.0, 0.0);
    //MainScene.AddObject(P);

	
    auto* P1 = new OPlane;
    P1->Position = Vector3(0.0, 0.0, 1000.0);
    P1->Mat = RMaterial::BluePlastic;
    P1->Normal = Vector3(0.0, 0.0, -1.0);
    //MainScene.AddObject(P1);
    

    auto* B1 = new OBox;
    B1->Position = Vector3(-7.0, 3.0, -22.0);
    B1->Mat = RMaterial::Glass;
    B1->Mat.Transmission = 0.95;
    B1->Extent = Vector3(5.0, 8.0, 5.0);
    MainScene.AddObject(B1);

    
    auto* B2 = new OBox;
    B2->Position = Vector3(0.0, 0.0, 0.0);
    B2->Mat = RMaterial::BluePlastic;
    B2->Extent = Vector3(60.0, 30.0, 45.0);
    MainScene.AddObject(B2);
    
    

    auto* L = new OLight;
    L->Position = Vector3(-20.0, -20.0, 0.0);
    L->Color = Vector3(1.0, 1.0, 1.0) * 6000.0;
    L->Radius = 5.0;
    MainScene.AddObject(L);

    
    auto* L1 = new OLight;
    L1->Position = Vector3(30.0, -20.0, 20.0);
    L1->Color = Vector3(1.0, 1.0, 1.0) * 7000.0;
    L1->Radius = 5.0;
    MainScene.AddObject(L1);
    

    MainScene.Render();

    
	
    Texture3D HDR = MainScene.GetTexture3D(Scene::SceneType3D::HDR);
    HDR.Resize(1080, 1920, false);
    SaveImage(HDR, "output_nearest.bmp");

    HDR = MainScene.GetTexture3D(Scene::SceneType3D::HDR);
    HDR.Resize(1920, 1080, true);
    SaveImage(HDR, "output_bilinear.bmp");


    Bloom(HDR);
    ToneCompression(HDR);
    GammaCorrection(HDR);
	
    SaveImage(HDR, "blurred.bmp");  
    SaveImage(MainScene.GetTexture3D(Scene::SceneType3D::BASE_COLOR), "BaseColor.bmp");

    Texture3D DepthBuffer = Texture3DFrom1D(MainScene.GetTexture1D(Scene::SceneType1D::DEPTH), 
													   EColorChannels::R | EColorChannels::G | EColorChannels::B);
	
    std::transform(DepthBuffer.Begin(), DepthBuffer.End(), DepthBuffer.Begin(), 
        [](const Vector3& A)
        {
            return A / 100.0;
        });
	
    SaveImage(DepthBuffer, "Depth.bmp");  
}

