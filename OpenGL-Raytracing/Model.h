#pragma once

#include <iostream>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <assimp\Importer.hpp>
#include <assimp\scene.h>
#include <assimp\postprocess.h>

#include "Mesh.h"

struct Vertex
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 texcoord;
	glm::vec3 normal;

};

class Model
{
public:
	Model();
	~Model();

	void Load(const char* filepath);

	std::vector<glm::vec3> GetVertices() { return m_vertices; };
	std::vector<unsigned int> GetIndices() { return m_indices; };

	void Move(const glm::vec3 position);
	void Rotate(const glm::vec3 rotation);
	void Scale(const glm::vec3 scale);

	void SetPosition(const glm::vec3 position);
	void SetRotation(const glm::vec3 rotation);
	void SetScale(const glm::vec3 scale);
	void SetOrigin(const glm::vec3 origin);

private:

	void LoadNode(aiNode* node, const aiScene* scene);
	void LoadMesh(aiMesh* mesh, const aiScene* scene);

	std::vector<glm::vec3> m_vertices;
	std::vector<unsigned int> m_indices;

};