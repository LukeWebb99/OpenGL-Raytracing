#pragma once
#include "Entity.h"
#include <GL/glew.h>

class Mesh : public Entity
{
public:
	Mesh();
	~Mesh();

	void Create(GLfloat* vertices, unsigned int *indices, unsigned int vertexCount, unsigned int indicesCount);
	void Render();
	void Clear();

private:

	GLuint VAO, VBO, IBO;
	GLsizei indexCount;

};

