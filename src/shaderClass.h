#pragma once

#include <glad/glad.h>
#include <string>

std::string get_file_contents(const char* filename);

class Shader {
public:
	GLuint ID;
	Shader(const char* vertexFile, const char* fragmentFile);
	~Shader();

	void Activate();
	void Delete();

private:
	void compileErrors(unsigned int shader, const char* type);
};