#pragma once

#include <glad/glad.h>
#include "Log.hpp"
#include <string>

// ============================================================================
// OpenGL Debug Infrastructure - Professional Game Engine Standard
// ============================================================================

namespace GLDebug
{
    inline const char* GLErrorToString(GLenum error)
    {
        switch (error)
        {
            case GL_NO_ERROR:          return "GL_NO_ERROR";
            case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
            case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
            case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
            case GL_STACK_OVERFLOW:    return "GL_STACK_OVERFLOW";
            case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
            case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
            case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
            default:                   return "UNKNOWN_GL_ERROR";
        }
    }

    inline void CheckGLError(const char* function, const char* file, int line)
    {
        GLenum error;
        while ((error = glGetError()) != GL_NO_ERROR)
        {
            CORE_ERROR("[OpenGL Error] {0} in {1} at {2}:{3}", 
                       GLErrorToString(error), function, file, line);
        }
    }
}

// Macro for debug builds - checks GL errors after every call
#ifdef _DEBUG
    #define GL_CALL(x) do { x; GLDebug::CheckGLError(#x, __FILE__, __LINE__); } while(0)
#else
    #define GL_CALL(x) x
#endif

// Always check critical operations even in release
#define GL_CALL_CRITICAL(x) do { x; GLDebug::CheckGLError(#x, __FILE__, __LINE__); } while(0)
