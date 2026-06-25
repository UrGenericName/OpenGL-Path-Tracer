#pragma once

#include <glm/glm.hpp>
#include <glad/glad.h>

struct Vertex {

	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 normal;
	glm::vec2 texUV;
	glm::vec2 _pad;

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