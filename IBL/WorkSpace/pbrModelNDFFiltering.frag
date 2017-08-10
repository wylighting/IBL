#version 330 core

out vec4 FragColor;

in vec2 TexCoords;
in vec3 WorldPos;
in vec3 Normal;
in mat3 TBN;

// material parameters

// constant paramter
// uniform vec3 albedo;
// uniform float ao;

uniform float metallic;
uniform float roughness;

//using texture map
uniform sampler2D texture_diffuse1;
//uniform sampler2D albedoMap;
uniform sampler2D texture_normal1;

uniform vec3 camPos;
uniform float exposure;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform bool normalMapping;
uniform bool useDiffuseTexture;


const float PI = 3.14159265359;

// test
uniform bool isAnisotropic;
uniform bool isRectangle;

float correlation;

struct ShadingFrame
{
	vec3 normal;
	vec3 tangent;
	vec3 bitangent;
};

ShadingFrame shadingFrame;

// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.

vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(texture_normal1, TexCoords).xyz * 2.0 - 1.0;
    return normalize(TBN * tangentNormal);
}



vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
	float r = 1 - cosTheta;
	return F0 + (1 - F0) * pow(r, 5);
}

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
	float a2 = roughness * roughness;
	float NdotH  = max(dot(N, H), 0.0f);
	float NdotH2 = NdotH * NdotH;

	float nominator = a2;
	float denominator = NdotH2 * (a2 - 1) + 1;
	denominator = PI * denominator * denominator;

	return nominator / denominator;
}

//both N & M are from world sapce coordinates
float DistributionBeckmann(vec3 N, vec3 M, float alpha)
{
	// const float e = 2.718281828459;
	float NdotM = max(dot(N, M), 0.00001f); // prevent divided by zero
	float NdotM2 = NdotM * NdotM;
	float a2 = alpha * alpha;

	float exponent = - (1 - NdotM2) / (a2 * NdotM2);

	float coefficient = 1 / (PI * a2 * NdotM2 * NdotM2);

	return coefficient * exp(exponent);
}

//this H is in tangent space
float DistributionBeckmannAnisotropic(vec2 Hpp, vec2 alphas, float rou)
{
	//rou is correlation coefficient
	// const float e = 2.718281828459;
	vec3 H = normalize(vec3(Hpp, 1.0f));
	float NdotM = max(H.z, 0.00001f); // prevent divided by zero
	float NdotM2 = NdotM * NdotM;
	float a2 = alphas.x * alphas.y;
	float alphaX2 = alphas.x * alphas.x;
	float alphaY2 = alphas.y * alphas.y;

	// float length = sqrt(H.x * H.x + H.y * H.y);
	// vec2 hpp = vec2(H.x / length, H.y / length);
	// float cosPhi2 = hpp.x * hpp.x;
	// float sinPhi2 = hpp.y * hpp.y;


	float cosTheta2 = NdotM2;
	float sinTheta2 = 1 - NdotM2;
	float tanTheta2 = sinTheta2 / cosTheta2;

	float correlationTerm1 = 1 / (1 - rou * rou + 0.00001f);
	float correlationTerm2 = 2 * Hpp.x * Hpp.y * rou / a2;

	//float exponent = - (1 - NdotM2) / (a2 * NdotM2);

	float exponent = - correlationTerm1 * (Hpp.x * Hpp.x / alphaX2 + Hpp.y * Hpp.y / alphaY2 - correlationTerm2);

	float coefficient = 1 / (PI * a2 * NdotM2 * NdotM2);

	return coefficient * exp(exponent);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
	//for direct lighting
	float kDirect = (roughness + 1) * (roughness + 1) / 8;
	float nominator = NdotV;
	float denominator = NdotV * (1 - kDirect) + kDirect;
	return nominator / denominator;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
	float NdotV = max(dot(N, V), 0.0f);
	float NdotL = max(dot(N, L), 0.0f);
	float ggx1 = GeometrySchlickGGX(NdotV, roughness);
	float ggx2 = GeometrySchlickGGX(NdotL, roughness);

	return ggx1 * ggx2;
}

