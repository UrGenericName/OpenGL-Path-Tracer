#version 430 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec4 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec3 color;

uniform mat4 u_camMatrix;
uniform mat4 u_modelMatrix;

void main()
{
	gl_Position = u_camMatrix * u_modelMatrix * aPos;

	color = aColor.xyz;

}