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
    #pragma omp parallel for schedule(dynamic)
    for(int j = 0; j < frameBuffer.height; j++){ 
        for(int i = 0; i < frameBuffer.width; i++){ 
            if (restartRaytrace) {  
                raytracingRunning = false; 
                return; 
            } 
            Homogeneous4 color(i/float(frameBuffer.width), j/float(frameBuffer.height), 0); 
            frameBuffer[j][i] = RGBAValue( 
                               linear_to_srgb(color.x), 
                               linear_to_srgb(color.y),  
                               linear_to_srgb(color.z),255); 
            } 
    } 
    raytracingRunning = false;
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
    


