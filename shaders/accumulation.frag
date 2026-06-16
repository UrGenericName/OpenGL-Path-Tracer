#version 430 core

out vec4 FragColor;

in vec3 color;

layout(rgba32f, binding = 2) uniform image2D frameBuffer;

void main()
{
	
	ivec2 pixelCoords = ivec2(gl_FragCoord.xy);

	FragColor = imageLoad(frameBuffer, pixelCoords);

}