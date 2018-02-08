// ==========================================================================
// Barebones OpenGL Core Profile Boilerplate
//    using the GLFW windowing system (http://www.glfw.org)
//
// Loosely based on
//  - Chris Wellons' example (https://github.com/skeeto/opengl-demo) and
//  - Camilla Berglund's example (http://www.glfw.org/docs/latest/quick.html)
//
// Author:  Sonny Chan, University of Calgary
// Co-Authors:
//			Jeremy Hart, University of Calgary
//			John Hall, University of Calgary
// Date:    December 2015
// ==========================================================================

#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iterator>
#include <glm-0.9.8.2/glm/glm.hpp>
#include <glm-0.9.8.2/glm/gtc/matrix_transform.hpp>
#include <glm-0.9.8.2/glm/gtc/type_ptr.hpp>


#include <glad/include/glad/glad.h>
#include <glfw/include/GLFW/glfw3.h>
#include <vector>

#include "texture.h"

using namespace std;
using namespace glm;
// --------------------------------------------------------------------------
// OpenGL utility and support function prototypes

void QueryGLVersion();
bool CheckGLErrors();

string LoadSource(const string &filename);
GLuint CompileShader(GLenum shaderType, const string &source);
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader);
#define GL_PI 3.1415f
MyTexture imageTextures;
string path = "images/image1-mandrill.png";
const char* p = path.c_str();
vector<vec2> vertexPoints;
vector<vec2> texturePoints;
float zoom = 1.0f;
float shift = 0.0f;
float theta = 0.0f;
int sepia = 0;
int sob = 0;
int gauss = 0;
struct Coordinate {
	double x;
	double y;

};

struct Mouse {
	Coordinate image_offset;
	Coordinate prev_image_offset;
};

Mouse mouse;

struct Luminance
{
	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;

};

Luminance luminance;

// --------------------------------------------------------------------------
// Functions to set up OpenGL shader programs for rendering

// load, compile, and link shaders, returning true if successful
GLuint InitializeShaders()
{
	// load shader source from files
	string vertexSource = LoadSource("shaders/vertex.glsl");
	string fragmentSource = LoadSource("shaders/fragment.glsl");
	if (vertexSource.empty() || fragmentSource.empty()) return false;

	// compile shader source into shader objects
	GLuint vertex = CompileShader(GL_VERTEX_SHADER, vertexSource);
	GLuint fragment = CompileShader(GL_FRAGMENT_SHADER, fragmentSource);

	// link shader program
	GLuint program = LinkProgram(vertex, fragment);

	glDeleteShader(vertex);
	glDeleteShader(fragment);

	// check for OpenGL errors and return false if error occurred
	return program;
}

// --------------------------------------------------------------------------
// Functions to set up OpenGL buffers for storing geometry data

struct Geometry
{
	// OpenGL names for array buffer objects, vertex array object
	GLuint  vertexBuffer;
	GLuint  textureBuffer;
	GLuint  colourBuffer;
	GLuint  vertexArray;
	GLsizei elementCount;

	// initialize object names to zero (OpenGL reserved value)
	Geometry() : vertexBuffer(0), colourBuffer(0), vertexArray(0), elementCount(0)
	{}
};

bool InitializeVAO(Geometry *geometry){

	const GLuint VERTEX_INDEX = 0;
	const GLuint TEXTURE_INDEX = 1;

	//Generate Vertex Buffer Objects
	// create an array buffer object for storing our vertices
	glGenBuffers(1, &geometry->vertexBuffer);

	// create another one for storing our colours
	glGenBuffers(1, &geometry->textureBuffer);

	//Set up Vertex Array Object
	// create a vertex array object encapsulating all our vertex attributes
	glGenVertexArrays(1, &geometry->vertexArray);
	glBindVertexArray(geometry->vertexArray);

	// associate the position array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glVertexAttribPointer(
		VERTEX_INDEX,		//Attribute index
		2, 					//# of components
		GL_FLOAT, 			//Type of component
		GL_FALSE, 			//Should be normalized?
		sizeof(vec2),		//Stride - can use 0 if tightly packed
		0);					//Offset to first element
	glEnableVertexAttribArray(VERTEX_INDEX);

	// associate the colour array with the vertex array object
	glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
	glVertexAttribPointer(
		TEXTURE_INDEX,		//Attribute index
		2, 					//# of components
		GL_FLOAT, 			//Type of component
		GL_FALSE, 			//Should be normalized?
		sizeof(vec2), 		//Stride - can use 0 if tightly packed
		0);					//Offset to first element
	glEnableVertexAttribArray(TEXTURE_INDEX);

	// unbind our buffers, resetting to default state
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	return !CheckGLErrors();
}

