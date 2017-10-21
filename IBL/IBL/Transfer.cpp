#include "Transfer.h"


vector<vector<float>> Transfer::transferVectorUnShadowed; // integrate without visibility
vector<vector<float>> Transfer::transferVectorShadowed[4];


//Transfer::Transfer():objModel(nullptr), rayTrancer(objModel)
//{
//	sampler = nullptr;
//}

//Transfer::Transfer(Model* model, const Sampler &raySampler, bool &isShadow): objModel(model), rayTrancer(objModel), isShadow(isShadow)
//{
//	sampler = &raySampler;// not suitable...
//	//initialize sampler
//}

Transfer::Transfer(Model* model, const Sampler &raySampler) : objModel(model), rayTrancer(objModel)
{
	sampler = &raySampler;// not suitable...
						  //initialize sampler
}

Transfer::~Transfer()
{
}

bool Transfer::GenerateUnShadowedCoeffs() const
{
	transferVectorUnShadowed.clear();
	transferVectorShadowed[0].clear();
	if (objModel == nullptr)
		throw runtime_error("Input model is NULL.");

	// For each mesh. Haven't implemented yet
	cout << "Generating Transfer Coefficents. Please wait...\nPorgress: ";
	int cnt = 0;
	vector<float> vTransferVector = vector<float>(9, 0);
	vector<float> vTransferVectorShadowed(9, 0);

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
			// Dot product in World Coordinates
			float cosineTerm = glm::dot(normal, sampleRayDir);
			if (cosineTerm > 0) // Why does nan(ind) occur if I drop this condition?D
			{
				// for each SHfunc
				for (GLuint k = 0; k < 9; ++k)
				{
					vTransferVector[k] += (*sampler)[j].SHcoeffs[k] * cosineTerm;
				}
				if (!SelfShadow(i, sampleRayDir))// Add Visibility
				{
					//vTransferVectorShadowed = vTransferVector; // FIXME: WHY IT DOESN'T WORK??
					for (GLuint k = 0; k < 9; ++k)
					{
						vTransferVectorShadowed[k] += (*sampler)[j].SHcoeffs[k] * cosineTerm;
					}
				}
			}
		}

		//rescale transfer coefficents
		const float scale = 4 * MY_PI / sampler->size();
		const float diffuseBRDF = 1.0 / MY_PI; // VEC3??
		
		for (auto &transferCoeff : vTransferVector)
		{
			transferCoeff *= diffuseBRDF * scale;
		}

		for (auto &transferCoeff : vTransferVectorShadowed)
		{
			transferCoeff *= diffuseBRDF * scale;
		}

		transferVectorUnShadowed.push_back(vTransferVector);
		transferVectorShadowed[0].push_back(vTransferVectorShadowed);
	}
	cout << "\n" << endl;
 	return true;
}

bool Transfer::GenerateInterreflectionShadowedCoeffs(size_t bounceTime) const
{
	assert(bounceTime > 0 && bounceTime < 3);
	// Initialize current bounce shadow coefficients with pervious coefficients
	// for each SHfunc
	//for (GLuint k = 0; k < 9; ++k)
	//{
	//	transferVectorShadowed[bounceTime][k] = transferVectorShadowed[bounceTime-1][k];
	//}
	transferVectorShadowed[bounceTime] = transferVectorShadowed[bounceTime-1];
	
	cout << "\nGenerating Interreflection Coeffs bounce " << bounceTime << endl;
	int cnt = 0;
	// for each vertex
	GLuint vertexNum = objModel->GetModelVertexSize();
	for (GLuint i = 0; i < vertexNum; ++i) //incorrect
	{
		// Show progress
		if (i == cnt)
		{
			cout << i * 100 / vertexNum << "% ";
			cnt += vertexNum / 10;
		}

		glm::vec3 normal = objModel->GetCurrentVertexNormal(i);
		normal = glm::normalize(normal);

		//rescale transfer coefficents
		const float scale = 4 * MY_PI / sampler->size();
		const float diffuseBRDF = 1.0 / MY_PI; // VEC3??
		float fScale = scale * diffuseBRDF * 0.5;
		// for each samples
		for (GLuint j = 0; j < sampler->size(); ++j)
		{
			glm::vec3 sampleRayDir = glm::normalize((*sampler)[j].direction);
			if (SelfShadow(i, sampleRayDir))
			{
				float cosineTerm = glm::dot(normal, sampleRayDir);
				if (cosineTerm > 0) // Why does nan(ind) occur if I drop this condition?
				{
					// for each SHfunc
					for (GLuint k = 0; k < 9; ++k)
					{
						transferVectorShadowed[bounceTime][i][k] += fScale * transferVectorShadowed[bounceTime - 1][i][k] * cosineTerm;
					}
				}
			}
		}

	}
	return true;
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
