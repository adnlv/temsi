#pragma once
#include <string>
#include <string_view>
#include <exception>
#include <stdexcept>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <type_traits>
#include <glad/gl.h>
#include "Paths.h"

enum class ShaderType
{
	VertexShader = GL_VERTEX_SHADER,
	FragmentShader = GL_FRAGMENT_SHADER
};

class Shader
{
public:
	Shader(ShaderType type, const std::string_view source);
	~Shader();

	Shader(const Shader&) = delete;
	Shader& operator=(const Shader&) = delete;

	Shader(Shader&& other) noexcept;
	Shader& operator=(Shader&& other) noexcept;

	GLuint Id() const noexcept;
	ShaderType Type() const noexcept;

	static std::string LoadFile(const std::filesystem::path& filepath);

private:
	GLuint m_Id;
	ShaderType m_Type;
};

inline Shader::Shader(ShaderType type, const std::string_view source)
	: m_Type(type)
{
	m_Id = glCreateShader(static_cast<GLenum>(type));

	const GLchar* sourceData = source.data();
	const GLint sourceLength = static_cast<GLint>(source.size());
	glShaderSource(m_Id, 1, &sourceData, &sourceLength);

	glCompileShader(m_Id);

	int success;
	glGetShaderiv(m_Id, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		GLint logLength = 0;
		glGetShaderiv(m_Id, GL_INFO_LOG_LENGTH, &logLength);

		std::string infoLog(logLength > 0 ? logLength : 0, '\0');
		GLsizei actualLength = 0;
		if (logLength > 0)
		{
			glGetShaderInfoLog(m_Id, logLength, &actualLength, infoLog.data());
			infoLog.resize(static_cast<size_t>(actualLength));
		}
		glDeleteShader(m_Id);
		throw std::runtime_error("GL: Shader compilation failed: " + infoLog);
	}
}

inline Shader::~Shader()
{
	glDeleteShader(m_Id);
}

inline Shader::Shader(Shader&& other) noexcept
	: m_Id(other.m_Id),
	m_Type(other.m_Type)
{
	other.m_Id = 0;
}

inline Shader& Shader::operator=(Shader&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}

	if (m_Id != 0)
	{
		glDeleteShader(m_Id);
	}

	m_Id = other.m_Id;
	m_Type = other.m_Type;

	other.m_Id = 0;
	return *this;
}

inline GLuint Shader::Id() const noexcept
{
	return m_Id;
}

inline ShaderType Shader::Type() const noexcept
{
	return m_Type;
}

inline std::string Shader::LoadFile(const std::filesystem::path& filepath)
{
	std::ifstream file;
	file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

	try
	{
		file.open(Paths::AssetsDir() / filepath);

		std::stringstream source;
		source << file.rdbuf();

		return source.str();
	}
	catch (const std::ifstream::failure& e)
	{
		throw std::runtime_error("GL: Failed to load shader source from " + (Paths::AssetsDir() / filepath).string() + ": " + e.what());
	}
}

class ShaderProgram
{
public:
	template<typename... Args>
	ShaderProgram(const Args&... shaders);
	~ShaderProgram();

	ShaderProgram(const ShaderProgram&) = delete;
	ShaderProgram& operator=(const ShaderProgram&) = delete;

	ShaderProgram(ShaderProgram&& other) noexcept;
	ShaderProgram& operator=(ShaderProgram&& other) noexcept;

	GLuint Id() const noexcept;

	void Use() const noexcept;
	GLint GetUniformLocation(const std::string& name) const noexcept;

private:
	GLuint m_Id;
};

template<typename ...Args>
inline ShaderProgram::ShaderProgram(const Args & ...shaders)
{
	static_assert(sizeof...(Args) > 0, "GL: ShaderProgram requires at least one Shader object");
	static_assert((std::is_same_v<Args, Shader> && ...), "GL: ShaderProgram constructor only accepts Shader objects");

	m_Id = glCreateProgram();

	(glAttachShader(m_Id, shaders.Id()), ...);

	glLinkProgram(m_Id);
	(glDetachShader(m_Id, shaders.Id()), ...);

	GLint success = 0;
	glGetProgramiv(m_Id, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		GLint logLength = 0;
		glGetProgramiv(m_Id, GL_INFO_LOG_LENGTH, &logLength);

		std::string infoLog(logLength > 0 ? logLength : 0, '\0');
		GLsizei actualLength = 0;
		if (logLength > 0)
		{
			glGetProgramInfoLog(m_Id, logLength, &actualLength, infoLog.data());
			infoLog.resize(static_cast<size_t>(actualLength));
		}
		glDeleteProgram(m_Id);
		throw std::runtime_error("GL: Shader program linking failed: " + infoLog);
	}
}

inline ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(m_Id);
}

inline ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
	: m_Id(other.m_Id)
{
	other.m_Id = 0;
}

inline ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}

	if (m_Id != 0)
	{
		glDeleteProgram(m_Id);
	}

	m_Id = other.m_Id;

	other.m_Id = 0;
	return *this;
}

inline GLuint ShaderProgram::Id() const noexcept
{
	return m_Id;
}

inline void ShaderProgram::Use() const noexcept
{
	glUseProgram(m_Id);
}

inline GLint ShaderProgram::GetUniformLocation(const std::string& name) const noexcept
{
	return glGetUniformLocation(m_Id, name.c_str());
}
