#include <iostream>
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <vector> 
#include <fstream> 
#include "ThreeDModel.h" 

using namespace std;  

bool initializeGL() 
{ 
    // Initialise GLFW 
    if (!glfwInit()) 
    { 
        cerr << "Failed to initialize GLFW" << endl; 
        return false; 
    } 
	//Variables 
	GLFWwindow* window; 
	int window_width = 1920; 
	int window_height = 1080;
	glfwWindowHint(GLFW_SAMPLES, 1); //no anti-aliasing 
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);  
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5); 
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy;  
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 
	window = glfwCreateWindow(window_width, window_height, "OpenGLRenderer", NULL, NULL);
	if (window == NULL) { 
		cerr<< "Failed to open GLFW window. If you have an Intel GPU, they may not be 4.5 compatible."<< endl; 
		glfwTerminate(); 
		return false; 
		} 
	glfwMakeContextCurrent(window);
	// Initialize GLEW 
	glewExperimental = true; // Needed for core profile 
	if (glewInit() != GLEW_OK) { 
		cerr << "Failed to initialize GLEW" << endl; 
		glfwTerminate(); 
		return false; 
	}
	// //without 
	// int NumberOfExtensions; 
	// glGetIntegerv(GL_NUM_EXTENSIONS, &NumberOfExtensions); 
	// for (int i = 0; i < NumberOfExtensions; i++) { 
	// 	const GLubyte* ccc = glGetStringi(GL_EXTENSIONS, i); 
	// 	if (strcmp(ccc, (const GLubyte*)"GL_ARB_debug_output") != 0) { 
	// 		return false; 
	// 	} 
	// } 
	//with 
	if (!GLEW_ARB_debug_output) return false;
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE); 
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
	glfwPollEvents(); 
	
	return true; 
}

int main(int argc, char**argv)
{
	if (argc != 3) 
	{ // bad arg count 
	// print an error message 
	std::cout << "Usage: " << argv[0] << " geometry material" << std::endl; 
	// and leave 
	return 0; 
	} // bad arg count 

	if (!initializeGL()) return -1;

	std::vector<ThreeDModel> objects; 
	std::ifstream geometryFile(argv[1]); 
	std::ifstream materialFile(argv[2]); 

	// try reading it 
	if (!(geometryFile.good()) || !(materialFile.good())) { 
		std::cout << "Read failed for object " << argv[1] << " or material " << argv[2] << std::endl; 
		return 0; 
	} // object read failed

	std::string s = argv[2]; 
	//if is actually passing a material. This will trigger the modified obj read code. 
	if (s.find(".mtl") != std::string::npos) { 
		objects = ThreeDModel::ReadObjectStreamMaterial(geometryFile, materialFile); 
	} 
	
	if (objects.size() == 0) { 
		std::cout << "Read failed for object " << argv[1] << " or material " << argv[2] << std::endl;
		return 0; 
	} // object read failed

	RenderParameters renderParameters; //can be at file level. 
 
	renderParameters.findLights(objects); 
	std::cout << renderParameters.lights.size() << std::endl;

	return 0;
}

