#version 330 core
layout (location = 0) out vec4 colorBlurX;
layout (location = 1) out vec4 colorBlurY;

in vec2 TexCoords;

uniform sampler2D inTexture;
uniform bool first;

void main()
{
	if(first)
		colorBlurX = texture(inTexture, TexCoords);
	else
		colorBlurY = texture(inTexture, TexCoords);
}