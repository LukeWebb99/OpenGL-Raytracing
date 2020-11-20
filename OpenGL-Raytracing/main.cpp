#define PI 3.14159265359

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
#include "Camera.h"
#include "Texture.h"

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

struct Sphere {
	glm::vec3 position;
	float radius;
	glm::vec4 colour;
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
			sin(glm::radians(-this->m_rotation.x)),  //y
			cos(glm::radians(this->m_rotation.x)) * cos(glm::radians(this->m_rotation.y))); //z

		return glm::normalize(temp);
	};
};

int main() {

	Window window("Window", true, 1024, 768);
	GuiLayer GuiLayer("Window?", window.GetWindow());
	Camera camera({ 0.f, 0.f, 0.f }, { 0.f, -1.f, 0.f }, 0, 0, 1, 1, 90.f);

	Shader shader; 
	shader.CreateFromFile("Shaders/Vertex.glsl", "Shaders/Frag.glsl");

	Shader compute; 
	compute.CreateFromFile("Shaders/Compute.glsl");
	compute.QueryWorkgroups();

	Mesh obj;
	obj.Create(quadVerts, quadIndices, 32, 6); 

	DirectionalLight directionalLight({0,0,0}, 1);


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

	//https://hdrihaven.com/hdris/
	Texture skybox("Textures/urban_alley_01_4k.hdr"); // Load skyobx
	skybox.Bind(1);// Bind to second tex slot

    // Uniform values
	int bounceLimit = 5; 
	glm::vec3 specular = glm::vec3(1);
	glm::vec3 albedo = glm::vec3(1);
	bool togglePlane = false;
	bool toggleShadows = false;

	while (window.IsOpen()) {
		
		if (window.UpdateOnFocus()) {
			camera.mouseControl(window.GetXChange(), window.GetYChange());
			camera.keyControl(window.GetsKeys(), window.GetDeltaTime());
		}

		// launch compute shaders
		compute.Bind();
		
		// Set uniforms for compute shader
		compute.SetUniformMat4f("u_cameraToWorld", camera.CalculateViewMatrix(), true);
		compute.SetUniformMat4f("u_cameraInverseProjection", glm::inverse(camera.CalculateProjectionMatrix(window.GetBufferWidth(), window.GetBufferHeight())), false);
		compute.Set1i(toggleShadows, "u_toggleShadow");
		compute.Set1i(togglePlane, "u_togglePlane");
		compute.Set1i(1, "u_skyboxTexture");
		compute.Set1i(bounceLimit, "u_rayBounceLimit");
		compute.SetVec3f(specular, "u_specular");
		compute.SetVec3f(albedo, "u_albedo");
		compute.SetVec4f(glm::vec4(directionalLight.CalculateDirection(), directionalLight.m_intensity), "u_directionalLight");

		
		glDispatchCompute((GLuint)tex_w, (GLuint)tex_h, 1);
		// prevent sampling befor all writes to image are done
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		
		window.Clear();
		{ // TODO Make better GUI Class
		    GuiLayer.Begin();
			ImGui::Begin("GUI");
			ImGui::Text("Application average %.2f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::SliderFloat("FOV", camera.GetFOVptr(), 0, 174.9);
			ImGui::SliderInt("BounceLimit", &bounceLimit, 0, 10);

			ImGui::LabelText("", "Sphere Options");
			ImGui::ColorEdit3("Specular", (float*)&specular);

			ImGui::LabelText("", "Directional Light Options");
			ImGui::ColorEdit3("Albedo", (float*)&albedo);
			ImGui::DragFloat3("Light Rotation", (float*)&directionalLight.m_rotation, 0.1);
			ImGui::DragFloat("Intensity", &directionalLight.m_intensity);
			
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
	
		obj.UpdateModel();
		obj.Render();


		glUseProgram(0);
		
		GuiLayer.End();
		window.Update();
	}
	

	return 0;
}