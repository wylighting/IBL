#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "EnvLight.h"
#include <model.h>

#include "Sampler.h"

using std::vector;

class Transfer
{
public:
	Transfer();
	explicit Transfer(Model* model, const Sampler &sampler);
	~Transfer();

	vector<vector<float>>& GenerateUnShadowedCoeffs() const;

private:
	static vector<vector<float>> transferVectorUnShadowed; // integrate without visibility
	
	Model* objModel;
	const Sampler* sampler;
};

