#include <string>
#include <fstream>

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "Material.h"

class Mesh {
public:

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	Material* material;

	VAO VAO;
	VBO* VBOptr;
	EBO* EBOptr;

	bool emissive = false;

	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, Material& i_material);
	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, bool i_emissive = false);
	Mesh(const char* fileName, Material& i_material, glm::vec4 importColor = { 1.0f, 1.0f, 1.0f, 0.0f });
	Mesh(const char* fileName, glm::vec4 importColor = { 1.0f, 1.0f, 1.0f, 0.0f }, bool i_emissive = false);
	~Mesh();

	void Draw(Shader& shader, Camera& camera, GLuint currentMesh);
	bool importObj(const char* fileName, glm::vec4 importColor);	// imports an obj mesh (MUST BE TRIANGLATED BEFORE IMPORT)
	void updateBuffers();

private:
	void MeshSetup();
};