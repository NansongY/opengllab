#include <iostream>
#include <GL/glew.h> 
#include <GLFW/glfw3.h>
#include <vector> 
#include <fstream> 
#include <sstream>
#include "ThreeDModel.h" 
#include <array>

using namespace std;  
//Variables 
GLFWwindow* window; 
int window_width = 1920; 
int window_height = 1080;

void ThreeDToGL(const ThreeDModel& model, 
vector<array<float,3>>& out_vertices, 
vector<array<float,2>>& out_uvs,  
vector<array<float,3>>& out_normals)  
{ 
for (unsigned int face = 0; face < model.faceVertices.size(); face++) 
{  
    for (unsigned int triangle = 0; triangle < model.faceVertices[face].size() - 2;  triangle++) 
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

bool readAndCompileShader(const char* shader_path, const GLuint& id) {  
	string shaderCode; 
    ifstream shaderStream(shader_path, std::ios::in); 
    if (shaderStream.is_open()) { 
        stringstream sstr; 
        sstr << shaderStream.rdbuf(); 
        shaderCode = sstr.str(); 
        shaderStream.close(); 
    } 
    else { 
        cout << "Impossible to open "<< shader_path << ". Are you in the right directory?"<< endl; 
        return false; 
    }
	cout << "Compiling shader :"<< shader_path<<endl; 
	char const* sourcePointer = shaderCode.c_str(); 
	glShaderSource(id, 1, &sourcePointer, NULL); 
	glCompileShader(id); 
	GLint Result = GL_FALSE; 
	int InfoLogLength; 
	glGetShaderiv(id, GL_COMPILE_STATUS, &Result); 
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &InfoLogLength); 
	if (InfoLogLength > 0) { 
		vector<char> shaderErrorMessage(InfoLogLength + 1); 
		glGetShaderInfoLog(id, InfoLogLength, NULL, &shaderErrorMessage[0]); 
		cout << &shaderErrorMessage[0] << endl; 
	} 
	cout << "Compilation of Shader: " << shader_path << " " << (Result == GL_TRUE ? "Success" : "Failed!") << endl; 
	return Result == 1; 
}

void LoadShaders(GLuint& program, const char* vertex_file_path, const char* 
fragment_file_path) 
{ 
    // Create the shaders - tasks 1 and 2  
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER); 
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
    bool vok = readAndCompileShader(vertex_file_path, VertexShaderID); 
    bool fok = readAndCompileShader(fragment_file_path, FragmentShaderID);
	if (vok && fok) {     
        GLint Result = GL_FALSE; 
        int InfoLogLength; 
 
        cout <<"Linking program"<<endl; 
        program = glCreateProgram(); 
        glAttachShader(program, VertexShaderID); 
        glAttachShader(program, FragmentShaderID); 
        glLinkProgram(program); 
 
        glGetProgramiv(program, GL_LINK_STATUS, &Result); 
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &InfoLogLength); 
        if (InfoLogLength > 0) { 
            std::vector<char> ProgramErrorMessage(InfoLogLength + 1); 
            glGetProgramInfoLog(program, InfoLogLength, NULL, &ProgramErrorMessage[0]); 
            cout << &ProgramErrorMessage[0]; 
        } 
        std::cout << "Linking program: " << (Result == GL_TRUE ? "Success" : "Failed!") << std::endl; 
    }else{ 
        std::cout << "Program will not be linked: one of the shaders has an error" << std::endl; 
    }
	glDeleteShader(VertexShaderID); 
	glDeleteShader(FragmentShaderID);
}

