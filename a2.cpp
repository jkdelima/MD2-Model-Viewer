/**
 * Kyle Delima
 * CPSC 453 Assignment 2
 * Most of the functions are based off of the example.cpp file in Erika Harrison's tutorial 8 package.
 */
// Header files for our program
#include <GL/glew.h>
#ifdef __APPLE__
#include <OpenGL/gl3.h>
#else
#include <GL/gl.h>
#endif
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // for transformation matrices
#include <glm/gtc/type_ptr.hpp> // for value_ptr
#include <glm/gtx/transform.hpp>
#include "md2.h"
#include <string.h>
#include <math.h>
// Constants to help with location bindings
#define VERTEX_DATA 0
#define VERTEX_NORMAL 1
// Define PI
#define PI 3.14159265
// Namespaces
using namespace std;
using namespace glm;
// VAO and VBO to handle vertex state and data
GLuint myVAO;
GLuint myVBO;
GLuint myIndexBuffer;
// Shader program to use
GLuint myShaderProgram;
// Global Variables
static GLubyte shaderText[8192];
float scaleValue = 1.0f, rotateValue = 0.0f, specPower = 128.0f;
vec3 specAlbedo = vec3(0.7), diffuseValue = vec3(0.5, 0.2, 0.7), ambientValue = vec3(0.1, 0.1, 0.1), intensityValue = vec3(1, 1, 1);

