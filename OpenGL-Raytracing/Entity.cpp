#include "Entity.h"

Entity::Entity()
{
	m_model = glm::mat4(1.f);
	m_translate = glm::vec3(0.f, 0.f, 0.f);
	m_rotation  = glm::vec3(0.f, 0.f, 0.f);
	m_scale     = glm::vec3(1.f, 1.f, 1.f);
}

Entity::~Entity()
{

}

void Entity::SetTranslate(glm::vec3 translate)
{
	m_translate = translate;
}

void Entity::SetRotation(glm::vec3 rotation)
{
	m_rotation = rotation;
}

void Entity::SetScale(glm::vec3 scale)
{
	m_scale = scale;
}

glm::vec3 Entity::GetTranslate()
{
	return m_translate;
}

glm::vec3 Entity::GetRotation()
{
	return m_rotation;
}

glm::vec3 Entity::GetScale()
{
	return m_scale;
}

glm::mat4 Entity::GetModel()
{
	return m_model;
}

void Entity::UpdateModel()
{
	m_model = glm::mat4(1.f);
	m_model = glm::translate(m_model, m_translate);
	
	m_model = glm::rotate(m_model, glm::radians(m_rotation.x), glm::vec3(1.f, 0.f, 0.f));
	m_model = glm::rotate(m_model, glm::radians(m_rotation.y), glm::vec3(0.f, 1.f, 0.f));
	m_model = glm::rotate(m_model, glm::radians(m_rotation.z), glm::vec3(0.f, 0.f, 1.f));

	m_model = glm::scale(m_model, m_scale);
}
