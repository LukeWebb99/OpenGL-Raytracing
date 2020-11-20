#pragma once

#include <iostream>
#include <fstream>
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:

	Shader();
	~Shader();

	void CreateFromString(const char* vertexShader, const char* fragmentShader);
	void CreateFromFile(const char* vertexPath, const char* fragmentPath);
	void CreateFromFile(const char* computePath);

	std::string ReadFile(const char* path);

	GLuint GetProjectionLocation();
	GLuint GetModelLocation();

	void Bind();
	void Unbind();

	void ClearShader();
	
	void Set1f(GLfloat value, const GLchar* name);
	void Set1i(GLint value, const GLchar* name);
	void SetVec3f(glm::fvec3 value, const GLchar* name);
	void SetVec4f(glm::vec4 value, const GLchar* name);
	void SetUniformMat4f(const char* uniformName, glm::mat4 value, bool transpose);

	void QueryWorkgroups();

private:

	GLuint shaderID;
	GLuint u_Projection, u_Model;

	void CompileShader(const char* vertexShader, const char* fragmentShader);
	void CompileShader(const char* computeShader);
	void AddShader(GLuint program, const char* shaderCode, GLenum type);

};

