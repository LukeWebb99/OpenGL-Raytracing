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

GLfloat quadVerts[] =  {
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

Texture skybox("Textures/SkyboxHDRs/cape_hill_4k.hdr"); // Load skyobx

Texture Albedo   ("Textures/PBR/Gold (Au)_schvfgwp_Metal/Albedo_4K__schvfgwp.jpg");
Texture Normal   ("Textures/PBR/Gold (Au)_schvfgwp_Metal/Normal_4K__schvfgwp.jpg");
Texture Roughness("Textures/PBR/Gold (Au)_schvfgwp_Metal/Roughness_4K__schvfgwp.jpg");
Texture AO       ("Textures/PBR/Gold (Au)_schvfgwp_Metal/Metalness_4K__schvfgwp.jpg");
Texture Metallic ("Textures/PBR/Gold (Au)_schvfgwp_Metal/Metalness_4K__schvfgwp.jpg");

/*
Texture Albedo   ("Textures/PBR/Dirty Metal Sheet/Albedo_4K__vbsieik.jpg");
Texture Normal   ("Textures/PBR/Dirty Metal Sheet/Normal_4K__vbsieik.jpg");
Texture Roughness("Textures/PBR/Dirty Metal Sheet/Roughness_4K__vbsieik.jpg");
Texture AO       ("Textures/PBR/Dirty Metal Sheet/AO_4K__vbsieik.jpg");
Texture Metallic ("Textures/PBR/Dirty Metal Sheet/Roughness_4K__vbsieik.jpg");
*/

/*
Texture Albedo   ("Textures/PBR/rustediron1-alt2-bl/rustediron2_basecolor.png");
Texture Normal   ("Textures/PBR/rustediron1-alt2-bl/rustediron2_normal.png");
Texture Roughness("Textures/PBR/rustediron1-alt2-bl/rustediron2_roughness.png");
Texture AO       ("Textures/PBR/rustediron1-alt2-bl/rustediron2_ao.png");
Texture Metallic ("Textures/PBR/rustediron1-alt2-bl/rustediron2_metallic.png");
*/


int main() {



	Window window("Window", true, 1460, 768);
	GuiLayer GuiLayer("Window?", window.GetWindow());
	
	//https://hdrihaven.com/hdris/
    //cape_hill_4k.hdr // urban_alley_01_4k.hdr //herkulessaulen_8k.hdr

	skybox.CreateHDRI();

	Camera camera({ 0.f, 0.f, 0.f }, { 0.f, -1.f, 0.f }, 0, 0, 1, 1, 90.f);

	Shader shader; 
	shader.CreateFromFile("Shaders/Vertex.glsl", "Shaders/Frag.glsl");

	Shader compute; 
	compute.CreateFromFile("Shaders/Compute.glsl");
	//compute.QueryWorkgroups();

	Mesh obj;
	obj.Create(quadVerts, quadIndices, 32, 6); 

	DirectionalLight directionalLight({0,0,0}, 0);

	Sphere sphere;
	sphere.position = glm::vec3(0.f, 3.f, 0.f);
	sphere.radius = 1.0f;
	sphere.specular = glm::vec3(0.8);
	sphere.albedo = glm::vec3(0.2);
	sphere.roughtness = 0;

	compute.Bind();

	GLuint tex_output = 0; //TODO move this into texture class
	int tex_w = window.GetBufferWidth(), tex_h = window.GetBufferHeight(); { // create the texture
		glGenTextures(1, &tex_output);
		glActiveTexture(0);
		glBindTexture(GL_TEXTURE_2D, tex_output);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		// linear allows us to scale the window up retaining reasonable quality
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		// same internal format as compute shader input
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, tex_w, tex_h, 0, GL_RGBA, GL_FLOAT, NULL);
		// bind to image unit so can write to specific pixels from the shader
		glBindImageTexture(0, tex_output, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
	}
	

	Albedo.CreateTexture2D();
	Normal.CreateTexture2D();
	Roughness.CreateTexture2D();
	AO.CreateTexture2D();
	Metallic.CreateTexture2D();
	
	std::vector<PointLight> pointLights;

	for (size_t i = 0; i < 4; i++) {
		PointLight tempLight;
		tempLight.m_Position = glm::vec4(0, 100, 0, 0);
		tempLight.m_Colour = glm::vec4(1, 1, 1, 0);
		pointLights.push_back(tempLight);
	}
	
	unsigned int pointlightUBO;  //Pointlights Uniform Buffer Object 
	glGenBuffers(1, &pointlightUBO);

	glBindBuffer(GL_UNIFORM_BUFFER, pointlightUBO);
	glBufferData(GL_UNIFORM_BUFFER, pointLights.size() * sizeof(PointLight), NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, pointlightUBO, 0, sizeof(PointLight));

	glBindBuffer(GL_UNIFORM_BUFFER, pointlightUBO);
	for (size_t i = 0; i < pointLights.size(); i++) {

		glBufferSubData(GL_UNIFORM_BUFFER, (sizeof(PointLight) * i), sizeof(glm::vec4), &pointLights[i].m_Colour);
		glBufferSubData(GL_UNIFORM_BUFFER, (sizeof(PointLight) * i) + sizeof(glm::vec4), sizeof(glm::vec4), &pointLights[i].m_Position);

	}
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	compute.Set1i(1, "u_skyboxTexture");
	compute.Set1i(2, "u_AlbedoMap");
	compute.Set1i(3, "u_NormalMap");
	compute.Set1i(4, "u_RoughnessMap");
	compute.Set1i(5, "u_AOMap");
	compute.Set1i(6, "u_MetallicMap");

	skybox.Bind(1);
	Albedo.Bind(2);
	Normal.Bind(3);
	Roughness.Bind(4);
	AO.Bind(5);
	Metallic.Bind(6);

    // Uniform values 
	int bounceLimit = 3; //ray bounce limit

	bool togglePlane = false;
	float u_sample = 0;
	bool noSample = false;

	glm::vec3 lastCameraPos, lastCameraDir;

	float u_metallic = 0.2f, u_ao = 0.3f, u_roughness = 0.1;

	while (window.IsOpen()) {
	
		window.Update();
		lastCameraDir = camera.getCameraDirection();
		lastCameraPos = camera.GetCameraPosition();

		if (window.UpdateOnFocus()) {
			camera.mouseControl(window.GetXChange(), window.GetYChange());
			camera.keyControl(window.GetsKeys(), window.GetDeltaTime());
		}

		// launch compute shaders
		compute.Bind();

		// Set uniforms for compute shader
		compute.SetMat4f("u_cameraToWorld", camera.CalculateViewMatrix(), true);
		compute.SetMat4f("u_cameraInverseProjection", glm::inverse(camera.CalculateProjectionMatrix(window.GetBufferWidth(), window.GetBufferHeight())), false);
		compute.Set2f(glm::vec2(Random::Get().Int(0, 1), Random::Get().Int(0, 1)), "u_pixelOffset");
		compute.Set1i(togglePlane, "u_togglePlane");
		compute.Set1f(Random::Get().Int(0, 1000), "u_seed");
		compute.Set1i(bounceLimit, "u_rayBounceLimit");



		glBindBuffer(GL_UNIFORM_BUFFER, pointlightUBO);
		for (size_t i = 0; i < 4; i++) {
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

			if (ImGui::CollapsingHeader("Renderer Options")) {
				ImGui::SliderInt("BounceLimit", &bounceLimit, 0, 10);
				ImGui::Checkbox("No Sample", &noSample);
			}

			if (ImGui::CollapsingHeader("Camera Options")) {
				ImGui::SliderFloat("FOV", camera.GetFOVptr(), 0, 174.9);
			}

			if (ImGui::CollapsingHeader("Scene Options")) {
				ImGui::Checkbox("Toggle Plane", &togglePlane);
			}

			if (ImGui::CollapsingHeader("Direction Light Options")) {
				ImGui::LabelText("", "Directional Light");
				ImGui::DragFloat3("Light Rotation", (float*)&directionalLight.m_rotation, 0.1);
				ImGui::SliderFloat("Directional Light Intensity", &directionalLight.m_intensity, 0, 5.f);
			}

			if (ImGui::CollapsingHeader("Point Light Options")) {

				if (ImGui::CollapsingHeader("Point Light [0] Options")) {
					ImGui::DragFloat4("PL [0] Position", (float*)&pointLights[0].m_Position, 0.1);
					ImGui::ColorEdit4("PL [0] Colour", (float*)&pointLights[0].m_Colour);
					ImGui::SliderFloat( "PL [0] Intensity", &pointLights[0].m_Colour.w, 0, 5000);
				}

				if (ImGui::CollapsingHeader("Point Light [1] Options")) {
					ImGui::DragFloat4("PL [1] Position", (float*)&pointLights[1].m_Position, 0.1);
					ImGui::ColorEdit4("PL [1] Colour", (float*)&pointLights[1].m_Colour);
					ImGui::SliderFloat("PL [1] Intensity", &pointLights[1].m_Colour.w, 0, 5000);
				}

				if (ImGui::CollapsingHeader("Point Light [2] Options")) {
					ImGui::DragFloat4("PL [2] Position", (float*)&pointLights[2].m_Position, 0.1);
					ImGui::ColorEdit4("PL [2] Colour", (float*)&pointLights[2].m_Colour);
					ImGui::SliderFloat("PL [2] Intensity", &pointLights[2].m_Colour.w, 0, 5000);
				}

				if (ImGui::CollapsingHeader("Point Light [3] Options")) {
					ImGui::DragFloat4("PL [3] Position", (float*)&pointLights[3].m_Position, 0.1);
					ImGui::ColorEdit4("PL [3] Colour", (float*)&pointLights[3].m_Colour);
					ImGui::SliderFloat("PL [3] Intensity", &pointLights[3].m_Colour.w, 0, 5000);
				}

			}


			ImGui::End();
		}
		
		shader.Bind();
		
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, tex_output);
	
		if (lastCameraDir != camera.getCameraDirection() || lastCameraPos != camera.GetCameraPosition()) {
			u_sample = 0;
		}

		if (!noSample) {
			shader.Set1f(u_sample, "u_sample");
		} else {
			shader.Set1f(0, "u_sample");
		} 

		glUniformMatrix4fv(shader.GetModelLocation(), 1, GL_FALSE, glm::value_ptr(obj.GetModel()));
		glUniformMatrix4fv(shader.GetProjectionLocation(), 1, GL_FALSE, glm::value_ptr(camera.CalculateProjectionMatrix(window.GetBufferWidth(), window.GetBufferHeight())));
		shader.Set1i(0, "u_TextureA");


		shader.Set2f(glm::vec2(window.GetBufferWidth(), window.GetBufferHeight()), "u_Resolution");
		obj.UpdateModel();
		obj.Render();


		u_sample += 1.f;

		glUseProgram(0);
		GuiLayer.End();
		
		window.Clear();
	}
	

	return 0;
}