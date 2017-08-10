#version 330 core
out vec4 BrightColor;

in vec2 TexCoords;

uniform sampler2D image;

void main()
{
	vec3 result = texture(image, TexCoords).rgb;

	// is not in a hdr domain....
	float brightness = dot(result, vec3(0.2126, 0.7152, 0.0722));
    
    // if(brightness > 0.5)
        BrightColor = vec4(result, 1.0);
       // else
       // BrightColor = vec4(result, 1.0f);
}