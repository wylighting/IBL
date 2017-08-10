#version 330 core
out vec4 color;

in vec2 TexCoords;


uniform sampler2D image;
uniform int iteration;

uniform vec2 streakDirection;
uniform float attenuation;
uniform int streakSamples;

void main()
{

	vec2 nStreakDirection = normalize(streakDirection);

	vec2 pixelSize = 1.0 / textureSize(image, 0);

	vec2 texCoordSample;
	vec3 result = vec3(0.0f);

	float b = pow(streakSamples, iteration);

	for (int s = 0; s < streakSamples; s++)
	{
		float weight = pow(attenuation, b * s);

		texCoordSample = TexCoords + (nStreakDirection * b * vec2(s, s) * pixelSize);

		//weight is ought to clamp?
		result += weight * texture(image, texCoordSample).rgb;

		texCoordSample = TexCoords - (nStreakDirection * b * vec2(s, s) * pixelSize);
		result += weight * texture(image, texCoordSample).rgb;
	}

	color = vec4(result / 2, 1.0f);
}