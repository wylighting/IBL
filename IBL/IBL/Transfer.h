#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "EnvLight.h"
#include <model.h>

#include "Sampler.h"
#include "RayTracer.h"

using std::vector;

enum LightType
{
	LIGHT_UNSHADOWED = 0,
	LIGHT_SHADOWED,
	LIGHT_SHADOWED_BOUNCE_1,
	LIGHT_SHADOWED_BOUNCE_2,
	LIGHT_SHADOWED_BOUNCE_3
};

class Transfer
{
public:
	//Transfer();
	explicit Transfer(Model* model, const Sampler &sampler, bool &isShadow);
	~Transfer();

	bool GenerateUnShadowedCoeffs() const;
	bool GenerateInterreflectionShadowedCoeffs() const;

	const vector<vector<float>>& GetTransferVector(LightType lightType) const;

private:
	static vector<vector<float>> transferVectorUnShadowed; // integrate without visibility // why static?
	static vector<vector<float>> transferVectorShadowed[4];

	Model* objModel;
	const Sampler* sampler;
	bool SelfShadow(unsigned vertexIndex, const glm::vec3 sampleRayDir) const;

	mutable RayTracer rayTrancer;
	
	//control
	bool &isShadow;
};

inline const vector<vector<float>>& Transfer::GetTransferVector(LightType lightType) const
{
	switch (lightType)
	{
	case LIGHT_UNSHADOWED:
		return transferVectorUnShadowed;
	case LIGHT_SHADOWED:
		return transferVectorShadowed[0];
	case LIGHT_SHADOWED_BOUNCE_1:
		return transferVectorShadowed[1];
	case LIGHT_SHADOWED_BOUNCE_2:
		return transferVectorShadowed[2];
	case LIGHT_SHADOWED_BOUNCE_3:
		return transferVectorShadowed[3];
	default:
		return vector<vector<float>>();
	}
}
