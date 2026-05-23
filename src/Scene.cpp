#include "Scene.h"

Scene::Scene() {}

Scene::~Scene() {

	for (Mesh* mesh : meshCollection) {
		delete mesh;
	}

}

void Scene::Draw(Shader& shader, Camera& camera) {

	for (Mesh* mesh : meshCollection) {
		mesh->Draw(shader, camera);
	}

}

GLuint Scene::createSSBO() {
	return 0;
}