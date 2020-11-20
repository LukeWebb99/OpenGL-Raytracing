#pragma once

#include <string>

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_glfw.h"
#include "ImGui/imgui_impl_opengl3.h"

class GuiLayer
{
public:
	GuiLayer();
	GuiLayer(const char* title, GLFWwindow* window);
	~GuiLayer();

	void Begin();
	void End();

private:
	const char* m_title;
};