bool initializeGL() 
{ 
    // Initialise GLFW 
    if (!glfwInit()) 
    { 
        cerr << "Failed to initialize GLFW" << endl; 
        return false; 
    } 
	
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

	GLuint programID = glCreateProgram(); 
	LoadShaders(programID,"Basic.vert","Phong.frag");

	glClearColor(0, 0, 0, 0.0f); 
	glEnable(GL_FRAMEBUFFER_SRGB); 
	glEnable(GL_DEPTH_TEST); 
	glDepthFunc(GL_LESS); 
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW); 
	glClipControl(GL_LOWER_LEFT, GL_ZERO_TO_ONE);
	glUseProgram(programID); 
	GLuint MatrixID = glGetUniformLocation(programID, "MVP"); 
	GLuint ViewMatrixID = glGetUniformLocation(programID, "V"); 
	GLuint ModelMatrixID = glGetUniformLocation(programID, "M"); 
	std::vector<GLuint> PosLightIDs; 
	std::vector<GLuint> ColLightIDs; 
	//Set the light position and color 
	int currentLight = 0; 
	for (Light* l : renderParameters.lights) { 
		PosLightIDs.push_back(glGetUniformLocation(programID, 
			("lights[" + std::to_string(currentLight) + "].position").c_str())); 
		ColLightIDs.push_back(glGetUniformLocation(programID, 
			("lights[" + std::to_string(currentLight) + "].color").c_str())); 
		currentLight++; 
	} 
	GLuint nLightsID = glGetUniformLocation(programID, "nLights"); 
	GLuint emissiveID = glGetUniformLocation(programID, "meshMaterial.emissiveColor"); 
	GLuint diffuseID = glGetUniformLocation(programID, "meshMaterial.diffuseColor"); 
	GLuint ambientID = glGetUniformLocation(programID, "meshMaterial.ambientColor"); 
	GLuint specularID = glGetUniformLocation(programID, "meshMaterial.specularColor"); 
	GLuint shininessID = glGetUniformLocation(programID, "meshMaterial.shininess"); 

	// For speed computation 
	double lastTime = glfwGetTime(); 
	int nbFrames = 0; 
	do { 
	// Measure speed
    	double currentTime = glfwGetTime(); 
		float deltaTime = float(currentTime - lastTime); 
		nbFrames++; 
		if (deltaTime >= 1.0) { // If last prinf() was more than 1sec ago 
			// printf and reset 
			printf("%f ms/frame\n", 1000.0 / double(nbFrames)); 
			nbFrames = 0; 
			lastTime += 1.0; 
		} 
		// Clear the screen 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 
		Matrix4 ModelMatrix = renderParameters.getModelMatrix(); 
		Matrix4 ViewMatrix = renderParameters.getViewMatrix(); 
		Matrix4 ProjectionMatrix = renderParameters.getProjectionMatrix(float(window_width)/2.0f,float(window_height)); 
		Matrix4 MVP = ProjectionMatrix * (ViewMatrix * ModelMatrix); 
	
		//First pass: Rasterisation 
		glUseProgram(programID); 
		// Send our transformation to the currently bound shader,  
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, MVP.columnMajor().coordinates); 
		glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, ModelMatrix.columnMajor().coordinates); 
		glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, ViewMatrix.columnMajor().coordinates); 
		//Set the light position and color 
		int cl = 0; 
		for (Light* l : renderParameters.lights) { 
			Homogeneous4 lp = l->GetPositionCenter(); 
			Homogeneous4 c = l->GetColor(); 
			glUniform3f(PosLightIDs[cl], lp.x, lp.y, lp.z); 
			glUniform3f(ColLightIDs[cl], c.x, c.y, c.z); 
			cl++; 
		} 
		glUniform1i(nLightsID,currentLight); 
		// This will draw on the left side
		glViewport(0, 0, GLsizei(window_width / 2.0f), window_height);

		for (int i = 0; i < vaoIDS.size(); i++) {
			// Setting material properties
			Cartesian3 d = objects[i].material->diffuse;
			Cartesian3 a = objects[i].material->ambient;
			Cartesian3 s = objects[i].material->specular;
			Cartesian3 e = objects[i].material->emissive;
			float shin = objects[i].material->shininess;

			glUniform3f(diffuseID, d.x, d.y, d.z);
			glUniform3f(ambientID, a.x, a.y, a.z);
			glUniform3f(specularID, s.x, s.y, s.z);
			glUniform3f(emissiveID, e.x, e.y, e.z);
			glUniform1f(shininessID, shin);

			glBindVertexArray(vaoIDS[i]);

			// Draw the triangles!
			glDrawArrays(GL_TRIANGLES, 0, counts[i]);
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	} // Check if the ESC key was pressed or the window was closed 
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS && glfwWindowShouldClose(window) == 0); 
	return 0;
}

