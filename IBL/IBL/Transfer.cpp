#include "Transfer.h"


vector<vector<float>> Transfer::transferVectorUnShadowed; // integrate without visibility


//Transfer::Transfer():objModel(nullptr), rayTrancer(objModel)
//{
//	sampler = nullptr;
//}

Transfer::Transfer(Model* model, const Sampler &raySampler, bool &isShadow): objModel(model), rayTrancer(objModel), isShadow(isShadow)
{
	sampler = &raySampler;// not suitable...
	//initialize sampler
}


Transfer::~Transfer()
{
}

vector<vector<float>>& Transfer::GenerateUnShadowedCoeffs() const
{
	transferVectorUnShadowed.clear();
	if (objModel == nullptr)
		throw runtime_error("Input model is NULL.");

	// For each mesh. Haven't implemented yet
	cout << "Generating Transfer Coefficents. Please wait...\nPorgress: ";
	int cnt = 0;
	vector<float> vTransferVector = vector<float>(9, 0);
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
		
		// for each samples
		for (GLuint j = 0; j < sampler->size(); ++j)
		{
			glm::vec3 sampleRayDir = glm::normalize((*sampler)[j].direction);
			float cosineTerm = glm::dot(normal, sampleRayDir);
			if (cosineTerm > 0) // Why does nan(ind) occur if I drop this condition?
			{
				if (!SelfShadow(i, sampleRayDir) || !isShadow)// Add Visibility
				{
					// for each SHfunc
					for (GLuint k = 0; k < 9; ++k)
					{
						vTransferVector[k] += (*sampler)[j].SHcoeffs[k] * cosineTerm;
					}
				}
			}
		}

		//rescale transfer coefficents
		const float scale = 4 * MY_PI / sampler->size();
		float diffuseBRDF = 1.0 / MY_PI; // VEC3??
		for (auto &transferCoeff : vTransferVector)
		{
			transferCoeff *= diffuseBRDF * scale;
		}

		transferVectorUnShadowed.push_back(vTransferVector);
	}
	cout << "\n" << endl;
 	return transferVectorUnShadowed;
}

// Deicide whether the current ray hits the object
bool Transfer::SelfShadow(unsigned vertexIndex, const glm::vec3 sampleRayDir) const
{
	glm::vec3 origin = objModel->GetCurrentVertexPosition(vertexIndex);
	// In selfshadow equals to be in blocked.
	bool isBlocked = rayTrancer.IntersectScene(origin, sampleRayDir);
	//bool isBlocked = false;
	
	//if (sampleRayDir.x < 0)
	//	return true;
	
	return isBlocked;
}
