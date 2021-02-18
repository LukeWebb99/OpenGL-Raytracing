#define PI 3.14159265359
/// http://blog.three-eyed-games.com/2018/05/03/gpu-ray-tracing-in-unity-part-1/
/// http://web.cse.ohio-state.edu/~shen.94/681/Site/Slides_files/shadow.pdf

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <SOIL2/SOIL2.h>

#include "Window.h"
#include "GuiLayer.h"
#include "Shader.h"
#include "Mesh.h"
#include "Model.h"
#include "Camera.h"
#include "Texture.h"
#include "Random.hpp"

Random Random::s_Instance;

void PrintMat4(glm::mat4 m) {
	std::cout << m[0][0] << ", " << m[0][1] << ", " << m[0][2] << ", " << m[0][3] << "\n"
	          << m[1][0] << ", " << m[1][1] << ", " << m[1][2] << ", " << m[1][3] << "\n"
		      << m[2][0] << ", " << m[2][1] << ", " << m[2][2] << ", " << m[2][3] << "\n"
	          << m[3][0] << ", " << m[3][1] << ", " << m[3][2] << ", " << m[3][3] << "\n\n";
}

void PrintVec3(glm::vec3 v) {
	std::cout << "x: " << v.x
		<< " y: " << v.y
		<< " z: " << v.z << "\n";
}

GLfloat quadVerts[] = {
	-1.f,   1.f, 0.0f,  1.0f, 0.0f,  0.0f, 0.0f, 1.0f,
	-1.f,  -1.f, 0.0f,  1.0f, 1.0f,  0.0f, 0.0f, 1.0f,
	 1.0f, -1.f, 0.0f,  0.0f, 1.0f,  0.0f, 0.0f, 1.0f,
	 1.0f,  1.f, 0.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f
};

unsigned int quadIndices[] = {
	0, 1, 2,
	0, 2, 3
};

struct DirectionalLight
{
	DirectionalLight(glm::vec3 rotation, float intensity) {
		m_rotation = rotation;
		m_intensity = intensity;
	}
	glm::vec3 m_rotation;
	float m_intensity;
	glm::vec3 CalculateDirection() {

		glm::vec3 temp(sin(glm::radians(this->m_rotation.y)) * cos(glm::radians(this->m_rotation.x)), //x
			          sin(glm::radians(-this->m_rotation.x)),                                         //y
			          cos(glm::radians(this->m_rotation.x)) * cos(glm::radians(this->m_rotation.y))); //z

		return glm::normalize(temp);
	};
};

struct Sphere {
	float radius;
	glm::vec3 position;
	glm::vec3 albedo;
	glm::vec3 specular;
	float roughtness;
};

void SetSphere(Shader* shader, Sphere sphere, const char* uniform) {
	

	std::string temp = uniform; temp.append(".radius");
	shader->Set1f(sphere.radius, temp.c_str());
	temp.clear();
	
	temp = uniform; temp.append(".position");
	shader->SetVec3f(sphere.position, temp.c_str());
	temp.clear();

	temp = uniform; temp.append(".specular");
	shader->SetVec3f(sphere.specular, temp.c_str());
	temp.clear();

	temp = uniform; temp.append(".albedo");
	shader->SetVec3f(sphere.albedo, temp.c_str());
	temp.clear();

	temp = uniform; temp.append(".roughtness");
	shader->Set1f(sphere.roughtness, temp.c_str());
	temp.clear();
}

struct PointLight {
	glm::vec4 m_Colour;
	glm::vec4 m_Position;
};

struct SpotLight {
	glm::vec3  m_Position;
	glm::vec3  m_Direction;
	glm::vec3 m_Colour;
	float m_CutOff;
	float m_OuterCuttoff;
	float m_Intensity;
};

