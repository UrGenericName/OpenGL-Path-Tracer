#include "Mesh.h"

using namespace std;

Mesh::Mesh(std::vector<Vertex>& i_vertices, std::vector<GLuint>& i_indices, Material& i_material) {

	vertices = i_vertices;
	indices = i_indices;

	MeshSetup();

	material = &i_material;

}

Mesh::Mesh(std::vector<Vertex>& i_vertices, std::vector<GLuint>& i_indices) {

	vertices = i_vertices;
	indices = i_indices;

	MeshSetup();

	material = new Material();

}

Mesh::Mesh(const char* fileName, Material& i_material, glm::vec3 importColor) {

	importObj(fileName, importColor);
	MeshSetup();

	material = &i_material;

}

Mesh::Mesh(const char* fileName, glm::vec3 importColor) {

	importObj(fileName, importColor);
	MeshSetup();

	material = new Material();

}

Mesh::~Mesh() {

	VAO.Delete();
	VBOptr->Delete();
	EBOptr->Delete();

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
	VAO.LinkAttribute(*VBOptr, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)0);	// defines the position (x, y, z); 3 floats with an offset of 0
	VAO.LinkAttribute(*VBOptr, 1, 3, GL_FLOAT, sizeof(Vertex), (void*)(3 * sizeof(float)));	// defines the color (r, g, b); 3 floats with an offset of 3 floats
	VAO.LinkAttribute(*VBOptr, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)(6 * sizeof(float)));
	VAO.LinkAttribute(*VBOptr, 3, 2, GL_FLOAT, sizeof(Vertex), (void*)(9 * sizeof(float)));

	// Unbind them, since the constructor binds them automatically
	VAO.Unbind();
	(*VBOptr).Unbind();
	(*EBOptr).Unbind();

}

void Mesh::Draw(Shader& shader, Camera& camera) {

	shader.Activate();
	VAO.Bind();

	material->albedo->texUnit(shader, "albedo", 0);
	glActiveTexture(GL_TEXTURE0);
	material->albedo->Bind();

	material->normal->texUnit(shader, "normal", 1);
	glActiveTexture(GL_TEXTURE1);
	material->normal->Bind();

	material->roughness->texUnit(shader, "roughness", 2);
	glActiveTexture(GL_TEXTURE2);
	material->roughness->Bind();

	material->metallic->texUnit(shader, "metallic", 3);
	glActiveTexture(GL_TEXTURE3);
	material->metallic->Bind();

	camera.Matrix(shader, "camMatrix");
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

}

bool Mesh::importObj(const char* fileName, glm::vec3 importColor) {

	ifstream file(fileName);

	if (file.is_open()) {

		vertices.clear();	// deletes all data in vertices/indices
		indices.clear();	

		std::vector<glm::vec3> parsedVertices;
		std::vector<glm::vec3> parsedNormals;
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
				parsedVertices.push_back(glm::vec3(stof(x), -stof(y), stof(z)));
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

				parsedNormals.push_back(glm::vec3(stof(x), stof(y), stof(z)));
				parsedNormals[parsedNormals.size() - 1] *= -1;	// inverts normals
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

					Vertex constructedOpenGLVertex(parsedVertices[vertexIndex], importColor, parsedNormals[normalIndex], parsedUVs[UVIndex]);
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

	VBOptr->Update(vertices); // updates the vertices stored in the VBO
	EBOptr->Update(indices); // updates the indices stored in the EBO

}