#include "Mesh.h"

using namespace std;

Mesh::Mesh(std::vector<Vertex>& i_vertices, std::vector<GLuint>& i_indices, Material* i_material, float i_emissive, glm::vec3 i_position, glm::vec3 i_rotation, glm::vec3 i_scale) {

	position = i_position;
	rotation = i_position;
	i_scale = i_position;

	vertices = i_vertices;
	indices = i_indices;

	MeshSetup();

	material = i_material;
	emissive = i_emissive;

}

Mesh::Mesh(std::vector<Vertex>& i_vertices, std::vector<GLuint>& i_indices, float i_emissive) {

	vertices = i_vertices;
	indices = i_indices;

	MeshSetup();

	material = new Material();
	emissive = i_emissive;

}

Mesh::Mesh(string fileName, Material* i_material, glm::vec3 importColor, float i_emissive, glm::vec3 i_position, glm::vec3 i_rotation, glm::vec3 i_scale) {

	position = i_position;
	rotation = i_position;
	i_scale = i_position;

	importObj(fileName, importColor);
	MeshSetup();

	material = i_material;
	emissive = i_emissive;

}

Mesh::Mesh(string fileName, glm::vec3 importColor, float i_emissive) {

	importObj(fileName, importColor);
	MeshSetup();

	material = new Material();
	emissive = i_emissive;

}

Mesh::~Mesh() {

	VAO.Delete();
	VBOptr->Delete();
	EBOptr->Delete();
	delete material;

}

void Mesh::MeshSetup() {

	// VAO (vertex array object) stores the configuration settings for vertex attributes (what each value might mean, how long a vertex is etc.)
	VAO.Bind();

	// VBO (vertex buffer object) allocates bytes for the vertex data in VRAM
	VBOptr = new VBO(vertices);

	// EBO (element buffer object) allocates bytes for the indices in VRAM (tells OpenGL which vertices need to be connected together to form a triangle)
	EBOptr = new EBO(indices);

	//			POS		  COLOR		NORMAL      UV
	// VAO : [x, y, z,   r, g, b,   x, y, z,   u, v,]
	VAO.LinkAttribute(*VBOptr, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);	// defines the position (x, y, z, 0.0f); 4 floats with an offset of 0
	VAO.LinkAttribute(*VBOptr, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(4 * sizeof(float)));	// defines the color (r, g, b, 0.0f); 4 floats with an offset of 4 floats
	VAO.LinkAttribute(*VBOptr, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(8 * sizeof(float)));
	VAO.LinkAttribute(*VBOptr, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(12 * sizeof(float)));

	// Unbind them, since the constructor binds them automatically
	VAO.Unbind();
	(*VBOptr).Unbind();
	(*EBOptr).Unbind();

}

void Mesh::Draw(Shader& shader, GLuint currentMesh, std::vector<glm::vec4> meshTextures) {

	shader.Activate();
	VAO.Bind();

	GLuint modelMatrixLoc = glGetUniformLocation(shader.ID, "u_modelMatrix");
	glUniformMatrix4fv(modelMatrixLoc, 1, GL_FALSE, glm::value_ptr(getModelMatrix()));

	// Uniforms for the index location of each texture
	GLuint albedoUniformLoc = glGetUniformLocation(shader.ID, "u_albedo");
	glUniform1ui(albedoUniformLoc, meshTextures[currentMesh].x);

	GLuint normalUniformLoc = glGetUniformLocation(shader.ID, "u_normal");
	glUniform1ui(normalUniformLoc, meshTextures[currentMesh].y);

	GLuint roughnessUniformLoc = glGetUniformLocation(shader.ID, "u_roughness");
	glUniform1ui(roughnessUniformLoc, meshTextures[currentMesh].z);

	GLuint metallicUniformLoc = glGetUniformLocation(shader.ID, "u_metallic");
	glUniform1ui(metallicUniformLoc, meshTextures[currentMesh].w);


	// Other unfiorms
	GLuint emissiveUniformLoc = glGetUniformLocation(shader.ID, "u_emissive");
	glUniform1f(emissiveUniformLoc, static_cast<GLfloat>(emissive) );

	GLuint currentMeshLoc = glGetUniformLocation(shader.ID, "u_currentMesh");
	glUniform1ui(currentMeshLoc, currentMesh);

	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

}

