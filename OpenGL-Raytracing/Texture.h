#pragma once

#include <iostream>
#include <string>
#include <assert.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include <SOIL2/SOIL2.h>
#include <stb_image.h>

#include "Shader.h"
#include "Utility.h"


class Texture
{
public:
	Texture();
	Texture(const char* path);
	~Texture();

	GLuint const GetTextureID();
	void Bind(const GLint textureUnit);
	void Unbind();

	void LoadImageData();
	void LoadHDRIData();

	void CreateTexture2D();

	void LoadCubemap(const char* rightFace, const char* leftFace,
		const char* topFace, const char* bottomFace,
		const char* backFace, const char* frontFace);

	void CreateHDRI();

	void CreateCubemapFromHDRI(Texture HDRI);
	void CreateIrradianceTexture(Texture* Cubemap);
	void CreatePrefilterMap(Texture* environmentMap);
	void CreateBRDFLookUpTable();

private:

	GLuint m_id;
	int m_width;
	int m_height;
	int m_components;
	int m_textureType;
	const char* m_path;
	unsigned char* m_image2D;
	float* m_imageHDRI;
	unsigned int m_captureFBO, m_captureRBO;

};