int main() {

	Window window("Window", true, 1460, 768);
	GuiLayer GuiLayer("Window?", window.GetWindow());
	
	Camera camera({ 0.f, 0.f, 0.f }, { 0.f, -1.f, 0.f }, 0, 0, 1, 1, 90.f);

	Shader shader; 
	shader.CreateFromFile("Shaders/Vertex.glsl", "Shaders/Frag.glsl");

	Shader compute; 
	compute.CreateFromFile("Shaders/Compute.glsl");
	//compute.QueryWorkgroups();
	compute.Bind();
	compute.Set1i(1, "u_skyboxTexture");

	Mesh obj;
	obj.Create(quadVerts, quadIndices, 32, 6); 

	DirectionalLight directionalLight({0,0,0}, 0);

	SpotLight spotlight;
	spotlight.m_Colour = { 0.8, 0.8, 0.8 };
	spotlight.m_Position = { 0, 15, 0 };
	spotlight.m_Direction = glm::vec3(0, -1, 0);
	spotlight.m_CutOff = glm::cos(glm::radians(12.5f));
	spotlight.m_Intensity = 0.f;

	Sphere sphere;
	sphere.position = glm::vec3(0.f, 3.f, 0.f);
	sphere.radius = 1.0f;
	sphere.specular = glm::vec3(0.8);
	sphere.albedo = glm::vec3(0.2);
	sphere.roughtness = 0;

	GLuint tex_output = 0; //TODO move this into texture class
	int tex_w = window.GetBufferWidth(), tex_h = window.GetBufferHeight(); { // create the texture
		glGenTextures(1, &tex_output);
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, tex_output);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// linear allows us to scale the window up retaining reasonable quality
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		// same internal format as compute shader input
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
		// bind to image unit so can write to specific pixels from the shader
		glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	}
	
	//https://hdrihaven.com/hdris/
	//cape_hill_4k.hdr // urban_alley_01_4k.hdr //herkulessaulen_8k.hdr
	Texture skybox("Textures/SkyboxHDRs/urban_alley_01_4k.hdr"); // Load skyobx
	skybox.Bind(1);// Bind to second tex slot
	
	std::vector<PointLight> pointLights;

	for (size_t i = 0; i < 4; i++) {
		PointLight tempLight;
		tempLight.m_Position = glm::vec4(0, 0, 0, 0);
		tempLight.m_Colour = glm::vec4(1, 1, 1, 10);
		pointLights.push_back(tempLight);
	}
	
	pointLights[1].m_Position = glm::vec4(-4, 6, 0, 0);
	pointLights[1].m_Colour = glm::vec4(1, 0, 0, Random::Get().Int(0, 25));

	pointLights[2].m_Position = glm::vec4(4, 6, 0, 0);
	pointLights[2].m_Colour = glm::vec4(0, 1, 0, Random::Get().Int(0, 25));

	pointLights[3].m_Position = glm::vec4(0, 6, 0, 0);
	pointLights[3].m_Colour = glm::vec4(0, 0, 1, Random::Get().Int(0, 25));

	unsigned int pointlightUBO;
	glGenBuffers(1, &pointlightUBO);

	glBindBuffer(GL_UNIFORM_BUFFER, pointlightUBO);
	glBufferData(GL_UNIFORM_BUFFER, pointLights.size() * sizeof(PointLight), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, pointlightUBO, 0, sizeof(PointLight));

    // Uniform values
	int bounceLimit = 5; 
	float u_testFloat = 0;

	bool togglePlane = false;
	bool toggleShadows = false;
	
	float u_sample = 0;
	int maxSample = 50;
	bool sampleReset = false;

	glm::vec3 lastCameraPos, lastCameraDir;

	glm::vec3 u_pointLightPosition(0.f);
	glm::vec3 u_pointLightColour(0.25f);

	float u_metallic = 0.2f, u_ao = 0.3f, u_roughness = 0.1;
	float u_Cutoff = 12.5; float u_OuterCutoff = 17.5;
	while (window.IsOpen()) {
	
		window.Clear();
		lastCameraDir = camera.getCameraDirection();
		lastCameraPos = camera.GetCameraPosition();
		if (window.UpdateOnFocus()) {
			camera.mouseControl(window.GetXChange(), window.GetYChange());
			camera.keyControl(window.GetsKeys(), window.GetDeltaTime());
			sampleReset = false;
		}

		// launch compute shaders
		compute.Bind();
		camera.GetCameraPosition();
		// Set uniforms for compute shader
		compute.SetUniformMat4f("u_cameraToWorld", camera.CalculateViewMatrix(), true);
		compute.SetUniformMat4f("u_cameraInverseProjection", glm::inverse(camera.CalculateProjectionMatrix(window.GetBufferWidth(), window.GetBufferHeight())), false);
		
		compute.Set2f(glm::vec2(Random::Get().Float(0, 10, 0.1f), Random::Get().Float(0, 10, 0.1f)), "u_pixelOffset");
		compute.Set1i(toggleShadows, "u_toggleShadow");
		compute.Set1i(togglePlane, "u_togglePlane");
		compute.Set1f(Random::Get().Float(0, 10, 0.1f), "u_seed");
		compute.Set1i(bounceLimit, "u_rayBounceLimit");
		compute.Set1f(u_metallic, "u_metallic");
		compute.Set1f(u_roughness, "u_roughness");
		compute.Set1f(u_ao, "u_ao");
		compute.SetVec3f(spotlight.m_Position, "u_spotlightPos");
		compute.SetVec3f(spotlight.m_Direction, "u_spotlightDir");
		compute.Set1f(glm::cos(glm::radians(u_Cutoff)), "u_spotlightCuttoff");
		compute.Set1f(glm::cos(glm::radians(u_OuterCutoff)), "u_spotlightOuterCuttoff");
		compute.Set1f(spotlight.m_Intensity, "u_spotlightIntensity");
		compute.SetVec3f(spotlight.m_Colour, "u_spotlightColour");

		glBindBuffer(GL_UNIFORM_BUFFER, pointlightUBO);
		for (size_t i = 0; i < pointLights.size(); i++) {

			glBufferSubData(GL_UNIFORM_BUFFER, (sizeof(PointLight) * i), sizeof(glm::vec4), &pointLights[i].m_Colour);
			glBufferSubData(GL_UNIFORM_BUFFER, (sizeof(PointLight) * i) + sizeof(glm::vec4), sizeof(glm::vec4), &pointLights[i].m_Position);

		}

		glBindBuffer(GL_UNIFORM_BUFFER, 0);

		compute.SetVec4f(glm::vec4(directionalLight.CalculateDirection(), directionalLight.m_intensity), "u_directionalLight");
		SetSphere(&compute, sphere, "u_sphere");
		
		glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);
		// prevent sampling before all writes to image are done
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// TODO Make better GUI Class
		{ 
		    GuiLayer.Begin();

			ImGui::Begin("GUI");
			ImGui::Text("Application average %.2f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::SliderFloat("FOV", camera.GetFOVptr(), 0, 174.9);
			
			ImGui::SliderInt("Max Smaple", &maxSample, 0, 100);
			ImGui::Checkbox("Reset Sample", &sampleReset);
            ImGui::SliderInt("BounceLimit", &bounceLimit, 0, 10);

			ImGui::LabelText("", "Sphere");
			ImGui::ColorEdit3("Specular", (float*)&sphere.specular);
			ImGui::ColorEdit3("Albedo", (float*)&sphere.albedo);
			ImGui::SliderFloat("roughtness", &sphere.roughtness, 0, 250, "", 1.f);
			ImGui::DragFloat("u_metallic", &u_metallic, 0.01, 0, 1.f);
			ImGui::DragFloat("u_ao", &u_ao, 0.01, 0, 1.f);
			ImGui::DragFloat("u_roughness", &u_roughness, 0.01, 0, 1.f);

			ImGui::LabelText("", "Directional Light");
			ImGui::DragFloat3("Light Rotation", (float*)&directionalLight.m_rotation, 0.1);
			ImGui::DragFloat("Directional Light Intensity", &directionalLight.m_intensity);

			ImGui::LabelText("", "Point Light");
			ImGui::DragFloat4("PL Position", (float*)&pointLights[0].m_Position, 0.1);
			ImGui::ColorEdit4("PL Colour", (float*)&pointLights[0].m_Colour);
			ImGui::DragFloat("PL Intensity", &pointLights[0].m_Colour.w);
			
			ImGui::LabelText("", "Spot Light ");
			ImGui::DragFloat3("SL Position", (float*)&spotlight.m_Position, 0.1);
			ImGui::DragFloat3("SL Direction", (float*)&spotlight.m_Direction, 0.1);
			ImGui::ColorEdit3("SL Colour", (float*)&spotlight.m_Colour);
			ImGui::DragFloat("SL Cutoff", &u_Cutoff, 0.1);
			ImGui::DragFloat("SL OuterCutoff", &u_OuterCutoff, 0.1);
			ImGui::DragFloat("SL Intensity", &spotlight.m_Intensity, 0.1);

			ImGui::Checkbox("Toggle Plane", &togglePlane);
			ImGui::Checkbox("Toggle Shadow", &toggleShadows);
			ImGui::End();
		}
		
		shader.Bind();
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_output);
	
		glUniformMatrix4fv(shader.GetModelLocation(), 1, GL_FALSE, glm::value_ptr(obj.GetModel()));
		glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(camera.CalculateProjectionMatrix(window.GetBufferWidth(), window.GetBufferHeight())));
		shader.Set1i(0, "u_TextureA");
		shader.Set1f(u_sample, "u_sample");
		obj.UpdateModel();
		obj.Render();

		if (lastCameraDir != camera.getCameraDirection() || lastCameraPos != camera.GetCameraPosition() || sampleReset == true) {
			u_sample = 0;
		}
		else if (u_sample < maxSample) {
			u_sample += 1.f;
		}
	    
		glUseProgram(0);
		GuiLayer.End();
		window.Update();
		
	}
	

	return 0;
}