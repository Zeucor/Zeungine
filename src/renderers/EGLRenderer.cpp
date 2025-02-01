#ifdef USE_EGL
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/renderers/EGLRenderer.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/textures/TextureFactory.hpp>
#include <zg/vaos/VAOFactory.hpp>
#if defined(LINUX) || defined(MACOS)
#include <dlfcn.h>
#endif
#ifdef WINDOWS
#include <zg/windows/WIN32Window.hpp>
#endif
#ifdef LINUX
#include <zg/windows/X11Window.hpp>
#endif
#ifdef MACOS
#include <zg/windows/MacOSWindow.hpp>
#endif
using namespace zg;
EGLRenderer::EGLRenderer()
{
	eglDisplay = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (eglDisplay == EGL_NO_DISPLAY)
	{
		throw std::runtime_error("EGL_NO_DISPLAY");
	}
	if (!eglInitialize(eglDisplay, &major, &minor))
	{
    	throw std::runtime_error("eglInitialize error");
	}
    EGLint numConfigs;
    EGLint eglAttribs[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_NONE
    };
    if (!eglChooseConfig(eglDisplay, eglAttribs, &eglConfig, 1, &numConfigs))
    {
        throw std::runtime_error("eglChooseConfig error");
    }
}
EGLRenderer::~EGLRenderer(){}
#if defined(WINDOWS) || defined(LINUX)
void EGLRenderer::createContext(IPlatformWindow *platformWindowPointer)
{
	this->platformWindowPointer = platformWindowPointer;
#ifdef WINDOWS
	auto window = (*dynamic_cast<WIN32Window*>(platformWindowPointer)).hwnd;
#endif
#ifdef LINUX
	auto window = (*dynamic_cast<X11Window*>(platformWindowPointer)).window;
#endif
    eglSurface = eglCreateWindowSurface(eglDisplay, eglConfig, window, nullptr);
    if (eglSurface == EGL_NO_SURFACE)
    {
        throw std::runtime_error("EGL_NO_SURFACE");
    }
    EGLint contextAttribs[] = { EGL_CONTEXT_MAJOR_VERSION, 3, EGL_CONTEXT_MINOR_VERSION, 1, EGL_NONE };
    eglContext = eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, contextAttribs);
    if (eglContext == EGL_NO_CONTEXT)
    {
        throw std::runtime_error("EGL_NO_CONTEXT");
    }
    if (!eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext))
    {
        throw std::runtime_error("eglMakeCurrent failed!");
    }
}
#endif
void EGLRenderer::init()
{
	glEnable(GL_DEPTH_TEST);
	GLcheck(*this, "glEnable");
	glEnable(GL_CULL_FACE);
	GLcheck(*this, "glEnable");
	glCullFace(GL_BACK);
	GLcheck(*this, "glCullFace");
	glFrontFace(GL_CCW);
	GLcheck(*this, "glFrontFace");
	glViewport(0, 0, platformWindowPointer->renderWindowPointer->windowWidth,
											platformWindowPointer->renderWindowPointer->windowHeight);
	GLcheck(*this, "glViewport");
	glClearDepthf(1.0);
	GLcheck(*this, "glClearDepth");
	glDepthRangef(0.0, 1.0);
	GLcheck(*this, "glDepthRange");
	glEnable(GL_BLEND);
	GLcheck(*this, "glEnable:GL_BLEND");
	glDisable(GL_DITHER);
	GLcheck(*this, "glDisable:GL_DITHER");
#ifndef MACOS
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLcheck(*this, "glBlendFunc");
	glEnable(GL_DEBUG_OUTPUT);
	GLcheck(*this, "glEnable");
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	GLcheck(*this, "glEnable");
	glDebugMessageCallback(
		[](GLuint source, GLuint type, GLuint id, GLuint severity, GLsizei length, const GLchar* message,
			 const void* userParam)
		{
			if (type == GL_DEBUG_TYPE_ERROR)
			{
				std::cerr << "OpenGL Debug Message: " << message << std::endl;
			}
		},
		nullptr);
#endif
}
void EGLRenderer::destroy() {}
std::shared_ptr<IRenderer> zg::createRenderer() { return std::shared_ptr<IRenderer>(new EGLRenderer()); }
void EGLRenderer::clearColor(glm::vec4 color)
{
	glClearColor(color.r, color.g, color.b, color.a);
	GLcheck(*this, "glClearColor");
}
void EGLRenderer::clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	GLcheck(*this, "glClear");
}
void EGLRenderer::viewport(glm::ivec4 vp) const
{
	glViewport(vp.x, vp.y, vp.z, vp.w);
	GLcheck(*this, "glViewport");
}
void EGLRenderer::setUniform(shaders::Shader& shader, const std::string_view name, const void* pointer, uint32_t size,
														enums::EUniformType uniformType)
{
	GLint location = glGetUniformLocation(shader.program, name.data());
	GLcheck(*this, "glGetUniformLocation");
	if (location == -1)
	{
		return;
	}
	switch (uniformType)
	{
	case enums::EUniformType::_1b:
		{
			glUniform1i(location, (int32_t)(*(bool*)pointer));
			GLcheck(*this, "glUniform1i");
			return;
		}
	case enums::EUniformType::_1i:
		{
			glUniform1i(location, *(int32_t*)pointer);
			GLcheck(*this, "glUniform1i");
			return;
		}
	case enums::EUniformType::_1ui:
		{
			glUniform1ui(location, *(uint32_t*)pointer);
			GLcheck(*this, "glUniform1ui");
			return;
		}
	case enums::EUniformType::_1fv:
		{
			glUniform1fv(location, size, &((float**)pointer)[0][0]);
			GLcheck(*this, "glUniform1fv");
			return;
		}
	case enums::EUniformType::_1f:
		{
			glUniform1f(location, *(float*)pointer);
			GLcheck(*this, "glUniform1f");
			return;
		}
	case enums::EUniformType::_2fv:
		{
			glUniform2fv(location, 1, &(*(glm::vec2*)pointer)[0]);
			GLcheck(*this, "glUniform2fv");
			return;
		}
	case enums::EUniformType::_3fv:
		{
			glUniform3fv(location, 1, &(*(glm::vec3*)pointer)[0]);
			GLcheck(*this, "glUniform3fv");
			return;
		}
	case enums::EUniformType::_4fv:
		{
			glUniform4fv(location, 1, &(*(glm::vec4*)pointer)[0]);
			GLcheck(*this, "glUniform4fv");
			return;
		}
	case enums::EUniformType::_Matrix2fv:
		{
			glUniformMatrix2fv(location, 1, GL_FALSE, &(*(glm::mat2*)pointer)[0][0]);
			GLcheck(*this, "glUniformMatrix2fv");
			return;
		}
	case enums::EUniformType::_Matrix3fv:
		{
			glUniformMatrix3fv(location, 1, GL_FALSE, &(*(glm::mat3*)pointer)[0][0]);
			GLcheck(*this, "glUniformMatrix3fv");
			return;
		}
	case enums::EUniformType::_Matrix4fv:
		{
			glUniformMatrix4fv(location, 1, GL_FALSE, &(*(glm::mat4*)pointer)[0][0]);
			GLcheck(*this, "glUniformMatrix4fv");
			return;
		}
	}
	assert(false && "Should not reach here");
}
void EGLRenderer::setBlock(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size)
{
	auto blockIndex = glGetUniformBlockIndex(shader.program, name.data());
	if (blockIndex == -1)
	{
		return;
	}
	auto& uboBinding = shader.uboBindings[name];
	auto& bindingIndex = std::get<0>(uboBinding);
	glUniformBlockBinding(shader.program, blockIndex, bindingIndex);
	GLcheck(*this, "glUniformBlockBinding");
	auto& uboBufferIndex = std::get<1>(uboBinding);
	glBindBuffer(GL_UNIFORM_BUFFER, uboBufferIndex);
	GLcheck(*this, "glBindBuffer");
	glBufferData(GL_UNIFORM_BUFFER, size, pointer, GL_STATIC_DRAW);
	GLcheck(*this, "glBufferData");
	glBindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, uboBufferIndex, 0, size);
	GLcheck(*this, "glBindBufferRange");
}
void EGLRenderer::deleteBuffer(uint32_t id)
{
	glDeleteBuffers(1, &id);
	GLcheck(*this, "glDeleteBuffers");
}
void EGLRenderer::bindShader(const shaders::Shader& shader)
{
	glUseProgram(shader.program);
	GLcheck(*this, "glUseProgram");
}
void EGLRenderer::unbindShader(const shaders::Shader& shader)
{
	glUseProgram(0);
	GLcheck(*this, "glUseProgram");
}
void EGLRenderer::addSSBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex)
{
	GLuint ssboBufferID;
	glGenBuffers(1, &ssboBufferID);
	GLcheck(*this, "glGenBuffers");
	auto& ssboBinding = shader.ssboBindings[name];
	std::get<0>(ssboBinding) = bindingIndex;
	std::get<1>(ssboBinding) = ssboBufferID;
}
void EGLRenderer::addUBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex)
{
	GLuint uboBufferID;
	glGenBuffers(1, &uboBufferID);
	GLcheck(*this, "glGenBuffers");
	auto& uboBinding = shader.uboBindings[name];
	std::get<0>(uboBinding) = bindingIndex;
	std::get<1>(uboBinding) = uboBufferID;
}
void EGLRenderer::setSSBO(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size)
{
	auto ssboIter = shader.ssboBindings.find(name.data());
	if (ssboIter == shader.ssboBindings.end())
	{
		return;
	}
	auto& bindingIndex = std::get<0>(ssboIter->second);
	auto& buffer = std::get<1>(ssboIter->second);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, buffer);
	GLcheck(*this, "glBindBufferBase");
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
	GLcheck(*this, "glBindBuffer");
	glBufferData(GL_SHADER_STORAGE_BUFFER, size, pointer, GL_STATIC_DRAW);
	GLcheck(*this, "glBufferData");
}
void EGLRenderer::setTexture(shaders::Shader& shader, const std::string_view name, const textures::Texture& texture,
														const int32_t unit)
{
	shader.setUniform(name, unit);
	glActiveTexture(GL_TEXTURE0 + unit);
	GLcheck(*this, "glActiveTexture");
	texture.bind();
}
bool EGLRenderer::compileShader(shaders::Shader& shader, shaders::ShaderType shaderType, shaders::ShaderPair& shaderPair)
{
	auto& shaderString = shaderPair.first;
	auto& shaderInt = shaderPair.second;
	shaderInt = glCreateShader(shaders::ShaderFactory::shaderTypes[shaderType]);
	GLcheck(*this, "glCreateShader");
	const GLchar* source = shaderString.c_str();
	GLint lengths[] = {(GLint)shaderString.size()};
	glShaderSource(shaderInt, 1, &(source), lengths);
	GLcheck(*this, "glShaderSource");
	glCompileShader(shaderInt);
	GLcheck(*this, "glCompileShader");
	auto success = checkCompileErrors(shader, shaderInt, true, shaders::ShaderFactory::shaderNames[shaderType].data());
	return success;
}
bool EGLRenderer::compileProgram(shaders::Shader& shader, const shaders::ShaderMap& shaderMap)
{
	shader.program = glCreateProgram();
	GLcheck(*this, "glCreateProgram");
	for (const auto& shaderMapPair : shaderMap)
	{
		glAttachShader(shader.program, shaderMapPair.second.second);
		GLcheck(*this, "glAttachShader");
	}
	glLinkProgram(shader.program);
	for (const auto& shaderMapPair : shaderMap)
	{
		glDeleteShader(shaderMapPair.second.second);
		GLcheck(*this, "glDeleteShader");
	}
	auto success = checkCompileErrors(shader, shader.program, false, "Program");
	return success;
}
bool EGLRenderer::checkCompileErrors(shaders::Shader& shader, const GLuint& id, bool isShader, const char* shaderType)
{
	GLint success = 0;
	if (isShader)
	{
		glGetShaderiv(id, GL_COMPILE_STATUS, &success);
		GLcheck(*this, "glGetShaderiv");
		if (!success)
		{
			GLint infoLogLength;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
			GLcheck(*this, "glGetShaderiv");
			GLchar* infoLog = new GLchar[infoLogLength + 1];
			glGetShaderInfoLog(id, infoLogLength, 0, infoLog);
			GLcheck(*this, "glGetShaderInfoLog");
			printf("SHADER_COMPILATION_ERROR(%s):\n%s\n", shaderType, infoLog);
			delete[] infoLog;
			return false;
		}
	}
	else
	{
		glGetProgramiv(id, GL_LINK_STATUS, &success);
		GLcheck(*this, "glGetProgramiv");
		if (!success)
		{
			GLint infoLogLength;
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
			GLcheck(*this, "glGetProgramiv");
			GLchar* infoLog = new GLchar[infoLogLength + 1];
			glGetProgramInfoLog(id, infoLogLength, 0, infoLog);
			GLcheck(*this, "glGetProgramInfoLog");
			printf("PROGRAM_LINKING_ERROR(%s):\n%s\n", shaderType, infoLog);
			delete[] infoLog;
			return false;
		}
	}
	return true;
}
void EGLRenderer::deleteShader(shaders::Shader& shader)
{
	glDeleteProgram(shader.program);
	GLcheck(*this, "glDeleteProgram");
}
void EGLRenderer::bindFramebuffer(const textures::Framebuffer& framebuffer) const
{
	auto& framebufferImpl = *(textures::GLFramebufferImpl*)framebuffer.rendererData;
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferImpl.id);
	GLcheck(*this, "glBindFramebuffer");
	viewport({0, 0, framebuffer.texture.size.x, framebuffer.texture.size.y});
}
void EGLRenderer::unbindFramebuffer(const textures::Framebuffer& framebuffer) const
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLcheck(*this, "glBindFramebuffer");
}
void EGLRenderer::initFramebuffer(textures::Framebuffer& framebuffer)
{
	framebuffer.rendererData = new textures::GLFramebufferImpl();
	auto& framebufferImpl = *(textures::GLFramebufferImpl*)framebuffer.rendererData;
	auto& textureImpl = *(textures::GLTextureImpl*)framebuffer.texture.rendererData;
	if (framebuffer.depthTexturePointer)
	{
		glGenRenderbuffers(1, &framebufferImpl.renderbufferID);
		GLcheck(*this, "glGenRenderbuffers");
	}
	glGenFramebuffers(1, &framebufferImpl.id);
	GLcheck(*this, "glGenFramebuffers");
	glBindFramebuffer(GL_FRAMEBUFFER, framebufferImpl.id);
	GLcheck(*this, "glBindFramebuffer");
	GLenum frameBufferTarget;
	switch (framebuffer.texture.format)
	{
	case textures::Texture::Depth:
	{
		frameBufferTarget = GL_DEPTH_ATTACHMENT;
		GLenum noneBuffer = GL_NONE;
		glDrawBuffers(1, &noneBuffer);
		glReadBuffer(GL_NONE);
		break;
	}
	default:
		frameBufferTarget = GL_COLOR_ATTACHMENT0;
		break;
	}
	if (framebuffer.depthTexturePointer)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, framebufferImpl.renderbufferID);
		GLcheck(*this, "glBindRenderbuffer");
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, framebuffer.depthTexturePointer->size.x,
																	 framebuffer.depthTexturePointer->size.y);
		GLcheck(*this, "glRenderbufferStorage");
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
																			 framebufferImpl.renderbufferID);
		GLcheck(*this, "glFramebufferRenderbuffer");
	}
	if (textureImpl.target == GL_TEXTURE_CUBE_MAP)
	{
#ifdef USE_GL
		glFramebufferTexture(GL_FRAMEBUFFER, frameBufferTarget, textureImpl.id, 0);
		GLcheck(*this, "glFramebufferTexture");
#elif defined(USE_EGL)
		int count = 0;
		for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; count < 6; count++, face++)
		{
			glFramebufferTexture2D(GL_FRAMEBUFFER, frameBufferTarget, face, textureImpl.id, 0);
			GLcheck(*this, "glFramebufferTexture2D");
		}
