#pragma once

#include <iostream>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

class Window
{
public:
	Window(const char* title, GLboolean resizeable, GLint width, GLint height);
	~Window();

	void Update();
	void Clear();
	
	bool IsOpen();
	bool IsFocused();
	bool UpdateOnFocus();
	
	void SetUpdateOnFocus(bool value) { this->m_updateOnFocues = value; };

	float GetDeltaTime();
	GLFWwindow* GetWindow() { return m_window; };
	GLfloat GetBufferWidth();
	GLfloat GetBufferHeight();

	bool* GetsKeys() { return m_keys; }
	GLfloat GetXChange();
	GLfloat GetYChange();

private:
	std::string m_title;
	GLFWwindow* m_window;
	GLint m_width, m_height;
	GLint m_bufferWidth, m_bufferHeight;
	GLboolean m_resizeable;
	
	//Delta Time 
	float dt;
	float cTime;
	float lTime;

	//Input
	bool m_keys[1024];
	bool m_updateOnFocues;

	GLfloat m_lastX;
	GLfloat m_lastY;
	GLfloat m_xChange;
	GLfloat m_yChange;
	bool m_mouseFirstMove;
	
	void CreateCallbacks();
	static void HandleKeys(GLFWwindow* window, int key, int code, int action, int mode);
	static void HandleMouse(GLFWwindow* window, double xPos, double yPos);

	void Init();

};

