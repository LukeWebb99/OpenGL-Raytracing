#include "Texture.h"

//view and projection matrix for prospective
const glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
const glm::mat4 captureViews[] = {
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
   glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
};

Texture::Texture() {

	m_id = NULL;
	m_width = NULL;
	m_height = NULL;
	m_textureType = NULL;
	m_captureFBO = NULL;
	m_captureRBO = NULL;
	m_components = NULL;
	m_path = nullptr;
	m_image2D = nullptr;
	m_imageHDRI = nullptr;
}

Texture::Texture(const char* path)
{
	m_id = NULL;
	m_width = NULL;
	m_height = NULL;
	m_textureType = GL_TEXTURE_2D;
	m_captureFBO = NULL;
	m_captureRBO = NULL;
	m_components = NULL;
	m_path = path;
	m_image2D = nullptr;
	m_imageHDRI = nullptr;
}

Texture::~Texture()
{
	glDeleteTextures(1, &m_id);
}

GLuint const Texture::GetTextureID()
{
	return m_id;
}

void Texture::Bind(const GLint textureUnit)
{
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(this->m_textureType, m_id);
}

void Texture::Unbind()
{
	glActiveTexture(0);
	glBindTexture(m_textureType, 0);
}

void Texture::LoadImageData()
{
	m_image2D = SOIL_load_image(m_path, &this->m_width, &this->m_height, NULL, SOIL_LOAD_RGBA);
}

void Texture::LoadHDRIData()
{
	stbi_set_flip_vertically_on_load(true);
	m_imageHDRI = stbi_loadf(m_path, &this->m_width, &this->m_height, &this->m_components, NULL);
}

void Texture::CreateTexture2D()
{
	LoadImageData();
	if (!m_image2D) {

		if (!m_path)
			std::cout << "[ERROR]: No image path to load \n";
		else
			std::cout << "[ERROR]: Failed to load texture " << '"' << m_path << '"' << "\n";
	}
	else {
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);

		//https://learnopengl.com/Getting-started/Textures
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, m_image2D);

		glGenerateMipmap(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, NULL);


	}
	SOIL_free_image_data(m_image2D);
}

void Texture::LoadCubemap(const char* rightFace, const char* leftFace,
	const char* topFace, const char* bottomFace,
	const char* backFace, const char* frontFace) {

	std::string imagePaths[6];
	imagePaths[0] = rightFace;
	imagePaths[1] = leftFace;
	imagePaths[2] = topFace;
	imagePaths[3] = bottomFace;
	imagePaths[4] = backFace;
	imagePaths[5] = frontFace;

	m_textureType = GL_TEXTURE_CUBE_MAP;
	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

	int width, height;

	for (size_t i = 0; i < 6; i++) {

		unsigned char* image = SOIL_load_image(imagePaths[i].c_str(), &width, &height, NULL, SOIL_LOAD_RGB);

		if (!image) {
			std::cout << "[ERROR]: Failed to load cubemap " << '"' << imagePaths[i].c_str() << '"' << "\n";
		}

		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		SOIL_free_image_data(image);

	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_2D, NULL);

}

void Texture::CreateHDRI() {

	m_textureType = GL_TEXTURE_2D;

	this->LoadHDRIData();
	if (!m_imageHDRI) {

		if (!m_path)
			std::cout << "[ERROR]: No HDRI image path to load \n";
		else
			std::cout << "[ERROR]: Failed to load texture " << '"' << m_path << '"' << "\n";
	}
	else {

		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, m_width, m_height, 0, GL_RGB, GL_FLOAT, m_imageHDRI);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


		glBindTexture(GL_TEXTURE_2D, NULL);

	}
	stbi_image_free(m_imageHDRI);

}

