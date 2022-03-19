#ifndef __TEXTURE_H__
#define __TEXTURE_H__

#ifdef _WIN32
#include <windows.h> // Standard windows header
#endif

#include <stdio.h> // Standard I/O header
#include <GL/gl.h> // Header for OpenGL32 library

namespace Tartan
{
	typedef struct
	{
		GLubyte *imageData; //!< Image Data (Up To 32 Bits)
		GLuint bpp;			//!< Image Color Depth In Bits Per Pixel
		GLuint width;		//!< Image Width
		GLuint height;		//!< Image Height
		GLuint texID;		//!< Texture ID Used To Select A Texture
		GLuint type;		//!< Image Type (GL_RGB, GL_RGBA)
	} Texture;
}

#endif // __TEXTURE_H__
