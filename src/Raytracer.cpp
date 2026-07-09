//////////////////////////////////////////////////////////////////////
//
//  University of Leeds
//  COMP 5892M Advanced Rendering
//  User Interface for Coursework
////////////////////////////////////////////////////////////////////////


#include <math.h>
#include <thread>
#include <random>
#include <omp.h>
#include <algorithm>
// include the header file
#include "Raytracer.h"
#include "Ray.h"

#define N_THREADS 16
#define N_LOOPS 600
#define N_BOUNCES 10
#define TERMINATION_FACTOR 0.35f

// constructor
Raytracer::Raytracer(std::vector<ThreeDModel> *newTexturedObject, RenderParameters *newRenderParameters):
    texturedObjects(newTexturedObject),
    renderParameters(newRenderParameters),
    raytraceScene(texturedObjects,renderParameters)
    { 
        std::srand(static_cast<unsigned int>(std::time(nullptr)));
        restartRaytrace = false;
        raytracingRunning = false;
    }     


Raytracer::~Raytracer()
    { 
    // all of our pointers are to data owned by another class
    // so we have no responsibility for destruction
    }                                                                  


// called every time the widget is resized
void Raytracer::resize(int w, int h)
    { // RaytraceRenderWidget::resizeGL()
    // resize the render image
    frameBuffer.Resize(w, h);
    } // RaytraceRenderWidget::resizeGL()
    
void Raytracer::stopRaytracer() {
    restartRaytrace = true;
    while (raytracingRunning) {
        std::chrono::milliseconds timespan(10);
        std::this_thread::sleep_for(timespan);
    }
    restartRaytrace = false;
}

inline
float linear_from_srgb(std::uint8_t aValue) noexcept
{
    float const fvalue = float(aValue) / 255.f;

    if (fvalue < 0.04045f)
        return (1.f / 12.92f) * fvalue;

    return std::pow((1.f / 1.055f) * (fvalue + 0.055f), 2.4f);

}

inline
std::uint8_t linear_to_srgb(float aValue) noexcept
{
    if (aValue < 0.0031308f)
        return std::uint8_t(255.f * 12.92f * aValue + 0.5f);
    return std::uint8_t(255.f * (1.055f * std::pow(aValue, 1.f / 2.4f) - 0.055f) + 0.5f);
}

void Raytracer::RaytraceThread()
{
    for(int j = 0; j < frameBuffer.height; j++){
        if (restartRaytrace) {  
                raytracingRunning = false; 
                return; 
            } 
        #pragma omp parallel for schedule(dynamic)
        for(int i = 0; i < frameBuffer.width; i++){ 
            // Homogeneous4 color(i/float(frameBuffer.width), j/float(frameBuffer.height), 0); 
            Ray r = calculateRay(i, j, !renderParameters->orthoProjection);
            Scene::CollisionInfo ci = raytraceScene.closestTriangle(r);
            Homogeneous4 color;
            if (ci.t > 0.0f)
                color = Homogeneous4(1.0f, 1.0f, 1.0f, 1.0f);
            else
                color = Homogeneous4(0.0f, 0.0f, 0.0f, 1.0f);
            frameBuffer[j][i] = RGBAValue( 
                               linear_to_srgb(color.x), 
                               linear_to_srgb(color.y),  
                               linear_to_srgb(color.z),255); 
        } 
    } 
    raytracingRunning = false;
}