//where is the normal??
float GGXRectIntegral(vec2 Hpp, vec3 H, vec2 roughness, vec2 footprint)
{
	//deal with close-form solution
	float a = roughness.x, b = roughness.y;
	float a_2 = a * a, b_2 = b * b;
	float x1 = Hpp.x - footprint.x, x2 = Hpp.x + footprint.x;
	float y1 = Hpp.y - footprint.y, y2 = Hpp.y + footprint.y;
	float x1_2 = x1 * x1, x2_2 = x2 * x2;
	float y1_2 = y1 * y1, y2_2 = y2 * y2;

	float val = (
		(x1 * (atan((y1 * a) / (sqrt(x1_2 + a_2) * b)) - atan((y2 * a) / (sqrt(x1_2 + a_2) * b)))) / sqrt(x1_2 + a_2) + 
		(x2 * (-atan((y1 * a) /(sqrt(x2_2 + a_2) * b)) + atan((y2 * a) / (sqrt(x2_2 + a_2) * b)))) / sqrt(x2_2 + a_2) + 
		( y1*sqrt(y2_2 + b_2) * (atan((x1*b)/(a*sqrt(y1_2 + b_2))) - atan((x2*b) / (a*sqrt(y1_2 + b_2)))) + 
		  y2*sqrt(y1_2 + b_2) *(-atan((x1*b)/(a*sqrt(y2_2 + b_2))) + atan((x2*b) / (a*sqrt(y2_2 + b_2)))) ) / 
		(sqrt((y1_2 + b_2)*(y2_2 + b_2)))   )
		/ ((2*PI) * (x1 - x2) * (y1 - y2));

	//return val;
	return val / max(1e-3f, H.z*H.z*H.z*H.z); // Jacobian hpp-> h

			// const int numSample = 10;
			// float hs = hpp.x - rectFp.x; //-> hs + rectFp.x;
			// float ht = hpp.y - rectFp.y;
			// float stepS = 2 * rectFp.x / numSample;
			// float stepT = 2 * rectFp.y / numSample;
			// float D_GGXFiltered = 0.0f;
			// vec3 h;
			// for(int i = 0; i < numSample; i++)
			// 	{
			// 		for(int j = 0; j < numSample; j++)
			// 		{
			// 			//calculate D(s,t)
			// 			h = normalize(vec3(hs, ht, 1.0f));
			// 			D_GGXFiltered += DistributionGGX(N, h, roughness);
			// 			ht += stepT;
			// 		}
			// 		hs += stepS;
			// 	}
			// D_GGXFiltered = D_GGXFiltered / (numSample * numSample);
			//n = normalize(vec3(hs, ht, 1.0f));
}

float GGXRectangularFilter(vec3 H, ShadingFrame shadingFrame, float roughness)
{
	float roughnessFiltered = roughness;
	vec3 hppWS = H / dot(H, shadingFrame.normal);
	vec2 hpp = vec2(dot(hppWS, shadingFrame.tangent), dot(hppWS, shadingFrame.bitangent));
	mat2 dhduv = mat2(dFdx(hpp), dFdy(hpp));
	vec2 fp = vec2(abs(dhduv[0]) + abs(dhduv[1])) * 0.5f;
	vec2 fp_min = min(vec2(roughness), vec2(1e-1f)); // why roughness??
	float fp_max = 0.3;
	fp = clamp(fp, fp_min, vec2(fp_max));
	//fp = max(fp, fp_min);

	return GGXRectIntegral(hpp, H, vec2(roughness), fp);
}

