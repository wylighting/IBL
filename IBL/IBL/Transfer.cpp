#include "Transfer.h"

vector<vector<float>> Transfer::transferVectorUnShadowed; // integrate without visibility


Transfer::Transfer()
{
	sampler = nullptr;
	objModel = nullptr;
}

Transfer::Transfer(Model* model, const Sampler &raySampler): objModel(model)
{
	sampler = &raySampler;// not suitable...
	//initialize sampler
}


Transfer::~Transfer()
{
}

vector<vector<float>>& Transfer::GenerateUnShadowedCoeffs() const
{
	if (objModel == nullptr)
		throw runtime_error("Input model is NULL.");

	// For each mesh. Haven't implemented yet
	cout << "Generate Transfer Coefficents\nPorgress: ";
	int cnt = 0;
	// for each vertex
	GLuint vertexNum = objModel->GetModelVertexSize();
	for (GLuint i = 0; i < vertexNum; ++i) //incorrect
	{
		// Show progress
		if (i == cnt)
		{
			cout << i * 100 / vertexNum << "% " ;
			cnt += vertexNum / 10;
		}

		glm::vec3 normal = objModel->GetCurrentVertexNormal(i);
		normal = glm::normalize(normal);
		
		vector<float> vTransferVector = vector<float>(9, 0);
		// for each samples
		for (GLuint j = 0; j < sampler->size(); ++j)
		{
			float cosineTerm = glm::dot(normal, glm::normalize((*sampler)[j].direction));
			if (cosineTerm > 0) // Why does nan(ind) occur if I drop this condition?
			{
				// for each SHfunc
				for (GLuint k = 0; k < 9; ++k)
				{
					vTransferVector[k] += (*sampler)[j].SHcoeffs[k] * cosineTerm;
				}
			}
		}

		//rescale transfer coefficents
		float scale = 4 * MY_PI / sampler->size();
		float diffuseBRDF = 0.5 / MY_PI;
		for (auto &transferCoeff : vTransferVector)
		{
			transferCoeff *= scale;
		}

		transferVectorUnShadowed.push_back(vTransferVector);
	}
 	return transferVectorUnShadowed;
}