Ray Raytracer::calculateRay(int pixelx, int pixely, bool perspective)
{
    const float width = float(frameBuffer.width);
    const float height = float(frameBuffer.height);
    const float aspect = width / height;

    float left, right, bottom, top;

    if (perspective)
    {
        const float halfY = renderParameters->near * tanf(renderParameters->fov * 0.5f);
        const float halfX = halfY * aspect;

        left = -halfX;
        right = halfX;
        bottom = halfY;
        top = -halfY;
    }
    else
    {
        left = aspect > 1.0f ? -aspect : -1.0f;
        right = aspect > 1.0f ? aspect : 1.0f;
        top = aspect > 1.0f ? -1.0f : -1.0f / aspect;
        bottom = aspect > 1.0f ? 1.0f : 1.0f / aspect;
    }

    const float u = (frameBuffer.width > 1) ? float(pixelx) / float(frameBuffer.width - 1) : 0.5f;
    const float v = (frameBuffer.height > 1) ? float(pixely) / float(frameBuffer.height - 1) : 0.5f;

    const float x = left + u * (right - left);
    const float y = top - v * (top - bottom);

    if (perspective)
    {
        Cartesian3 origin(0.0f, 0.0f, 0.0f);
        Cartesian3 direction(x, y, renderParameters->near);
        return Ray(origin, direction.unit(), Ray::primary);
    }

    Cartesian3 origin(x, y, renderParameters->near);
    Cartesian3 direction(0.0f, 0.0f, 1.0f);
    return Ray(origin, direction, Ray::primary);
}

    // routine that generates the image
void Raytracer::Raytrace()
{ // RaytraceRenderWidget::Raytrace()
    stopRaytracer();
    //To make our lifes easier, lets calculate things on VCS.
    //So we need to process our scene to get a triangle soup in VCS.
    raytraceScene.updateScene();
    frameBuffer.clear(RGBAValue(0.0f, 0.0f, 0.0f,1.0f));
    std::thread raytracingThread(&Raytracer::RaytraceThread,this);
    raytracingThread.detach();
    raytracingRunning = true;
} // RaytraceRenderWidget::Raytrace()
    
