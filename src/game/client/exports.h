// CL_DLLEXPORT is the client version of dllexport.  It's turned off for secure clients.
#ifdef _WIN32
#define CL_DLLEXPORT __declspec(dllexport)
#else
#define CL_DLLEXPORT __attribute__ ((visibility("default")))
#endif

extern "C"
{
	// From hl_weapons
	void CL_DLLEXPORT HUD_PostRunCmd(struct local_state_s *from, struct local_state_s *to, struct usercmd_s *cmd, int runfuncs, double time, unsigned int random_seed);

	// From cdll_int
	int CL_DLLEXPORT Initialize(cl_enginefunc_t *pEnginefuncs, int iVersion);
	int CL_DLLEXPORT HUD_VidInit(void);
	void CL_DLLEXPORT HUD_Init(void);
	int CL_DLLEXPORT HUD_Redraw(float flTime, int intermission);
	int CL_DLLEXPORT HUD_UpdateClientData(client_data_t *cdata, float flTime);
	void CL_DLLEXPORT HUD_Reset(void);
	void CL_DLLEXPORT HUD_PlayerMove(struct playermove_s *ppmove, int server);
	void CL_DLLEXPORT HUD_PlayerMoveInit(struct playermove_s *ppmove);
	char CL_DLLEXPORT HUD_PlayerMoveTexture(char *name);
	int CL_DLLEXPORT HUD_ConnectionlessPacket(const struct netadr_s *net_from, const char *args, char *response_buffer, int *response_buffer_size);
	int CL_DLLEXPORT HUD_GetHullBounds(int hullnumber, float *mins, float *maxs);
	void CL_DLLEXPORT HUD_Frame(double time);
	void CL_DLLEXPORT HUD_VoiceStatus(int entindex, qboolean bTalking);
	void CL_DLLEXPORT HUD_DirectorMessage(int iSize, void *pbuf);
	void CL_DLLEXPORT HUD_ChatInputPosition(int *x, int *y);

	// From demo
	void CL_DLLEXPORT Demo_ReadBuffer(int size, unsigned char *buffer);

	// From entity
	int CL_DLLEXPORT HUD_AddEntity(int type, struct cl_entity_s *ent, const char *modelname);
	void CL_DLLEXPORT HUD_CreateEntities(void);
	void CL_DLLEXPORT HUD_StudioEvent(const struct mstudioevent_s *event, const struct cl_entity_s *entity);
	void CL_DLLEXPORT HUD_TxferLocalOverrides(struct entity_state_s *state, const struct clientdata_s *client);
	void CL_DLLEXPORT HUD_ProcessPlayerState(struct entity_state_s *dst, const struct entity_state_s *src);
	void CL_DLLEXPORT HUD_TxferPredictionData(struct entity_state_s *ps, const struct entity_state_s *pps, struct clientdata_s *pcd, const struct clientdata_s *ppcd, struct weapon_data_s *wd, const struct weapon_data_s *pwd);
	void CL_DLLEXPORT HUD_TempEntUpdate(double frametime, double client_time, double cl_gravity, struct tempent_s **ppTempEntFree, struct tempent_s **ppTempEntActive, int (*Callback_AddVisibleEntity)(struct cl_entity_s *pEntity), void (*Callback_TempEntPlaySound)(struct tempent_s *pTemp, float damp));
	struct cl_entity_s CL_DLLEXPORT *HUD_GetUserEntity(int index);

	// From in_camera
	void CL_DLLEXPORT CAM_Think(void);
	int CL_DLLEXPORT CL_IsThirdPerson(void);
	void CL_DLLEXPORT CL_CameraOffset(float *ofs);

	// From input
	struct kbutton_s CL_DLLEXPORT *KB_Find(const char *name);
	void CL_DLLEXPORT CL_CreateMove(float frametime, struct usercmd_s *cmd, int active);
	void CL_DLLEXPORT HUD_Shutdown(void);
	int CL_DLLEXPORT HUD_Key_Event(int eventcode, int keynum, const char *pszCurrentBinding);

	// From inputw32
	void CL_DLLEXPORT IN_ActivateMouse(void);
	void CL_DLLEXPORT IN_DeactivateMouse(void);
	void CL_DLLEXPORT IN_MouseEvent(int mstate);
	void CL_DLLEXPORT IN_Accumulate(void);
	void CL_DLLEXPORT IN_ClearStates(void);

	// From tri
	void CL_DLLEXPORT HUD_DrawNormalTriangles(void);
	void CL_DLLEXPORT HUD_DrawTransparentTriangles(void);

	// From view
	void CL_DLLEXPORT V_CalcRefdef(struct ref_params_s *pparams);

	// From GameStudioModelRenderer
	int CL_DLLEXPORT HUD_GetStudioModelInterface(int version, struct r_studio_interface_s **ppinterface, struct engine_studio_api_s *pstudio);
}

