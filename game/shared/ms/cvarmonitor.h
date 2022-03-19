// MiB AUG2019_28 - Monitors cvars for changes
#ifndef CVARMONITOR_H
#define CVARMONITOR_H

#include "hl/extdll.h"
#include "hl/vector.h"
#include "sharedutil.h"
#include "iscript.h"

class CVarListener;
class CVarMonitor;
class CVarMonitorManager;

#define CVAR_ALERTSCRIPT_EXTRAPARMS(pScript,vsCvar,vsOldValue,vsNewValue,bInit,EXTRA) \
    { \
        msstringlist vParms; \
        msstring vsEventName("game_cvarchange_"); \
        vsEventName += vsCvar; \
        vParms.add(vsOldValue); \
        vParms.add(vsNewValue); \
        vParms.add( bInit ? "1" : "0" ); \
        EXTRA \
        pScript->CallScriptEvent( vsEventName, &vParms ); \
    }
#define CVAR_ALERTSCRIPT(pScript,vsCvar,vsOldValue,vsNewValue,bInit) CVAR_ALERTSCRIPT_EXTRAPARMS(pScript,vsCvar,vsOldValue,vsNewValue,bInit,;)

#define CVAR_ALERTSERVER_CMD "alert_cvarchange"
#define CVAR_ALERTTYPE_HARDCODE 0
#define CVAR_ALERTSERVERTYPE_PLAYERSCRIPT 1
#define CVAR_ALERTCLIENTTYPE_PLAYERSCRIPT 2

class CVarMonitorManager
{
public:
    static mslist<CVarMonitor *> mMonitorList;
    static bool mbInit;
    static void Init();
    static void Think();
    static void Reset();
#ifdef VALVE_DLL
    static void ClientConnect(CBasePlayer * pPlayer);
#endif
};

class CVarMonitor
{
private:
    msstring msName;
    msstring msLastValue;
    mslist<CVarListener *> mListenerList;
    float mNextThink;
    float mThinkDelay;
public:
    CVarMonitor(msstring vsName, float vThinkDelay = 1);
    msstring GetName() { return msName; }
    void Think(bool bInit);
    void ValueChange(bool bInit, msstring sOldValue, msstring sNewValue, CBasePlayer * pPlayer = NULL);
    void AddListener(CVarListener * pListener);
    void RemoveListener(CVarListener * pListener, bool bDelete = true);
    ~CVarMonitor();
#ifdef VALVE_DLL
    void ClientConnect(CBasePlayer * pPlayer );
#endif
};

class CVarListener
{
private:
    CVarMonitor * mpParent;
public:
    CVarListener() { mpParent = NULL; }
    CVarMonitor *GetParent() { return mpParent; }
    void SetParent( CVarMonitor * pParent ) { mpParent = pParent; }
    virtual void ValueChange(bool bInit, msstring sOldValue, msstring sNewValue, CBasePlayer * pPlayer = NULL) = 0;
#ifdef VALVE_DLL
    virtual bool IsClient() const { return false; }
#endif
};

#ifdef VALVE_DLL

class AlertClients_CVarListener : public CVarListener
{
public:
    virtual void ValueChange(bool bInit, msstring sOldValue, msstring sNewValue, CBasePlayer * pPlayer = NULL);
    virtual int GetType() const = 0;
    virtual void WriteExtraData() {}
#ifdef VALVE_DLL
    virtual bool IsClient() const { return true; }
#endif
};

class AlertClientsScript_CVarListener : public AlertClients_CVarListener
{
public:
    virtual int GetType() const { return CVAR_ALERTCLIENTTYPE_PLAYERSCRIPT; }
};

class AlertClientsHardCode_CVarListener : public AlertClients_CVarListener
{
public:
    virtual int GetType() const { return CVAR_ALERTTYPE_HARDCODE; }
};

#else

class AlertServer_CVarListener : public CVarListener
{
public:
    virtual void ValueChange(bool bInit, msstring sOldValue, msstring sNewValue, CBasePlayer * pPlayer = NULL);
    virtual int GetType() const = 0;
    virtual msstring GetExtraData() { return ""; }
};

class AlertServerScript_CVarListener : public AlertServer_CVarListener
{
public:
    virtual int GetType() const { return CVAR_ALERTSERVERTYPE_PLAYERSCRIPT; }
};

class AlertServerHardCode_CVarListener : public AlertServer_CVarListener
{
public:
    virtual int GetType() const { return CVAR_ALERTTYPE_HARDCODE; }
};

#endif

class AlertScript_CVarListener : public CVarListener
{
private:
    IScripted * mpScripted;
public:
    AlertScript_CVarListener(IScripted * pScripted);
    virtual void ValueChange(bool bInit, msstring sOldValue, msstring sNewValue, CBasePlayer * pPlayer = NULL);
};

template<typename AType>
class SetValue_CVarListener : public CVarListener
{
private:
    AType * mpValu;
public:
    SetValue_CVarListener(
      typename AType * pValu
    )
    {
        mpValu = pValu;
    }

    virtual void ValueChange(bool bInit, msstring sOldValue, msstring sNewValue, CBasePlayer * pPlayer = NULL)
    {
        (*mpValu) = ConvertValue(sNewValue);
    }

    virtual AType ConvertValue(msstring sValue);
};

typedef SetValue_CVarListener<msstring> SetStringValue_CVarListener;
typedef SetValue_CVarListener<int>      SetIntValue_CVarListener;
typedef SetValue_CVarListener<float>    SetFloatValue_CVarListener;
typedef SetValue_CVarListener<Vector>   SetVectorValue_CVarListener;

#endif // CVARMONITOR_H