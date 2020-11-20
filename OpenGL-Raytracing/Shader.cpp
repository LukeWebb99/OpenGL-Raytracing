#include "Shader.h"

Shader::Shader()
{
	shaderID = 0;
	u_Model = 0;
	u_Projection = 0;
}

Shader::~Shader()
{

}

void Shader::CreateFromString(const char* vertexShader, const char* fragmentShader)
{
	CompileShader(vertexShader, fragmentShader);
}

void Shader::CreateFromFile(const char* vertexPath, const char* fragmentPath)
{
	std::string vertexStr = ReadFile(vertexPath);
	std::string fragmentStr = ReadFile(fragmentPath);

	CompileShader(vertexStr.c_str(), fragmentStr.c_str());

}

void Shader::CreateFromFile(const char* computePath)
{
	std::string computeStr = ReadFile(computePath);
	CompileShader(computeStr.c_str());
}

std::string Shader::ReadFile(const char* path)
{
	std::string content;
	std::ifstream fileStream(path, std::ios::in);

	if (!fileStream.is_open()) {

		printf("[ERROR READING FILE]: Failed to open file {%s}\n", path);
		return "";
	}

	std::string line = "";

	while (!fileStream.eof())
	{
		std::getline(fileStream, line);
		content.append(line + "\n");
	}

	fileStream.close();
	return content;

}

GLuint Shader::GetProjectionLocation()
{
	return u_Projection;
}

GLuint Shader::GetModelLocation()
{
	return u_Model;
}

void Shader::Bind()
{
	if (shaderID != 0)
		glUseProgram(shaderID);
	else
		std::cout << "[SHADER ERROR]: Shader did not bind..\n";
}

void Shader::Unbind()
{
	glUseProgram(0);
}

void Shader::ClearShader()
{
	if (shaderID != 0) {
		glDeleteProgram(shaderID);
		shaderID = 0;
	}

	u_Projection = 0;
	u_Model = 0;
}

void Shader::Set1f(GLfloat value, const GLchar* name)
{
	glUniform1f(glGetUniformLocation(shaderID, name), value);
}

void Shader::SetUniformMat4f(const char* uniformName, glm::mat4 value, bool transpose)
{
	glUniformMatrix4fv(glGetUniformLocation(shaderID, uniformName), 1, transpose, glm::value_ptr(value));
}

void Shader::Set1i(GLint value, const GLchar* name)
{
	glUniform1i(glGetUniformLocation(this->shaderID, name), value);
}

void Shader::SetVec3f(glm::fvec3 value, const GLchar* name)
{
	glUniform3fv(glGetUniformLocation(shaderID, name), 1, glm::value_ptr(value));
}

void Shader::SetVec4f(glm::vec4 value, const GLchar* name)
{
	glUniform4fv(glGetUniformLocation(shaderID, name), 1, glm::value_ptr(value));
}

void Shader::QueryWorkgroups()
{
	int work_grp_size[3], work_grp_inv;
	// maximum global work group (total work in a dispatch)
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_size[2]);
	printf("max global (total) work group size x:%i y:%i z:%i\n", work_grp_size[0], work_grp_size[1], work_grp_size[2]);
	// maximum local work group (one shader's slice)
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &work_grp_size[0]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &work_grp_size[1]);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &work_grp_size[2]);
	printf("max local (in one shader) work group sizes x:%i y:%i z:%i\n", work_grp_size[0], work_grp_size[1], work_grp_size[2]);
	// maximum compute shader invocations (x * y * z)
	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &work_grp_inv);
	printf("max computer shader invocations %i\n", work_grp_inv);
}

void Shader::CompileShader(const char* vertexShader, const char* fragmentShader)
{
	shaderID = glCreateProgram();

	AddShader(shaderID, vertexShader, GL_VERTEX_SHADER);
	AddShader(shaderID, fragmentShader, GL_FRAGMENT_SHADER);

	GLint result = 0;
	GLchar errorLog[1024] = { 0 };

	glLinkProgram(shaderID);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, sizeof(errorLog), NULL, errorLog);
		printf("[ERROR]: Linking program failed: '%s'\n", errorLog);
		return;
	}

	glValidateProgram(shaderID);
	glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, sizeof(errorLog), NULL, errorLog);
		printf("[ERROR]: Validating program failed: '%s'\n", errorLog);
		return;
	}

	if (!shaderID) {
		printf("[ERROR]: shader wasn't complied..\n");
		return;
	}

	u_Model = glGetUniformLocation(shaderID, "u_Model");
	u_Projection = glGetUniformLocation(shaderID, "u_Projection");

}

void Shader::CompileShader(const char* computeShader)
{
	
	shaderID = glCreateProgram();

	AddShader(shaderID, computeShader, GL_COMPUTE_SHADER);

	GLint result = 0;
	GLchar errorLog[1024] = { 0 };

	glLinkProgram(shaderID);
	glGetProgramiv(shaderID, GL_LINK_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, sizeof(errorLog), NULL, errorLog);
		printf("[ERROR]: Linking program failed: '%s'\n", errorLog);
		return;
	}

	glValidateProgram(shaderID);
	glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &result);
	if (!result) {
		glGetProgramInfoLog(shaderID, sizeof(errorLog), NULL, errorLog);
		printf("[ERROR]: Validating program failed: '%s'\n", errorLog);
		return;
	}

	if (!shaderID) {
		printf("[ERROR]: shader wasn't complied..\n");
		return;
	}

}

void Shader::AddShader(GLuint program, const char* shaderCode, GLenum type)
{
	GLuint shader = glCreateShader(type);
	const char* programCode[1];
	programCode[0] = shaderCode;

	GLint codeLength[1];
	codeLength[0] = strlen(shaderCode);

	glShaderSource(shader, 1, (const GLchar* const*)programCode, codeLength);
	glCompileShader(shader);

	GLint result = 0;
	GLchar errorLog[1024] = { 0 };
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (!result) {
		glGetShaderInfoLog(shader, sizeof(errorLog), NULL, errorLog);
		printf("[ERROR]: Compiliing %d shader: '%s'\n", type, errorLog);
		return;
	}
	glAttachShader(program, shader);
}