#endif
	}
	else
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, frameBufferTarget, GL_TEXTURE_2D, textureImpl.id, 0);
		GLcheck(*this, "glFramebufferTexture2D");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	GLcheck(*this, "glBindFramebuffer");
}
void EGLRenderer::destroyFramebuffer(textures::Framebuffer& framebuffer)
{
	auto& framebufferImpl = *(textures::GLFramebufferImpl*)framebuffer.rendererData;
	glDeleteFramebuffers(1, &framebufferImpl.id);
	GLcheck(*this, "glDeleteFramebuffers");
	delete &framebufferImpl;
}
void EGLRenderer::bindTexture(const textures::Texture& texture)
{
	auto& textureImpl = *(textures::GLTextureImpl*)texture.rendererData;
	glBindTexture(textureImpl.target, textureImpl.id);
	GLcheck(*this, "glBindTexture");
}
void EGLRenderer::unbindTexture(const textures::Texture& texture)
{
	auto& textureImpl = *(textures::GLTextureImpl*)texture.rendererData;
	glBindTexture(textureImpl.target, 0);
	GLcheck(*this, "glBindTexture");
}
void EGLRenderer::preInitTexture(textures::Texture& texture)
{
	texture.rendererData = new textures::GLTextureImpl();
	auto& textureImpl = *(textures::GLTextureImpl*)texture.rendererData;
	glGenTextures(1, &textureImpl.id);
	GLcheck(*this, "glGenTextures");
	if (texture.size.w > 0)
		textureImpl.target = GL_TEXTURE_CUBE_MAP;
	else if (texture.size.y == 0)
		throw std::runtime_error("GL_TEXTURE_1D not supported in EGL");
	else if (texture.size.z <= 1)
		textureImpl.target = GL_TEXTURE_2D;
	else
		textureImpl.target = GL_TEXTURE_3D;
	texture.bind();
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	GLcheck(*this, "glPixelStorei");
}
void EGLRenderer::midInitTexture(const textures::Texture& texture,
																const std::vector<images::ImageLoader::ImagePair>& images)
{
	auto& textureImpl = *(textures::GLTextureImpl*)texture.rendererData;
	if (textureImpl.target == GL_TEXTURE_2D)
	{
		void* data = images.size() ? std::get<1>(images[0]).get() : 0;
		glTexImage2D(textureImpl.target, 0, textures::TextureFactory::internalFormats[texture.format], texture.size.x,
													texture.size.y, 0, textures::TextureFactory::formats[texture.format],
													textures::TextureFactory::types[{texture.format, texture.type}], data);
		GLcheck(*this, "glTexImage2D");
	}
	else if (textureImpl.target == GL_TEXTURE_3D)
	{
		void* data = images.size() ? std::get<1>(images[0]).get() : 0;
		glTexImage3D(textureImpl.target, 0, textures::TextureFactory::internalFormats[texture.format], texture.size.x,
													texture.size.y, texture.size.z, 0, textures::TextureFactory::formats[texture.format],
													textures::TextureFactory::types[{texture.format, texture.type}], data);
		GLcheck(*this, "glTexImage3D");
	}
	else if (textureImpl.target == GL_TEXTURE_CUBE_MAP)
	{
		for (uint8_t face = 0; face < 6; ++face)
		{
			bool haveImage = images.size() > face;
			auto imageSize = haveImage ? std::get<0>(images[face]) : glm::uvec2(0, 0);
			void* data = haveImage ? std::get<1>(images[face]).get() : 0;
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
														textures::TextureFactory::internalFormats[texture.format], imageSize.x, imageSize.y, 0,
														textures::TextureFactory::formats[texture.format],
														textures::TextureFactory::types[{texture.format, texture.type}], data);
			GLcheck(*this, "glTexImage2D");
		}
	}
}
void EGLRenderer::postInitTexture(const textures::Texture& texture)
{
	auto& textureImpl = *(textures::GLTextureImpl*)texture.rendererData;
	GLenum filterType;
	switch (texture.filterType)
	{
	case textures::Texture::FilterType::Nearest:
		filterType = GL_NEAREST;
		break;
	case textures::Texture::FilterType::Linear:
		filterType = GL_LINEAR;
		break;
	}
	glTexParameteri(textureImpl.target, GL_TEXTURE_MIN_FILTER, filterType);
	GLcheck(*this, "glTexParameteri");
	glTexParameteri(textureImpl.target, GL_TEXTURE_MAG_FILTER, filterType);
	GLcheck(*this, "glTexParameteri");
	glTexParameteri(textureImpl.target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	GLcheck(*this, "glTexParameteri");
	glTexParameteri(textureImpl.target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	GLcheck(*this, "glTexParameteri");
	glTexParameteri(textureImpl.target, GL_TEXTURE_WRAP_R, GL_REPEAT);
	GLcheck(*this, "glTexParameteri");
	texture.unbind();
}
void EGLRenderer::destroyTexture(textures::Texture& texture)
{
	auto& textureImpl = *(textures::GLTextureImpl*)texture.rendererData;
	glDeleteTextures(1, &textureImpl.id);
	GLcheck(*this, "glDeleteTextures");
	delete &textureImpl;
}
void EGLRenderer::updateIndicesVAO(const vaos::VAO &vao, const std::vector<uint32_t>& indices)
{
	auto& vaoImpl = *(vaos::GLVAOImpl*)vao.rendererData;
	glBindVertexArray(vaoImpl.vao);
	GLcheck(*this, "glBindVertexArray");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vaoImpl.ebo);
	GLcheck(*this, "glBindBuffer");
	auto& constantSize = vaos::VAOFactory::constantSizes["Indice"];
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, vao.indiceCount * std::get<1>(constantSize), indices.data(),
												GL_STATIC_DRAW);
	GLcheck(*this, "glBufferData");
	glBindVertexArray(0);
	GLcheck(*this, "glBindVertexArray");
}
void EGLRenderer::updateElementsVAO(const vaos::VAO &vao, const std::string_view constant, uint8_t* elementsAsChar)
{
	auto& vaoImpl = *(vaos::GLVAOImpl*)vao.rendererData;
	glBindVertexArray(vaoImpl.vao);
	GLcheck(*this, "glBindVertexArray");
	glBindBuffer(GL_ARRAY_BUFFER, vaoImpl.vbo);
	GLcheck(*this, "glBindBuffer");
	auto& constantSize = vaos::VAOFactory::constantSizes[constant];
	auto offset = vaos::VAOFactory::getOffset(vao.constants, constant);
	auto elementStride = std::get<0>(constantSize) * std::get<1>(constantSize);
	for (size_t index = offset, c = 1, elementIndex = 0; c <= vao.elementCount;
			 index += vao.stride, c++, elementIndex += elementStride)
	{
		glBufferSubData(GL_ARRAY_BUFFER, index, elementStride, &elementsAsChar[elementIndex]);
		GLcheck(*this, "glBufferSubData");
	}
	glBindVertexArray(0);
	GLcheck(*this, "glBindVertexArray");
}
void EGLRenderer::drawVAO(const vaos::VAO &vao)
{
	auto& vaoImpl = *(vaos::GLVAOImpl*)vao.rendererData;
	glBindVertexArray(vaoImpl.vao);
	GLcheck(*this, "glBindVertexArray");
	GLenum drawMode = GL_TRIANGLES;
	glDrawElements(drawMode, vao.indiceCount, ZG_UNSIGNED_INT, 0);
	GLcheck(*this, "glDrawElements");
}
void EGLRenderer::generateVAO(vaos::VAO &vao)
{
	vao.rendererData = new vaos::GLVAOImpl();
	auto& vaoImpl = *(vaos::GLVAOImpl*)vao.rendererData;
	glGenVertexArrays(1, &vaoImpl.vao);
	GLcheck(*this, "glGenVertexArrays");
	glGenBuffers(1, &vaoImpl.ebo);
	GLcheck(*this, "glGenBuffers");
	glGenBuffers(1, &vaoImpl.vbo);
	GLcheck(*this, "glGenBuffers");
	glBindVertexArray(vaoImpl.vao);
	GLcheck(*this, "glBindVertexArray");
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vaoImpl.ebo);
	GLcheck(*this, "glBindBuffer");
	glBindBuffer(GL_ARRAY_BUFFER, vaoImpl.vbo);
	GLcheck(*this, "glBindBuffer");
	auto stride = vaos::VAOFactory::getStride(vao.constants);
	glBufferData(GL_ARRAY_BUFFER, stride * vao.elementCount, 0, GL_STATIC_DRAW);
	GLcheck(*this, "glBufferData");
	size_t attribIndex = 0;
	size_t offset = 0;
	for (auto& constant : vao.constants)
	{
		if (!vaos::VAOFactory::isVAOConstant(constant))
			continue;
		glEnableVertexAttribArray(attribIndex);
		GLcheck(*this, "glEnableVertexAttribArray");
		auto& constantSize = vaos::VAOFactory::constantSizes[constant];
		if (std::get<2>(constantSize) == ZG_FLOAT)
		{
			glVertexAttribPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize),
																								GL_FALSE, stride, (const void*)offset);
			GLcheck(*this, "glVertexAttribPointer");
		}
		else
		{
			glVertexAttribIPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize),
																								 stride, (const void*)offset);
			GLcheck(*this, "glVertexAttribIPointer");
		}
		offset += std::get<0>(constantSize) * std::get<1>(constantSize);
		attribIndex++;
	}
}
void EGLRenderer::destroyVAO(vaos::VAO &vao)
{
	auto& vaoImpl = *(vaos::GLVAOImpl*)vao.rendererData;
	glDeleteVertexArrays(1, &vaoImpl.vao);
	GLcheck(*this, "glDeleteVertexArrays");
	vaoImpl.vao = 0;
	glDeleteBuffers(1, &vaoImpl.vbo);
	GLcheck(*this, "glDeleteBuffers");
	vaoImpl.vbo = 0;
	glDeleteBuffers(1, &vaoImpl.ebo);
	GLcheck(*this, "glDeleteBuffers");
	vaoImpl.ebo = 0;
	delete &vaoImpl;
}
const bool zg::GLcheck(const EGLRenderer& renderer, const char* fn, const bool egl)
{
#ifndef NDEBUG
	while (true)
	{
		uint32_t err = 0;
		if (!egl)
			err = glGetError();
		else if (egl)
			err = eglGetError();
		if (err == GL_NO_ERROR || err == EGL_SUCCESS)
		{
			/*|
				|*/
			break;
		}
		switch (err)
		{
		case GL_INVALID_ENUM:
			{
				Logger::print(Logger::Error, "GL_INVALID_ENUM", "(", fn, "): ",
											"An unacceptable value is specified for an enumerated argument. The offending command is ignored "
											"and has no other side effect than to set the error flag.");
				break;
			};
		case GL_INVALID_VALUE:
			{
				Logger::print(Logger::Error, "GL_INVALID_VALUE", "(", fn, "): ",
											"A numeric argument is out of range. The offending command is ignored and has no other side "
											"effect than to set the error flag.");
				break;
			};
		case GL_INVALID_OPERATION:
			{
				Logger::print(Logger::Error, "GL_INVALID_OPERATION", "(", fn, "): ",
											"The specified operation is not allowed in the current state. The offending command is ignored "
											"and has no other side effect than to set the error flag.");
				break;
			};
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			{
				Logger::print(Logger::Error, "GL_INVALID_FRAMEBUFFER_OPERATION", "(", fn, "): ",
											"The framebuffer object is not complete. The offending command is ignored and has no other side "
											"effect than to set the error flag.");
				break;
			};
		case GL_OUT_OF_MEMORY:
			{
				Logger::print(Logger::Error, "GL_OUT_OF_MEMORY", "(", fn, "): ",
											"There is not enough memory left to execute the command. The state of the GL is undefined, "
											"except for the state of the error flags, after this error is recorded.");
				break;
			};
		case GL_STACK_UNDERFLOW:
			{
				Logger::print(
					Logger::Error, "GL_STACK_UNDERFLOW", "(", fn,
					"): ", "An attempt has been made to perform an operation that would cause an internal stack to underflow.");
				break;
			};
		case GL_STACK_OVERFLOW:
			{
				Logger::print(
					Logger::Error, "GL_STACK_OVERFLOW", "(", fn,
					"): ", "An attempt has been made to perform an operation that would cause an internal stack to overflow.");
				break;
			};
		case EGL_NOT_INITIALIZED:
			{
				Logger::print(Logger::Error, "EGL_NOT_INITIALIZED", "(", fn, "): ",
											"EGL is not initialized, or could not be initialized, for the specified EGL display connection.");
				break;
			};
		case EGL_BAD_ACCESS:
			{
				Logger::print(Logger::Error, "EGL_BAD_ACCESS", "(", fn, "): ",
											"EGL cannot access a requested resource (for example a context is bound in another thread).");
				break;
			};
		case EGL_BAD_ALLOC:
			{
				Logger::print(Logger::Error, "EGL_BAD_ALLOC", "(", fn,
											"): ", "EGL failed to allocate resources for the requested operation.");
				break;
			};
		case EGL_BAD_ATTRIBUTE:
			{
				Logger::print(Logger::Error, "EGL_BAD_ATTRIBUTE", "(", fn,
											"): ", "An unrecognized attribute or attribute value was passed in the attribute list.");
				break;
			};
		case EGL_BAD_CONTEXT:
			{
				Logger::print(Logger::Error, "EGL_BAD_CONTEXT", "(", fn,
											"): ", "An EGLContext argument does not name a valid EGL rendering context.");
				break;
			};
		case EGL_BAD_CONFIG:
			{
				Logger::print(Logger::Error, "EGL_BAD_CONFIG", "(", fn,
											"): ", "An EGLConfig argument does not name a valid EGL frame buffer configuration.");
				break;
			};
		case EGL_BAD_CURRENT_SURFACE:
			{
				Logger::print(
					Logger::Error, "EGL_BAD_CURRENT_SURFACE", "(", fn, "): ",
					"The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid.");
				break;
			};
		case EGL_BAD_DISPLAY:
			{
				Logger::print(Logger::Error, "EGL_BAD_DISPLAY", "(", fn,
											"): ", "An EGLDisplay argument does not name a valid EGL display connection.");
				break;
			};
		case EGL_BAD_SURFACE:
			{
				Logger::print(Logger::Error, "EGL_BAD_SURFACE", "(", fn, "): ",
											"An EGLSurface argument does not name a valid surface (window, pixel buffer or pixmap) "
											"configured for GL rendering.");
				break;
			};
		case EGL_BAD_MATCH:
			{
				Logger::print(Logger::Error, "EGL_BAD_MATCH", "(", fn, "): ",
											"Arguments are inconsistent (for example, a valid context requires buffers not supplied by a "
											"valid surface).");
				break;
			};
		case EGL_BAD_PARAMETER:
			{
				Logger::print(Logger::Error, "EGL_BAD_PARAMETER", "(", fn, "): ", "One or more argument values are invalid.");
				break;
			};
		case EGL_BAD_NATIVE_PIXMAP:
			{
				Logger::print(Logger::Error, "EGL_BAD_NATIVE_PIXMAP", "(", fn,
											"): ", "A NativePixmapType argument does not refer to a valid native pixmap.");
				break;
			};
		case EGL_BAD_NATIVE_WINDOW:
			{
				Logger::print(Logger::Error, "EGL_BAD_NATIVE_WINDOW", "(", fn,
											"): ", "A NativeWindowType argument does not refer to a valid native window.");
				break;
			};
		case EGL_CONTEXT_LOST:
			{
				Logger::print(Logger::Error, "EGL_CONTEXT_LOST", "(", fn, "): ",
											"A power management event has occurred. The application must destroy all contexts and "
											"reinitialise OpenGL ES state and objects to continue rendering.");
				break;
			};
		}
	}
#endif
	return true;
}
#endif
