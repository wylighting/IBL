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

GLFWwindow* Renderer::window = nullptr;
void Renderer::InitGLFW(GLuint scr_width, GLuint scr_height)
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
	window = glfwCreateWindow(scr_width, scr_height, "LearnOpenGL", NULL, NULL);
	glfwMakeContextCurrent(window);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, key_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// init glew
	glewExperimental = GL_TRUE;
	glewInit();

}


Renderer::Renderer(Camera &initCamera, const Sampler &sampler, const EnvLight &envMap) : sampler(sampler), envMap(envMap)
{
	//InitGLFW();

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL); // set depth function to less than AND equal for skybox depth trick.

	//pbrShader = Shader(nullptr, nullptr);
	//backgroundShader = Shader(nullptr, nullptr);
	camera = initCamera;
	//transferCalculator = Transfer(objModel, sampler);
}

Renderer::~Renderer()
{
	delete transferCalculator;
	glfwTerminate();
}

vector<glm::vec3> Renderer::vertex_color;

void Renderer::PrecomputeTransferCoeffs()
{
	transferCalculator = new Transfer(objModel, sampler);
	if(!transferCalculator->GenerateUnShadowedCoeffs())
	{
		cerr << "Generate UnShadower or Shadowed Coeffs failed!" << endl;
	}

	for (unsigned i = 1; i < 3; ++i)
	if (!transferCalculator->GenerateInterreflectionShadowedCoeffs(i))
	{
		cerr << "Generate Interreflecion shadow Coeffs " << i << " failed!" << endl;
	}
}

void Renderer::PrecomputeLightCoeffs() const
{
	if (!envMap.CalcLightCoeffs(sampler))
		cerr << "Generate Light Coeffs failed!" << endl;
}


vector<glm::vec3> Renderer::L_lm_rotated;
void Renderer::CalAndSetupVertexColor(float theta, float phi) const // MOVE INTO SHADER
{
	vertex_color.clear();
	L_lm_rotated.clear();

	// Get precomputed coefficients
	auto &transferCoeffs = transferCalculator->GetTransferVector(lightType);
	vector<glm::vec3> &L_lm = envMap.GetLightCoeffs();
	//L_lm_rotated = L_lm;
	SphericalH::SHRotation::RotateSHCoefficientsVector(L_lm, L_lm_rotated, theta, phi);
	// Rotate light coeffs through 
	//L_lm[0] = glm::vec3(0.79, 0.44, 0.54);
	//L_lm[1] = glm::vec3(0.39, 0.35, 0.60);
	//L_lm[2] = glm::vec3(-0.34, -0.18, -0.27);
	//L_lm[3] = glm::vec3(-0.29, -0.06, 0.01);
	//L_lm[4] = glm::vec3(-0.11, -0.05, -0.12);
	//L_lm[5] = glm::vec3(-0.26, -0.22, -0.47);
	//L_lm[6] = glm::vec3(-0.16, -0.09, -0.15);
	//L_lm[7] = glm::vec3(0.56, 0.21, 0.14);
	//L_lm[8] = glm::vec3(0.21, -0.05, -0.3);

	// Per vertex Sum
	unsigned vSize = transferCoeffs.size();
	for (unsigned i = 0; i < vSize; ++i)
	{
		vector<float> vTransferCoeffs = transferCoeffs[i];
		// Calculate sum of lightCoeff & transferCoeff in current vertex
		glm::vec3 color;
		for (unsigned j = 0; j < L_lm_rotated.size(); ++j)
		{
			color += vTransferCoeffs[j] * L_lm_rotated[j];
		}
		//color = glm::normalize(objModel->GetCurrentVertexNormal(i));
		vertex_color.push_back(color);
	}
	objModel->SetVertexColor(vertex_color);
}

void Renderer::Render(Shader &pbrShader, Shader &backgroundShader, unsigned int irradianceMap)
{
	PrecomputeTransferCoeffs();
	PrecomputeLightCoeffs();
	// then before rendering, configure the viewport to the original framebuffer's screen dimensions
	int scrWidth, scrHeight;
	glfwGetFramebufferSize(window, &scrWidth, &scrHeight);
	glViewport(0, 0, scrWidth, scrHeight);
	CalAndSetupVertexColor();

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

		//Remember that the actual transformation order should be read in reverse: 
		//even though in code we first translate and then later rotate, the actual transformations first apply a rotation and then a translation. 
		model = glm::mat4();
		model = glm::translate(model, glm::vec3(
			0.0f, 0.0f, -3.0f
		));
		float theta = static_cast<float>(glfwGetTime());
		model = glm::rotate(model, theta, glm::vec3(0.0, 1.0, 0.0));
		model = glm::scale(model, glm::vec3(0.5f));

		pbrShader.setMat4("model", model);
		//ModelBox::RenderSphere();
		processInput(window);

		//CalAndSetupVertexColor(0, theta / (2 * MY_PI) * 180.0f);
		//CalAndSetupVertexColor(beta, alpha); // manual light rotation control for debugging

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
		glBindTexture(GL_TEXTURE_CUBE_MAP, envMap.GetCubeMap());
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
bool Renderer::isShadow = false;
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
void Renderer::processInput(GLFWwindow *window) const
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

	// NOT USE
	if (keys[GLFW_KEY_E] && !keysPressed[GLFW_KEY_E])
	{
		isShadow = !isShadow;
		keysPressed[GLFW_KEY_E] = true;
		CalAndSetupVertexColor();
	}
	
	if (keys[GLFW_KEY_0] && !keysPressed[GLFW_KEY_0])
	{
		//isShadow = !isShadow;
		lightType = LIGHT_UNSHADOWED;
		keysPressed[GLFW_KEY_0] = true;
		CalAndSetupVertexColor();
	}

	// Select Shadow interreflection
	if (keys[GLFW_KEY_1] && !keysPressed[GLFW_KEY_1])
	{
		//isShadow = !isShadow;
		lightType = LIGHT_SHADOWED;
		keysPressed[GLFW_KEY_1] = true;
		CalAndSetupVertexColor();
	}

	if (keys[GLFW_KEY_2] && !keysPressed[GLFW_KEY_2])
	{
		//isShadow = !isShadow;
		lightType = LIGHT_SHADOWED_BOUNCE_1;
		keysPressed[GLFW_KEY_2] = true;
		CalAndSetupVertexColor();
	}

	if (keys[GLFW_KEY_3] && !keysPressed[GLFW_KEY_3])
	{
		//isShadow = !isShadow;
		lightType = LIGHT_SHADOWED_BOUNCE_2;
		keysPressed[GLFW_KEY_3] = true;
		CalAndSetupVertexColor();
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


float Renderer::alpha = 0.f;
float Renderer::beta = 0.f;
bool Renderer::locked = false;
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

	if (locked)
	{
		alpha += xoffset / 10.f;
		beta += yoffset / 10.f;
		cout << "alpha: " << alpha << endl;
		cout << "beta:  " << beta << endl;
	}
	else
		camera.ProcessMouseMovement(xoffset, yoffset);

	lastX = xpos;
	lastY = ypos;


}

void Renderer::mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button != GLFW_MOUSE_BUTTON_LEFT)
		return;

	if (action == GLFW_PRESS) {
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		locked = true;
	}
	else {
		locked = false;
		//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void Renderer::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

#pragma endregion 