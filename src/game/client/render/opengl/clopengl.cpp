//OpenGL setup

// Include Windows and OpenGL headers first for proper WGL function declarations
#ifdef _WIN32
#include <windows.h>
#include <GL/gl.h>
#endif

#include "inc_weapondefs.h"
#include "../hud.h"
#include "../cl_util.h"
#include "com_model.h"
#include "studio.h"
#include "entity_state.h"
#include "cl_entity.h"
#include "triangleapi.h"
#include "../studio_util.h"
#include "../r_studioint.h"
#include "clopengl.h" // OpenGL stuff
#include "mslogger.h"

#include "clglobal.h"
#include "../clrender.h"

//Log GL info and load the extension functions used by PushHLStates/PopHLStates
bool CRender::InitGL()
{
	if (!IEngineStudio.IsHardware()) //Not using openGL
		return false;

	const char *VendorString = (const char *)glGetString(GL_VENDOR);
	const char *CardString = (const char *)glGetString(GL_RENDERER);
	const char *VersionString = (const char *)glGetString(GL_VERSION);
	const char *ExtensionsString = (const char *)glGetString(GL_EXTENSIONS);
	MS_RENDER_INFO("Video Card Vendor: %s", VendorString);
	MS_RENDER_INFO("Video Card: %s", CardString);
	MS_RENDER_INFO("OpenGL Version: %s", VersionString);
	MS_RENDER_INFO("OpenGL Extensions: %s", ExtensionsString);

	if (atof(VersionString) < 1.1) //Not high enough OpenGL version
	{
		MS_RENDER_WARN("OpenGL Version Not High Enough (Needs 1.1)");
		return false;
	}

#ifdef _WIN32
	glMultiTexCoord2fARB = (PFNGLMULTITEXCOORD2FARBPROC)wglGetProcAddress("glMultiTexCoord2fARB");
	glActiveTextureARB = (PFNGLACTIVETEXTUREARBPROC)wglGetProcAddress("glActiveTextureARB");
#endif

	MS_RENDER_INFO("OpenGL ActiveTexture Extention: %s", (glActiveTextureARB ? "FOUND" : "NOT FOUND"));

	return glActiveTextureARB ? true : false;
}

bool CRender::CheckOpenGL()
{
	//Thothie attempting to enable D3D (failed)
	if (!IEngineStudio.IsHardware())
	{
		//MessageBox(NULL, "Master Sword uses features only available in OpenGL mode! Attempting other modes may result in instability.", "Invalid Video Mode", MB_OK);
		//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Invalid Video Mode", "We make use of features only in OpenGL!", NULL);
		MS_RENDER_ERROR("CRender::CheckOpenGL - User chose non-opengl video mode (semi-fatal)");
		//exit( 0 );
		return false;
	}

	return true;
}
