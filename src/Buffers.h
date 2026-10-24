#pragma once
#include <glad/gl.h>

class Buffer;

class VertexArray
{
public:
	VertexArray();
	~VertexArray();

	VertexArray(const VertexArray&) = delete;
	VertexArray& operator=(const VertexArray&) = delete;

	VertexArray(VertexArray&& other) noexcept;
	VertexArray& operator=(VertexArray&& other) noexcept;

	GLuint Id() const noexcept;

	void Bind() const noexcept;
	void EnableAttribArray(GLuint index) const noexcept;
	void DisableAttribArray(GLuint index) const noexcept;
	void SetAttribPointer(const Buffer& vbo, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) const noexcept;
	void SetAttribDivisor(GLuint index, GLuint divisor) const noexcept;
	void SetElementBuffer(const Buffer& ebo) const noexcept;

	static void Unbind() noexcept;

private:
	GLuint m_Id = 0;
};

inline VertexArray::VertexArray()
{
	glGenVertexArrays(1, &m_Id);
}

inline VertexArray::~VertexArray()
{
	glDeleteVertexArrays(1, &m_Id);
}

inline VertexArray::VertexArray(VertexArray&& other) noexcept
	: m_Id(other.m_Id)
{
	other.m_Id = 0;
}

inline VertexArray& VertexArray::operator=(VertexArray&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}

	// Unused names in arrays are silently ignored, as is the value zero.
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteVertexArrays.xhtml
	glDeleteVertexArrays(1, &m_Id);

	m_Id = other.m_Id;

	other.m_Id = 0;
	return *this;
}

inline GLuint VertexArray::Id() const noexcept
{
	return m_Id;
}

inline void VertexArray::Bind() const noexcept
{
	glBindVertexArray(m_Id);
}

inline void VertexArray::EnableAttribArray(GLuint index) const noexcept
{
	Bind();
	glEnableVertexAttribArray(index);
}

inline void VertexArray::DisableAttribArray(GLuint index) const noexcept
{
	Bind();
	glDisableVertexAttribArray(index);
}

inline void VertexArray::SetAttribDivisor(GLuint index, GLuint divisor) const noexcept
{
	Bind();
	glVertexAttribDivisor(index, divisor);
}

inline void VertexArray::Unbind() noexcept
{
	glBindVertexArray(0);
}

enum class BufferType
{
	ArrayBuffer = GL_ARRAY_BUFFER,
	AtomicCounterBuffer = GL_ATOMIC_COUNTER_BUFFER,
	CopyReadBuffer = GL_COPY_READ_BUFFER,
	CopyWriteBuffer = GL_COPY_WRITE_BUFFER,
	DispatchIndirectBuffer = GL_DISPATCH_INDIRECT_BUFFER,
	DrawIndirectBuffer = GL_DRAW_INDIRECT_BUFFER,
	ElementArrayBuffer = GL_ELEMENT_ARRAY_BUFFER,
	PixelPackBuffer = GL_PIXEL_PACK_BUFFER,
	PixelUnpackBuffer = GL_PIXEL_UNPACK_BUFFER,
	QueryBuffer = GL_QUERY_BUFFER,
	ShaderStorageBuffer = GL_SHADER_STORAGE_BUFFER,
	TextureBuffer = GL_TEXTURE_BUFFER,
	TransformFeedbackBuffer = GL_TRANSFORM_FEEDBACK_BUFFER,
	UniformBuffer = GL_UNIFORM_BUFFER,
};

enum class BufferUsage
{
	StreamDraw = GL_STREAM_DRAW,
	StreamRead = GL_STREAM_READ,
	StreamCopy = GL_STREAM_COPY,
	StaticDraw = GL_STATIC_DRAW,
	StaticRead = GL_STATIC_READ,
	StaticCopy = GL_STATIC_COPY,
	DynamicDraw = GL_DYNAMIC_DRAW,
	DynamicRead = GL_DYNAMIC_READ,
	DynamicCopy = GL_DYNAMIC_COPY
};

class Buffer
{
public:
	Buffer() = default;
	Buffer(BufferType type, GLsizeiptr size, const void* data, BufferUsage usage);
	~Buffer();

	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	Buffer(Buffer&& other) noexcept;
	Buffer& operator=(Buffer&& other) noexcept;

	GLuint Id() const noexcept;
	GLenum Type() const noexcept;
	void Bind() const noexcept;
	void SubData(GLintptr offset, GLsizeiptr size, const void* data) const noexcept;

	static void Unbind(BufferType type) noexcept;

private:
	GLuint m_Id = 0;
	GLenum m_Type = 0;
};

inline Buffer::Buffer(BufferType type, GLsizeiptr size, const void* data, BufferUsage usage)
	: m_Type(static_cast<GLenum>(type))
{
	glGenBuffers(1, &m_Id);
	glBindBuffer(m_Type, m_Id);
	glBufferData(m_Type, size, data, static_cast<GLenum>(usage));
}

inline Buffer::~Buffer()
{
	glDeleteBuffers(1, &m_Id);
}

inline Buffer::Buffer(Buffer&& other) noexcept
	: m_Id(other.m_Id),
	m_Type(other.m_Type)
{
	other.m_Id = 0;
	other.m_Type = 0;
}

inline Buffer& Buffer::operator=(Buffer&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}

	// glDeleteBuffers silently ignores 0's and names that do not correspond to existing buffer objects.
	// https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDeleteBuffers.xhtml
	glDeleteBuffers(1, &m_Id);

	m_Id = other.m_Id;
	m_Type = other.m_Type;

	other.m_Id = 0;
	other.m_Type = 0;
	return *this;
}

inline GLuint Buffer::Id() const noexcept
{
	return m_Id;
}

inline GLenum Buffer::Type() const noexcept
{
	return m_Type;
}

inline void Buffer::Bind() const noexcept
{
	if (m_Id != 0 && m_Type != 0)
	{
		glBindBuffer(m_Type, m_Id);
	}
}

inline void Buffer::SubData(GLintptr offset, GLsizeiptr size, const void* data) const noexcept
{
	if (m_Id != 0 && m_Type != 0)
	{
		Bind();
		glBufferSubData(m_Type, offset, size, data);
	}
}

inline void Buffer::Unbind(BufferType type) noexcept
{
	glBindBuffer(static_cast<GLenum>(type), 0);
}

inline void VertexArray::SetAttribPointer(const Buffer& vbo, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer) const noexcept
{
	Bind();
	vbo.Bind();
	glVertexAttribPointer(index, size, type, normalized, stride, pointer);
}

inline void VertexArray::SetElementBuffer(const Buffer& ebo) const noexcept
{
	Bind();
	ebo.Bind();
}
