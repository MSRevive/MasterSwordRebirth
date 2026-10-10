#pragma once

struct loadtex_t;

class CEnvMgr
{
public:
	static void Init();
	static void InitNewLevel();
	static void Think_DrawTransparentTriangles();
	static void RenderSky();
	static void RenderFog(bool bRender);
	static void Cleanup();

	static void ChangeSkyTexture(const char* NewTexture);
	static void ChangeTint(const Color4F &Color);
	static void SetLightGamma(float Value);

	static float m_LightGamma;
	static float m_MaxViewDistance;

	//Fog
	struct fog_t
	{
		bool Enabled;
		Vector Color;
		float Density, Start, End;
		int Type;
	};
	static fog_t m_Fog;

	//OpenGL
	static void PushHLStates();
	static void PopHLStates();
	static bool LoadGLTexture(const char *FileName, unsigned int &TextureID);
	static bool LoadGLTexture(const char *FileName, loadtex_t &LoadTex);
	static void DeleteGLTextures();

private:
	static void InitGL();

	static int m_OldHLTexture[10];
	static bool m_OldMultiTextureEnabled;
};