/*
extern cldll_func_dst_t *g_pcldstAddrs;

// Macros for the client receiving calls from the engine
#define RecClInitialize(a, b)			(g_pcldstAddrs->pInitFunc(&a, &b))
#define RecClHudInit()					(g_pcldstAddrs->pHudInitFunc())
#define RecClHudVidInit()				(g_pcldstAddrs->pHudVidInitFunc())
#define RecClHudRedraw(a, b)			(g_pcldstAddrs->pHudRedrawFunc(&a, &b))
#define RecClHudUpdateClientData(a, b)	(g_pcldstAddrs->pHudUpdateClientDataFunc(&a, &b))
#define RecClHudReset()					(g_pcldstAddrs->pHudResetFunc())
#define RecClClientMove(a, b)			(g_pcldstAddrs->pClientMove(&a, &b))
#define RecClClientMoveInit(a)			(g_pcldstAddrs->pClientMoveInit(&a))
#define RecClClientTextureType(a)		(g_pcldstAddrs->pClientTextureType(&a))
#define RecClIN_ActivateMouse()			(g_pcldstAddrs->pIN_ActivateMouse())
#define RecClIN_DeactivateMouse()		(g_pcldstAddrs->pIN_DeactivateMouse())
#define RecClIN_MouseEvent(a)			(g_pcldstAddrs->pIN_MouseEvent(&a))
#define RecClIN_ClearStates()			(g_pcldstAddrs->pIN_ClearStates())
#define RecClIN_Accumulate()			(g_pcldstAddrs->pIN_Accumulate())
#define RecClCL_CreateMove(a, b, c)		(g_pcldstAddrs->pCL_CreateMove(&a, &b, &c))
#define RecClCL_IsThirdPerson()			(g_pcldstAddrs->pCL_IsThirdPerson())
#define RecClCL_GetCameraOffsets(a)		(g_pcldstAddrs->pCL_GetCameraOffsets(&a))
#define RecClFindKey(a)					(g_pcldstAddrs->pFindKey(&a))
#define RecClCamThink()					(g_pcldstAddrs->pCamThink())
#define RecClCalcRefdef(a)				(g_pcldstAddrs->pCalcRefdef(&a))
#define RecClAddEntity(a, b, c)			(g_pcldstAddrs->pAddEntity(&a, &b, &c))
#define RecClCreateEntities()			(g_pcldstAddrs->pCreateEntities())
#define RecClDrawNormalTriangles()		(g_pcldstAddrs->pDrawNormalTriangles())
#define RecClDrawTransparentTriangles()	(g_pcldstAddrs->pDrawTransparentTriangles())
#define RecClStudioEvent(a, b)			(g_pcldstAddrs->pStudioEvent(&a, &b))
#define RecClPostRunCmd(a, b, c, d, e, f)		(g_pcldstAddrs->pPostRunCmd(&a, &b, &c, &d, &e, &f))
#define RecClShutdown()					(g_pcldstAddrs->pShutdown())
#define RecClTxferLocalOverrides(a, b)	(g_pcldstAddrs->pTxferLocalOverrides(&a, &b))
#define RecClProcessPlayerState(a, b)	(g_pcldstAddrs->pProcessPlayerState(&a, &b))
#define RecClTxferPredictionData(a, b, c, d, e, f)		(g_pcldstAddrs->pTxferPredictionData(&a, &b, &c, &d, &e, &f))
#define RecClReadDemoBuffer(a, b)		(g_pcldstAddrs->pReadDemoBuffer(&a, &b))
#define RecClConnectionlessPacket(a, b, c, d)		(g_pcldstAddrs->pConnectionlessPacket(&a, &b, &c, &d))
#define RecClGetHullBounds(a, b, c)		(g_pcldstAddrs->pGetHullBounds(&a, &b, &c))
#define RecClHudFrame(a)				(g_pcldstAddrs->pHudFrame(&a))
#define RecClKeyEvent(a, b, c)			(g_pcldstAddrs->pKeyEvent(&a, &b, &c))
#define RecClTempEntUpdate(a, b, c, d, e, f, g)	(g_pcldstAddrs->pTempEntUpdate(&a, &b, &c, &d, &e, &f, &g))
#define RecClGetUserEntity(a)			(g_pcldstAddrs->pGetUserEntity(&a))
#define RecClVoiceStatus(a, b)			(g_pcldstAddrs->pVoiceStatus(&a, &b))
#define RecClDirectorMessage(a, b)		(g_pcldstAddrs->pDirectorMessage(&a, &b))
#define RecClStudioInterface(a, b, c)	(g_pcldstAddrs->pStudioInterface(&a, &b, &c))
#define RecClChatInputPosition(a, b)	(g_pcldstAddrs->pChatInputPosition(&a, &b))
*/

