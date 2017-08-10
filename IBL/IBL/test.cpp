// Std. Includes
#include <string>
#include <sstream>
#include <iostream>
#include <memory>

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>
#pragma comment(lib, "glew32.lib")

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include <shader.h>
#include <camera.h>
#include <model.h>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libs
//#include <SOIL.h>

#include "text_render.h"
#include "light_streak.h"

#define STAR_FILTER

// Properties
const GLuint SCR_WIDTH = 1280, SCR_HEIGHT = 720;

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void Do_Movement();
GLuint loadTexture(GLchar const * path);
void RenderQuad();
void renderSphere();


// camera
//Camera camera(glm::vec3(-6.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 0);
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
//Camera camera(glm::vec3(3.24144, 0.742261, -2.31242), glm::vec3(0.0f, 1.0f, 0.0f), -155.75, -13.5);//WAVE PLANE
//Camera camera(glm::vec3(1.3069, 0.685992, 1.49709), glm::vec3(0.0f, 1.0f, 0.0f), -122, 18);//WAVE PLANE

// timing
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

//float a(1);

GLboolean normalMapping = true;
GLuint chooseStarFilter = 0;

GLfloat roughness = 0.01;

//here to change model
GLuint modelIndex = 1;

// The MAIN function, from here we start our application and run our Game loop
int main()
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);


	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr); // Windowed
	glfwMakeContextCurrent(window);

	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowPos(window, 200, 100);

	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);
	//options for text rendering
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Setup and compile our shaders
	Shader shader("pbr.vs", "pbr.frag");
	Shader shaderFilter("pbr.vs", "pbrModelNDFFiltering.frag");
	Shader lamp("lamp.vs", "lamp.frag");
	Shader text("text.vs", "text.frag");
	Shader quad("quad.vs", "quad.frag");
	Shader starFilter("starFilter.vs", "starFilter.frag");
	Shader textHighlight("quad.vs", "textHighlight.frag");
	Shader kawaseLightStreak("quad.vs", "lightStreakFilter.frag");
	//Shader saveMaps("starFilter.vs", "saveMapFromPingPong.frag");// no use

	//shared_ptr<Model> choosedModel;

	//Model rose("../../Resources/objects/rock/rock.obj");
	//Model ogre("../../Resources/objects/ogre/bs_smile.obj");
	//GLuint normal = loadTexture("../../Resources/objects/ogre/normalmap.png");
	//GLuint albedo = loadTexture("../../Resources/objects/ogre/diffuse.png");


#pragma region loadModel
	Model* choosedModel = NULL;

	GLuint normal;

	if (modelIndex == 0)
	{
		//Model cyborg("../../Resources/objects/cyborg/cyborg.obj");
		choosedModel = new Model("../../Resources/objects/cyborg/cyborg.obj");
		//choosedModel = make_shared<Model>((GLchar*)"../../Resources/objects/cyborg/cyborg.obj");
	}
	else if (modelIndex == 1)
	{
		choosedModel = new Model("../../../Resources/objects/ogre/bs_smile.obj");
		normal = loadTexture("../../../Resources/objects/ogre/normalmap.png");
	}
	else if (modelIndex == 2)
	{
		choosedModel = new Model("../../Resources/objects/WavePlane/wave_plane.obj");
	}
	else if (modelIndex == 3)
	{
		choosedModel = new Model("../../Resources/objects/MODELS/aruan.obj");
	}
	else if (modelIndex == 4)
	{
		choosedModel = new Model("../../Resources/objects/MODELS/sphere.obj");
	}

#pragma endregion