// create buffers and fill with geometry data, returning true if successful
bool LoadGeometry(Geometry *geometry, vec2 *vertices, vec2 *textures, int elementCount)
{
	geometry->elementCount = elementCount;

	// create an array buffer object for storing our vertices
	glBindBuffer(GL_ARRAY_BUFFER, geometry->vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*geometry->elementCount, vertices, GL_STATIC_DRAW);

	// create another one for storing our colours
	glBindBuffer(GL_ARRAY_BUFFER, geometry->textureBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2)*geometry->elementCount, textures, GL_STATIC_DRAW);

	//Unbind buffer to reset to default state
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// check for OpenGL errors and return false if error occurred
	return !CheckGLErrors();
}

// deallocate geometry-related objects
void DestroyGeometry(Geometry *geometry)
{
	// unbind and destroy our vertex array object and associated buffers
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &geometry->vertexArray);
	glDeleteBuffers(1, &geometry->vertexBuffer);
	glDeleteBuffers(1, &geometry->colourBuffer);
}

// --------------------------------------------------------------------------
// Rendering function that draws our scene to the frame buffer

void RenderScene(Geometry *geometry, GLuint program)
{
	// clear screen to a dark grey colour
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// bind our shader program and the vertex array object containing our
	// scene geometry, then tell OpenGL to draw our geometry
	glUseProgram(program);
	glBindVertexArray(geometry->vertexArray);
	glDrawArrays(GL_TRIANGLES, 0, geometry->elementCount);

	// reset state to default (no shader or geometry bound)
	glBindVertexArray(0);
	glUseProgram(0);

	// check for an report any OpenGL errors
	CheckGLErrors();
}

// --------------------------------------------------------------------------
// GLFW callback functions

// reports GLFW errors
void ErrorCallback(int error, const char* description)
{
	cout << "GLFW ERROR " << error << ":" << endl;
	cout << description << endl;
}

// handles keyboard input events
void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_1 && action == GLFW_PRESS)
	{
		zoom = 1.0f;
		path = "images/image1-mandrill.png";
		p = path.c_str();

	}

	if (key == GLFW_KEY_2 && action == GLFW_PRESS)
	{
		zoom = 1.0f;
		path = "images/image2-uclogo.png";
		p = path.c_str();

	}

	if (key == GLFW_KEY_3 && action == GLFW_PRESS)
	{
		zoom = 1.0f;
		path = "images/image3-aerial.jpg";
		p = path.c_str();

	}

	if (key == GLFW_KEY_4 && action == GLFW_PRESS)
	{
		zoom = 1.0f;
		path = "images/image4-thirsk.jpg";
		p = path.c_str();

	}

	if (key == GLFW_KEY_5 && action == GLFW_PRESS)
	{
		zoom = 1.0f;
		path = "images/image5-pattern.png";
		p = path.c_str();

	}

	if (key == GLFW_KEY_6 && action == GLFW_PRESS)
	{
		zoom = 1.0f;
		path = "images/image6.jpg";
		p = path.c_str();

	}

	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
	{
		zoom*=1.2;
		

	}

	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
	{
		zoom/=1.2;
		

	}

	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
	{
		shift+=0.1;
		

	}

	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
	{
		shift-=0.1;
		

	}

	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		theta+= GL_PI/3;
		

	}

	if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
		sob = 0;
		gauss = 0;
		luminance.r = 0.333f;
		luminance.g = 0.333f;
		luminance.b = 0.333f;

	}

	if (key == GLFW_KEY_S && action == GLFW_PRESS)
	{
		sob = 0;
		gauss = 0;
		luminance.r = 0.299f;
		luminance.g = 0.587f;
		luminance.b = 0.114f;

	}

	if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
		sob = 0;
		gauss = 0;
		luminance.r = 0.213f;
		luminance.g = 0.715f;
		luminance.b = 0.072f;

	}

	if (key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		sob = 0;
		sepia = 1;
		gauss = 0;
		luminance.r = 1.0f;
		luminance.g = 1.0f;
		luminance.b = 1.0f;

	}

	if (key == GLFW_KEY_0 && action == GLFW_PRESS)
	{
		sepia = 0;
		sob = 0;
		gauss = 0;
		luminance.r = 1.0f;
		luminance.g = 1.0f;
		luminance.b = 1.0f;

	}

	if (key == GLFW_KEY_H && action == GLFW_PRESS)
	{
		sob = 1;
		gauss = 0;

	}

	if (key == GLFW_KEY_J && action == GLFW_PRESS)
	{
		sob = 2;
		gauss = 0;

	}

	if (key == GLFW_KEY_K && action == GLFW_PRESS)
	{
		sob = 3;
		gauss = 0;

	}

	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		sob = 0;
		gauss = 3;

	}

	if (key == GLFW_KEY_Q && action == GLFW_PRESS)
	{
		sob = 0;
		gauss = 5;

	}

	if (key == GLFW_KEY_W && action == GLFW_PRESS)
	{
		sob = 0;
		gauss = 7;

	}
}