// Function type declarations for client destination functions
typedef void (*DST_INITIALIZE_FUNC)	(struct cl_enginefuncs_s**, int*);
typedef void (*DST_HUD_INIT_FUNC)		(void);
typedef void (*DST_HUD_VIDINIT_FUNC)	(void);
typedef void (*DST_HUD_REDRAW_FUNC)	(float*, int*);
typedef void (*DST_HUD_UPDATECLIENTDATA_FUNC) (struct client_data_s**, float*);
typedef void (*DST_HUD_RESET_FUNC)    (void);
typedef void (*DST_HUD_CLIENTMOVE_FUNC)(struct playermove_s**, qboolean*);
typedef void (*DST_HUD_CLIENTMOVEINIT_FUNC)(struct playermove_s**);
typedef void (*DST_HUD_TEXTURETYPE_FUNC)(char**);
typedef void (*DST_HUD_IN_ACTIVATEMOUSE_FUNC) (void);
typedef void (*DST_HUD_IN_DEACTIVATEMOUSE_FUNC)		(void);
typedef void (*DST_HUD_IN_MOUSEEVENT_FUNC)		(int*);
typedef void (*DST_HUD_IN_CLEARSTATES_FUNC)		(void);
typedef void (*DST_HUD_IN_ACCUMULATE_FUNC) (void);
typedef void (*DST_HUD_CL_CREATEMOVE_FUNC)		(float*, struct usercmd_s**, int*);
typedef void (*DST_HUD_CL_ISTHIRDPERSON_FUNC) (void);
typedef void (*DST_HUD_CL_GETCAMERAOFFSETS_FUNC)(float**);
typedef void (*DST_HUD_KB_FIND_FUNC) (const char**);
typedef void (*DST_HUD_CAMTHINK_FUNC)(void);
typedef void (*DST_HUD_CALCREF_FUNC) (struct ref_params_s**);
typedef void (*DST_HUD_ADDENTITY_FUNC) (int*, struct cl_entity_s**, const char**);
typedef void (*DST_HUD_CREATEENTITIES_FUNC) (void);
typedef void (*DST_HUD_DRAWNORMALTRIS_FUNC) (void);
typedef void (*DST_HUD_DRAWTRANSTRIS_FUNC) (void);
typedef void (*DST_HUD_STUDIOEVENT_FUNC) (const struct mstudioevent_s**, const struct cl_entity_s**);
typedef void (*DST_HUD_POSTRUNCMD_FUNC) (struct local_state_s**, struct local_state_s**, struct usercmd_s**, int*, double*, unsigned int*);
typedef void (*DST_HUD_SHUTDOWN_FUNC) (void);
typedef void (*DST_HUD_TXFERLOCALOVERRIDES_FUNC)(struct entity_state_s**, const struct clientdata_s**);
typedef void (*DST_HUD_PROCESSPLAYERSTATE_FUNC)(struct entity_state_s**, const struct entity_state_s**);
typedef void (*DST_HUD_TXFERPREDICTIONDATA_FUNC) (struct entity_state_s**, const struct entity_state_s**, struct clientdata_s**, const struct clientdata_s**, struct weapon_data_s**, const struct weapon_data_s**);
typedef void (*DST_HUD_DEMOREAD_FUNC) (int*, unsigned char**);
typedef void (*DST_HUD_CONNECTIONLESS_FUNC)(const struct netadr_s**, const char**, char**, int**);
typedef void (*DST_HUD_GETHULLBOUNDS_FUNC) (int*, float**, float**);
typedef void (*DST_HUD_FRAME_FUNC)		(double*);
typedef void (*DST_HUD_KEY_EVENT_FUNC) (int*, int*, const char**);
typedef void (*DST_HUD_TEMPENTUPDATE_FUNC) (double*, double*, double*, struct tempent_s***, struct tempent_s***, int (**Callback_AddVisibleEntity)(struct cl_entity_s* pEntity), void (**Callback_TempEntPlaySound)(struct tempent_s* pTemp, float damp));
typedef void (*DST_HUD_GETUSERENTITY_FUNC) (int*);
typedef void (*DST_HUD_VOICESTATUS_FUNC)(int*, qboolean*);
typedef void (*DST_HUD_DIRECTORMESSAGE_FUNC)(int*, void**);
typedef void (*DST_HUD_STUDIO_INTERFACE_FUNC) (int*, struct r_studio_interface_s***, struct engine_studio_api_s**);
typedef void (*DST_HUD_CHATINPUTPOSITION_FUNC)(int**, int**);
typedef void (*DST_HUD_GETPLAYERTEAM)(int);