float BeckmannRectangularFilter(vec3 H, ShadingFrame shadingFrame, float roughness)
{
	vec2 roughnessFiltered = vec2(roughness);
	//-----------------------NDF Filtering-------------------------
	// compute plane-plane half vector
	//vec3 hppWS = vec3(1.0f);
	// if(dot(H, TBN[2]) > 0)
	// {
		vec3 hppWS = H / dot(H, shadingFrame.normal);
		//hpp = vec2(hs, ht);
		vec2 hpp = vec2(dot(hppWS, shadingFrame.tangent), dot(hppWS, shadingFrame.bitangent));
		// vec3 h = normalize(vec3(hpp, 1.f));

		//thanks to quad shading
		mat2 dhduv = mat2(dFdx(hpp), dFdy(hpp));

		//compute filtering rectangular region
		//clamp : make sure we do not do unnecessary overfiltering at grazing viewing angles
		//rectFp.x = dhs / 2; rectFp.y = dht / 2;
		// clamp for the maxinum to 1.0f (0.1 - 1f) roughness. to aviod a very large filter region
		// clamp for min in GGX filtering.
		// multiplication with 0.5 means the derivation of filer is 0.5 delta h, i.e. 0.5 pixel wide 
		// multiplication with 0.5 is caused by H is the center of rectangle
		vec2 rectFp = min((abs(dhduv[0]) + abs(dhduv[1])) * 0.5f, 0.3f);
		// covariance matrix of pixel filter's gaussian
		float mag = rectFp.x > rectFp.y ? rectFp.x : rectFp.y;
		float covMxMag = mag * mag * 2.f;
		vec2 covMx = rectFp * rectFp * 2.f;
		roughnessFiltered = sqrt(roughness * roughness + covMx);
		float magRoughnessFiltered = sqrt(roughness * roughness + covMxMag);

		if(!isAnisotropic)
			return DistributionBeckmann(shadingFrame.normal, H, magRoughnessFiltered);
		else
			return DistributionBeckmannAnisotropic(hpp, roughnessFiltered, 0);

	// }
	// else roughnessFiltered = roughness;

	//roughness = 0.5f;
	//------------------------Filtering end-------------------------
	//return roughnessFiltered;
}

float BeckmannParallelogramFilter(vec3 H, ShadingFrame shadingFrame, float roughness)
{
	vec2 roughnessFiltered = vec2(roughness);
	//-----------------------NDF Filtering-------------------------
	// compute plane-plane half vector
	//vec3 hppWS = vec3(1.0f);
	// if(dot(H, TBN[2]) > 0)
	// {
		vec3 hppWS = H / dot(H, shadingFrame.normal);
		//hpp = vec2(hs, ht);
		vec2 hpp = vec2(dot(hppWS, shadingFrame.tangent), dot(hppWS, shadingFrame.bitangent));
		// vec3 h = normalize(vec3(hpp, 1.f));

		//thanks to quad shading
		mat2 dhduv = mat2(dFdx(hpp), dFdy(hpp));


		vec2 p1 = min(abs(dhduv[0]), 0.3f);
		vec2 p2 = min(abs(dhduv[1]), 0.3f);
		// float length1 = dot(p1, p1);
		// float length2 = dot(p2, p2);
		// correlation = dot(p1, p2) / sqrt(length1 * length2);
		//correlation = dot(p1, p2);
		correlation = dot(dhduv[0], dhduv[1]);
		// correlation = dot(normalize(p1), normalize(p2));
		vec2 covMx = vec2(dot(p1, p1), dot(p2, p2)) * 2.f;
		//compute filtering rectangular region
		//clamp : make sure we do not do unnecessary overfiltering at grazing viewing angles
		//rectFp.x = dhs / 2; rectFp.y = dht / 2;
		// clamp for the maxinum to 1.0f (0.1 - 1f) roughness. to aviod a very large filter region
		// clamp for min in GGX filtering.
		// multiplication with 0.5 means the derivation of filer is 0.5 delta h, i.e. 0.5 pixel wide 
		// vec2 rectFp = min((abs(dhduv[0]) + abs(dhduv[1])) * 0.5f, 0.3f);
		// // covariance matrix of pixel filter's gaussian
		// vec2 covMx = rectFp * rectFp * 2.f;
		roughnessFiltered = sqrt(roughness * roughness + covMx);

		correlation = 2 * correlation / (roughnessFiltered.x * roughnessFiltered.y);

		correlation = clamp(correlation, -0.99999f, 0.99999f);
		return DistributionBeckmannAnisotropic(hpp, roughnessFiltered, correlation);

	// }
	// else roughnessFiltered = roughness;

	//roughness = 0.5f;
	//------------------------Filtering end-------------------------
	//return roughnessFiltered;
}

