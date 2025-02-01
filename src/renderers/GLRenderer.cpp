#ifdef USE_GL
#include <zg/Logger.hpp>
#include <zg/Window.hpp>
#include <zg/entities/Plane.hpp>
#include <zg/renderers/GLRenderer.hpp>
#include <zg/shaders/ShaderFactory.hpp>
#include <zg/textures/TextureFactory.hpp>
#include <zg/vaos/VAOFactory.hpp>
#ifdef _WIN32
#include <zg/SharedLibrary.hpp>
#endif
#if defined(LINUX) || defined(MACOS)
#include <dlfcn.h>
#endif
#ifdef LINUX
#include <zg/windows/X11Window.hpp>
#endif
using namespace zg;
#ifdef _WIN32
typedef BOOL(APIENTRY* PFNWGLSWAPINTERVALEXTPROC)(int interval);
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = nullptr;
zg::SharedLibrary opengl32Library("opengl32.dll");
#endif
void* getProc(const char* name)
{
#ifdef _WIN32
	void* p = (void*)wglGetProcAddress(name);
	if (p == 0 || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
	{
		return opengl32Library.getProc<void*>(name);
	}
	return p;
#elif defined(LINUX) || defined(MACOS)
	return dlsym(RTLD_DEFAULT, name);
#endif
}
GLRenderer::GLRenderer() : glContext(new GladGLContext) {}
GLRenderer::~GLRenderer() { delete glContext; }
#ifdef LINUX
#define GLX_CONTEXT_MAJOR_VERSION_ARB 0x2091
#define GLX_CONTEXT_MINOR_VERSION_ARB 0x2092
typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
static bool ctxErrorOccurred = false;
static int ctxErrorHandler(Display* dpy, XErrorEvent* ev)
{
	ctxErrorOccurred = true;
	return 0;
}
static bool isExtensionSupported(const char* extList, const char* extension)
{
	const char* start;
	const char *where, *terminator;

	/* Extension names should not have spaces. */
	where = strchr(extension, ' ');
	if (where || *extension == '\0')
		return false;

	/* It takes a bit of care to be fool-proof about parsing the
		 OpenGL extensions string. Don't be fooled by sub-strings,
		 etc. */
	for (start = extList;;)
	{
		where = strstr(start, extension);

		if (!where)
			break;

		terminator = where + strlen(extension);

		if (where == start || *(where - 1) == ' ')
			if (*terminator == ' ' || *terminator == '\0')
				return true;

		start = terminator;
	}

	return false;
}
#endif
#if defined(WINDOWS) || defined(LINUX)
void GLRenderer::createContext(IPlatformWindow* platformWindowPointer)
{
	this->platformWindowPointer = platformWindowPointer;
#ifdef WINDOWS
	HGLRC hTempRC = wglCreateContext(hDeviceContext);
	wglMakeCurrent(hDeviceContext, hTempRC);
	wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	wglMakeCurrent(nullptr, nullptr);
	int attribList[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB,		4, WGL_CONTEXT_MINOR_VERSION_ARB, 3, WGL_CONTEXT_PROFILE_MASK_ARB,
		WGL_CONTEXT_CORE_PROFILE_BIT_ARB, 0};
	hRenderingContext = wglCreateContextAttribsARB(hDeviceContext, 0, attribList);
	wglDeleteContext(hTempRC);
	wglMakeCurrent(hDeviceContext, hRenderingContext);
#endif
#ifdef LINUX
	auto& x11Window = *dynamic_cast<X11Window*>(platformWindowPointer);
	const char* glxExts = glXQueryExtensionsString(x11Window.display, x11Window.screen);
	glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 0;
	glXCreateContextAttribsARB =
		(glXCreateContextAttribsARBProc)glXGetProcAddressARB((const GLubyte*)"glXCreateContextAttribsARB");
	ctxErrorOccurred = false;
	int (*oldHandler)(Display*, XErrorEvent*) = XSetErrorHandler(&ctxErrorHandler);
	if (!isExtensionSupported(glxExts, "GLX_ARB_create_context") || !glXCreateContextAttribsARB)
	{
		x11Window.glcontext = glXCreateNewContext(x11Window.display, x11Window.bestFbc, GLX_RGBA_TYPE, 0, True);
	}
	else
	{
		int context_attribs[] = {
			GLX_CONTEXT_MAJOR_VERSION_ARB,		4,	 GLX_CONTEXT_MINOR_VERSION_ARB, 6, GLX_CONTEXT_PROFILE_MASK_ARB,
			GLX_CONTEXT_CORE_PROFILE_BIT_ARB, None};
		x11Window.glcontext = glXCreateContextAttribsARB(x11Window.display, x11Window.bestFbc, 0, True, context_attribs);
		XSync(x11Window.display, False);
	}
	glXMakeCurrent(x11Window.display, x11Window.window, x11Window.glcontext);
#endif
}
#endif
void GLRenderer::init()
{
	gladLoadGLContext(glContext, (GLADloadfunc)getProc);
	glContext->Enable(GL_DEPTH_TEST);
	GLcheck(*this, "glEnable");
	glContext->Enable(GL_CULL_FACE);
	GLcheck(*this, "glEnable");
	glContext->CullFace(GL_BACK);
	GLcheck(*this, "glCullFace");
	glContext->FrontFace(GL_CCW);
	GLcheck(*this, "glFrontFace");
	glContext->Viewport(0, 0, platformWindowPointer->renderWindowPointer->windowWidth,
											platformWindowPointer->renderWindowPointer->windowHeight);
	GLcheck(*this, "glViewport");
	glContext->ClearDepth(1.0);
	GLcheck(*this, "glClearDepth");
	glContext->DepthRange(0.0, 1.0);
	GLcheck(*this, "glDepthRange");
	// glContext->Enable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	// GLcheck(*this, "glEnable:GL_SAMPLE_ALPHA_TO_COVERAGE");
	glContext->Enable(GL_BLEND);
	GLcheck(*this, "glEnable:GL_BLEND");
	glContext->Disable(GL_DITHER);
	GLcheck(*this, "glDisable:GL_DITHER");
	glContext->Disable(GL_POLYGON_SMOOTH);
	GLcheck(*this, "glDisable:GL_POLYGON_SMOOTH");
#ifndef MACOS
	// glContext->BlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
	// GLcheck(*this, "glBlendEquationSeparate");
	glContext->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	GLcheck(*this, "glBlendFunc");
	glContext->Enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	GLcheck(*this, "glEnable:GL_TEXTURE_CUBE_MAP_SEAMLESS");
	glContext->Enable(GL_DEBUG_OUTPUT);
	GLcheck(*this, "glEnable");
	glContext->Enable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	GLcheck(*this, "glEnable");
	glContext->DebugMessageCallback(
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
#ifdef _WIN32
	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
	wglSwapIntervalEXT(platformWindowPointer->renderWindowPointer->vsync);
#elif defined(LINUX)
	PFNGLXSWAPINTERVALEXTPROC glXSwapIntervalEXT =
		(PFNGLXSWAPINTERVALEXTPROC)glXGetProcAddressARB((const GLubyte*)"glXSwapIntervalEXT");
	auto& x11Window = *(X11Window*)platformWindowPointer;
	glXSwapIntervalEXT(x11Window.display, x11Window.window, platformWindowPointer->renderWindowPointer->vsync);
#endif
}
void GLRenderer::destroy() {}
std::shared_ptr<IRenderer> zg::createRenderer() { return std::shared_ptr<IRenderer>(new GLRenderer()); }
void GLRenderer::clearColor(glm::vec4 color)
{
	glContext->ClearColor(color.r, color.g, color.b, color.a);
	GLcheck(*this, "glClearColor");
}
void GLRenderer::clear()
{
	glContext->Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	GLcheck(*this, "glClear");
}
void GLRenderer::viewport(glm::ivec4 vp) const
{
	glContext->Viewport(vp.x, vp.y, vp.z, vp.w);
	GLcheck(*this, "glViewport");
}
void GLRenderer::setUniform(shaders::Shader& shader, const std::string_view name, const void* pointer, uint32_t size,
														enums::EUniformType uniformType)
{
	GLint location = glContext->GetUniformLocation(shader.program, name.data());
	GLcheck(*this, "glGetUniformLocation");
	if (location == -1)
	{
		return;
	}
	switch (uniformType)
	{
	case enums::EUniformType::_1b:
		{
			glContext->Uniform1i(location, (int32_t)(*(bool*)pointer));
			GLcheck(*this, "glUniform1i");
			return;
		}
	case enums::EUniformType::_1i:
		{
			glContext->Uniform1i(location, *(int32_t*)pointer);
			GLcheck(*this, "glUniform1i");
			return;
		}
	case enums::EUniformType::_1ui:
		{
			glContext->Uniform1ui(location, *(uint32_t*)pointer);
			GLcheck(*this, "glUniform1ui");
			return;
		}
	case enums::EUniformType::_1fv:
		{
			glContext->Uniform1fv(location, size, &((float**)pointer)[0][0]);
			GLcheck(*this, "glUniform1fv");
			return;
		}
	case enums::EUniformType::_1f:
		{
			glContext->Uniform1f(location, *(float*)pointer);
			GLcheck(*this, "glUniform1f");
			return;
		}
	case enums::EUniformType::_2fv:
		{
			glContext->Uniform2fv(location, 1, &(*(glm::vec2*)pointer)[0]);
			GLcheck(*this, "glUniform2fv");
			return;
		}
	case enums::EUniformType::_3fv:
		{
			glContext->Uniform3fv(location, 1, &(*(glm::vec3*)pointer)[0]);
			GLcheck(*this, "glUniform3fv");
			return;
		}
	case enums::EUniformType::_4fv:
		{
			glContext->Uniform4fv(location, 1, &(*(glm::vec4*)pointer)[0]);
			GLcheck(*this, "glUniform4fv");
			return;
		}
	case enums::EUniformType::_Matrix2fv:
		{
			glContext->UniformMatrix2fv(location, 1, GL_FALSE, &(*(glm::mat2*)pointer)[0][0]);
			GLcheck(*this, "glUniformMatrix2fv");
			return;
		}
	case enums::EUniformType::_Matrix3fv:
		{
			glContext->UniformMatrix3fv(location, 1, GL_FALSE, &(*(glm::mat3*)pointer)[0][0]);
			GLcheck(*this, "glUniformMatrix3fv");
			return;
		}
	case enums::EUniformType::_Matrix4fv:
		{
			glContext->UniformMatrix4fv(location, 1, GL_FALSE, &(*(glm::mat4*)pointer)[0][0]);
			GLcheck(*this, "glUniformMatrix4fv");
			return;
		}
	}
	assert(false && "Should not reach here");
}
void GLRenderer::setBlock(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size)
{
	auto blockIndex = glContext->GetUniformBlockIndex(shader.program, name.data());
	if (blockIndex == -1)
	{
		return;
	}
	auto& uboBinding = shader.uboBindings[name];
	auto& bindingIndex = std::get<0>(uboBinding);
	glContext->UniformBlockBinding(shader.program, blockIndex, bindingIndex);
	GLcheck(*this, "glUniformBlockBinding");
	auto& uboBufferIndex = std::get<1>(uboBinding);
	glContext->BindBuffer(GL_UNIFORM_BUFFER, uboBufferIndex);
	GLcheck(*this, "glBindBuffer");
	glContext->BufferData(GL_UNIFORM_BUFFER, size, pointer, GL_STATIC_DRAW);
	GLcheck(*this, "glBufferData");
	glContext->BindBufferRange(GL_UNIFORM_BUFFER, bindingIndex, uboBufferIndex, 0, size);
	GLcheck(*this, "glBindBufferRange");
}
void GLRenderer::deleteBuffer(uint32_t id)
{
	glContext->DeleteBuffers(1, &id);
	GLcheck(*this, "glDeleteBuffers");
}
void GLRenderer::bindShader(const shaders::Shader& shader)
{
	glContext->UseProgram(shader.program);
	GLcheck(*this, "glUseProgram");
}
void GLRenderer::unbindShader(const shaders::Shader& shader)
{
	glContext->UseProgram(0);
	GLcheck(*this, "glUseProgram");
}
void GLRenderer::addSSBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex)
{
	GLuint ssboBufferID;
	glContext->GenBuffers(1, &ssboBufferID);
	GLcheck(*this, "glGenBuffers");
	auto& ssboBinding = shader.ssboBindings[name];
	std::get<0>(ssboBinding) = bindingIndex;
	std::get<1>(ssboBinding) = ssboBufferID;
}
void GLRenderer::addUBO(shaders::Shader& shader, const std::string_view name, uint32_t bindingIndex)
{
	GLuint uboBufferID;
	glContext->GenBuffers(1, &uboBufferID);
	GLcheck(*this, "glGenBuffers");
	auto& uboBinding = shader.uboBindings[name];
	std::get<0>(uboBinding) = bindingIndex;
	std::get<1>(uboBinding) = uboBufferID;
}
void GLRenderer::setSSBO(shaders::Shader& shader, const std::string_view name, const void* pointer, size_t size)
{
	auto ssboIter = shader.ssboBindings.find(name.data());
	if (ssboIter == shader.ssboBindings.end())
	{
		return;
	}
	auto& bindingIndex = std::get<0>(ssboIter->second);
	auto& buffer = std::get<1>(ssboIter->second);
	glContext->BindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, buffer);
	GLcheck(*this, "glBindBufferBase");
	glContext->BindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);
	GLcheck(*this, "glBindBuffer");
	glContext->BufferData(GL_SHADER_STORAGE_BUFFER, size, pointer, GL_STATIC_DRAW);
	GLcheck(*this, "glBufferData");
}
void GLRenderer::setTexture(shaders::Shader& shader, const std::string_view name, const textures::Texture& texture,
														const int32_t unit)
{
	shader.setUniform(name, unit);
	glContext->ActiveTexture(GL_TEXTURE0 + unit);
	GLcheck(*this, "glActiveTexture");
	texture.bind();
}
bool GLRenderer::compileShader(shaders::Shader& shader, shaders::ShaderType shaderType, shaders::ShaderPair& shaderPair)
{
	auto& shaderString = shaderPair.first;
	auto& shaderInt = shaderPair.second;
	shaderInt = glContext->CreateShader(shaders::ShaderFactory::shaderTypes[shaderType]);
	GLcheck(*this, "glCreateShader");
	const GLchar* source = shaderString.c_str();
	GLint lengths[] = {(GLint)shaderString.size()};
	glContext->ShaderSource(shaderInt, 1, &(source), lengths);
	GLcheck(*this, "glShaderSource");
	glContext->CompileShader(shaderInt);
	GLcheck(*this, "glCompileShader");
	auto success = checkCompileErrors(shader, shaderInt, true, shaders::ShaderFactory::shaderNames[shaderType].data());
	return success;
}
bool GLRenderer::compileProgram(shaders::Shader& shader, const shaders::ShaderMap& shaderMap)
{
	shader.program = glContext->CreateProgram();
	GLcheck(*this, "glCreateProgram");
	for (const auto& shaderMapPair : shaderMap)
	{
		glContext->AttachShader(shader.program, shaderMapPair.second.second);
		GLcheck(*this, "glAttachShader");
	}
	glContext->LinkProgram(shader.program);
	for (const auto& shaderMapPair : shaderMap)
	{
		glContext->DeleteShader(shaderMapPair.second.second);
		GLcheck(*this, "glDeleteShader");
	}
	auto success = checkCompileErrors(shader, shader.program, false, "Program");
	return success;
}
bool GLRenderer::checkCompileErrors(shaders::Shader& shader, const GLuint& id, bool isShader, const char* shaderType)
{
	GLint success = 0;
	if (isShader)
	{
		glContext->GetShaderiv(id, GL_COMPILE_STATUS, &success);
		GLcheck(*this, "glGetShaderiv");
		if (!success)
		{
			GLint infoLogLength;
			glContext->GetShaderiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
			GLcheck(*this, "glGetShaderiv");
			GLchar* infoLog = new GLchar[infoLogLength + 1];
			glContext->GetShaderInfoLog(id, infoLogLength, 0, infoLog);
			GLcheck(*this, "glGetShaderInfoLog");
			printf("SHADER_COMPILATION_ERROR(%s):\n%s\n", shaderType, infoLog);
			delete[] infoLog;
			return false;
		}
	}
	else
	{
		glContext->GetProgramiv(id, GL_LINK_STATUS, &success);
		GLcheck(*this, "glGetProgramiv");
		if (!success)
		{
			GLint infoLogLength;
			glContext->GetProgramiv(id, GL_INFO_LOG_LENGTH, &infoLogLength);
			GLcheck(*this, "glGetProgramiv");
			GLchar* infoLog = new GLchar[infoLogLength + 1];
			glContext->GetProgramInfoLog(id, infoLogLength, 0, infoLog);
			GLcheck(*this, "glGetProgramInfoLog");
			printf("PROGRAM_LINKING_ERROR(%s):\n%s\n", shaderType, infoLog);
			delete[] infoLog;
			return false;
		}
	}
	return true;
}
void GLRenderer::deleteShader(shaders::Shader& shader)
{
	glContext->DeleteProgram(shader.program);
	GLcheck(*this, "glDeleteProgram");
}
void GLRenderer::bindFramebuffer(const textures::Framebuffer& framebuffer) const
{
	auto& framebufferImpl = *(textures::GLFramebufferImpl*)framebuffer.rendererData;
	glContext->BindFramebuffer(GL_FRAMEBUFFER, framebufferImpl.id);
	GLcheck(*this, "glBindFramebuffer");
	viewport({0, 0, framebuffer.texture.size.x, framebuffer.texture.size.y});
}
void GLRenderer::unbindFramebuffer(const textures::Framebuffer& framebuffer) const
{
	glContext->BindFramebuffer(GL_FRAMEBUFFER, 0);
	GLcheck(*this, "glBindFramebuffer");
}
void GLRenderer::initFramebuffer(textures::Framebuffer& framebuffer)
{
	auto& framebufferImpl = *(textures::GLFramebufferImpl*)framebuffer.rendererData;
	auto& textureImpl = *(textures::GLTextureImpl*)framebuffer.texture.rendererData;
	if (framebuffer.depthTexturePointer)
	{
		glContext->GenRenderbuffers(1, &framebufferImpl.renderbufferID);
		GLcheck(*this, "glGenRenderbuffers");
	}
	glContext->GenFramebuffers(1, &framebufferImpl.id);
	GLcheck(*this, "glGenFramebuffers");
	glContext->BindFramebuffer(GL_FRAMEBUFFER, framebufferImpl.id);
	GLcheck(*this, "glBindFramebuffer");
	GLenum frameBufferTarget;
	switch (framebuffer.texture.format)
	{
	case textures::Texture::Depth:
		frameBufferTarget = GL_DEPTH_ATTACHMENT;
		glContext->DrawBuffer(GL_NONE);
		glContext->ReadBuffer(GL_NONE);
		break;
	default:
		frameBufferTarget = GL_COLOR_ATTACHMENT0;
		break;
	}
	if (framebuffer.depthTexturePointer)
	{
		glContext->BindRenderbuffer(GL_RENDERBUFFER, framebufferImpl.renderbufferID);
		GLcheck(*this, "glBindRenderbuffer");
		glContext->RenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT32F, framebuffer.depthTexturePointer->size.x,
																	 framebuffer.depthTexturePointer->size.y);
		GLcheck(*this, "glRenderbufferStorage");
		glContext->FramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
																			 framebufferImpl.renderbufferID);
		GLcheck(*this, "glFramebufferRenderbuffer");
	}
	if (textureImpl.target == GL_TEXTURE_CUBE_MAP)
	{
		glContext->FramebufferTexture(GL_FRAMEBUFFER, frameBufferTarget, textureImpl.id, 0);
		GLcheck(*this, "glFramebufferTexture");
	}
	else
	{
		glContext->FramebufferTexture2D(GL_FRAMEBUFFER, frameBufferTarget, GL_TEXTURE_2D, textureImpl.id, 0);
		GLcheck(*this, "glFramebufferTexture2D");
	}
	glContext->BindFramebuffer(GL_FRAMEBUFFER, 0);
	GLcheck(*this, "glBindFramebuffer");
}
void GLRenderer::destroyFramebuffer(textures::Framebuffer& framebuffer)
{
	auto& framebufferImpl = *(textures::GLFramebufferImpl*)framebuffer.rendererData;
	glContext->DeleteFramebuffers(1, &framebufferImpl.id);
	GLcheck(*this, "glDeleteFramebuffers");
	delete &framebufferImpl;
}
void GLRenderer::bindTexture(const textures::Texture& texture)
{
	auto& textureImpl = *(textures::GLTextureImpl*)texture.rendererData;
	glContext->BindTexture(textureImpl.target, textureImpl.id);
	GLcheck(*this, "glBindTexture");
}
void GLRenderer::unbindTexture(const textures::Texture& texture)
{
	auto& textureImpl = *(textures::GLTextureImpl*)texture.rendererData;
	glContext->BindTexture(textureImpl.target, 0);
	GLcheck(*this, "glBindTexture");
}
void GLRenderer::preInitTexture(textures::Texture& texture)
{
	texture.rendererData = new textures::GLTextureImpl();
	auto& textureImpl = *(textures::GLTextureImpl*)texture.rendererData;
	glContext->GenTextures(1, &textureImpl.id);
	GLcheck(*this, "glGenTextures");
	if (texture.size.w > 0)
		textureImpl.target = GL_TEXTURE_CUBE_MAP;
	else if (texture.size.y == 0)
		textureImpl.target = GL_TEXTURE_1D;
	else if (texture.size.z <= 1)
		textureImpl.target = GL_TEXTURE_2D;
	else
		textureImpl.target = GL_TEXTURE_3D;
	texture.bind();
	glContext->PixelStorei(GL_PACK_ALIGNMENT, 1);
	GLcheck(*this, "glPixelStorei");
}
void GLRenderer::midInitTexture(const textures::Texture& texture,
																const std::vector<images::ImageLoader::ImagePair>& images)
{
	auto& textureImpl = *(textures::GLTextureImpl*)texture.rendererData;
	if (textureImpl.target == GL_TEXTURE_1D)
	{
		void* data = images.size() ? std::get<1>(images[0]).get() : 0;
		glContext->TexImage1D(textureImpl.target, 0, textures::TextureFactory::internalFormats[texture.format],
													texture.size.x, 0, textures::TextureFactory::formats[texture.format],
													textures::TextureFactory::types[{texture.format, texture.type}], data);
		GLcheck(*this, "glTexImage1D");
	}
	else if (textureImpl.target == GL_TEXTURE_2D)
	{
		void* data = images.size() ? std::get<1>(images[0]).get() : 0;
		glContext->TexImage2D(textureImpl.target, 0, textures::TextureFactory::internalFormats[texture.format],
													texture.size.x, texture.size.y, 0, textures::TextureFactory::formats[texture.format],
													textures::TextureFactory::types[{texture.format, texture.type}], data);
		GLcheck(*this, "glTexImage2D");
	}
	else if (textureImpl.target == GL_TEXTURE_3D)
	{
		void* data = images.size() ? std::get<1>(images[0]).get() : 0;
		glContext->TexImage3D(textureImpl.target, 0, textures::TextureFactory::internalFormats[texture.format],
													texture.size.x, texture.size.y, texture.size.z, 0,
													textures::TextureFactory::formats[texture.format],
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
			glContext->TexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0,
														textures::TextureFactory::internalFormats[texture.format], imageSize.x, imageSize.y, 0,
														textures::TextureFactory::formats[texture.format],
														textures::TextureFactory::types[{texture.format, texture.type}], data);
			GLcheck(*this, "glTexImage2D");
		}
	}
}
void GLRenderer::postInitTexture(const textures::Texture& texture)
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
	glContext->TexParameteri(textureImpl.target, GL_TEXTURE_MIN_FILTER, filterType);
	GLcheck(*this, "glTexParameteri");
	glContext->TexParameteri(textureImpl.target, GL_TEXTURE_MAG_FILTER, filterType);
	GLcheck(*this, "glTexParameteri");
	glContext->TexParameteri(textureImpl.target, GL_TEXTURE_WRAP_S, GL_REPEAT);
	GLcheck(*this, "glTexParameteri");
	glContext->TexParameteri(textureImpl.target, GL_TEXTURE_WRAP_T, GL_REPEAT);
	GLcheck(*this, "glTexParameteri");
	glContext->TexParameteri(textureImpl.target, GL_TEXTURE_WRAP_R, GL_REPEAT);
	GLcheck(*this, "glTexParameteri");
	GLfloat maxAniso = 1.0f;
	glContext->TexParameterf(textureImpl.target, GL_TEXTURE_MAX_ANISOTROPY, maxAniso);
	GLcheck(*this, "glTexParameterf");
	texture.unbind();
}
void GLRenderer::destroyTexture(textures::Texture& texture)
{
	auto& textureImpl = *(textures::GLTextureImpl*)texture.rendererData;
	glContext->DeleteTextures(1, &textureImpl.id);
	GLcheck(*this, "glDeleteTextures");
	delete &textureImpl;
}
void GLRenderer::updateIndicesVAO(const vaos::VAO& vao, const std::vector<uint32_t>& indices)
{
	glContext->BindVertexArray(vao.vao);
	GLcheck(*this, "glBindVertexArray");
	glContext->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ebo);
	GLcheck(*this, "glBindBuffer");
	auto& constantSize = vaos::VAOFactory::constantSizes["Indice"];
	glContext->BufferData(GL_ELEMENT_ARRAY_BUFFER, vao.indiceCount * std::get<1>(constantSize), indices.data(),
												GL_STATIC_DRAW);
	GLcheck(*this, "glBufferData");
	glContext->BindVertexArray(0);
	GLcheck(*this, "glBindVertexArray");
}
void GLRenderer::updateElementsVAO(const vaos::VAO& vao, const std::string_view constant, uint8_t* elementsAsChar)
{
	glContext->BindVertexArray(vao.vao);
	GLcheck(*this, "glBindVertexArray");
	glContext->BindBuffer(GL_ARRAY_BUFFER, vao.vbo);
	GLcheck(*this, "glBindBuffer");
	auto& constantSize = vaos::VAOFactory::constantSizes[constant];
	auto offset = vaos::VAOFactory::getOffset(vao.constants, constant);
	auto elementStride = std::get<0>(constantSize) * std::get<1>(constantSize);
	for (size_t index = offset, c = 1, elementIndex = 0; c <= vao.elementCount;
			 index += vao.stride, c++, elementIndex += elementStride)
	{
		glContext->BufferSubData(GL_ARRAY_BUFFER, index, elementStride, &elementsAsChar[elementIndex]);
		GLcheck(*this, "glBufferSubData");
	}
	glContext->BindVertexArray(0);
	GLcheck(*this, "glBindVertexArray");
}
void GLRenderer::drawVAO(const vaos::VAO& vao)
{
	glContext->BindVertexArray(vao.vao);
	GLcheck(*this, "glBindVertexArray");
	GLenum drawMode = GL_TRIANGLES;
	GLenum polygonMode = GL_FILL;
	glContext->PolygonMode(GL_FRONT_AND_BACK, polygonMode);
	GLcheck(*this, "glPolygonMode");
	glContext->DrawElements(drawMode, vao.indiceCount, ZG_UNSIGNED_INT, 0);
	GLcheck(*this, "glDrawElements");
}
void GLRenderer::generateVAO(vaos::VAO& vao)
{
	glContext->GenVertexArrays(1, &vao.vao);
	GLcheck(*this, "glGenVertexArrays");
	glContext->GenBuffers(1, &vao.ebo);
	GLcheck(*this, "glGenBuffers");
	glContext->GenBuffers(1, &vao.vbo);
	GLcheck(*this, "glGenBuffers");
	glContext->BindVertexArray(vao.vao);
	GLcheck(*this, "glBindVertexArray");
	glContext->BindBuffer(GL_ELEMENT_ARRAY_BUFFER, vao.ebo);
	GLcheck(*this, "glBindBuffer");
	glContext->BindBuffer(GL_ARRAY_BUFFER, vao.vbo);
	GLcheck(*this, "glBindBuffer");
	auto stride = vaos::VAOFactory::getStride(vao.constants);
	glContext->BufferData(GL_ARRAY_BUFFER, stride * vao.elementCount, 0, GL_STATIC_DRAW);
	GLcheck(*this, "glBufferData");
	size_t attribIndex = 0;
	size_t offset = 0;
	for (auto& constant : vao.constants)
	{
		if (!vaos::VAOFactory::isVAOConstant(constant))
			continue;
		glContext->EnableVertexAttribArray(attribIndex);
		GLcheck(*this, "glEnableVertexAttribArray");
		auto& constantSize = vaos::VAOFactory::constantSizes[constant];
		if (std::get<2>(constantSize) == ZG_FLOAT)
		{
			glContext->VertexAttribPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize), GL_FALSE,
																		 stride, (const void*)offset);
			GLcheck(*this, "glVertexAttribPointer");
		}
		else
		{
			glContext->VertexAttribIPointer(attribIndex, std::get<0>(constantSize), std::get<2>(constantSize), stride,
																			(const void*)offset);
			GLcheck(*this, "glVertexAttribIPointer");
		}
		offset += std::get<0>(constantSize) * std::get<1>(constantSize);
		attribIndex++;
	}
}
void GLRenderer::destroyVAO(vaos::VAO& vao)
{
	glContext->DeleteVertexArrays(1, &vao.vao);
	GLcheck(*this, "glDeleteVertexArrays");
	vao.vao = 0;
	glContext->DeleteBuffers(1, &vao.vbo);
	GLcheck(*this, "glDeleteBuffers");
	vao.vbo = 0;
	glContext->DeleteBuffers(1, &vao.ebo);
	GLcheck(*this, "glDeleteBuffers");
	vao.ebo = 0;
}
const bool zg::GLcheck(const GLRenderer& renderer, const char* fn, const bool egl)
{
#ifndef NDEBUG
	while (true)
	{
		uint32_t err = 0;
		if (!egl)
			err = renderer.glContext->GetError();
#if defined(_Android)
		else if (egl)
			err = eglGetError();
#endif
		if (err == GL_NO_ERROR
#if defined(_Android)
				|| err == EGL_SUCCESS
#endif
		)
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
#if defined(_Android)
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
#endif
		}
	}
#endif
	return true;
}
#endif
