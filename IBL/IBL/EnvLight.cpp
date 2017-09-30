#include "EnvLight.h"
#include "SphericalHarmonic.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/detail/type_mat.hpp>
#include <vector>
#include <glm/detail/type_mat.hpp>
#include <vector>

EnvLight::EnvLight(string envMapPath, EnvMapType envMapType, const Shader &equirShader)
{
	LoadHDREnvMapEquirectangular(envMapPath);
	if (envMapType == EQUIRECTANGULAR_ENVMAP)
	{
		//prepare for converting to cube map
		setupFrameBuffer();
		setupEnvCubeMap();
		Equirectangular2CubeMap(equirShader);
	}
}

EnvLight::EnvLight(vector<string>& faces, EnvMapType envMapType)
{
	if (envMapType == CUBE_ENVMAP)
	{
		//prepare for converting to cube map
		//setupFrameBuffer();
		setupEnvCubeMap();
		//Equirectangular2CubeMap();
		if(!LoadCubeMap(faces))
			std::cerr << "Cube map is not loaded correctly." << std::ends;
	}

}
EnvLight::~EnvLight()
{
	delete equirectangularToCubemapShader;
	equirectangularToCubemapShader = nullptr;
	stbi_image_free(imgData);
}

void EnvLight::LoadHDREnvMapEquirectangular(string envMapPath)
{
	stbi_set_flip_vertically_on_load(true);

	imgData = stbi_loadf(envMapPath.c_str(), &width, &height, &nrComponents, 0);

	if (imgData)
	{
		glGenTextures(1, &hdrTexture);
		glBindTexture(GL_TEXTURE_2D, hdrTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, imgData); // note how we specify the texture's data value to be float

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		//stbi_image_free(data);
	}
	else
	{
		std::cout << "Failed to load HDR image." << std::endl;
		throw std::runtime_error("Failed to load HDR image.");
	}

}

bool EnvLight::LoadCubeMap(vector<string>& faces) const
{
	// envCubeMap
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	int width, height, channels;
	for (unsigned int i = 0; i < faces.size(); ++i)
	{
		unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &channels, 0);
		if(data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
			return false;
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return true;
}



void EnvLight::Equirectangular2CubeMap(const Shader &equirectangularShader)
{
	equirectangularToCubemapShader = new Shader(equirectangularShader);

	// pbr: set up projection and view matrices for capturing data onto the 6 cubemap face directions
	// ----------------------------------------------------------------------------------------------
	captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	captureViews = new glm::mat4[6]
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	// pbr: convert HDR equirectangular environment map to cubemap equivalent
	// ----------------------------------------------------------------------
	assert(equirectangularToCubemapShader != nullptr);
	equirectangularToCubemapShader->use();
	equirectangularToCubemapShader->setInt("equirectangularMap", 0);
	equirectangularToCubemapShader->setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		equirectangularToCubemapShader->setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, envCubemap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//ModelBox::RenderSphere();
		ModelBox::RenderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//return envCubemap;
}

void EnvLight::setupEnvCubeMap()
{
	// pbr: setup cubemap to render to and attach to framebuffer
	// ---------------------------------------------------------
	glGenTextures(1, &envCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

}

void EnvLight::setupFrameBuffer()
{	// pbr: setup framebuffer
	// ----------------------

	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	//A renderbuffer object has the added advantage though that it stores its data in OpenGL's native rendering format making it optimized for off-screen rendering to a framebuffer.
	//their data is already in its native format they are quite fast when writing data or simply copying their data to other buffers.
	//When we're not sampling from these buffers, a renderbuffer object is generally preferred since it's more optimized.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);//allocate the memory without filling the buffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, captureRBO);
	//I wonder what's the concrete optimization of renderbuffer object
}

GLuint EnvLight::CreateIrradianceMapWithSampling(const Shader &_irradianceShader)
{
	// pbr: create an irradiance cubemap, and re-scale capture FBO to irradiance scale.
	// --------------------------------------------------------------------------------
	irradianceShader = new Shader(_irradianceShader);

	unsigned int irradianceMap;
	glGenTextures(1, &irradianceMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	// pbr: solve diffuse integral by convolution to create an irradiance (cube)map.
	// -----------------------------------------------------------------------------
	irradianceShader->use();
	irradianceShader->setInt("environmentMap", 0);
	irradianceShader->setMat4("projection", captureProjection);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, envCubemap);

	glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);
	for (unsigned int i = 0; i < 6; ++i)
	{
		irradianceShader->setMat4("view", captureViews[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, irradianceMap, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ModelBox::RenderCube();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return irradianceMap;
}

vector<glm::vec3> EnvLight::L_lm = vector<glm::vec3>(9);
bool EnvLight::sampleInitialized = false;

//vector<glm::vec3>& EnvLight::CalcLightCoeffs(const vector<Sample> &samples) const
//{
//	//assert(sampleInitialized == true);
//
//	float scale = 4 * MY_PI / samples.size();
//	for(auto &sampleRay : samples)
//	{
//		glm::vec3 Lenv = GetLightFromEquirectEnvMap(sampleRay);
//		for (int i = 0; i < 9; ++i)
//			L_lm[i] += sampleRay.SHcoeffs[i] * Lenv;
//	}
//	for (auto &coeff : L_lm)
//		coeff *= scale;
//	return L_lm;
//}

vector<glm::vec3>& EnvLight::CalcLightCoeffs(const Sampler &sampler) const
{
		float scale = 4 * MY_PI / sampler.size();
		
		for(unsigned i = 0; i < sampler.size(); ++i)
		{
			const auto &sampleRay = sampler[i];
			glm::vec3 Lenv = GetLightFromEquirectEnvMap(sampleRay); // why scale??
			for (int j = 0; j < 9; ++j)
				L_lm[j] += sampleRay.SHcoeffs[j] * Lenv;
		}
		for (auto &coeff : L_lm)
			coeff *= scale;
		return L_lm;
}

inline glm::vec3 EnvLight::GetLightFromCubeEnvMap(const Sample& sampleRay) const
{
	// Select proper face
	
	// decide the texel in the face
}

inline glm::vec3 EnvLight::GetLightFromEquirectEnvMap(const Sample &sampleRay) const
{
	assert(imgData);
	float x = sampleRay.phi / (2 * MY_PI);
	float y = (MY_PI - sampleRay.theta) / (MY_PI);

	size_t idx = x * width + y * height * width;
	idx *= 3;
	return glm::vec3(imgData[idx], imgData[idx + 1], imgData[idx + 2]);
}



//Sample EnvLight::GetSampleById(unsigned i)
//{
//	return samples[i];
//}

