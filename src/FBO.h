#pragma once

#include <glad/glad.h>
#include "Texture.h"

class FBO {
public:
	GLuint ID;
	Texture* texture;

	FBO(unsigned int width, unsigned int height, GLenum slot);

	void Bind();
	void Unbind();
	void Delete();
};