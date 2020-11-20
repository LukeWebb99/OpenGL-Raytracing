#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Entity
{
public:
	Entity();
	~Entity();

	void SetTranslate(glm::vec3 translate);
	void SetRotation(glm::vec3 rotation);
	void SetScale(glm::vec3 scale);

	glm::vec3 GetTranslate();
	glm::vec3 GetRotation ();
	glm::vec3 GetScale    ();

	glm::mat4 GetModel();
	void UpdateModel();

private:
	glm::mat4 m_model;
	glm::vec3 m_translate;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;
};

