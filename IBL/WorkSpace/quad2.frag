#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D colorBuffer;
uniform sampler2D highLightBlured;

void main()
{
	//vec3 color = texture(colorBuffer, TexCoords).rgb;
	vec3 color = texture(highLightBlured, TexCoords).rgb;

	//tone mapping & gamma transformation
	color = color / (color + vec3(1.0f));
	color = pow(color, vec3(1 / 2.2));

	FragColor = vec4(color, 1.0f);

}