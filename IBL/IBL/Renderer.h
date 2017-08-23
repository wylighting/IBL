#pragma once
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <camera.h>
#include <shader.h>
#include <model.h>

//#include "ToolFunctions.h"

class Renderer
{
public:
	Renderer(GLuint scr_width, GLuint scr_height, Camera &initCamera);
	~Renderer();
	void InitGLFW();
	void Render(Shader &pbrShader, Shader &backgroundShader, unsigned int envCubemap, unsigned int);
	void addModelFromFile(std::string path);

private:
	void GenerateTransferVectors();



	GLuint SCR_WIDTH, SCR_HEIGHT;
	GLFWwindow* window;
	//Shader	pbrShader;
	//Shader backgroundShader;

	//unsigned int envCubemap;

	const int nrRows = 1;
	const int nrColumns = 1;
	const float spacing = 2.5;
	Model *objModel;

	// callback functions
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	static void processInput(GLFWwindow *window);
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

	// camera
	static Camera camera;
	static float lastX;
	static float lastY;
	static bool firstMouse;

	// timing
	static float deltaTime;
	static float lastFrame;

	//control
	static bool convert;

};
