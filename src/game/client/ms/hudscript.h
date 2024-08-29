#ifndef HUDSCRIPT_H
#define HUDSCRIPT_H
#include "hudbase.h"
#include "screenfade.h"

enum hae_e
{
	HAE_EITHER,
	HAE_NEW,
	HAE_ATTACH,
};

//Holds the Client-side scripts that are currently causing an effect (drunk, stun, etc.)  Usually temporary.
class CHudScript : public CHudBase, public IScripted
{
public:
	int Init(void);
	int VidInit(void);
	int Draw(float flTime);
	void Think(void);
	void Reset(void);
	int MsgFunc_ClientScript(const char *pszName, int iSize, void *pbuf);
	void InitHUDData(void);

	CScript *CreateScript(msstring_ref ScriptName, msstringlist &Parameters, bool AllowDupe = true, int UniqueID = DEFAULT_SCRIPT_ID);
	void HandleAnimEvent(msstring_ref Options, const struct cl_entity_s *clentity, hae_e Type);

	Vector Effects_GetMoveScale();
	Vector Effects_GetMove(Vector &OriginalMove);
	void Effects_GetView(struct ref_params_s *pparams, struct cl_entity_s *ViewModel);

	void Effects_GetFade(screenfade_t &ScreenFade);
	void Effects_UpdateTempEnt(msstring_ref EventName, msstringlist *Parameters = NULL);
	void Effects_PreRender();
	void Effects_Render(cl_entity_t &Ent, bool InMirror);
	void Effects_DrawTransPararentTriangles();
};
#define HUDScript gHUD.m_HUDScript

#endif //HUDSCRIPT_H
