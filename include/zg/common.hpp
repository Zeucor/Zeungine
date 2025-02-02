#pragma once
#ifdef _WIN32
#include <windows.h>
#endif
#if defined(USE_GL) || defined(USE_EGL)
#ifdef USE_GL
#include <glad/gl.h>
#endif
#ifdef USE_EGL
#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>
#endif
#ifdef _WIN32
#include <GL/wglext.h>
#endif
#define ZG_FLOAT GL_FLOAT
#define ZG_UNSIGNED_INT GL_UNSIGNED_INT
#define ZG_UNSIGNED_INT_24_8 GL_UNSIGNED_INT_24_8
#define ZG_FLOAT_32_UNSIGNED_INT_24_8_REV GL_FLOAT_32_UNSIGNED_INT_24_8_REV
#elif defined(USE_VULKAN)
#define ZG_FLOAT 0
#define ZG_UNSIGNED_INT 0
#endif