/* Load the shader from the specified file. Returns false if the shader could not be loaded. */
bool loadShaderFile(const char *filename, GLuint shader)
{
	GLint shaderLength = 0;
	FILE *fp;
	// Open the shader file
	fp = fopen(filename, "r");
	if(fp != NULL) {
		// See how long the file is
		while (fgetc(fp) != EOF)
			shaderLength++;
		// Go back to beginning of file
		rewind(fp);
		// Read the whole file in
		fread(shaderText, 1, shaderLength, fp);
		// Make sure it is null terminated and close the file
		shaderText[shaderLength] = '\0';
		fclose(fp);
	} else {
		return false;
	}
	// Load the string into the shader object
	GLchar* fsStringPtr[1];
	fsStringPtr[0] = (GLchar *)((const char*)shaderText);
	glShaderSource(shader, 1, (const GLchar **)fsStringPtr, NULL );
	return true;
}
/* This function simply prompts the user for a model they want to load. */
string getModel()
{
	string modelName;
	cout << "Viewable models are: astro, faerie, invader, sdswpall, terminator, and zumlin" << endl;
	cout << "Enter the model you would like to view: " << endl;
	cin >> modelName;
	return "Models/" + modelName + "/tris.md2";
}
/* This changes specular albedo. */
void getSpecAlbedo()
{
	float x, y, z;
	cin >> x; cin >> y; cin >> z;
	specAlbedo.x = x;
	specAlbedo.y = y;
	specAlbedo.z = z;
}
/* This changes specular power. */
void getSpecPower()
{
	float x;
	cin >> x;
	specPower = x;
}
/* This changes diffuse albedo. */
void getDiffuse()
{
	float x, y, z;
	cin >> x; cin >> y; cin >> z;
	diffuseValue.x = x;
	diffuseValue.y = y;
	diffuseValue.z = z;
}
/* This changes ambience. */
void getAmbience()
{
	float x, y, z;
	cin >> x; cin >> y; cin >> z;
	ambientValue.x = x;
	ambientValue.y = y;
	ambientValue.z = z;
}
/* This changes light intensity. */
void getIntensity()
{
	float x;
	cin >> x;
	intensityValue.x = x;
	intensityValue.y = x;
	intensityValue.z = x;
}
/* This function just displays the menu for changing lighting properties. */
int menu()
{
	cout << "Enter 1 to change specular albedo..." << endl;
	cout << "Enter 2 to change specular power..." << endl;
	cout << "Enter 3 to change diffuse albedo..." << endl;
	cout << "Enter 4 to change ambience..." << endl;
	cout << "Enter 5 to change light intensity..." << endl;
	int choice;	
	cin >> choice;
	return choice;
}
/* This processes any menu changes. */
void processMenu(int choice)
{
	if (choice == 1){
		cout << "Enter specular albedo (3 floats. Enter them one by one.):" << endl;
		getSpecAlbedo();
	} else if (choice == 2) {
		cout << "Enter specular power (1 float.):" << endl;
		getSpecPower();
	} else if (choice == 3) {
		cout << "Enter diffuse albedo (3 floats. Enter them one by one.):" << endl;
		getDiffuse();
	} else if (choice == 4) {
		cout << "Enter ambience values (3 floats. Enter them one by one.):" << endl;
		getAmbience();
	} else if (choice == 5) {
		cout << "Enter light intensity value (1 float.):" << endl;
		getIntensity();
	} else {
		cout << "Wrong input!" << endl;
		menu();
	}
}
/* This function does any needed initialization on the rendering context. */
void init(string modelName)
{
	MD2* model = new MD2();
	model->LoadModel(modelName.c_str());
	float vertices[model->num_xyz * 4];
	float normals[model->num_xyz * 3];
	short faces[model->num_tris * 3];
	// Create vertex data for buffer
	for (int i = 0; i < model->num_xyz; i++) { // For each vertex
		vertices[4*i] = (model->m_vertices[i][0]);
		vertices[4*i+1] = (model->m_vertices[i][1]);
		vertices[4*i+2] = (model->m_vertices[i][2]);
		vertices[4*i+3] = 1.0f;
	}
	// Create normals data for buffer
	for (int i = 0; i < model->num_xyz; i++) { // For each vertex
		short vertFaces[30];
		int faceNumber = 0;
		for (int j = 0; j < model->num_tris; j++) { // For each face
			if (model->tris[j].index_xyz[0] == i || model->tris[j].index_xyz[1] == i || model->tris[j].index_xyz[2] == i) {
				vertFaces[faceNumber] = j;
				faceNumber++;
			}
		}
		vec3 vertNormals = vec3(0.0f, 0.0f, 0.0f);
		vec3 faceNormals = vec3(0.0f, 0.0f, 0.0f);
		for (int k = 0; k < faceNumber; k++) { // For each face in array
			float vert1x = model->m_vertices[(model->tris[vertFaces[k]].index_xyz[0])][0];
			float vert1y = model->m_vertices[(model->tris[vertFaces[k]].index_xyz[0])][1];
			float vert1z = model->m_vertices[(model->tris[vertFaces[k]].index_xyz[0])][2];
			float vert2x = model->m_vertices[(model->tris[vertFaces[k]].index_xyz[1])][0];
			float vert2y = model->m_vertices[(model->tris[vertFaces[k]].index_xyz[1])][1];
			float vert2z = model->m_vertices[(model->tris[vertFaces[k]].index_xyz[1])][2];
			float vert3x = model->m_vertices[(model->tris[vertFaces[k]].index_xyz[2])][0];
			float vert3y = model->m_vertices[(model->tris[vertFaces[k]].index_xyz[2])][1];
			float vert3z = model->m_vertices[(model->tris[vertFaces[k]].index_xyz[2])][2];
			vec3 a = vec3(vert2x - vert1x, vert2y - vert1y, vert2z - vert1z);
			vec3 b = vec3(vert3x - vert1x, vert3y - vert1y, vert3z - vert1z);
			faceNormals = cross(b, a);			
			vertNormals = vertNormals + faceNormals;
		}
		faceNumber++;
		vertNormals = vec3(vertNormals.x/faceNumber, vertNormals.y/faceNumber, vertNormals.z/faceNumber);
		normals[3*i] = vertNormals.x;
		normals[3*i+1] = vertNormals.y;
		normals[3*i+2] = vertNormals.z;
	}
	// Create face data for buffer
	for (int i = 0; i < model->num_tris; i++) {
		faces[3*i] = model->tris[i].index_xyz[0];
		faces[3*i+1] = model->tris[i].index_xyz[1];
		faces[3*i+2] = model->tris[i].index_xyz[2];
	}
	// Initialize background
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
	glEnable(GL_DEPTH_TEST);
	// Setup the shaders
	GLuint hVertexShader = glCreateShader(GL_VERTEX_SHADER);
	GLuint hFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	myShaderProgram = (GLuint)NULL;
	GLint testVal;
	if( !loadShaderFile("per-fragment-phong.vs.glsl", hVertexShader) ) {
		glDeleteShader( hVertexShader );
		glDeleteShader( hFragmentShader );
		cout << "The shader " << "per-fragment-phong.vs.glsl" << " could not be found." << endl;
	}
	if( !loadShaderFile("per-fragment-phong.fs.glsl", hFragmentShader) ) {
		glDeleteShader( hVertexShader );
		glDeleteShader( hFragmentShader );
		cout << "The shader " << "per-fragment-phong.fs.glsl" << " could not be found." << endl;
	}
	glCompileShader(hVertexShader);
	glCompileShader(hFragmentShader);
	// Check for any error generated during shader compilation
	glGetShaderiv(hVertexShader, GL_COMPILE_STATUS, &testVal);
	if( testVal == GL_FALSE ) {
		char source[8192];
		char infoLog[8192];
		glGetShaderSource( hVertexShader, 8192, NULL, source );
		glGetShaderInfoLog( hVertexShader, 8192, NULL, infoLog);
		cout << "The shader: " << endl << (const char*)source << endl << " failed to compile:" << endl;
		fprintf( stderr, "%s\n", infoLog);
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
	}
	glGetShaderiv(hFragmentShader, GL_COMPILE_STATUS, &testVal);
	if( testVal == GL_FALSE ) {
		char source[8192];
		char infoLog[8192];
		glGetShaderSource( hFragmentShader, 8192, NULL, source);
		glGetShaderInfoLog( hFragmentShader, 8192, NULL, infoLog);
		cout << "The shader: " << endl << (const char*)source << endl << " failed to compile:" << endl;
		fprintf( stderr, "%s\n", infoLog);    
		glDeleteShader(hVertexShader);
		glDeleteShader(hFragmentShader);
	}
	// Create the shader program and bind locations for the vertex attributes before linking, the linking process can also generate errors
	myShaderProgram = glCreateProgram();
	glAttachShader(myShaderProgram, hVertexShader);
	glAttachShader(myShaderProgram, hFragmentShader);
	glBindAttribLocation( myShaderProgram, VERTEX_DATA, "position" );
	glBindAttribLocation( myShaderProgram, VERTEX_NORMAL, "normal" );
	glLinkProgram( myShaderProgram );
	glDeleteShader( hVertexShader );
	glDeleteShader( hFragmentShader );
	glGetProgramiv( myShaderProgram, GL_LINK_STATUS, &testVal );
	// At some point we failed - eg. bad shader! need to quit out
	if( testVal == GL_FALSE ) {
		char infoLog[1024];
		glGetProgramInfoLog( myShaderProgram, 1024, NULL, infoLog);
		cout << "The shader program" << "(per-fragment-phong.vs.glsl + per-fragment-phong.fs.glsl) failed to link:" << endl << (const char*)infoLog << endl;
		glDeleteProgram(myShaderProgram);
		myShaderProgram = (GLuint)NULL;
	}
	// Create buffers
	glGenVertexArrays(1, &myVAO);
	glBindVertexArray(myVAO);
	glGenBuffers(1, &myVBO);
	glBindBuffer( GL_ARRAY_BUFFER, myVBO );
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(normals), &normals[0]);
	// Load face indices into the index buffer
	glGenBuffers(1, &myIndexBuffer );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, myIndexBuffer );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(faces), &faces[0], GL_STATIC_DRAW );
	// Now we'll use the attribute locations to map our vertex data (in the VBO) to the shader
	glEnableVertexAttribArray( VERTEX_DATA );
	glVertexAttribPointer( VERTEX_DATA, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 );
	glEnableVertexAttribArray( VERTEX_NORMAL );
	glVertexAttribPointer( VERTEX_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, (const GLvoid*)(sizeof(vertices)));
}
/* Gets proper viewing matrices and displays the scene. */
void renderScene(string modelName)
{
	// Get bounding box
	MD2* model = new MD2();
	model->LoadModel(modelName.c_str());
	float xmax, ymax, zmax, xmin, ymin, zmin;
	vec3_t * vertices = model->m_vertices;
	xmax = vertices[0][0]; ymax = vertices[0][1]; zmax = vertices[0][2];
	xmin = vertices[0][0]; ymin = vertices[0][1]; zmin = vertices[0][2];
	for (int i = 1; i < model->num_xyz; i++) {
		if (vertices[i][0] > xmax) xmax = vertices[i][0];
		if (vertices[i][1] > ymax) ymax = vertices[i][1];
		if (vertices[i][2] > zmax) zmax = vertices[i][2];
		if (vertices[i][0] < xmin) xmin = vertices[i][0];
		if (vertices[i][1] < ymin) ymin = vertices[i][1];
		if (vertices[i][2] < zmin) zmin = vertices[i][2];
	}
	vec3 max = vec3(xmax, ymax, zmax);
	vec3 min = vec3(xmin, ymin, zmin);
	vec3 camera = vec3(2*((length(max-min)/2)/tan(22.5*PI/180)),0,0); // Computation for camera placement
	vec3 center = vec3((xmax+xmin)/2, (ymax+ymin)/2, (zmax+zmin)/2); // Computation for translation
	// Compute matrices for proper rendering	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram( myShaderProgram );
	mat4 proj_matrix = ::perspective(45.0f, 1.0f, 1.0f, 500.0f);
	mat4 view_matrix = lookAt(
		camera,
		vec3(0,0,0),
		vec3(0,1,0)
	);
	mat4 scaled = scale(scaleValue, scaleValue, scaleValue);
	mat4 stand = rotate(270.0f, vec3(1.0f, 0.0f, 0.0f));
	mat4 rotated = rotate(rotateValue, vec3(0.0f, 0.0f, 1.0f));
	mat4 translated = translate(-center);
	mat4 mv_matrix = view_matrix * scaled * stand * rotated * translated;
	glUniform3f (glGetUniformLocation(myShaderProgram, "specular_albedo"), specAlbedo.x, specAlbedo.y, specAlbedo.z);
	glUniform1f (glGetUniformLocation(myShaderProgram, "specular_power"), specPower);
	glUniform3f (glGetUniformLocation(myShaderProgram, "diffuse_albedo"), diffuseValue.x, diffuseValue.y, diffuseValue.z);
	glUniform3f (glGetUniformLocation(myShaderProgram, "ambient"), ambientValue.x, ambientValue.y, ambientValue.z);
	glUniform3f (glGetUniformLocation(myShaderProgram, "light_intensity"), intensityValue.x, intensityValue.y, intensityValue.z);
	glUniformMatrix4fv (glGetUniformLocation(myShaderProgram, "mv_matrix"), 1, GL_FALSE, value_ptr(mv_matrix));
	glUniformMatrix4fv (glGetUniformLocation(myShaderProgram, "proj_matrix"), 1, GL_FALSE, value_ptr(proj_matrix));
	glDrawElements( GL_TRIANGLES, model->num_tris * 3, GL_UNSIGNED_SHORT, 0);
}
/* Keyboard callback function, also applies the inputs for transforming our model. */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	if (key == GLFW_KEY_UP && action == GLFW_PRESS)
		scaleValue += 0.05f;
	if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
		scaleValue -= 0.05f;
	if (key == GLFW_KEY_UP && action == GLFW_REPEAT)
		scaleValue += 0.05f;
	if (key == GLFW_KEY_DOWN && action == GLFW_REPEAT)
		scaleValue -= 0.05f;
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
		rotateValue -= 2.0f;
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
		rotateValue += 2.0f;
	if (key == GLFW_KEY_LEFT && action == GLFW_REPEAT)
		rotateValue -= 2.0f;
	if (key == GLFW_KEY_RIGHT && action == GLFW_REPEAT)
		rotateValue += 2.0f;
}
/**
* Mouse button callback function
*/
static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		processMenu(menu());
}
/* Window resize callback function. */
void resize_callback(GLFWwindow* window, int w, int h)
{ 
	// Set Viewport to window dimensions
	glViewport(0, 0, w, h);
}
/* Error callback function. */
static void error_callback(int error, const char* description)
{
	fputs(description, stderr);
}
/* Main function for our program. */
int main(int argc, char **argv)
{
	// Setup error callback and initialize glfw
	GLFWwindow* window;
	glfwSetErrorCallback(error_callback);
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	// Setup window and window context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// Anti-aliasing - This will have smooth polygon edges
	glfwWindowHint(GLFW_SAMPLES, 4); 
	window = glfwCreateWindow(800, 800, "Assignment 2", NULL, NULL);
	if (!window) {
		if( myShaderProgram ) {
			glDeleteProgram( myShaderProgram );
			glDeleteVertexArrays(1, &myVAO);
		}
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);
	glfwSetWindowSizeCallback(window, resize_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSwapInterval(1);
	// Inform OpenGL we're working with the new version
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if(err!=GLEW_OK) {
		//Problem: glewInit failed, something is seriously wrong.
		printf("glewInit failed, aborting.\n");
		exit(EXIT_FAILURE);
	}
	string modelName = getModel();
	// Initialize rendering context
	init(modelName);
	// Main drawing loop
	int width, height;
	if( myShaderProgram ) {
		while (!glfwWindowShouldClose(window)) {
			glfwGetFramebufferSize(window, &width, &height);
			glViewport(0, 0, width, height);
			renderScene(modelName);
			glfwSwapBuffers(window);
			glfwPollEvents();
		}
	}
	// Quits the program
	glfwDestroyWindow(window);
	glfwTerminate();
	exit(EXIT_SUCCESS);
}

