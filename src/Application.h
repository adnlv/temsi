#pragma once
#include <glad/gl.h>
#include <GLFW/glfw3.h>

class Application
{
public:
	Application();
	~Application() noexcept;

	Application(const Application&) = delete;
	Application& operator=(const Application&) = delete;
	Application(Application&&) = delete;
	Application& operator=(Application&&) = delete;

	void Run();

private:
	GLFWwindow* m_Window{ nullptr };
	bool m_ImGuiInitialized{ false };

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
};
