#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "EnvLight.h"
#include <model.h>

#include "Sampler.h"
#include "RayTracer.h"

using std::vector;

class Transfer
{
public:
	//Transfer();
	explicit Transfer(Model* model, const Sampler &sampler, bool &isShadow);
	~Transfer();

	vector<vector<float>>& GenerateUnShadowedCoeffs() const;

private:
	static vector<vector<float>> transferVectorUnShadowed; // integrate without visibility
	//static 

	Model* objModel;
	const Sampler* sampler;
	bool SelfShadow(unsigned vertexIndex, const glm::vec3 sampleRayDir) const;

	mutable RayTracer rayTrancer;
	
	//control
	bool &isShadow;
};

