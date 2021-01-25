#include "Window.h"

Window::Window(const char* title, GLboolean resizeable, GLint width, GLint height)
{
	m_title = title;
	m_resizeable = resizeable;
	m_width = width;
	m_height = height;
	m_window = nullptr;
	m_mouseFirstMove = true;
	m_updateOnFocues = true;
	Init();
}

Window::~Window()
{
	glfwTerminate();
}

void Window::Update()
{
	glfwPollEvents();
	glfwSwapBuffers(m_window);
	glfwGetFramebufferSize(m_window, &m_bufferWidth, &m_bufferHeight);
	glViewport(0, 0, m_bufferWidth, m_bufferHeight);

	//Delta Time
	cTime = static_cast<float>(glfwGetTime());
	dt = cTime - lTime;
	lTime = cTime;

}

void Window::Clear()
{
	//glClearColor(0.f, 0.f, 0.f, 0.001f);
	glClear(GL_DEPTH_BUFFER_BIT); //
	glClearColor(0.f, 0.f, 0.f, 0.f);
	
}

bool Window::IsOpen()
{
	return !glfwWindowShouldClose(m_window);
}

bool Window::IsFocused() {

	return(glfwGetWindowAttrib(m_window, GLFW_FOCUSED));

}

bool Window::UpdateOnFocus()
{
	return m_updateOnFocues;
}

float Window::GetDeltaTime()
{
	return dt;
}

GLfloat Window::GetBufferWidth()
{
	return m_bufferWidth;
}

GLfloat Window::GetBufferHeight()
{
	return m_bufferHeight;
}

GLfloat Window::GetXChange()
{
	GLfloat theChange = m_xChange;
	m_xChange = 0.0f;
	return theChange;
}

GLfloat Window::GetYChange()
{
	GLfloat theChange = m_yChange;
	m_yChange = 0.0f;
	return theChange;
}

void Window::CreateCallbacks()
{
	glfwSetKeyCallback(m_window, HandleKeys);
	glfwSetCursorPosCallback(m_window, HandleMouse);
}

void Window::HandleKeys(GLFWwindow* window, int key, int code, int action, int mode)
{
	Window* l_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	if (key >= 0 && key < 1024) {

		if (action == GLFW_PRESS)
		{
			l_window->m_keys[key] = true;
		}
		else if (action == GLFW_RELEASE)
		{
			l_window->m_keys[key] = false;
		}
	}

	if (l_window->m_keys[GLFW_KEY_LEFT_ALT]) {
		l_window->SetUpdateOnFocus(false);
		glfwSetInputMode(l_window->m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && l_window->m_keys[GLFW_KEY_LEFT_ALT]) {
		l_window->SetUpdateOnFocus(true);
		glfwSetInputMode(l_window->m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

}

void Window::HandleMouse(GLFWwindow* window, double xPos, double yPos)
{
	Window* t_window = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (t_window->m_mouseFirstMove) {
		t_window->m_lastX = xPos;
		t_window->m_lastY = yPos;
		t_window->m_mouseFirstMove = false;
	}

	t_window->m_xChange = xPos - t_window->m_lastX;
	t_window->m_yChange = t_window->m_lastY - yPos;
	
	t_window->m_lastX = xPos;
	t_window->m_lastY = yPos;

}

void Window::Init() {

	//INIT GLFW
	if (!glfwInit()) {
		std::cout << "[GLFW]: ERROR\n";
		glfwTerminate();
	}
	else
		std::cout << "[GLFW]: OK\n";

	// Setup GLFW Windows Properties
	// OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_RESIZABLE, m_resizeable);


	// Create the window
	m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);
	if (!m_window)
	{
		printf("[ERROR]: GLFW window didn't init..");
		glfwTerminate();
	}

	glfwGetFramebufferSize(m_window, &m_bufferWidth, &m_bufferHeight); //static frame buffer

	// Set the current context
	glViewport(0, 0, m_bufferWidth, m_bufferHeight);
	glfwMakeContextCurrent(m_window);

	CreateCallbacks();

	//INIT GLEW
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK) {
		std::cout << "[ERROR]: GLEW_INIT_FAILED \n";
		glfwTerminate();
	}
	else
		std::cout << "[GLEW]: OK\n" << glGetString(GL_VERSION) << std::endl;

	//CreateCallbacks();
	glEnable(GL_DEPTH_TEST);
	glfwSetWindowUserPointer(m_window, this);
	glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	glEnable(GL_BLEND); //use alpha
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
}
