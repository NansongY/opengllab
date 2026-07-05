#include <iostream>
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <vector> 
#include <fstream> 
#include "ThreeDModel.h" 
#include <array>

using namespace std;  

void ThreeDToGL(const ThreeDModel& model, 
vector<array<float,3>>& out_vertices, 
vector<array<float,2>>& out_uvs,  
vector<array<float,3>>& out_normals)  
{ 
for (unsigned int face = 0; face < model.faceVertices.size(); face++) 
        {  
            for (unsigned int triangle = 0; triangle < model.faceVertices[face].size() - 2;  
                 triangle++) 
            {  
                for (unsigned int vertex = 0; vertex < 3; vertex++) 
                { 
                unsigned int faceVertex = 0; 
                    if (vertex != 0) 
                        faceVertex = triangle + vertex; 
					out_normals.push_back(array<float, 3>{ 
                    model.normals[model.faceNormals[face][faceVertex]].x, 
                    model.normals[model.faceNormals[face][faceVertex]].y, 
                    model.normals[model.faceNormals[face][faceVertex]].z 
                }); 
                out_uvs.push_back(array<float, 2>{ 
                    model.textureCoords[model.faceTexCoords[face][faceVertex]].x, 
                    model.textureCoords[model.faceTexCoords[face][faceVertex]].y 
                }); 
                out_vertices.push_back(array<float, 3>{ 
                    model.vertices[model.faceVertices[face][faceVertex]].x, 
                    model.vertices[model.faceVertices[face][faceVertex]].y, 
                    model.vertices[model.faceVertices[face][faceVertex]].z 
                });
				} // per vertex 
        } // per triangle 
    } // per face 
}

void loadModelGL(const std::vector<ThreeDModel>& objects,  
    std::vector<GLuint>& vaoIDS, 
    std::vector<GLuint>& vbIDS, 
    std::vector<GLuint>& nbIDS, 
    std::vector<GLuint>& tbIDS, 
    std::vector<unsigned int>& count) 
{ 
    for (const auto& to : objects)  
    { 
        GLuint VertexArrayID; 
        GLuint vertexbuffer; 
        GLuint uvbuffer; 
        GLuint normalbuffer;
        glGenVertexArrays(1, &VertexArrayID); 
        glBindVertexArray(VertexArrayID); 
        std::vector<std::array<float,3>> n; 
        std::vector<std::array<float, 2>> t; 
        std::vector<std::array<float, 3>> v; 
        ThreeDToGL(to, v, t, n);
		glEnableVertexAttribArray(0); 
        glGenBuffers(1, &vertexbuffer); 
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer); 
        glBufferData(GL_ARRAY_BUFFER, v.size() * sizeof(std::array<float, 3>),  
                     &v[0], GL_STATIC_DRAW); 
        glVertexAttribPointer( 
            0,                  // attribute 
            3,                  // size (we have x y z) 
            GL_FLOAT,           // type of each individual element 
            GL_FALSE,           // normalized 
            0,                  // stride 
            (void*)0            // array buffer offset 
        );
		glEnableVertexAttribArray(1); 
		glGenBuffers(1, &uvbuffer); 
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer); 
		glBufferData(GL_ARRAY_BUFFER, t.size() * sizeof(std::array<float, 2>), &t[0],   
					GL_STATIC_DRAW); 
		glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,(void*)0); 
		
		glEnableVertexAttribArray(2); 
		glGenBuffers(1, &normalbuffer); 
		glBindBuffer(GL_ARRAY_BUFFER, normalbuffer); 
		glBufferData(GL_ARRAY_BUFFER, n.size() * sizeof(std::array<float, 3>), &n[0], 
		GL_STATIC_DRAW); 
		glVertexAttribPointer(2,3,GL_FLOAT,GL_FALSE,0,(void*)0); 
		
		vaoIDS.push_back(VertexArrayID); 
		vbIDS.push_back(vertexbuffer); 
		nbIDS.push_back(normalbuffer); 
		tbIDS.push_back(uvbuffer); 
		count.push_back(GLuint(v.size())); 
	} // per object
}

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

	std::vector<GLuint> vaoIDS; 
	std::vector<GLuint> vbIDS; 
	std::vector<GLuint> nbIDS; 
	std::vector<GLuint> tbIDS; 
	std::vector<unsigned int> counts; 
	
	//setting up opengl 
	loadModelGL(objects, vaoIDS, vbIDS, nbIDS, tbIDS, counts); 

	return 0;
}

