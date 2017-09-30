// **************************************************************** 
// @file: EnvLight.h
// @description: 1. load environment light file as environment light
//				 2. convert environment light to spherical harmonic coefficients
// ****************************************************************
#pragma once
#define GLEW_STATIC
#include <GL/glew.h>

#include <string>
#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <Shader.h>


#include "Sampler.h"
#include "ToolFunctions.h"

using std::string;
//using glm::vec3;
using std::vector;


enum EnvMapType
{
	EQUIRECTANGULAR_ENVMAP,
	CUBE_ENVMAP,
	SPHERE_ENVMAP
};

struct CubeMap
{
	
};

class EnvLight
{
public:
	EnvLight(string envMapPath, EnvMapType envMapType, const Shader &);
	EnvLight(vector<string> &faces, EnvMapType envMapType);
	~EnvLight();
	GLuint CreateIrradianceMapWithSampling(const Shader &irradianceShader);
	//vector<glm::vec3>& CalcLightCoeffs(const vector<Sample> &samples) const;
	vector<glm::vec3>& CalcLightCoeffs(const Sampler &sampler) const;
	//void GenerateSamples(int sqrtNumSamples);
	//Sample GetSampleById(unsigned i);
	
	GLuint GetCubeMap() const { return envCubemap; };

private:
	bool LoadCubeMap(vector<string> &faces) const;
	void Equirectangular2CubeMap(const Shader &equirectangularShader);
	void LoadHDREnvMapEquirectangular(string envMapPath);
	static bool sampleInitialized;
	static vector<glm::vec3> L_lm;

	//friend ?

	float* imgData;

	glm::vec3 GetLightFromEquirectEnvMap(const Sample &sampleRay) const;
	glm::vec3 GetLightFromCubeEnvMap(const Sample &sampleRay) const;
	int width, height, nrComponents;
	Shader *equirectangularToCubemapShader, *irradianceShader;
	unsigned int hdrTexture;
	unsigned int envCubemap;

	glm::mat4 captureProjection, *captureViews;
	unsigned int captureFBO;
	unsigned int captureRBO;
	void setupEnvCubeMap();
	void setupFrameBuffer();
};
