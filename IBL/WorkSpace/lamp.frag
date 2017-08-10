#version 330 core
out vec4 color;

in vec3 Normal;

// lights
uniform vec3 lightColor;

void main()
{
	//vec3 colorLDR = lightColor / (lightColor + vec3(1.0));
	color = vec4(lightColor, 1.0f);
}