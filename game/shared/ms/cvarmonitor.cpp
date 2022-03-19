#include "cvarmonitor.h"
#include "global.h"
#ifdef VALVE_DLL
    #include "hl/util.h"
    #include "cbase.h"
    #include "player/player.h"
#else
    #include "wrect.h"
    #include "cl_dll.h"
    #include "inc_huditem.h"
    #include "parsemsg.h"
    #include "hudscript.h"
#endif

mslist<CVarMonitor *> CVarMonitorManager::mMonitorList;
bool CVarMonitorManager::mbInit = false;
extern globalvars_t	* gpGlobals;

void CVarMonitorManager::Init()
{
#ifdef VALVE_DLL
#else
    CVarMonitor * pMonitor;

    pMonitor = new CVarMonitor( "ms_aim_type" );
    pMonitor->AddListener( new AlertServerScript_CVarListener() );
    mMonitorList.add( pMonitor );

    pMonitor = new CVarMonitor( "ms_glowcolor" );
    pMonitor->AddListener( new AlertServerHardCode_CVarListener() );
    mMonitorList.add( pMonitor );

#endif
    CVarMonitorManager::mbInit = true;
}

void CVarMonitorManager::Think()
{
    bool bDidInit = false;
    if ( !CVarMonitorManager::mbInit )
    {
        bDidInit = true;
        Init();
    }
    
    for(int i = 0; i < mMonitorList.size(); i++)
    {
        mMonitorList[i]->Think( bDidInit );
    }
}

void CVarMonitorManager::Reset(
)
{
    for(int i = 0; i < mMonitorList.size(); i++)
    {
        delete mMonitorList[i];
    }
    mMonitorList.clear();
    mbInit = false;
}

#ifdef VALVE_DLL
void CVarMonitorManager::ClientConnect(CBasePlayer *  pPlayer)
{
    for(int i = 0; i < mMonitorList.size(); i++)
    {
        mMonitorList[i]->ClientConnect( pPlayer );
    }
}
#endif

CVarMonitor::CVarMonitor(msstring vsName, float vThinkDelay)
{
    msName = vsName;
    mThinkDelay = vThinkDelay;
    mNextThink = 0;
}

CVarMonitor::~CVarMonitor()
{
    for(int i = 0; i < mListenerList.size(); i++)
    {
        delete mListenerList[i];
    }
    mListenerList.clear();
}

void CVarMonitor::Think(bool bInit)
{
    float vCurTime = gpGlobals->time;
    if ( vCurTime < mNextThink ) return;

    #ifdef VALVE_DLL
        msstring sCurValue = EngineFunc::CVAR_GetString( msName );
    #else
        msstring sCurValue = CVAR_GET_STRING( msName );
    #endif
    if (bInit || sCurValue != msLastValue)
    {
        ValueChange(bInit,msLastValue,sCurValue);
        msLastValue = sCurValue;
    }
    mNextThink = vCurTime + mThinkDelay;
}

void CVarMonitor::ValueChange(bool bInit, msstring sOldValue, msstring sNewValue, CBasePlayer * pPlayer)
{
    for(int i = 0; i < mListenerList.size(); i++)
    {
        #ifdef VALVE_DLL
            if ( mListenerList[i]->IsClient() )
            {
                // Don't send out client messages with no clients
                // (causes an error message during server start up
                // because the messages aren't initiated and this
                // is a quicker fix for that)
                if ( !UTIL_NumPlayers() ) continue;
            }
            else
            {
                // Don't alert serverside listeners when it's just a client connecting
                if ( pPlayer ) continue; 
            }
        #endif
        mListenerList[i]->ValueChange(bInit, sOldValue, sNewValue, pPlayer);
    }
}

void CVarMonitor::AddListener(CVarListener * pListener)
{
    mListenerList.add( pListener );
    pListener->SetParent( this );
}

void CVarMonitor::RemoveListener(CVarListener * pListener, bool bDelete)
{
    for(size_t i = mListenerList.size(); i >= 0; --i)
    {
        if ( mListenerList[i] == pListener )
        {
            mListenerList.erase( i );
        }
    }
    
    if ( bDelete )
    {
        delete pListener;
    }
}

#ifdef VALVE_DLL
void CVarMonitor::ClientConnect(CBasePlayer * pPlayer)
{
    ValueChange(true, msLastValue, msLastValue, pPlayer);
}
#endif

#ifdef VALVE_DLL

void AlertClients_CVarListener::ValueChange(bool bInit, msstring sOldValue, msstring sNewValue, CBasePlayer * pPlayer)
{
    if ( pPlayer )
    {
        MESSAGE_BEGIN( MSG_ONE, g_netmsg[NETMSG_ALERTCVARCHANGE], NULL, pPlayer->pev );
    }
    else
    {
        MESSAGE_BEGIN( MSG_ALL, g_netmsg[NETMSG_ALERTCVARCHANGE], NULL );
    }

    WRITE_STRING( GetParent()->GetName().c_str() );
    WRITE_STRING( sOldValue );
    WRITE_STRING( sNewValue );
    WRITE_BOOL( bInit );
    WRITE_BYTE( GetType() );
    WriteExtraData();

    MESSAGE_END();
}

#else

void AlertServer_CVarListener::ValueChange(bool bInit, msstring sOldValue, msstring sNewValue, CBasePlayer * pPlayer)
{
    msstring vsExtraData = GetExtraData();
    msstring vsCmd = UTIL_VarArgs( "%s %s \"%s\" \"%s\" %d %d\n"
                      , CVAR_ALERTSERVER_CMD
                      , GetParent()->GetName().c_str()
                      , sOldValue.c_str()
                      , sNewValue.c_str()
                      , bInit ? 1 : 0
                      , GetType()
                     );
                     
    if ( vsExtraData.len() )
    {
        vsCmd += msstring(" ") + vsExtraData;
    }
    
    vsCmd += "\n";
    ClientCmd( vsCmd.c_str() );
}

#endif

AlertScript_CVarListener::AlertScript_CVarListener(IScripted * pScripted)
{
    mpScripted = pScripted;
}

void AlertScript_CVarListener::ValueChange(bool bInit, msstring sOldValue, msstring sNewValue, CBasePlayer * pPlayer)
{
    CVAR_ALERTSCRIPT( mpScripted, GetParent()->GetName(), sOldValue, sNewValue, bInit);
}

msstring SetValue_CVarListener<msstring>::ConvertValue(msstring sValue)
{
    return sValue;
}

int SetValue_CVarListener<int>::ConvertValue(msstring sValue)
{
    return atoi(sValue);
}

float SetValue_CVarListener<float>::ConvertValue(msstring sValue)
{
    return atof(sValue);
}

Vector SetValue_CVarListener<Vector>::ConvertValue(msstring sValue)
{
    return StringToVec(sValue);
}

#ifndef VALVE_DLL
int __MsgFunc_CvarChange( const char *pszName, int iSize, void *pbuf )
{
	BEGIN_READ( pbuf, iSize);
    msstring vsCvar = READ_STRING();
    msstring vsOldValue = READ_STRING();
    msstring vsNewValue = READ_STRING();
    bool bInit = READ_BOOL();
    int vType = READ_BYTE();
    switch( vType )
    {
        case CVAR_ALERTTYPE_HARDCODE:
            break;
        case CVAR_ALERTCLIENTTYPE_PLAYERSCRIPT:
            CVAR_ALERTSCRIPT(gHUD.m_HUDScript, vsCvar, vsOldValue, vsNewValue, bInit);
            break;
    }
    return 1;
}
#endif