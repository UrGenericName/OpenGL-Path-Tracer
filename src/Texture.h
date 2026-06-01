#pragma once

#include <glad/glad.h>
#include <stb/stb_image.h>
#include <filesystem>
#include <string>

#include "shaderClass.h";

#define DEFAULT_TEX_TYPE	GL_TEXTURE_2D
#define DEFAULT_SLOT		GL_TEXTURE0
#define DEFAULT_FORMAT		GL_RGB
#define DEFAULT_PIXEL_TYPE	GL_UNSIGNED_BYTE

class Texture {
public:
	GLuint ID;
	GLenum type;
	Texture(const char* image, GLenum texType = DEFAULT_TEX_TYPE, GLenum slot = DEFAULT_SLOT, GLenum format = DEFAULT_FORMAT, GLenum pixelType = DEFAULT_PIXEL_TYPE);

	void texUnit(Shader &shader, const char* uniform, GLuint unit);
	void Bind();
	void Unbind();
	void Delete();

	static void loadTextureArray(GLuint textureArrayID, std::vector<std::string>& textures, int width, int height);
};