// Pointers to the client destination functions
typedef struct
{
	DST_INITIALIZE_FUNC						pInitFunc;
	DST_HUD_INIT_FUNC						pHudInitFunc;
	DST_HUD_VIDINIT_FUNC					pHudVidInitFunc;
	DST_HUD_REDRAW_FUNC						pHudRedrawFunc;
	DST_HUD_UPDATECLIENTDATA_FUNC			pHudUpdateClientDataFunc;
	DST_HUD_RESET_FUNC						pHudResetFunc;
	DST_HUD_CLIENTMOVE_FUNC					pClientMove;
	DST_HUD_CLIENTMOVEINIT_FUNC				pClientMoveInit;
	DST_HUD_TEXTURETYPE_FUNC				pClientTextureType;
	DST_HUD_IN_ACTIVATEMOUSE_FUNC			pIN_ActivateMouse;
	DST_HUD_IN_DEACTIVATEMOUSE_FUNC			pIN_DeactivateMouse;
	DST_HUD_IN_MOUSEEVENT_FUNC				pIN_MouseEvent;
	DST_HUD_IN_CLEARSTATES_FUNC				pIN_ClearStates;
	DST_HUD_IN_ACCUMULATE_FUNC				pIN_Accumulate;
	DST_HUD_CL_CREATEMOVE_FUNC				pCL_CreateMove;
	DST_HUD_CL_ISTHIRDPERSON_FUNC			pCL_IsThirdPerson;
	DST_HUD_CL_GETCAMERAOFFSETS_FUNC		pCL_GetCameraOffsets;
	DST_HUD_KB_FIND_FUNC					pFindKey;
	DST_HUD_CAMTHINK_FUNC					pCamThink;
	DST_HUD_CALCREF_FUNC					pCalcRefdef;
	DST_HUD_ADDENTITY_FUNC					pAddEntity;
	DST_HUD_CREATEENTITIES_FUNC				pCreateEntities;
	DST_HUD_DRAWNORMALTRIS_FUNC				pDrawNormalTriangles;
	DST_HUD_DRAWTRANSTRIS_FUNC				pDrawTransparentTriangles;
	DST_HUD_STUDIOEVENT_FUNC				pStudioEvent;
	DST_HUD_POSTRUNCMD_FUNC					pPostRunCmd;
	DST_HUD_SHUTDOWN_FUNC					pShutdown;
	DST_HUD_TXFERLOCALOVERRIDES_FUNC		pTxferLocalOverrides;
	DST_HUD_PROCESSPLAYERSTATE_FUNC			pProcessPlayerState;
	DST_HUD_TXFERPREDICTIONDATA_FUNC		pTxferPredictionData;
	DST_HUD_DEMOREAD_FUNC					pReadDemoBuffer;
	DST_HUD_CONNECTIONLESS_FUNC				pConnectionlessPacket;
	DST_HUD_GETHULLBOUNDS_FUNC				pGetHullBounds;
	DST_HUD_FRAME_FUNC						pHudFrame;
	DST_HUD_KEY_EVENT_FUNC					pKeyEvent;
	DST_HUD_TEMPENTUPDATE_FUNC				pTempEntUpdate;
	DST_HUD_GETUSERENTITY_FUNC				pGetUserEntity;
	DST_HUD_VOICESTATUS_FUNC				pVoiceStatus;	// Possibly null on old client dlls.
	DST_HUD_DIRECTORMESSAGE_FUNC			pDirectorMessage;	// Possibly null on old client dlls.
	DST_HUD_STUDIO_INTERFACE_FUNC			pStudioInterface;  // Not used by all clients
	DST_HUD_CHATINPUTPOSITION_FUNC			pChatInputPosition;  // Not used by all clients
	DST_HUD_GETPLAYERTEAM					pGetPlayerTeam; // Not used by all clients
} cldll_func_dst_t;

