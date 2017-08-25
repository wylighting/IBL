#include "ToolFunctions.h"
#include "Renderer.h"
#include <string>

// mouse control
//====================================
float Renderer::lastX = 800.0f / 2;
float Renderer::lastY = 400.0f / 2;
bool Renderer::firstMouse = true;
Camera Renderer::camera = Camera();

// timming
//========================================
float Renderer::deltaTime = 0.0f;
float Renderer::lastFrame = 0.0f;


Renderer::Renderer(GLuint scr_width, GLuint scr_height, Camera &initCamera) : SCR_WIDTH(scr_width), SCR_HEIGHT(scr_height)
{
	InitGLFW();
	// init glew
	glewExperimental = GL_TRUE;
	glewInit();

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.

	//pbrShader = Shader(nullptr, nullptr);
	//backgroundShader = Shader(nullptr, nullptr);
	camera = initCamera;

	//transferCalculator = Transfer(nullptr);
}

Renderer::~Renderer()
{
	glfwTerminate();
}

void Renderer::InitGLFW()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

	// glfw window creation
	// --------------------
	window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

}

vector<glm::vec3> Renderer::vertex_color;

void Renderer::CalculateVertexColor(const Sampler &sampler, const EnvLight &envMap)
{
	
	transferCalculator = Transfer(objModel, sampler);
	vector<vector<float>> &transferCoeffs = transferCalculator.GenerateUnShadowedCoeffs(); // VERY slow???
	//
	vector<glm::vec3> &L_lm = envMap.CalcLightCoeffs(sampler);

	//L_lm[0] = glm::vec3(0.79, 0.44, 0.54);
	//L_lm[1] = glm::vec3(0.39, 0.35, 0.60);
	//L_lm[2] = glm::vec3(-0.34, -0.18, -0.27);
	//L_lm[3] = glm::vec3(-0.29, -0.06, 0.01);
	//L_lm[4] = glm::vec3(-0.11, -0.05, -0.12);
	//L_lm[5] = glm::vec3(-0.26, -0.22, -0.47);
	//L_lm[6] = glm::vec3(-0.16, -0.09, -0.15);
	//L_lm[7] = glm::vec3(0.56, 0.21, 0.14);
	//L_lm[8] = glm::vec3(0.21, -0.05, -0.3);

	// Per vertex
	unsigned vSize = transferCoeffs.size();
	for (unsigned i = 0; i < vSize; ++i)
	{
		vector<float> vTransferCoeffs = transferCoeffs[i];
		// Calculate sum of lightCoeff & transferCoeff in current vertex
		glm::vec3 color;
		for (unsigned j = 0; j < L_lm.size(); ++j)
		{
			color += vTransferCoeffs[j] * L_lm[j];
		}
		//color = glm::normalize(objModel->GetCurrentVertexNormal(i));
		vertex_color.push_back(color);
	}
}

void Renderer::Render(Shader &pbrShader, Shader &backgroundShader, unsigned int envCubemap, unsigned int irradianceMap, const Sampler &sampler) const
{
	objModel->SetVertexColor(vertex_color);
	// then before rendering, configure the viewport to the original framebuffer's screen dimensions
	int scrWidth, scrHeight;
	glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
	glViewport(0, 0, scrWidth, scrHeight);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);
		pbrShader.use();
		pbrShader.setBool("convert", convert);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// render scene, supplying the convoluted irradiance map to the final shader.
		// ------------------------------------------------------------------------------------------
		pbrShader.use();
		glm::mat4 view = camera.GetViewMatrix();
		pbrShader.setMat4("view", view);
		pbrShader.setVec3("camPos", camera.Position);

		// bind pre-computed IBL data
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);


		// render rows*column number of spheres with material properties defined by textures (they all have the same material properties)
		glm::mat4 model;
		pbrShader.setFloat("metallic", 0.2f);
		pbrShader.setFloat("roughness", 0.5f);

		model = glm::mat4();
		model = glm::scale(model, glm::vec3(0.5f));
		model = glm::translate(model, glm::vec3(
			0.0f, 0.0f, -2.0f
		));
		pbrShader.setMat4("model", model);
		//ModelBox::RenderSphere();
		objModel->Draw(pbrShader);



		// render light source (simply re-render sphere at light positions)
		// this looks a bit off as we use the same shader, but it'll make their positions obvious and 
		// keeps the codeprint small.
		//for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
		//{
		//	glm::vec3 newPos = lightPositions[i] + glm::vec3(sin(glfwGetTime() * 5.0) * 5.0, 0.0, 0.0);
		//	newPos = lightPositions[i];
		//	pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
		//	pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

		//	model = glm::mat4();
		//	model = glm::translate(model, newPos);
		//	model = glm::scale(model, glm::vec3(0.5f));
		//	pbrShader.setMat4("model", model);
		//	ModelBox::RenderSphere();
		//}

		// render skybox (render as last to prevent overdraw)
		backgroundShader.use();
		backgroundShader.setMat4("view", view);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
		//glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap); // display irradiance map
		ModelBox::RenderCube();

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

}

void Renderer::addModelFromFile(std::string path)
{
	objModel = new Model(const_cast<GLchar*>(path.c_str()));
}


#pragma region "user input"
// CALLBACK FUNCTIONS FOR GLFW
bool keys[1024];
bool keysPressed[1024];
bool Renderer::convert = false;
// Is called whenever a key is pressed/released via GLFW
void Renderer::key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key >= 0 && key <= 1024)
	{
		if (action == GLFW_PRESS)
			keys[key] = true;
		else if (action == GLFW_RELEASE)
		{
			keys[key] = false;
			keysPressed[key] = false;
		}
	}

}
// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Renderer::processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = 2.5 * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (keys[GLFW_KEY_SPACE] && !keysPressed[GLFW_KEY_SPACE])
	{
		convert = ! convert;
		keysPressed[GLFW_KEY_SPACE] = true;
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void Renderer::framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void Renderer::mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void Renderer::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

#pragma endregion 