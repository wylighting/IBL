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


//use texture from model.H
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_normal1;

//using texture map

// uniform sampler2D albedoMap;
// uniform sampler2D normalMap;

// all defined manually in main function
// uniform sampler2D metallicMap;
// uniform sampler2D roughnessMap;
// uniform sampler2D aoMap;

uniform vec3 camPos;
uniform float exposure;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

uniform bool normalMapping;

uniform bool useDiffuseTexture;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap()
{
    vec3 tangentNormal = texture(texture_normal1, TexCoords).xyz * 2.0 - 1.0;

    // vec3 Q1  = dFdx(WorldPos);
    // vec3 Q2  = dFdy(WorldPos);
    // vec2 st1 = dFdx(TexCoords);
    // vec2 st2 = dFdy(TexCoords);

    // vec3 N   = normalize(Normal);
    // vec3 T  = normalize(Q1*st2.t - Q2*st1.t);

    // //T = normalize(T - dot(T, N) * N);
    // //vec3 B  = -normalize(cross(N, T));
    // //vec3 B = normalize(Q1*st2.s - Q2*st1.s);

    // //get new tangent 
    // // vec3 X = cross(N, T);
    // // T = normalize(cross(X, N));
    // // //get updated bitanget
    // // X = cross(B, N);
    // // B = normalize(cross(N, X));
    // vec3 B  = normalize(cross(N, T));

    // TBN = mat3(T, B, N);

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


void main()
{
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

	//calculate F
	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);//mix???


	vec3 Lo = vec3(0.0);
	int i = 0;
	// for(int i = 0; i < 4; i++)
	// {
		//get the radiance of specific light
		vec3  L = normalize(lightPositions[i] - WorldPos);
		vec3 H = normalize(V + L);
		float distance = length(lightPositions[i] - WorldPos);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColors[i] * attenuation;

		//COOK-TORRANCE BRDF
		//float NDF = DistributionGGX(N, H, roughness);
		float NDF = DistributionBeckmann(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);
		//~~~~~~~~~~~~~ why dot(H, V) ???? dot(N, V) ~~~~~~~~~~~~~~~~~~~~
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

		//||||vec3||||| nullifying kD if the surface is metallic
		 vec3 kD = 1 - F;
		 kD *= 1.0 - metallic;

		 vec3 nominator = NDF * G * F;
		//vec3 nominator = NDF * vec3(1.0f);
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

	//mat3 test = TBN;
	// FragColor = vec4(color, 1.0f);

	FragColor = vec4(color, 1.0f);


}