bool Mesh::importObj(string fileName, glm::vec3 importColor) {

	this->fileName = fileName;
	this->tint = importColor;

	ifstream file(fileName);

	if (file.is_open()) {

		vertices.clear();	// deletes all data in vertices/indices
		indices.clear();	

		std::vector<glm::vec4> parsedVertices;
		std::vector<glm::vec4> parsedNormals;
		std::vector<glm::vec2> parsedUVs;

		string line;

		// Parses components
		while (getline(file, line)) {
			
			// Parses vertices
			if (line.substr(0, 2) == "v ") {

				int i = 2;
				string x, y, z;

				// parses x
				while(i < line.length()) {

					if (line[i] == ' ') { ++i; break; };
					x += line[i++];

				}

				// parses z
				while (i < line.length()) {

					if (line[i] == ' ') { ++i; break; };
					z += line[i++];

				}
				
				// parses y
				while (i < line.length()) {

					y += line[i++];

				}

				// for some reason we have to invert the y
				parsedVertices.push_back(glm::vec4(stof(x), -stof(y), stof(z), 1.0f));
				continue;
			}

			// Parses normals
			if (line.substr(0, 3) == "vn ") {

				int i = 3;
				string x, y, z;

				// parses x
				while (i < line.length()) {

					if (line[i] == ' ') { ++i; break; };
					x += line[i++];

				}

				// parses z
				while (i < line.length()) {

					if (line[i] == ' ') { ++i; break; };
					z += line[i++];

				}

				// parses y
				while (i < line.length()) {

					y += line[i++];

				}

				parsedNormals.push_back(glm::vec4(stof(x), -stof(y), stof(z), 1.0f));
				continue;
			}

			// Parses UVs
			if (line.substr(0, 3) == "vt ") {

				int i = 3;
				string u, v;

				// parses u
				while (i < line.length()) {

					if (line[i] == ' ') { ++i; break; };
					u += line[i++];

				}

				// parses v
				while (i < line.length()) {

					v += line[i++];
				}

				parsedUVs.push_back(glm::vec2(stof(u), stof(v)));
				continue;
			}

			// Parses faces
			if (line.substr(0, 2) == "f ") {

				int i = 2;
				int j = 0;
				vector<string> faceParse = { "", "", "" };

				// parses ["*/*/*", "*/*/*", "*/*/*"] into faceParse
				while (i < line.length()) {

					if (line[i] == ' ') { ++j; ++i; continue; };
					faceParse[j].push_back(line[i++]);

				}

				// splits the strings in faceParse into actual OpenGL vertices, which are then pushed to the member "vertices"
				for (i = 0; i < 3; ++i) {

					// For each face, find the vertex, UV, and normal index it needs
					j = 0;
					int k = 0;
					vector<string> faceIndicesParse = { "", "", "" };
					while (j < faceParse[i].length()) {

						if (faceParse[i][j] == '/') { ++k; ++j; continue; };
						faceIndicesParse[k].push_back(faceParse[i][j++]);

					}

					// after again parsing actual values in faceIndicesParse, we can then actually grab the index for each component
					int vertexIndex = stoi(faceIndicesParse[0]) - 1;
					int UVIndex = stoi(faceIndicesParse[1]) - 1;
					int normalIndex = stoi(faceIndicesParse[2]) - 1;

					Vertex constructedOpenGLVertex(parsedVertices[vertexIndex], glm::vec4(tint, 1.0f), parsedNormals[normalIndex], parsedUVs[UVIndex]);
					//printf("{ [%f, %f, %f], [%f, %f, %f], [%f, %f] }\n", constructedOpenGLVertex.position.x, constructedOpenGLVertex.position.y, constructedOpenGLVertex.position.z, constructedOpenGLVertex.normal.x, constructedOpenGLVertex.normal.y, constructedOpenGLVertex.normal.z, constructedOpenGLVertex.texUV.x, constructedOpenGLVertex.texUV.y);
					vertices.push_back(constructedOpenGLVertex);

				}
				continue;
			}

		}
	
		// generates indices; because we're generating a unique vertex for every face, this is trival since its really just [0, 1, 2, 3, 4 ...]
		for (int i = 0; i < vertices.size(); ++i) {
			indices.push_back(i);
		}

		return true;

	} else {
		return false;
	}
}

void Mesh::updateBuffers() {


	for (int i = 0; i < vertices.size(); ++i) {
		vertices[i].color = glm::vec4(tint, 1.0f);
	}

	VBOptr->Update(vertices); // updates the vertices stored in the VBO
	EBOptr->Update(indices); // updates the indices stored in the EBO

}

glm::mat4 Mesh::getModelMatrix() {

	// TRANSLATION
	glm::mat4 translationMatrix{
		1.0f,		0.0f,		0.0f,		0.0f,
		0.0f,		1.0f,		0.0f,		0.0f,
		0.0f,		0.0f,		1.0f,		0.0f,
		position.x,	position.y,	position.z,	1.0f
	};

	// ROTATION
	float cosTheta, sinTheta;

	cosTheta = cos(rotation.x);
	sinTheta = sin(rotation.x);
	glm::mat4 rotationX {
		1.0f,	0.0f,		0.0f,		0.0f,
		0.0f,	cosTheta,	sinTheta,	0.0f,
		0.0f,	-sinTheta,	cosTheta,	0.0f,
		0.0f,	0.0f,		0.0f,		1.0f
	};

	cosTheta = cos(rotation.y);
	sinTheta = sin(rotation.y);
	glm::mat4 rotationY{
		cosTheta,	0.0f,	-sinTheta,	0.0f,
		0.0f,		1.0f,	0.0f,		0.0f,
		sinTheta,	0.0f,	cosTheta,	0.0f,
		0.0f,		0.0f,	0.0f,		1.0f,
	};

	cosTheta = cos(rotation.z);
	sinTheta = sin(rotation.z);
	glm::mat4 rotationZ{
		cosTheta,	sinTheta,	0.0f,	0.0f,
		-sinTheta,	cosTheta,	0.0f,	0.0f,
		0.0f,		0.0f,		1.0f,	0.0f,
		0.0f,		0.0f,		0.0f,	1.0f
	};

	glm::mat4 rotationMatrix{ rotationZ * rotationY * rotationX };

	// SCALE
	glm::mat4 scaleMatrix{
		scale.x,	0.0f,		0.0f,		0.0f,
		0.0f,		scale.y,	0.0f,		0.0f,
		0.0f,		0.0f,		scale.z,	0.0f,
		0.0f,		0.0f,		0.0f,		1.0f
	};

	modelMatrix = translationMatrix * rotationMatrix * scaleMatrix;
	return modelMatrix;
}