void Texture::CreateCubemapFromHDRI(Texture HDRI) {

	this->m_textureType = GL_TEXTURE_CUBE_MAP;

	//Create Shader for converting;
	Shader equirectangularToCubemapShader;
	equirectangularToCubemapShader.CreateFromFile("Shaders/EquirectangularToCubemapVert.glsl", "Shaders/EquirectangularToCubemapFrag.glsl");

	//create framebuffer and renderbuffer
	glGenFramebuffers(1, &m_captureFBO);
	glGenRenderbuffers(1, &m_captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);

	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO);

	//create empty cubemap to place results into
	glGenTextures(1, &this->m_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, this->m_id);
	for (unsigned int i = 0; i < 6; i++)
	{
		// note that we store each face with 16 bit floating point values
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 512, 512, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	// convert HDR equirectangular environment map to cubemap equivalent
	HDRI.Bind(0);
	equirectangularToCubemapShader.Bind();
	equirectangularToCubemapShader.Set1i(0, "u_equirectangularMap");
	equirectangularToCubemapShader.SetMat4f("u_projection", captureProjection, false);
	
	glViewport(0, 0, 512, 512);
	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	for (unsigned int i = 0; i < 6; i++)
	{

		equirectangularToCubemapShader.SetMat4f("u_view", captureViews[i], false);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, this->m_id, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteBuffers(1, &m_captureFBO);
	glDeleteRenderbuffers(1, &m_captureRBO);
}

void Texture::CreateIrradianceTexture(Texture* Cubemap) {

	this->m_textureType = GL_TEXTURE_CUBE_MAP;

	Shader IrradianceConvolutionShader;
	IrradianceConvolutionShader.CreateFromFile("Shaders/SkyboxVert.glsl", "Shaders/IrradianceConvolutionFrag.glsl");
	IrradianceConvolutionShader.Bind();

	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

	for (size_t i = 0; i < 6; i++)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 32, 32, 0, GL_RGB, GL_FLOAT, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &m_captureFBO);
	glGenRenderbuffers(1, &m_captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 32, 32);

	Cubemap->Bind(0);
	IrradianceConvolutionShader.Bind();
	IrradianceConvolutionShader.Set1i(0, "u_environmentMap");
	IrradianceConvolutionShader.SetMat4f("u_projectionMatrix", captureProjection, false);

	glViewport(0, 0, 32, 32);
	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	for (unsigned int i = 0; i < 6; i++)
	{
		IrradianceConvolutionShader.SetMat4f("u_viewMatrix", captureViews[i], false);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_id, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		renderCube();
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteBuffers(1, &m_captureFBO);
	glDeleteRenderbuffers(1, &m_captureRBO);

}

void Texture::CreatePrefilterMap(Texture* Cubemap) {

	this->m_textureType = GL_TEXTURE_CUBE_MAP;

	Shader prefilterShader;
	prefilterShader.CreateFromFile("Shaders/SkyboxVert.glsl", "Shaders/PrefilterFrag.glsl");

	glBindTexture(GL_TEXTURE_CUBE_MAP, Cubemap->GetTextureID());
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);

	glBindTexture(GL_TEXTURE_CUBE_MAP, m_id);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, 128, 128, 0, GL_RGB, GL_FLOAT, nullptr);
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	prefilterShader.Bind();
	prefilterShader.Set1i(0, "u_environmentMap");
	prefilterShader.SetMat4f("u_projectionMatrix", captureProjection, false);
	Cubemap->Bind(0);

	glGenFramebuffers(1, &m_captureFBO);
	glGenRenderbuffers(1, &m_captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);

	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = 128 * std::pow(0.5, mip);
		unsigned int mipHeight = 128 * std::pow(0.5, mip);
		glBindRenderbuffer(GL_RENDERBUFFER, m_captureFBO);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
		glViewport(0, 0, mipWidth, mipHeight);

		float roughness = (float)mip / (float)(maxMipLevels - 1);
		prefilterShader.Set1f(roughness, "u_roughness");
		for (unsigned int i = 0; i < 6; ++i)
		{
			prefilterShader.SetMat4f("u_viewMatrix", captureViews[i], false);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, m_id, mip);

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			renderCube();
		}
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDeleteBuffers(1, &m_captureFBO);
	glDeleteRenderbuffers(1, &m_captureRBO);

}

void Texture::CreateBRDFLookUpTable() {

	this->m_textureType = GL_TEXTURE_2D;

	Shader brdfShader;
	brdfShader.CreateFromFile("Shaders/BRDFShaderVert.glsl", "Shaders/BRDFShaderFrag.glsl");
	brdfShader.Bind();

	glGenTextures(1, &m_id);
	glBindTexture(GL_TEXTURE_2D, m_id);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glGenFramebuffers(1, &m_captureFBO);
	glGenRenderbuffers(1, &m_captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
	glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_id, 0);

	glViewport(0, 0, 512, 512);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	renderQuad();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &m_captureFBO);
	glDeleteRenderbuffers(1, &m_captureRBO);
	
}