void main()
{
	//pixel footprint
	// mat2 dpduv = mat2(dFdx(WorldPos), dFdy(WorldPos));
	// vec2 pixelFootprint = vec2(abs(dpduv[0]), abs(dpduv[1]));
	// float F_area = pixelFootprint.x * pixelFootprint.y;

	//vec3 albedo = pow(texture(texture_diffuse1, TexCoords).rgb, vec3(2.2));
	vec3 albedo = pow(vec3(0.5f), vec3(2.2));
	if(useDiffuseTexture)
		albedo = pow(texture(texture_diffuse1, TexCoords).rgb, vec3(2.2));
    // float metallic  = texture(metallicMap, TexCoords).r;
    // float roughness = texture(roughnessMap, TexCoords).r;
    float ao = 1;

	//usual calculations
	//vec3 N = normalize(Normal);
	vec3 V = normalize(camPos - WorldPos);

	vec3 N = normalize(Normal);
    if(normalMapping)
	{
		N = getNormalFromMap();
	}

	//re-orthogonal
	vec3 T = TBN[0];
	T = normalize(T - dot(T, N) * N);

	shadingFrame.normal = N;
	shadingFrame.tangent = T;
	shadingFrame.bitangent = normalize(cross(N, T));

	//calculate F
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);//mix???


	vec3 Lo = vec3(0.0);
	int i = 0;
	// for(int i = 0; i < 4; i++)
	// {
		//get the radiance of specific light
		vec3 L = normalize(lightPositions[i] - WorldPos);
		vec3 H = normalize(V + L);
		float distance = length(lightPositions[i] - WorldPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColors[i] * attenuation;

		//COOK-TORRANCE BRDF
		//float NDF = DistributionGGX(N, H, roughness);
		//float NDF = GGXRectangularFilter(H, shadingFrame, roughness);
		//float NDF = DistributionBeckmann(N, H, roughness);
		float NDF;
		if(isRectangle)
			NDF = BeckmannRectangularFilter(H, shadingFrame, roughness);
		else
			NDF = BeckmannParallelogramFilter(H, shadingFrame, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		//~~~~~~~~~~~~~ why dot(H, V) ???? dot(N, V) ~~~~~~~~~~~~~~~~~~~~
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

		//||||vec3||||| nullifying kD if the surface is metallic
		 vec3 kD = 1 - F;
		 kD *= 1.0 - metallic;

		vec3 nominator = NDF * G * F;
		// L = wi, V = wo
		//~~~~~~~~~~~~~~~~~ prevent divide by zero!!!!!~~~~~~~~~~~~~~~~~~~~~~
		float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		vec3 brdf = nominator / denominator;

		float NdotL = max(dot(N, L), 0.0);
		//Lambertian
		//Lo += (albedo / PI) * radiance * NdotL;
		//pbr
		//Lo += brdf * radiance * NdotL;
		Lo += (kD * albedo / PI + brdf) * radiance * NdotL; 
	//}

	vec3 ambient = vec3(0.03) * albedo * ao;
	vec3 color = ambient + Lo;

	// color = color / (color + vec3(1.0));
	// color = pow(color, vec3(1.0/2.2));

	// correlation = (correlation + 1) / 2;
	// if(correlation > 1.0f || correlation < 0.0f)
	// FragColor = vec4(1.0, 0.0, 0.0, 1.0f);
	// else
	// FragColor = vec4(vec3(correlation), 1.0f);
	FragColor = vec4(color, 1.0f);


	//FragColor = vec4(TBN[0], 1.0f);
	//FragColor = vec4(GGXRectangularFilter(H, shadingFrame, roughness), 1.0f);


}