void mouse_callback(GLFWwindow* window, int button, int action, int mods)
{
	bool press = false;
	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		if(GLFW_PRESS == action)
			press = true;
		else if(GLFW_RELEASE == action)
			press = false;
	}

	if(press){
		glfwGetCursorPos(window, &mouse.image_offset.x, &mouse.image_offset.y);
	}

		
	

}
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if ( yoffset == -1)
	{
		zoom*=1.2;
	}
	else{
		zoom/=1.2;
	}
	
}

void generateRectangle(vector<vec2>* vertexPoints, vector<vec2>* texturePoints)
{
	vertexPoints->clear();
	texturePoints->clear();

	vertexPoints->push_back(vec2( -1.0f, -1.0f ));
	vertexPoints->push_back(vec2( 1.0f,  -1.0f ));
	vertexPoints->push_back(vec2( 1.0f, 1.0f ));
	vertexPoints->push_back(vec2( -1.0f, -1.0f ));
	vertexPoints->push_back(vec2( 1.0f, 1.0f ));
	vertexPoints->push_back(vec2( -1.0f, 1.0f ));

	texturePoints->push_back(vec2(0.0f,0.0f));
	texturePoints->push_back(vec2(float(imageTextures.width),0.0f));
	texturePoints->push_back(vec2(float(imageTextures.width),float(imageTextures.height)));
	texturePoints->push_back(vec2(0.0f,0.0f));
	texturePoints->push_back(vec2(float(imageTextures.width),float(imageTextures.height)));
	texturePoints->push_back(vec2(0.0f,float(imageTextures.height)));




}

void magnify(vector<vec2>* vertexPoints, vector<vec2>* texturePoints)
{

}

// ==========================================================================
// PROGRAM ENTRY POINT

int main(int argc, char *argv[])
{
	// initialize the GLFW windowing system
	if (!glfwInit()) {
		cout << "ERROR: GLFW failed to initialize, TERMINATING" << endl;
		return -1;
	}
	glfwSetErrorCallback(ErrorCallback);

	// attempt to create a window with an OpenGL 4.1 core profile context
	GLFWwindow *window = 0;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	int width = 512, height = 512;
	window = glfwCreateWindow(width, height, "CPSC 453 OpenGL Boilerplate", 0, 0);
	if (!window) {
		cout << "Program failed to create GLFW window, TERMINATING" << endl;
		glfwTerminate();
		return -1;
	}

	// set keyboard callback function and make our context current (active)
	
	glfwSetKeyCallback(window, KeyCallback);
	glfwSetMouseButtonCallback(window, mouse_callback);
	glfwSetScrollCallback(window, ScrollCallback);
	glfwMakeContextCurrent(window);

	//Intialize GLAD
	if (!gladLoadGL())
	{
		cout << "GLAD init failed" << endl;
		return -1;
	}

	// query and print out information about our OpenGL environment
	QueryGLVersion();

	// call function to load and compile shader programs
	GLuint program = InitializeShaders();
	if (program == 0) {
		cout << "Program could not initialize shaders, TERMINATING" << endl;
		return -1;
	}


	/* three vertex positions and assocated colours of a triangle
	vec2 vertices[] = {


	};
*/
/*
	vec2 textureCoords[] = {
		
	};
*/
	// call function to create and fill buffers with geometry data

	Geometry geometry;
	if (!InitializeVAO(&geometry))
		cout << "Program failed to intialize geometry!" << endl;

	if(!LoadGeometry(&geometry, vertexPoints.data(), texturePoints.data(), vertexPoints.size()))
		cout << "Failed to load geometry" << endl;


	// run an event-triggered main loop
	while (!glfwWindowShouldClose(window))
	{

		//initialize
		InitializeTexture(&imageTextures, p, GL_TEXTURE_RECTANGLE);
		//coordinates
		generateRectangle(&vertexPoints, &texturePoints);
		LoadGeometry(&geometry, vertexPoints.data(), texturePoints.data(), vertexPoints.size());
		glUseProgram(program);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_RECTANGLE, imageTextures.textureID);

		GLuint uniformLocation = glGetUniformLocation(program, "tex");
		glUniform1i(uniformLocation,0);

		GLint zoomLevel = glGetUniformLocation(program, "zoom");
		glUniform1f(zoomLevel, zoom);

		GLint shifting = glGetUniformLocation(program, "shift");
		glUniform2f(shifting, mouse.image_offset.x, mouse.image_offset.y);

		GLint thetaLo = glGetUniformLocation(program, "theta");
		glUniform1f(thetaLo, theta);

		GLint lum = glGetUniformLocation(program, "luminance");
		glUniform3f(lum, luminance.r, luminance.g, luminance.b);

		GLint sep = glGetUniformLocation(program, "sepia");
		glUniform1i(sep, sepia);

		GLint sobel = glGetUniformLocation(program, "sob");
		glUniform1i(sobel, sob);

		GLint gaussian = glGetUniformLocation(program, "gauss");
		glUniform1i(gaussian, gauss);
		// call function to draw our scene
		RenderScene(&geometry, program);

		glfwSwapBuffers(window);

		glfwPollEvents();
	}

	// clean up allocated resources before exit
	DestroyGeometry(&geometry);
	glUseProgram(0);
	glDeleteProgram(program);
	glfwDestroyWindow(window);
	glfwTerminate();

	cout << "Goodbye!" << endl;
	return 0;
}