#pragma region shaderUniformDefinition

	// set (constant) material properties
	shader.Use();
	//glUniform1i(glGetUniformLocation(shader.program, "texture_diffuse1"), 0);
	glUniform1i(glGetUniformLocation(shader.program, "texture_normal1"), 1);
	glm::mat4 projection = glm::perspective(camera.Zoom, ((float)SCR_WIDTH / 2) / (float)SCR_HEIGHT, 0.1f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(shader.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	shaderFilter.Use();
	//glUniform1i(glGetUniformLocation(shaderFilter.program, "texture_diffuse1"), 0);
	glUniform1i(glGetUniformLocation(shaderFilter.program, "texture_normal1"), 1);
	glUniformMatrix4fv(glGetUniformLocation(shaderFilter.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	lamp.Use();
	glUniformMatrix4fv(glGetUniformLocation(lamp.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	text.Use();
	glm::mat4 textProjection = glm::ortho(0.0f, static_cast<GLfloat>(SCR_WIDTH), 0.0f, static_cast<GLfloat>(SCR_HEIGHT));
	glUniformMatrix4fv(glGetUniformLocation(text.program, "projection"), 1, GL_FALSE, glm::value_ptr(textProjection));

	starFilter.Use();
	glUniform1i(glGetUniformLocation(starFilter.program, "colorBuffer"), 0);

	quad.Use();
	glUniform1i(glGetUniformLocation(quad.program, "colorBuffer"), 0);
	glUniform1i(glGetUniformLocation(quad.program, "colorBlurX"), 1);
	glUniform1i(glGetUniformLocation(quad.program, "colorBlurY"), 2);

#pragma endregion


#pragma region Buffers
	GLuint framebuffer;
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// create  & attach color buffer
	GLuint colorbuffer;
	glGenTextures(1, &colorbuffer);
	glBindTexture(GL_TEXTURE_2D, colorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorbuffer, 0);
	// create & attach depth-stencil buffer
	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//create framebuffer with colorbuffer (extract specular highlight)
	//don't do post process for frame with text..
	GLuint HighlightFBO;
	glGenFramebuffers(1, &HighlightFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, HighlightFBO);
	//create two color buffers
	GLuint highlightMap;
	glGenTextures(1, &highlightMap);
	glBindTexture(GL_TEXTURE_2D, highlightMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // We clamp to the edge as the blur filter would otherwise sample repeated texture values!
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, highlightMap, 0);
	//check if this framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "starFiler FrameBuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//generate a two color buffer(X, Y blur image) framebuffer for star filter
	GLuint starFilterFBO;
	glGenFramebuffers(1, &starFilterFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, starFilterFBO);
	//create two color buffers
	GLuint blurImageXY[2];
	glGenTextures(2, blurImageXY);
	for (GLuint i = 0; i < 2; i++)
	{
		glBindTexture(GL_TEXTURE_2D, blurImageXY[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // We clamp to the edge as the blur filter would otherwise sample repeated texture values!
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, blurImageXY[i], 0);
	}
	//otherwise OpenGL only renders to a framebuffer's first color attachment ignoring all others
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	//check if this framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "starFiler FrameBuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

#pragma endregion


	LightStreaker lightStreaker(highlightMap, SCR_WIDTH, SCR_HEIGHT);
	LightStreaker lightStreaker2(highlightMap, SCR_WIDTH, SCR_HEIGHT);

	TextRenderer textRenderer(text);

	// lights
	//glm::vec3 lightPositions[] = {
	//	glm::vec3(-10.0f, 3.0f, -10.0f),
	//	//glm::vec3(-10.0f, 10.0f, 10.0f),
	//	glm::vec3(10.0f, 10.0f, 10.0f),
	//	glm::vec3(-10.0f, -10.0f, 10.0f),
	//	glm::vec3(10.0f, -10.0f, 10.0f),
	//};
	//glm::vec3 lightColors[] = {
	//	glm::vec3(500.0f, 500.0f, 500.0f),
	//	glm::vec3(500.0f, 300.0f, 300.0f),
	//	glm::vec3(300.0f, 100.0f, 300.0f),
	//	glm::vec3(300.0f, 300.0f, 500.0f)
	//};
	// lights
	glm::vec3 lightPositions[] = {
		glm::vec3(-10.0f, 10.0f, 10.0f),
		glm::vec3(10.0f, 10.0f, 10.0f),
		glm::vec3(-10.0f, -10.0f, 10.0f),
		glm::vec3(10.0f, -10.0f, 10.0f),
	};
	glm::vec3 lightColors[] = {
		glm::vec3(500.0f, 500.0f, 500.0f),
		glm::vec3(500.0f, 300.0f, 300.0f),
		glm::vec3(300.0f, 100.0f, 300.0f),
		glm::vec3(300.0f, 300.0f, 500.0f)
	};

	// Game loop
	while (!glfwWindowShouldClose(window))
	{
		// set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//be better to calculate average frame rates during 0.25 - 0.5 s . prevent from fps quick change.
		//int frameRate = (int) (1 / deltaTime);
		stringstream ss;
		ss << roughness;
		string fpsShow = "Roughness : ";
		fpsShow += ss.str();

		// check and call events
		glfwPollEvents();
		Do_Movement();

		// clear the colorbuffer
		//glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

		//--------------------------------PASS 1-----------------------------------------------------------//
		//draw main scenes (orignal & NDFFilering)
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		Shader *currentShader;
		for (int i = 0; i < 2; i++)
		{
			if (i == 0)
			{
				//shader.Use();
				glViewport(0, 0, SCR_WIDTH / 2, SCR_HEIGHT);
				currentShader = &shader;
				//textRenderer.RenderText(text, "Original", 25.0f, 25.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
				//textRenderer.RenderText(text, fpsShow, 25, 850, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
				//textRenderer.RenderText(text, "Beckmann", 25.0f, 25.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
				currentShader->Use();

				//glUniform1i(glGetUniformLocation(shaderFilter.program, "isRectangle"), GL_TRUE);
			}
			else
			{
				//shaderFilter.Use();
				glViewport(SCR_WIDTH / 2, 0, SCR_WIDTH / 2, SCR_HEIGHT);
				currentShader = &shaderFilter;
				//textRenderer.RenderText(text, "NDF Filtering", 25.0f, 25.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
				//textRenderer.RenderText(text, "GGX", 25.0f, 25.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
				currentShader->Use();

				glUniform1i(glGetUniformLocation(shaderFilter.program, "isRectangle"), GL_TRUE);
				glUniform1i(glGetUniformLocation(shaderFilter.program, "isAnisotropic"), GL_TRUE);

			}
			currentShader->Use();

			// configure view matrix
			glm::mat4 view = camera.GetViewMatrix();
			glUniformMatrix4fv(glGetUniformLocation(currentShader->program, "view"), 1, GL_FALSE, glm::value_ptr(view));

			// setup relevant shader uniforms
			glUniform3fv(glGetUniformLocation(currentShader->program, "camPos"), 1, &camera.Position[0]);
			//glUniform1f(glGetUniformLocation(shader.program, "exposure"), 1.0f);

			//set manually
			glUniform1f(glGetUniformLocation(currentShader->program, "roughness"), roughness);
			glUniform1f(glGetUniformLocation(currentShader->program, "metallic"), 0.2f);
			//cout << "roughness = " << roughness << endl;

			//---------------------------------------------------------------------------------------
			// draw model
			//---------------------------------------------------------------------------------------
			glm::mat4 model;
			if (modelIndex == 0) // cyborg
			{
				model = glm::mat4();
				model = glm::translate(model, glm::vec3(0.0, -5.0, 0.0));
				//model = glm::rotate(model, (GLfloat)glfwGetTime() * -1, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
				model = glm::scale(model, glm::vec3(2.0f));

				glUniform1i(glGetUniformLocation(currentShader->program, "useDiffuseTexture"), GL_TRUE);
			}
			else if (modelIndex == 1) // ogre
			{
				model = glm::mat4();
				model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
				//model = glm::rotate(model, (GLfloat)glfwGetTime() * -1, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
				model = glm::scale(model, glm::vec3(2.0f));

				glActiveTexture(GL_TEXTURE1);
				glBindTexture(GL_TEXTURE_2D, normal);
			}
			else if (modelIndex == 2) // wave plane
			{
				model = glm::mat4();
				model = glm::translate(model, glm::vec3(0.0, 0.0, -2.0));
				model = glm::scale(model, glm::vec3(0.05f));

				normalMapping = false;
			}
			else if (modelIndex == 3)
			{
				model = glm::mat4();
				model = glm::translate(model, glm::vec3(0.0f, -4.0f, -6.0f)); // Translate it down a bit so it's at the center of the scene
				model = glm::scale(model, glm::vec3(0.0005f, 0.0005f, 0.0005f));	// It's a bit too big for our scene, so scale it down
				model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

				normalMapping = false;
			}
			glUniformMatrix4fv(glGetUniformLocation(currentShader->program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			glUniform1i(glGetUniformLocation(currentShader->program, "normalMapping"), normalMapping);
			choosedModel->Draw(*currentShader);

			//glm::mat4 model;
			//model = glm::mat4();
			//model = glm::translate(model, glm::vec3(0.0, 0.0, 0.0));
			////model = glm::rotate(model, (GLfloat)glfwGetTime() * -1, glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
			//model = glm::scale(model, glm::vec3(2.0f));
			//glUniformMatrix4fv(glGetUniformLocation(currentShader->program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			//glUniform1i(glGetUniformLocation(currentShader->program, "normalMapping"), normalMapping);
			////cyborg.Draw(shader);
			////rose.Draw(shader);
			//rose.Draw(*currentShader);

			//------------------------------------------------------------------------------------
			// render light source
			//------------------------------------------------------------------------------------
			for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
			{
				currentShader->Use();
				glUniform3fv(glGetUniformLocation(currentShader->program, ("lightPositions[" + std::to_string(i) + "]").c_str()), 1, &lightPositions[i][0]);
				glUniform3fv(glGetUniformLocation(currentShader->program, ("lightColors[" + std::to_string(i) + "]").c_str()), 1, &lightColors[i][0]);
				lamp.Use();
				glUniformMatrix4fv(glGetUniformLocation(lamp.program, "view"), 1, GL_FALSE, glm::value_ptr(view));
				glm::vec3 currentLightColor = glm::normalize(lightColors[i]);
				glUniform3fv(glGetUniformLocation(lamp.program, "lightColor"), 1, glm::value_ptr(currentLightColor));
				model = glm::mat4();
				model = glm::translate(model, lightPositions[i]);
				model = glm::scale(model, glm::vec3(0.5f));
				glUniformMatrix4fv(glGetUniformLocation(lamp.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
				renderSphere();
			}
		}


#ifdef STAR_FILTER
		//--------------------------------------------PASS 2-------------------------------------------------//
		// ensure processing with hdr color value!!!
		glBindFramebuffer(GL_FRAMEBUFFER, HighlightFBO);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		textHighlight.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorbuffer);
		RenderQuad();

		if (chooseStarFilter == 2)
		{
			lightStreaker.draw(kawaseLightStreak, 4, glm::vec2(1, 1), 0.5, 3);
			lightStreaker2.draw(kawaseLightStreak, 4, glm::vec2(-1, 1), 0.5, 3);
		}

		////--------------------------------------------PASS 3-------------------------------------------------//
		//do star filtering
		if (chooseStarFilter == 1)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, starFilterFBO);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
			starFilter.Use();
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, highlightMap);
			RenderQuad();
		}
#endif
		//--------------------------------------------PASS 4----------------------------------------------------//
		//do composition & draw to final screen
		//do tone mapping at last...... ensure hdr processing before rendering to screen...
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		textRenderer.RenderText(text, "Original", 25.0f, 25.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
		textRenderer.RenderText(text, fpsShow, 25.0f, SCR_HEIGHT - 25.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
		textRenderer.RenderText(text, "NDF Filtering", SCR_WIDTH / 2 + 25.0f, 25.0f, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));
		//textRenderer.RenderText(text, "Beckmann(parallelogram)", SCR_WIDTH / 2 + 25.0f, 50, 0.5f, glm::vec3(0.5, 0.8f, 0.2f));

		quad.Use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorbuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);
#ifdef STAR_FILTER
		if (chooseStarFilter == 1)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, blurImageXY[0]);
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, blurImageXY[1]);
		}
		else if (chooseStarFilter == 2)
		{
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, lightStreaker.getFilteredTexture());
			glActiveTexture(GL_TEXTURE2);
			glBindTexture(GL_TEXTURE_2D, lightStreaker2.getFilteredTexture());
		}
#endif

		RenderQuad();


		// Swap the buffers
		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}


#pragma region "primitive data"

// renders (and builds if necessary) a sphere
unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
	if (sphereVAO == 0)
	{
		glGenVertexArrays(1, &sphereVAO);

		unsigned int vbo, ebo;
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec3> tangents;
		std::vector<glm::vec3> bitangents;
		std::vector<unsigned int> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359;
		for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)//球的纵向分割数
		{
			for (unsigned int x = 0; x <= X_SEGMENTS; ++x)//横向分割数
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.push_back(glm::vec3(xPos, yPos, zPos));
				uv.push_back(glm::vec2(xSegment, ySegment));
				normals.push_back(glm::vec3(xPos, yPos, zPos));
			}
		}

		bool oddRow = false;
		for (int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.push_back(y       * (X_SEGMENTS + 1) + x);
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.push_back(y       * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		indexCount = indices.size();
		std::vector<float> data;
		for (int i = 0; i < positions.size(); ++i)
		{
			data.push_back(positions[i].x);
			data.push_back(positions[i].y);
			data.push_back(positions[i].z);
			if (uv.size() > 0)
			{
				data.push_back(uv[i].x);
				data.push_back(uv[i].y);
			}
			if (normals.size() > 0)
			{
				data.push_back(normals[i].x);
				data.push_back(normals[i].y);
				data.push_back(normals[i].z);
			}
			if (tangents.size() > 0)
			{
				data.push_back(tangents[i].x);
				data.push_back(tangents[i].y);
				data.push_back(tangents[i].z);
			}
			if (bitangents.size() > 0)
			{
				data.push_back(bitangents[i].x);
				data.push_back(bitangents[i].y);
				data.push_back(bitangents[i].z);
			}
		}
		glBindVertexArray(sphereVAO);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
		float stride = (3 + 2 + 3) * sizeof(float);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (GLvoid*)(5 * sizeof(float)));
	}

	glBindVertexArray(sphereVAO);
	glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}


// RenderQuad() Renders a 1x1 quad in NDC
GLuint quadVAO = 0;
GLuint quadVBO;
void RenderQuad()
{
	if (quadVAO == 0)
	{
		GLfloat quadVertices[] = {
			// Positions     //TexCoords  // Normal
			-1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
			1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0,
		};
		// Setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

#pragma endregion

// This function loads a texture from file. Note: texture loading functions like these are usually 
// managed by a 'Resource Manager' that manages all resources (like textures, models, audio). 
// For learning purposes we'll just define it as a utility function.
GLuint loadTexture(GLchar const * path)
{
	//Generate texture ID and load texture data 
	GLuint textureID;
	glGenTextures(1, &textureID);
	int width, height, nChannels;
	unsigned char* image = stbi_load(path, &width, &height, &nChannels, 0);
	//unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
	if (!image) cout << "error loading image." << endl;
	// Assign texture to ID
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);

	// Parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	//SOIL_free_image_data(image);
	stbi_image_free(image);
	return textureID;
}

#pragma region "User input"

bool keys[1024];
bool keysPressed[1024];
// Moves/alters the camera positions based on user input
void Do_Movement()
{
	// Camera controls
	if (keys[GLFW_KEY_W])
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (keys[GLFW_KEY_S])
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (keys[GLFW_KEY_A])
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (keys[GLFW_KEY_D])
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (keys[GLFW_KEY_SPACE] && !keysPressed[GLFW_KEY_SPACE])
	{
		normalMapping = !normalMapping;
		keysPressed[GLFW_KEY_SPACE] = true;
	}

	if (keys[GLFW_KEY_ENTER] && !keysPressed[GLFW_KEY_ENTER])
	{
		chooseStarFilter++;
		if (chooseStarFilter == 0 || chooseStarFilter == 1) chooseStarFilter = 2;
		if (chooseStarFilter == 3) chooseStarFilter = 0;
		keysPressed[GLFW_KEY_ENTER] = true;
	}

	if (keys[GLFW_KEY_Q])
	{
		//roughness += 0.5f * deltaTime;
		//cout << "roughness = " << roughness << endl;
		roughness = roughness < 0.2f ? roughness + 0.05f * deltaTime : 0.2f;
	}

	if (keys[GLFW_KEY_E])
	{
		roughness -= 0.05f * deltaTime;
		roughness = roughness > 0.0f ? roughness : 0.0001f;
	}

	if (keys[GLFW_KEY_C] && !keysPressed[GLFW_KEY_C])
	{
		cout << "camera position:" << endl << "X: " << camera.Position.x << " Y: " << camera.Position.y << " Z: " << camera.Position.z << " yaw: " << camera.Yaw << " pitch: " << camera.Pitch << endl;
		keysPressed[GLFW_KEY_C] = true;
	}
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
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

GLfloat lastX = 400, lastY = 300;
bool firstMouse = true;
// Moves/alters the camera positions based on user input
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}

#pragma endregion