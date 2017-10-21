#pragma once
// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <camera.h>
#include <shader.h>
#include <model.h>

#include "Transfer.h"

class Renderer
{
public:
	Renderer(Camera &initCamera, const Sampler &sampler, const EnvLight &envMap);
	~Renderer();
	static void InitGLFW(GLuint scr_width, GLuint scr_height);
	void Render(Shader &pbrShader, Shader &backgroundShader, unsigned int);
	void addModelFromFile(std::string path);

private:
	inline void CalAndSetupVertexColor(float theta = 0, float phi = 0) const;
	void PrecomputeTransferCoeffs();
	void PrecomputeLightCoeffs() const;

	static vector<glm::vec3> L_lm_rotated;

	Transfer* transferCalculator;

	mutable LightType lightType = LIGHT_UNSHADOWED; // default unshadowed

	const Sampler &sampler;
	const EnvLight &envMap;

	//GLuint SCR_WIDTH, SCR_HEIGHT;
	static GLFWwindow* window;

	const int nrRows = 1;
	const int nrColumns = 1;
	const float spacing = 2.5;
	Model *objModel;

	static vector<glm::vec3> vertex_color;

	// callback functions
	static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
	static void mouse_callback(GLFWwindow* window, double xpos, double ypos);
	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
	void processInput(GLFWwindow *window) const;
	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

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
	static bool isShadow;
	static bool locked;

	// light rotation control 
	static float alpha;
	static float beta;
};