// ********************************************************
// Functions exported by the client .dll
// ********************************************************

// Function type declarations for client exports
typedef int (*INITIALIZE_FUNC)	(struct cl_enginefuncs_s*, int);
typedef void (*HUD_INIT_FUNC)		(void);
typedef int (*HUD_VIDINIT_FUNC)	(void);
typedef int (*HUD_REDRAW_FUNC)	(float, int);
typedef int (*HUD_UPDATECLIENTDATA_FUNC) (struct client_data_s*, float);
typedef void (*HUD_RESET_FUNC)    (void);
typedef void (*HUD_CLIENTMOVE_FUNC)(struct playermove_s* ppmove, qboolean server);
typedef void (*HUD_CLIENTMOVEINIT_FUNC)(struct playermove_s* ppmove);
typedef char (*HUD_TEXTURETYPE_FUNC)(char* name);
typedef void (*HUD_IN_ACTIVATEMOUSE_FUNC) (void);
typedef void (*HUD_IN_DEACTIVATEMOUSE_FUNC)		(void);
typedef void (*HUD_IN_MOUSEEVENT_FUNC)		(int mstate);
typedef void (*HUD_IN_CLEARSTATES_FUNC)		(void);
typedef void (*HUD_IN_ACCUMULATE_FUNC) (void);
typedef void (*HUD_CL_CREATEMOVE_FUNC)		(float frametime, struct usercmd_s* cmd, int active);
typedef int (*HUD_CL_ISTHIRDPERSON_FUNC) (void);
typedef void (*HUD_CL_GETCAMERAOFFSETS_FUNC)(float* ofs);
typedef struct kbutton_s* (*HUD_KB_FIND_FUNC) (const char* name);
typedef void (*HUD_CAMTHINK_FUNC)(void);
typedef void (*HUD_CALCREF_FUNC) (struct ref_params_s* pparams);
typedef int	 (*HUD_ADDENTITY_FUNC) (int type, struct cl_entity_s* ent, const char* modelname);
typedef void (*HUD_CREATEENTITIES_FUNC) (void);
typedef void (*HUD_DRAWNORMALTRIS_FUNC) (void);
typedef void (*HUD_DRAWTRANSTRIS_FUNC) (void);
typedef void (*HUD_STUDIOEVENT_FUNC) (const struct mstudioevent_s* event, const struct cl_entity_s* entity);
typedef void (*HUD_POSTRUNCMD_FUNC) (struct local_state_s* from, struct local_state_s* to, struct usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed);
typedef void (*HUD_SHUTDOWN_FUNC) (void);
typedef void (*HUD_TXFERLOCALOVERRIDES_FUNC)(struct entity_state_s* state, const struct clientdata_s* client);
typedef void (*HUD_PROCESSPLAYERSTATE_FUNC)(struct entity_state_s* dst, const struct entity_state_s* src);
typedef void (*HUD_TXFERPREDICTIONDATA_FUNC) (struct entity_state_s* ps, const struct entity_state_s* pps, struct clientdata_s* pcd, const struct clientdata_s* ppcd, struct weapon_data_s* wd, const struct weapon_data_s* pwd);
typedef void (*HUD_DEMOREAD_FUNC) (int size, unsigned char* buffer);
typedef int (*HUD_CONNECTIONLESS_FUNC)(const struct netadr_s* net_from, const char* args, char* response_buffer, int* response_buffer_size);
typedef	int	(*HUD_GETHULLBOUNDS_FUNC) (int hullnumber, float* mins, float* maxs);
typedef void (*HUD_FRAME_FUNC)		(double);
typedef int (*HUD_KEY_EVENT_FUNC) (int eventcode, int keynum, const char* pszCurrentBinding);
typedef void (*HUD_TEMPENTUPDATE_FUNC) (double frametime, double client_time, double cl_gravity, struct tempent_s** ppTempEntFree, struct tempent_s** ppTempEntActive, int (*Callback_AddVisibleEntity)(struct cl_entity_s* pEntity), void (*Callback_TempEntPlaySound)(struct tempent_s* pTemp, float damp));
typedef struct cl_entity_s* (*HUD_GETUSERENTITY_FUNC) (int index);
typedef void (*HUD_VOICESTATUS_FUNC)(int entindex, qboolean bTalking);
typedef void (*HUD_DIRECTORMESSAGE_FUNC)(int iSize, void* pbuf);
typedef int (*HUD_STUDIO_INTERFACE_FUNC)(int version, struct r_studio_interface_s** ppinterface, struct engine_studio_api_s* pstudio);
typedef void (*HUD_CHATINPUTPOSITION_FUNC)(int* x, int* y);
typedef int (*HUD_GETPLAYERTEAM)(int iplayer);
typedef void* (*CLIENTFACTORY)(); // this should be CreateInterfaceFn but that means including interface.h
// which is a C++ file and some of the client files a C only... 
// so we return a void * which we then do a typecast on later.


