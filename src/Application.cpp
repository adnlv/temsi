#include "Application.h"

#include <iostream>
#include <exception>
#include <string>
#include <string_view>
#include <vector>
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Log.h"
#include "Shader.h"
#include "Buffers.h"
#include "Timer.h"
#include "World.h"
#include "Simulation.h"

Application::Application()
{
	GLFWInit();
	GLFWCreateWindow();

	GLADLoadGL();

	GLFWSetFramebufferSizeCallback();
	GLFWSetKeyCallback();

	GLSetDebugOutputCallback();
	imgui_init();

	Update();
}

Application::~Application()
{
	imgui_shutdown();
	GLFWDestroyWindow();
	GLFWTerminate();
}

void Application::GLFWInit()
{
	if (glfwInit() != GLFW_TRUE)
	{
		throw std::runtime_error("GLFW: Initialization failed");
	}
	Log::Info("GLFW: Initialized");

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
}

void Application::GLFWTerminate() noexcept
{
	glfwTerminate();
	Log::Info("GLFW: Terminated");
}

void Application::GLFWCreateWindow()
{
	auto scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	auto w = static_cast<int>(960 * scale);
	auto h = static_cast<int>(540 * scale);
	m_Window = glfwCreateWindow(w, h, "temsi - 2D ideal gas simulation", nullptr, nullptr);
	if (m_Window == nullptr)
	{
		throw std::runtime_error("GLFW: Window creation failed");
	}
	Log::Info("GLFW: Window created");

	glfwMakeContextCurrent(m_Window);
	glfwSetWindowUserPointer(m_Window, this);
}

void Application::GLFWDestroyWindow() noexcept
{
	glfwDestroyWindow(m_Window);
	Log::Info("GLFW: Window destroyed");
}

void Application::GLFWSetFramebufferSizeCallback()
{
	glfwSetFramebufferSizeCallback(m_Window,
		[](GLFWwindow* window, int width, int height)
		{
			Log::Info("GL: Viewport resolution changed | Width: {} px, Height: {} px", width, height);
		});
}

void Application::GLFWSetKeyCallback()
{
	glfwSetKeyCallback(m_Window,
		[](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			bool isHandlable = true;
			if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE)
			{
				glfwSetWindowShouldClose(window, true);
			}
			else
			{
				isHandlable = false;
			}

			if (isHandlable)
			{
				Log::Info("GL: Key action | Key: {:#x}, Scancode: {:#x}, Action: {}, Mods: {:#x}", key, scancode, action, mods);
			}
		});
}

