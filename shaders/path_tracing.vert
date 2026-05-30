#version 430 core
layout (location = 0) in vec4 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec4 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec3 color;
out vec2 texCoord;
out vec3 geometricFaceNormal;
out vec3 intersectionPoint;

uniform mat4 camMatrix;
uniform vec3 camPos;

void main()
{
	gl_Position = camMatrix * vec4(aPos);

	color = aColor.xyz;
	texCoord = aTexCoord;
	geometricFaceNormal = aNormal.xyz;
	intersectionPoint = aPos.xyz;
}