// Pointers to the exported client functions themselves
typedef struct
{
	INITIALIZE_FUNC						pInitFunc;
	HUD_INIT_FUNC						pHudInitFunc;
	HUD_VIDINIT_FUNC					pHudVidInitFunc;
	HUD_REDRAW_FUNC						pHudRedrawFunc;
	HUD_UPDATECLIENTDATA_FUNC			pHudUpdateClientDataFunc;
	HUD_RESET_FUNC						pHudResetFunc;
	HUD_CLIENTMOVE_FUNC					pClientMove;
	HUD_CLIENTMOVEINIT_FUNC				pClientMoveInit;
	HUD_TEXTURETYPE_FUNC				pClientTextureType;
	HUD_IN_ACTIVATEMOUSE_FUNC			pIN_ActivateMouse;
	HUD_IN_DEACTIVATEMOUSE_FUNC			pIN_DeactivateMouse;
	HUD_IN_MOUSEEVENT_FUNC				pIN_MouseEvent;
	HUD_IN_CLEARSTATES_FUNC				pIN_ClearStates;
	HUD_IN_ACCUMULATE_FUNC				pIN_Accumulate;
	HUD_CL_CREATEMOVE_FUNC				pCL_CreateMove;
	HUD_CL_ISTHIRDPERSON_FUNC			pCL_IsThirdPerson;
	HUD_CL_GETCAMERAOFFSETS_FUNC		pCL_GetCameraOffsets;
	HUD_KB_FIND_FUNC					pFindKey;
	HUD_CAMTHINK_FUNC					pCamThink;
	HUD_CALCREF_FUNC					pCalcRefdef;
	HUD_ADDENTITY_FUNC					pAddEntity;
	HUD_CREATEENTITIES_FUNC				pCreateEntities;
	HUD_DRAWNORMALTRIS_FUNC				pDrawNormalTriangles;
	HUD_DRAWTRANSTRIS_FUNC				pDrawTransparentTriangles;
	HUD_STUDIOEVENT_FUNC				pStudioEvent;
	HUD_POSTRUNCMD_FUNC					pPostRunCmd;
	HUD_SHUTDOWN_FUNC					pShutdown;
	HUD_TXFERLOCALOVERRIDES_FUNC		pTxferLocalOverrides;
	HUD_PROCESSPLAYERSTATE_FUNC			pProcessPlayerState;
	HUD_TXFERPREDICTIONDATA_FUNC		pTxferPredictionData;
	HUD_DEMOREAD_FUNC					pReadDemoBuffer;
	HUD_CONNECTIONLESS_FUNC				pConnectionlessPacket;
	HUD_GETHULLBOUNDS_FUNC				pGetHullBounds;
	HUD_FRAME_FUNC						pHudFrame;
	HUD_KEY_EVENT_FUNC					pKeyEvent;
	HUD_TEMPENTUPDATE_FUNC				pTempEntUpdate;
	HUD_GETUSERENTITY_FUNC				pGetUserEntity;
	HUD_VOICESTATUS_FUNC				pVoiceStatus;		// Possibly null on old client dlls.
	HUD_DIRECTORMESSAGE_FUNC			pDirectorMessage;	// Possibly null on old client dlls.
	HUD_STUDIO_INTERFACE_FUNC			pStudioInterface;	// Not used by all clients
	HUD_CHATINPUTPOSITION_FUNC			pChatInputPosition;	// Not used by all clients
	HUD_GETPLAYERTEAM					pGetPlayerTeam; // Not used by all clients
	CLIENTFACTORY						pClientFactory;
} cldll_func_t;
