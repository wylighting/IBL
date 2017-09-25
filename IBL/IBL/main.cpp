//#include <glad/glad.h>
//skybox differences with previous demo?
//convert the equirectangular map into cubemap

 //GLEW
#define GLEW_STATIC
#include <GL/glew.h>
#pragma comment(lib, "glew32.lib")

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>
#include <camera.h>
#include <string>

#include "Renderer.h"
#include "EnvLight.h"

using std::to_string;

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const string resource_path = "../../../Resources/";

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// lights
// ------
glm::vec3 lightPositions[] = {
	glm::vec3(-10.0f,  10.0f, 10.0f),
	glm::vec3(10.0f,  10.0f, 10.0f),
	glm::vec3(-10.0f, -10.0f, 10.0f),
	glm::vec3(10.0f, -10.0f, 10.0f),
};
glm::vec3 lightColors[] = {
	glm::vec3(300.0f, 300.0f, 300.0f),
	glm::vec3(300.0f, 300.0f, 300.0f),
	glm::vec3(300.0f, 300.0f, 300.0f),
	glm::vec3(300.0f, 300.0f, 300.0f)
};

int main()
{
	Renderer::InitGLFW(SCR_WIDTH, SCR_HEIGHT);

	// Initialize Sampler
	Sampler raySampler(100);

	// Setup Environment Light & Get Light Coefficents
	EnvLight envMap(resource_path + "textures/hdr/Newport_Loft/Newport_Loft_Ref.hdr", EQUIRECTANGULAR_ENVMAP);
	//EnvLight envMap(resource_path + "textures/hdr/Milkyway/Milkyway_small.hdr", EQUIRECTANGULAR_ENVMAP);
	//EnvLight envMap(resource_path + "textures/hdr/Frozen_Waterfall/Frozen_Waterfall_Ref.hdr", EQUIRECTANGULAR_ENVMAP);

	Renderer PRTrenderer = Renderer(camera, raySampler, envMap);

	// build and compile shaders
	// -------------------------
	Shader pbrShader("pbr.vs", "pbr.fs");
	Shader equirectangularToCubemapShader("cubemap.vs", "cubemap.fs");
	Shader backgroundShader("background.vs", "background.fs");
	Shader irradianceShader("cubemap.vs", "irradiance_convolution.fs");

	pbrShader.use();
	pbrShader.setVec3("albedo", 0.5f, 0.0f, 0.0f);
	pbrShader.setFloat("ao", 1.0f);

	backgroundShader.use();
	//backgroundShader.setInt("environmentMap", 0);
	glUniform1i(glGetUniformLocation(backgroundShader.program, "ao"), 0);

	
	// initialize static shader uniforms before rendering
	// --------------------------------------------------
	glm::mat4 projection = glm::perspective(camera.Zoom, static_cast<float>(SCR_WIDTH) / static_cast<float>(SCR_HEIGHT), 0.1f, 100.0f);
	pbrShader.use();
	pbrShader.setMat4("projection", projection);
	backgroundShader.use();
	backgroundShader.setMat4("projection", projection);


	// Compute using ravi's method directly in pbrShader
	vector<glm::vec3> L_lm = envMap.CalcLightCoeffs(raySampler);

	//set light coeffs in pbrShader.
	int i = 0;
	for(int l = 0; l <= 2; ++l)
		for(int m = -l; m <= l; ++m)
		{
			string coeff_name = "L" + to_string(l);
			if (m < 0) coeff_name += '_' + to_string(abs(m));
			else coeff_name += to_string(m);

			pbrShader.use();
			pbrShader.setVec3(coeff_name, L_lm[i]);
			i++;
		}


	//load model
	//PRTrenderer.addModelFromFile(resource_path + "objects/dragon_10w.obj");
#ifndef _DEBUG
	PRTrenderer.addModelFromFile(resource_path + "objects/bunny_normal.obj");
#else
	PRTrenderer.addModelFromFile(resource_path + "objects/rock/rock.obj");
#endif
	//PRTrenderer.addModelFromFile(resource_path + "objects/MODELS/sphere.obj");


	// Convert equirectangular map to CubeMap
	GLuint envCubemap = envMap.Equirectangular2CubeMap(equirectangularToCubemapShader);
	// Irradiance Map using Traditional Sampling method
	GLuint irradianceMap = envMap.CreateIrradianceMapWithSampling(irradianceShader);

	// start render
	PRTrenderer.Render(pbrShader, backgroundShader, envCubemap, irradianceMap);

	return 0;
}
