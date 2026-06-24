#pragma once

#include <string>
#include <functional>
#include "VAO.h"
#include "EBO.h"
#include "Material.h"

class Scene;

struct BoundingBox {

	glm::vec4 min;
	glm::vec4 max;

};

class Mesh {
public:

	friend Scene;

	glm::vec3 position { 0.0f, 0.0f, 0.0f };
	glm::vec3 rotation { 0.0f, 0.0f, 0.0f };
	glm::vec3 scale { 1.0f, 1.0f, 1.0f };

	float emissive = 0.0f;
	Material* material;

	string fileName;
	glm::vec3 tint;

	std::function<void(Mesh&, unsigned int)> animation = nullptr;

	Mesh(string fileName, Material* i_material, glm::vec3 importColor = { 1.0f, 1.0f, 1.0f }, float i_emissive = 0.0f, glm::vec3 i_position = glm::vec3(0.0f), glm::vec3 i_rotation = glm::vec3(0.0f), glm::vec3 i_scale = glm::vec3(0.0f));
	Mesh(string fileName, glm::vec3 importColor = { 1.0f, 1.0f, 1.0f }, float i_emissive = 0.0f);
	Mesh(const Mesh& other);
	~Mesh();

	BoundingBox getBoundingBox();
	void DrawGizmo(Shader& shader, int axis);
	void Draw(Shader& shader, GLuint currentMesh, std::vector<glm::vec4> meshHeader);
	bool importObj(string fileName, glm::vec3 importColor);	// imports an obj mesh (MUST BE TRIANGLATED BEFORE IMPORT)
	void updateBuffers();

	glm::mat4 getModelMatrix();

	glm::mat4 getTranslationMatrix();
	glm::mat4 getRotationMatrix();
	glm::mat4 getScaleMatrix();

private:

	VAO VAO;
	VBO* VBOptr;
	EBO* EBOptr;

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	void MeshSetup();
};