// ==========================================================================
// SUPPORT FUNCTION DEFINITIONS

// --------------------------------------------------------------------------
// OpenGL utility functions

void QueryGLVersion()
{
	// query opengl version and renderer information
	string version = reinterpret_cast<const char *>(glGetString(GL_VERSION));
	string glslver = reinterpret_cast<const char *>(glGetString(GL_SHADING_LANGUAGE_VERSION));
	string renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));

	cout << "OpenGL [ " << version << " ] "
		<< "with GLSL [ " << glslver << " ] "
		<< "on renderer [ " << renderer << " ]" << endl;
}

bool CheckGLErrors()
{
	bool error = false;
	for (GLenum flag = glGetError(); flag != GL_NO_ERROR; flag = glGetError())
	{
		cout << "OpenGL ERROR:  ";
		switch (flag) {
		case GL_INVALID_ENUM:
			cout << "GL_INVALID_ENUM" << endl; break;
		case GL_INVALID_VALUE:
			cout << "GL_INVALID_VALUE" << endl; break;
		case GL_INVALID_OPERATION:
			cout << "GL_INVALID_OPERATION" << endl; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			cout << "GL_INVALID_FRAMEBUFFER_OPERATION" << endl; break;
		case GL_OUT_OF_MEMORY:
			cout << "GL_OUT_OF_MEMORY" << endl; break;
		default:
			cout << "[unknown error code]" << endl;
		}
		error = true;
	}
	return error;
}

// --------------------------------------------------------------------------
// OpenGL shader support functions

// reads a text file with the given name into a string
string LoadSource(const string &filename)
{
	string source;

	ifstream input(filename.c_str());
	if (input) {
		copy(istreambuf_iterator<char>(input),
			istreambuf_iterator<char>(),
			back_inserter(source));
		input.close();
	}
	else {
		cout << "ERROR: Could not load shader source from file "
			<< filename << endl;
	}

	return source;
}

// creates and returns a shader object compiled from the given source
GLuint CompileShader(GLenum shaderType, const string &source)
{
	// allocate shader object name
	GLuint shaderObject = glCreateShader(shaderType);

	// try compiling the source as a shader of the given type
	const GLchar *source_ptr = source.c_str();
	glShaderSource(shaderObject, 1, &source_ptr, 0);
	glCompileShader(shaderObject);

	// retrieve compile status
	GLint status;
	glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetShaderInfoLog(shaderObject, info.length(), &length, &info[0]);
		cout << "ERROR compiling shader:" << endl << endl;
		cout << source << endl;
		cout << info << endl;
	}

	return shaderObject;
}

// creates and returns a program object linked from vertex and fragment shaders
GLuint LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
	// allocate program object name
	GLuint programObject = glCreateProgram();

	// attach provided shader objects to this program
	if (vertexShader)   glAttachShader(programObject, vertexShader);
	if (fragmentShader) glAttachShader(programObject, fragmentShader);

	// try linking the program with given attachments
	glLinkProgram(programObject);

	// retrieve link status
	GLint status;
	glGetProgramiv(programObject, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint length;
		glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &length);
		string info(length, ' ');
		glGetProgramInfoLog(programObject, info.length(), &length, &info[0]);
		cout << "ERROR linking shader program:" << endl;
		cout << info << endl;
	}

	return programObject;
}
