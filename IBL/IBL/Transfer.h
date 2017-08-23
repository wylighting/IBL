#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "EnvLight.h"
#include <model.h>

using std::vector;

class Transfer
{
public:
	explicit Transfer(Model* model);
	~Transfer();

	void GenerateUnShadowedCoeffs();

	vector<glm::vec3> transferVectorUnShadowed; // integrate without visibility
	
	Model* objModel;
};

