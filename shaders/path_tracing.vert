#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in vec2 aTexCoord;

out vec3 color;
out vec2 texCoord;
out vec3 faceNormal;
out vec3 intersectionPoint;

uniform mat4 camMatrix;

void main()
{
	gl_Position = camMatrix * vec4(aPos, 1.0);

	color = aColor;
	texCoord = aTexCoord;
	faceNormal = aNormal;
	intersectionPoint = aPos;
}