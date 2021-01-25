#pragma once

#include <iostream>
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <SOIL2/SOIL2.h>

class Texture
{
public:
	Texture();
	Texture(const char* fileName);
	~Texture();

	GLuint const getTextureID();
	void Bind(const GLint textureUnit);
	void Unbind();
	void loadFromFile(const char* fileName);
	void LoadCubeMap(const char* fileName);

private:
	GLuint id;
	int m_width;
	int m_height;
};