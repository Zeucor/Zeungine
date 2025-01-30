#pragma once
#ifdef _WIN32
#include <windows.h>
#endif
#ifdef USE_GL
#include <glad/gl.h>
#endif
#ifdef USE_EGL
#define GL_GLEXT_PROTOTYPES
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#endif
#ifdef _WIN32
#include <GL/wglext.h>
#endif