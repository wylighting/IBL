#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoords;
layout (location = 3) in vec3 tangent;
layout (location = 4) in vec3 bitangent;

//status variables in world coordinate
//provide data for calculations in fragment shader
out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;
out mat3 TBN;

//vertex transformation
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main()
{
	TexCoords = texCoords;
	WorldPos = vec3(model * vec4(position, 1.0f));
	Normal = mat3(model) * normal;

	mat3 normalMatrix = transpose(inverse(mat3(model)));
	vec3 T = normalize(normalMatrix * tangent);
	vec3 B = normalize(normalMatrix * bitangent);
	vec3 N = normalize(normalMatrix * normal);
	T = normalize(T - dot(T, N) * N);
	//B = normalize(cross(N, T));

	TBN = mat3(T, B, N);

	gl_Position = projection * view * model * vec4(position, 1.0f);
}