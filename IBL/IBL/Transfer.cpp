#include "Transfer.h"



Transfer::Transfer(Model* model): objModel(model)
{
	//sampler = new Sampler();
}


Transfer::~Transfer()
{
}

void Transfer::GenerateUnShadowedCoeffs()
{
	try
	{
		if (objModel == nullptr)
			throw runtime_error("Input model is NULL.");
	}
	catch (runtime_error err)
	{

	}
	// for each vertex
	for (GLuint i = 0; i < objModel->GetModelVertexSize(); ++i)
	{
		glm::vec3 normal = objModel->GetCurrentVertexNormal(i);
		// for each samples
		for(GLuint j = 0; j < sampler->size(); ++j)
		float cosineTerm = glm::dot(normal, (*sampler)[j].direction);

		// for each SHfunc
	}
		
}
