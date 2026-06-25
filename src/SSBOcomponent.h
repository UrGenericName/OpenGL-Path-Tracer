#pragma once

#include "Mesh.h"
#include <set>

struct MeshHeader {

	GLuint indicesStartPointer;
	GLuint indicesSize;
	float emissiveValue;

};


class SSBOcomponent {
public:

	SSBOcomponent();

	void updateVertexSSBO();
	void updateIndicesSSBO();
	void updateMeshTexturesSSBO();
	void updateMeshHeaderSSBO();
	void updateBoundingBoxesSSBO();

	void generateGlobalVertices(vector<Mesh*>& meshCollection);
	void generateGlobalIndices(vector<Mesh*>& meshCollection);
	void generateMeshTextures(vector<Mesh*>& meshCollection);
	void generateMeshHeader(vector<Mesh*>& meshCollection);
	void generateBoundingBoxes(vector<Mesh*>& meshCollection);

	void generateSSBOs(unsigned int texture_width, unsigned int texture_height);

	void deleteSSBOs();

	GLuint vertexSSBO, indicesSSBO, meshTextureSSBO, meshHeaderSSBO, textureArray, boundingBoxesSSBO;

	vector<Vertex> globalVertices;
	vector<GLuint> globalIndices;
	vector<MeshTextures> meshTextures; // <albedoIndex, normalIndex, roughnessIndex, metallicIndex>
	vector<MeshHeader> meshHeader;	// <indicesStartPointer, indicesSize, emissiveValue>
	vector<BoundingBox> boundingBoxes;

	set<string> texturePool;

};