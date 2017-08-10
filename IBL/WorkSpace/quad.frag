#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D colorBuffer;
uniform sampler2D colorBlurX;
uniform sampler2D colorBlurY;

/*
    Post-process Star Filter.
    Code from NVIDIA Shader Library
    http://developer.download.nvidia.com/shaderlibrary/webpages/shader_library.html
 
    Quartz Composer conversion
    toneburst 2009
*/
 
//Composite pass
vec3 compBlur(sampler2D Image, sampler2D HPass, sampler2D VPass, vec2 TexCoords, float StarBright)
{
    vec3 img    = texture(Image, TexCoords).rgb;
    vec3 hBlur  = texture(HPass, TexCoords).rgb;
    vec3 vBlur  = texture(VPass, TexCoords).rgb;
     
    // Return composited images
    return img + StarBright * (hBlur + vBlur);
}


void main()
{
	//color = texture(colorBuffer, TexCoords);
	//color = vec4(vec3(1.0 - texture(colorBuffer, TexCoords)), 1.0);

	vec3 color = compBlur(colorBuffer, colorBlurY, colorBlurX, TexCoords, 0.1f);
    // vec3 color = texture(colorBlurX, TexCoords).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0f);
}