void Application::GLADLoadGL()
{
	if (!gladLoadGL(glfwGetProcAddress))
	{
		throw std::runtime_error("GLAD: GL loaded failed");
	}
	Log::Info("GLAD: GL loaded");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Application::GLSetDebugOutputCallback()
{
	int flags = 0;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(
			[](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
			{
				std::string source_str;
				switch (source) {
				case GL_DEBUG_SOURCE_API:
					source_str = "API";
					break;
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
					source_str = "WINDOW_SYSTEM";
					break;
				case GL_DEBUG_SOURCE_SHADER_COMPILER:
					source_str = "SHADER_COMPILER";
					break;
				case GL_DEBUG_SOURCE_THIRD_PARTY:
					source_str = "THIRD_PARTY";
					break;
				case GL_DEBUG_SOURCE_APPLICATION:
					source_str = "APPLICATION";
					break;
				case GL_DEBUG_SOURCE_OTHER:
					source_str = "OTHER";
					break;
				}

				std::string type_str;
				switch (type) {
				case GL_DEBUG_TYPE_ERROR:
					type_str = "ERROR";
					break;
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
					type_str = "DEPRECATED_BEHAVIOR";
					break;
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
					type_str = "UNDEFINED_BEHAVIOR";
					break;
				case GL_DEBUG_TYPE_PORTABILITY:
					type_str = "PORTABILITY";
					break;
				case GL_DEBUG_TYPE_PERFORMANCE:
					type_str = "PERFORMANCE";
					break;
				case GL_DEBUG_TYPE_MARKER:
					type_str = "MARKER";
					break;
				case GL_DEBUG_TYPE_PUSH_GROUP:
					type_str = "PUSH_GROUP";
					break;
				case GL_DEBUG_TYPE_POP_GROUP:
					type_str = "POP_GROUP";
					break;
				case GL_DEBUG_TYPE_OTHER:
					type_str = "OTHER";
					break;
				}

				LogLevel level;
				switch (type) {
				case GL_DEBUG_TYPE_ERROR:
					level = LogLevel::Err;
					break;
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
				case GL_DEBUG_TYPE_PORTABILITY:
				case GL_DEBUG_TYPE_PERFORMANCE:
					level = LogLevel::Warn;
					break;
				default:
					level = LogLevel::Info;
					break;
				}

				std::string severity_str;
				switch (severity) {
				case GL_DEBUG_SEVERITY_HIGH:
					severity_str = "HIGH";
					break;
				case GL_DEBUG_SEVERITY_MEDIUM:
					severity_str = "MEDIUM";
					break;
				case GL_DEBUG_SEVERITY_LOW:
					severity_str = "LOW";
					break;
				case GL_DEBUG_SEVERITY_NOTIFICATION:
					severity_str = "NOTIFICATION";
					break;
				}

				Log::Message(level, "GL: {} severity {} message from {} ({}): {}", severity_str, type_str, source_str, id, message);
			}, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
}

void Application::imgui_init() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui::StyleColorsDark();

	float scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor());
	ImGuiStyle& style = ImGui::GetStyle();
	style.ScaleAllSizes(scale);
	style.FontScaleDpi = scale;

	ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

void Application::imgui_shutdown() noexcept
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

struct ViewportRect { int x, y, w, h; };

// Largest rectangle with the box's aspect ratio that fits in the framebuffer, centered.
static ViewportRect ComputeLetterbox(int fbw, int fbh) noexcept
{
	if (fbw <= 0 || fbh <= 0) return { 0, 0, 0, 0 }; // minimised
	int w = fbw;
	int h = static_cast<int>(fbw / World::kAspect);
	if (h > fbh)
	{
		h = fbh;
		w = static_cast<int>(fbh * World::kAspect);
	}
	return { (fbw - w) / 2, (fbh - h) / 2, w, h };
}

void Application::Update()
{
	Simulation simulation;

	std::vector<float> vertices{
		1.0f,  1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		-1.0f,  1.0f, 0.0f
	};
	std::vector<GLuint> indices{
		0, 1, 3,
		1, 2, 3
	};

	VertexArray vao;

	Buffer vbo(BufferType::ArrayBuffer, vertices.size() * sizeof(vertices.at(0)), vertices.data(), BufferUsage::StaticDraw);
	vao.SetAttribPointer(vbo, 2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(vertices.at(0)), nullptr);
	vao.EnableAttribArray(2);

	const auto& particles = simulation.Particles();

	Buffer radius_vbo(BufferType::ArrayBuffer, particles.num_max_particles * sizeof(particles.radii.at(0)), particles.radii.data(), BufferUsage::StaticDraw);
	vao.SetAttribPointer(radius_vbo, 0, 1, GL_FLOAT, GL_FALSE, sizeof(particles.radii.at(0)), nullptr);
	vao.EnableAttribArray(0);
	vao.SetAttribDivisor(0, 1);

	Buffer center_vbo(BufferType::ArrayBuffer, particles.num_max_particles * sizeof(particles.positions.at(0)), nullptr, BufferUsage::DynamicDraw);
	vao.SetAttribPointer(center_vbo, 1, 2, GL_FLOAT, GL_FALSE, sizeof(particles.positions.at(0)), nullptr);
	vao.EnableAttribArray(1);
	vao.SetAttribDivisor(1, 1);

	Buffer color_vbo(BufferType::ArrayBuffer, particles.num_max_particles * sizeof(particles.colors.at(0)), particles.colors.data(), BufferUsage::StaticDraw);
	vao.SetAttribPointer(color_vbo, 3, 3, GL_FLOAT, GL_FALSE, sizeof(particles.colors.at(0)), nullptr);
	vao.EnableAttribArray(3);
	vao.SetAttribDivisor(3, 1);

	Buffer ebo(BufferType::ElementArrayBuffer, indices.size() * sizeof(indices.at(0)), indices.data(), BufferUsage::StaticDraw);
	vao.SetElementBuffer(ebo);

	ShaderProgram program = []()
		{
			auto vertexShaderSource = Shader::LoadFile("shaders/circle.vert");
			Shader vertexShader(ShaderType::VertexShader, vertexShaderSource);

			auto fragmentShaderSource = Shader::LoadFile("shaders/circle.frag");
			Shader fragmentShader(ShaderType::FragmentShader, fragmentShaderSource);

			return ShaderProgram(vertexShader, fragmentShader);
		}();

	GLint half_extent_location = program.GetUniformLocation("u_half_extent");
	GLint viewport_px_location = program.GetUniformLocation("u_viewport_px");

	bool vsync_enabled = false;
	glfwSwapInterval(vsync_enabled);

	float sim_ps_per_s = 10.0f;
	double sim_time_s = 0.0;

	Timer timer;
	while (!glfwWindowShouldClose(m_Window))
	{
		timer.start_frame();

		// Clamp frame hitches (window drag, debugger) so particles can't tunnel through each other.
		const float real_dt = std::min(timer.dt(), 1.0f / 30.0f);
		const float dt = real_dt * sim_ps_per_s * World::kPicosecond; // simulated seconds this frame
		sim_time_s += dt;

		simulation.Update(dt);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		{
			ImGui::Begin("Simulation Panel");

			const size_t fps = timer.fps();
			const float ms_per_frame = fps != 0 ? 1000.0f / static_cast<float>(fps) : timer.dt() * 1000.0f;

			if (ImGui::CollapsingHeader("Performance", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Text("Frames per second: %zu", fps);
				ImGui::Text("Frame time: %.2f ms", ms_per_frame);
				ImGui::Text("Frame count: %zu", timer.frame_count());
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Playback Controls", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::Checkbox("VSync", &vsync_enabled))
				{
					glfwSwapInterval(vsync_enabled);
				}

				ImGui::SliderFloat("Simulation speed", &sim_ps_per_s, 0.0f, 20.0f, "%.3f ps/s", ImGuiSliderFlags_Logarithmic);
				
				int active = simulation.ActiveParticles();
				if (ImGui::SliderInt("Active particles", &active, 0, simulation.MaxParticles()))
				{
					simulation.SetActiveParticles(active);
				}
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Physical Properties", ImGuiTreeNodeFlags_DefaultOpen))
			{
				const auto& temp = simulation.GetTemperature();
				const auto momentum = simulation.TotalMomentum();

				ImGui::Text("Box: %.1f x %.1f nm", World::kWidth * 1e9f, World::kHeight * 1e9f);
				ImGui::Text("Simulated time: %.2f ns", sim_time_s * 1e9);
				ImGui::Text("Temperature: %.2f (K), %.2f (C), %.2f (F)", temp.kelvin(), temp.celsius(), temp.fahrenheit());
				ImGui::Text("Average kinetic energy: %.4e (J/particle)", simulation.AvgKineticEnergy());
				ImGui::Text("Total kinetic energy: %.4e (J)", simulation.TotalKineticEnergy());
				ImGui::Text("Total momentum: (x: %.4e; y: %.4e) (kg * m/s)", momentum.x, momentum.y);
			}

			ImGui::Separator();

			if (ImGui::CollapsingHeader("Speed Distribution (Maxwell-Boltzmann)", ImGuiTreeNodeFlags_DefaultOpen))
			{
				const auto& dist = simulation.GetSpeedDistribution();
				const float plot_max = dist.max_pdf_value > 0.0f ? dist.max_pdf_value * 1.15f : 0.005f;

				ImGui::Text("Empirical speed probability density (PDF):");
				ImGui::PlotHistogram("##empirical_pdf", dist.empirical_pdf.data(), SpeedDistribution::kNumBins,
					0, nullptr, 0.0f, plot_max, ImVec2(0, 90.0f));

				ImGui::Text("Theoretical 2D Rayleigh distribution:");
				ImGui::PlotLines("##theory_pdf", dist.theoretical_pdf.data(), SpeedDistribution::kNumBins,
					0, nullptr, 0.0f, plot_max, ImVec2(0, 70.0f));

				ImGui::TextDisabled("Domain: 0 to %.0f m/s (bin width: %.1f m/s)",
					SpeedDistribution::kMaxSpeed, SpeedDistribution::kBinWidth);

				ImGui::Spacing();
				ImGui::Text("Characteristic speeds (theoretical):");
				ImGui::BulletText("Most probable speed (v_p): %.1f m/s", dist.peak_speed_theoretical);
				ImGui::BulletText("Mean speed (<v>):           %.1f m/s", dist.mean_speed_theoretical);
				ImGui::BulletText("Root-mean-square (v_rms):    %.1f m/s", dist.rms_speed_theoretical);
			}

			ImGui::End();
		}

		int framebufferWidth, framebufferHeight;
		glfwGetFramebufferSize(m_Window, &framebufferWidth, &framebufferHeight);

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		const ViewportRect vp = ComputeLetterbox(framebufferWidth, framebufferHeight);
		glViewport(vp.x, vp.y, vp.w, vp.h);

		vao.Bind();
		program.Use();

		glUniform2fv(half_extent_location, 1, glm::value_ptr(World::kHalfExtent));
		glUniform2f(viewport_px_location, static_cast<float>(vp.w), static_cast<float>(vp.h));

		center_vbo.SubData(0, simulation.ActiveParticles() * sizeof(particles.positions.at(0)), particles.positions.data());
		glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, simulation.ActiveParticles());

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(m_Window);
		glfwPollEvents();

		timer.end_frame();
	}
}

int main()
{
	Log::Init();
	Application app;
	return 0;
}
