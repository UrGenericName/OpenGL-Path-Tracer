#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

struct Vertex {

	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 normal;
	glm::vec2 texUV;

};

class VBO {
public:
	GLuint ID;
	VBO(std::vector<Vertex>& vertices);

	void Update(std::vector<Vertex>& vertices);
	void Bind();
	void Unbind();
	void Delete();
};