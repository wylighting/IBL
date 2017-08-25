#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
//layout (location = 2) in vec2 aTexCoords;

layout (location = 2) in vec3 aColor;

// 1. Push Color per vertex
// Qs:

// 2. Push Transfer coeffs per vertex
// How to bind 9 coeffs for each vertex?
// use mat3x3 ?

// 3. Calculate transfer coeffs in Vertex Shader
// Need to push every sample's info into vs.
// Of course, as a precomtutation process, it cannot be calculated at runtime.
// After this GPU precomputation pass, we can select 1 & 2 for the next.

//out vec2 TexCoords;
out vec3 WorldPos;
out vec3 Normal;

out vec3 Color;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main()
{
    //TexCoords = aTexCoords;
    WorldPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(model) * aNormal;   
    Normal = normalize(aNormal);
    gl_Position =  projection * view * vec4(WorldPos, 1.0);

	Color = aColor;
}