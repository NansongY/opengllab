//////////////////////////////////////////////////////////////////////
//
//	University of Leeds
//	COMP 5892M Advanced Rendering
//	User Interface for Coursework
//
//	September, 2020
//
//  -----------------------------
//  Raytrace Render Widget
//  -----------------------------
//
//	Provides a widget that displays a fixed image
//	Assumes that the image will be edited (somehow) when Render() is called
//  
////////////////////////////////////////////////////////////////////////

// include guard
#ifndef RAYTRACE_RENDER_WIDGET_H
#define RAYTRACE_RENDER_WIDGET_H

#include <vector>
#include <mutex>

// and include all of our own headers that we need
#include "ThreeDModel.h"
#include "RenderParameters.h"
#include "Scene.h"

class Raytracer 										
	{ 
	
	private:	
	// the geometric object to be rendered
    std::vector<ThreeDModel> *texturedObjects;
	// the render parameters to use
	RenderParameters *renderParameters;
	// An image to use as a framebuffer
    //A friendly Scene representation that we control
    Scene raytraceScene;

	public:
	// constructor
	Raytracer
			(
	 		// the geometric object to show
            std::vector<ThreeDModel> 		*newTexturedObject,
			// the render parameters to use
			RenderParameters 	*newRenderParameters
			);
	
	// destructor
	~Raytracer();
	
	void resize(int w, int h);
	void stopRaytracer();
	RGBAImage frameBuffer;

	protected:

	// called every time the widget needs painting

	public:

    // routine that generates the image
    void Raytrace();
    //threading stuff
    void RaytraceThread();
	//debug
	void RaytraceDebug();
    private:

	std::atomic<bool> raytracingRunning;
	std::atomic<bool> restartRaytrace;

	}; // class RaytraceRenderWidget

#endif