void Raytracer::RaytraceDebug()  
{ 
    renderParameters->ModelPosition = Cartesian3(0.0, 0.0, 0.0); 
    renderParameters->CameraPosition = Cartesian3(0.0, 0.0, 0.0); 
    renderParameters->ModelArcball = Quaternion(0, 1, 0, 0); 
    renderParameters->CameraArcball = Quaternion(0, 0, 0, 1); 
    raytraceScene.updateScene(); 
     
    std::cout << std::endl; 
    std::cout << "###############################" << std::endl; 
    std::cout << "#Debugging the first few steps# " << std::endl; 
    std::cout << "###############################" << std::endl; 
    std::cout << std::endl; 
 
    std::cout << std::endl; 
    std::cout << "#Task 1# " << std::endl; 
    std::cout << std::endl; 
    std::cout << "#No transformations: # " << std::endl; 
    Triangle tri = raytraceScene.triangles[0]; 
    std::cout << "v0:"<<tri.verts[0] << std::endl; 
    std::cout << "v1:" << tri.verts[1] << std::endl; 
    std::cout << "v2:" << tri.verts[2] << std::endl; 
 
    std::cout << "#Model at z=2, 1 to left: # " << std::endl; 
    renderParameters->ModelPosition = Cartesian3(-1.0, 0.0, 2.0); 
    raytraceScene.updateScene(); 
    tri = raytraceScene.triangles[0]; 
    std::cout << "v0:" << tri.verts[0] << std::endl; 
    std::cout << "v1:" << tri.verts[1] << std::endl; 
    std::cout << "v2:" << tri.verts[2] << std::endl; 
 
    std::cout << "#Just rotate 180 (signals flip): # " << std::endl; 
    renderParameters->ModelPosition = Cartesian3(0.0, 0.0, 0.0); 
    renderParameters->ModelArcball.currentRotation = Quaternion(0, 0, 1, 0); 
    raytraceScene.updateScene(); 
    tri = raytraceScene.triangles[0]; 
    std::cout << "v0:" << tri.verts[0] << std::endl; 
    std::cout << "v1:" << tri.verts[1] << std::endl; 
    std::cout << "v2:" << tri.verts[2] << std::endl; 
 
    std::cout << "#Rotate 180 and translate: Did it go left? # " << std::endl; 
    renderParameters->ModelPosition = Cartesian3(-1.0, 0.0, 2.0); 
    renderParameters->ModelArcball.currentRotation = Quaternion(0, 0, 1, 0); 
    raytraceScene.updateScene(); 
    tri = raytraceScene.triangles[0]; 
    std::cout << "v0:" << tri.verts[0] << std::endl; 
    std::cout << "v1:" << tri.verts[1] << std::endl; 
    std::cout << "v2:" << tri.verts[2] << std::endl; 
 
    std::cout << std::endl; 
    std::cout << "#Task 2# " << std::endl; 
    std::cout << std::endl; 
    std::cout << "#One ray to each corner!# " << std::endl; 
    Ray r0 = calculateRay(0, 0, true); 
    Ray r1 = calculateRay(0, frameBuffer.height-1,true); 
    Ray r2 = calculateRay(frameBuffer.width-1, frameBuffer.height-1, true); 
    Ray r3 = calculateRay(frameBuffer.width - 1, 0, true); 
    std::cout << "pixel is " << 0 <<" "<< 0 << " d: " << r0.direction << std::endl; 
    std::cout << "pixel is " << 0 << " " << frameBuffer.height-1 << " d: " << r1.direction << std::endl; 
    std::cout << "pixel is " << frameBuffer.width - 1 << " " << frameBuffer.height - 1 << " d: " << r2.direction << std::endl; 
    std::cout << "pixel is " << frameBuffer.width - 1 << " " <<0 << " d: " << r3.direction << std::endl; 
     
    std::cout << "#Rays to the center on a diagonal!# " << std::endl; 
 
    Ray rc = calculateRay(((frameBuffer.width - 1) / 2.0f)-1, ((frameBuffer.height - 1) / 2.0f)-1, true); 
    Ray rc1 = calculateRay(((frameBuffer.width-1) / 2.0f), ((frameBuffer.height-1) / 2.0f), true); 
    Ray rc2 = calculateRay(((frameBuffer.width - 1) / 2.0f)+1, ((frameBuffer.height - 1) / 2.0f)+1, true); 
    Ray rc3 = calculateRay(((frameBuffer.width - 1) / 2.0f) + 2, ((frameBuffer.height - 1) / 2.0f) + 2, true); 
 
    std::cout << "pixel is " << ((frameBuffer.width - 1) / 2.0f) - 1 << " " << ((frameBuffer.height - 1) / 2.0f) - 1 << " d: " << rc.direction << std::endl; 
    std::cout << "pixel is " << (frameBuffer.width - 1) / 2.0f << " " << ((frameBuffer.height - 1) / 2.0f) << " d: " << rc1.direction << std::endl; 
    std::cout << "pixel is " << ((frameBuffer.width - 1) / 2.0f) +1<< " " << ((frameBuffer.height - 1) / 2.0f) + 1 << " d: " << rc2.direction << std::endl; 
    std::cout << "pixel is " << ((frameBuffer.width - 1) / 2.0f) + 2 << " " << ((frameBuffer.height - 1) / 2.0f) + 2 << " d: " << rc3.direction << std::endl; 
 
 
    renderParameters->ModelPosition = Cartesian3(0.0, 0.0, 2.0); 
    renderParameters->CameraPosition = Cartesian3(0.0, 0.0, 0.0); 
    renderParameters->ModelArcball = Quaternion(0, 1, 0, 0); 
    renderParameters->CameraArcball = Quaternion(0, 0, 0, 1); 
    raytraceScene.updateScene(); 
 
    Scene::CollisionInfo ci0 = raytraceScene.closestTriangle(r0); 
    Scene::CollisionInfo ci1 = raytraceScene.closestTriangle(r1); 
    Scene::CollisionInfo ci2 = raytraceScene.closestTriangle(r2); 
    Scene::CollisionInfo ci3 = raytraceScene.closestTriangle(r3); 
     
    std::cout << "#Do they hit at initial position? (z=2)# " << std::endl; 
    std::cout << ci0.tri.isValid() << "== 0" <<std::endl; 
    std::cout << ci1.tri.isValid() << "== 1"<<std::endl; 
    std::cout << ci2.tri.isValid() << "== 1"<<std::endl; 
    std::cout << ci3.tri.isValid() << "== 1"<<std::endl; 
 
}

