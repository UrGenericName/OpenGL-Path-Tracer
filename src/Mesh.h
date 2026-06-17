#include <string>
#include <fstream>

#include "VAO.h"
#include "EBO.h"
#include "Camera.h"
#include "Material.h"

class Mesh {
public:

	float emissive = false;
	Material* material;

	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;

	VAO VAO;
	VBO* VBOptr;
	EBO* EBOptr;

	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, Material* i_material, float i_emissive = 0.0f);
	Mesh(std::vector<Vertex>& vertices, std::vector<GLuint>& indices, float i_emissive = 0.0f);
	Mesh(const char* fileName, Material* i_material, glm::vec4 importColor = { 1.0f, 1.0f, 1.0f, 0.0f }, float i_emissive = 0.0f);
	Mesh(const char* fileName, glm::vec4 importColor = { 1.0f, 1.0f, 1.0f, 0.0f }, float i_emissive = 0.0f);
	~Mesh();

	void Draw(Shader& shader, Camera& camera, GLuint currentMesh, std::vector<glm::vec4> meshHeader);
	void Draw(Shader& shader, Camera& camera);
	bool importObj(const char* fileName, glm::vec4 importColor);	// imports an obj mesh (MUST BE TRIANGLATED BEFORE IMPORT)
	void updateBuffers();

private:
	void MeshSetup();
};