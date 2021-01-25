#include "Texture.h"

Texture::
Texture()
{
}

Texture::Texture(const char* fileName)
{
	loadFromFile(fileName);
}

Texture::~Texture()
{
	glDeleteTextures(1, &id);
}

GLuint const Texture::getTextureID()
{
	return 0;
}

void Texture::Bind(const GLint textureUnit)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::Unbind()
{
	glActiveTexture(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::loadFromFile(const char* fileName)
{

	unsigned char* image = SOIL_load_image(fileName, &this->m_width, &this->m_height, NULL, SOIL_LOAD_RGBA);

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	//https://learnopengl.com/Getting-started/Textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	if (image) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
		std::cout << "[ERROR]: Texture loading failed {" << fileName << "}\n";

	glBindTexture(GL_TEXTURE_2D, NULL);
	SOIL_free_image_data(image);
}