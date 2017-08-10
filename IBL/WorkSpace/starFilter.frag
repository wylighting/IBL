#version 330 core
layout( location = 0) out vec4 colorBlurX;
layout( location = 1) out vec4 colorBlurY;

in vec2 TexCoords;

uniform sampler2D colorBuffer;

/*
    Post-process Star Filter.
    Code from NVIDIA Shader Library
    http://developer.download.nvidia.com/shaderlibrary/webpages/shader_library.html
 
    Quartz Composer conversion
    toneburst 2009
*/
 
// Tap weighs
const float wt0 = 1.0;
const float wt1 = 0.8;
const float wt2 = 0.6;
const float wt3 = 0.4;
const float wt4 = 0.2;
const float wtNorm = (wt0 + 2.0 * (wt1 + wt2 + wt3 + wt4));
 
// X-Pass (Horizontal blur)
vec4 blurX(sampler2D Image, vec2 TexCoords, float Offset)
{
    // // Tap offsets for horizontal (x-axis) blur
    // vec2 tx0 = vec2(Offset, 0.0);
    // vec2 tx1 = vec2(Offset * 2.0, 0.0);
    // vec2 tx2 = vec2(Offset * 3.0, 0.0);
    // vec2 tx3 = vec2(Offset * 4.0, 0.0);
    // //vec2 tx4 = vec2(0.0, 0.0);
    // vec2 tx5 = vec2(-Offset, 0.0);
    // vec2 tx6 = vec2(-Offset * 2.0, 0.0);
    // vec2 tx7 = vec2(-Offset * 3.0, 0.0);
    // vec2 tx8 = vec2(-Offset * 4.0, 0.0);

    vec2 tx0 = vec2(Offset);
    vec2 tx1 = vec2(Offset * 2.0);
    vec2 tx2 = vec2(Offset * 3.0);
    vec2 tx3 = vec2(Offset * 4.0);
    //vec2 tx4 = vec2(0.0, 0.0);
    vec2 tx5 = vec2(-Offset);
    vec2 tx6 = vec2(-Offset * 2.0);
    vec2 tx7 = vec2(-Offset * 3.0);
    vec2 tx8 = vec2(-Offset * 4.0);
     
    // Init output color
    vec4 outPix = texture(Image, TexCoords + tx0) * (wt1 / wtNorm);
    // Sample other taps and accumulate color
    outPix += texture(Image, TexCoords + tx1) * (wt2 / wtNorm);
    outPix += texture(Image, TexCoords + tx2) * (wt3 / wtNorm);
    outPix += texture(Image, TexCoords + tx3) * (wt4 / wtNorm);
    outPix += texture(Image, TexCoords) * (wt0 / wtNorm);
    outPix += texture(Image, TexCoords + tx5) * (wt1 / wtNorm);
    outPix += texture(Image, TexCoords + tx6) * (wt2 / wtNorm);
    outPix += texture(Image, TexCoords + tx7) * (wt3 / wtNorm);
    outPix += texture(Image, TexCoords + tx8) * (wt3 / wtNorm);
     
    // Return blurred pixel
    return outPix;
}
 
// Y-Pass (Vertical blur)
vec4 blurY(sampler2D Image, vec2 TexCoords, float Offset)
{
    // Tap offsets for vertical (y-axis) blur
    // vec2 ty0 = vec2(0.0, Offset);
    // vec2 ty1 = vec2(0.0, Offset * 2.0);
    // vec2 ty2 = vec2(0.0, Offset * 3.0);
    // vec2 ty3 = vec2(0.0, Offset * 4.0);
    // //vec2 ty4 = vec2(0.0, 0.0);
    // vec2 ty5 = vec2(0.0, -Offset);
    // vec2 ty6 = vec2(0.0, -Offset * 2.0);
    // vec2 ty7 = vec2(0.0, -Offset * 3.0);
    // vec2 ty8 = vec2(0.0, -Offset * 4.0);
 

    vec2 ty0 = vec2(-Offset, Offset);
    vec2 ty1 = vec2(-Offset * 2.0, Offset * 2.0);
    vec2 ty2 = vec2(-Offset * 3.0, Offset * 3.0);
    vec2 ty3 = vec2(-Offset * 4.0, Offset * 4.0);
    //vec2 ty4 = vec2(0.0, 0.0);
    vec2 ty5 = vec2(Offset, -Offset);
    vec2 ty6 = vec2(Offset * 2.0, -Offset * 2.0);
    vec2 ty7 = vec2(Offset * 3.0, -Offset * 3.0);
    vec2 ty8 = vec2(Offset * 4.0, -Offset * 4.0);

    // Init output color
    vec4 outPix = texture(Image, TexCoords + ty0) * (wt1 / wtNorm);
    // Sample other taps and accumulate color
    outPix += texture(Image, TexCoords + ty1) * (wt2 / wtNorm);
    outPix += texture(Image, TexCoords + ty2) * (wt3 / wtNorm);
    outPix += texture(Image, TexCoords + ty3) * (wt4 / wtNorm);
    outPix += texture(Image, TexCoords) * (wt0 / wtNorm);
    outPix += texture(Image, TexCoords + ty5) * (wt1 / wtNorm);
    outPix += texture(Image, TexCoords + ty6) * (wt2 / wtNorm);
    outPix += texture(Image, TexCoords + ty7) * (wt3 / wtNorm);
    outPix += texture(Image, TexCoords + ty8) * (wt3 / wtNorm);
     
    // Return blurred pixel
    return outPix;
}
 
// Composite pass
// vec4 compBlur(sampler2D Image, sampler2D HPass, sampler2D VPass, float StarBright)
// {
//     vec4 img    = texture(Image, TexCoords);
//     vec4 hBlur  = texture(HPass, samplerCoord(HPass));
//     vec4 vBlur  = texture(VPass, samplerCoord(VPass));
     
//     // Return composited images
//     return img + StarBright * (hBlur + vBlur);
// }


void main()
{
	//color = texture(colorBuffer, TexCoords);
	//color = vec4(vec3(1.0 - texture(colorBuffer, TexCoords)), 1.0);
	colorBlurX = blurX(colorBuffer, TexCoords, 0.001f);
	colorBlurY = blurY(colorBuffer, TexCoords, 0.001f);
}


