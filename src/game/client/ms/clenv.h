//The client-side enviroment
//

//Used by various source files

class CEnvMgr
{
public:
	static void Init();
	static void InitNewLevel();
	static void Think_DrawTransparentTriangles();
	static void RenderSky();
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
};

//Mastersword Special rendering
class CRender
{
public:
	static void PushHLStates();
	static void PopHLStates();

	static void Cleanup();

	static bool InitGL();	   //Load GL extension functions.  Called each level load
	static bool CheckOpenGL(); //MS only works in openGL

	static int m_OldHLTexture[10];
	static bool m_OldMultiTextureEnabled;
};
