#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>

class Application
{
public:
	Application();
	~Application() noexcept;

private:
	GLFWwindow* m_Window{ nullptr };

	void GLFWInit();
	void GLFWTerminate() noexcept;
	void GLFWCreateWindow();
	void GLFWDestroyWindow() noexcept;
	void GLFWSetFramebufferSizeCallback();
	void GLFWSetKeyCallback();

	void GLADLoadGL();
	void GLSetDebugOutputCallback();

	void imgui_init() noexcept;
	void imgui_shutdown() noexcept;

	void Update();
};
