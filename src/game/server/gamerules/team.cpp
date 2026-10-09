#include "msdllheaders.h"
#include "player/player.h"
#include "teams.h"

#include <climits>

extern int gmsgTeamInfo;
mslist<CTeam *> CTeam::Teams;

//Never returns 0, so an ID of 0 always means "no party"
static ulong NewTeamID()
{
	ulong ID;
	do
		ID = (ulong)RANDOM_LONG(1, LONG_MAX);
	while (CTeam::GetTeam(ID));

	return ID;
}

//Pass ID 0 to create a brand new party.  Pass a saved ID to restore a party: all members with
//that ID end up in the same team, no matter who loads first.
CTeam *CTeam::CreateTeam(const char *pszName, ulong ID)
{
	if (!pszName || !pszName[0])
		return NULL;

	//Team exists (another member already restored it), use it
	if (ID)
	{
		CTeam *pTeam = GetTeam(ID);
		if (pTeam)
			return pTeam;
	}

	//A different party already uses this name.  Don't merge into a stranger's party
	if (GetTeam(pszName))
		return NULL;

	//Create new team
	CTeam *pNewTeam = msnew CTeam;
	strncpy((char *)pNewTeam->m_TeamName, pszName, MAX_TEAMNAME_LEN);
	pNewTeam->m_ID = (int)(ID ? ID : NewTeamID());

	Teams.add(pNewTeam);

	return pNewTeam;
}

CTeam *CTeam::GetTeam(const char *pszName)
{
	if (!pszName || !pszName[0])
		return NULL;
	for (int i = 0; i < Teams.size(); i++)
		if (FStrEq(Teams[i]->TeamName(), pszName))
			return Teams[i];

	return NULL;
}
CTeam *CTeam::GetTeam(ulong ID)
{
	for (int i = 0; i < Teams.size(); i++)
		if (Teams[i]->m_ID == ID)
			return Teams[i];

	return NULL;
}

void CTeam ::ValidateUnits()
{
	for (int i = 0; i < MemberList.size(); i++)
	{
		teamunit_t &Unit = MemberList[i];
		if ((ulong)UTIL_PlayerByIndex(Unit.idx) != Unit.ID)
		{
			//Member doesn't exist.  Remove from team
			MemberList.erase(i);
			i--;
		}
	}
}
CBasePlayer *CTeam ::GetPlayer(int idx)
{
	if (idx >= (signed)MemberList.size())
		return NULL;
	CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(MemberList[idx].idx);
	if (!pPlayer || (ulong)pPlayer != MemberList[idx].ID)
		return NULL;
	return pPlayer;
}

void CTeam ::AddToTeam(CBasePlayer *pPlayer)
{
	if (ExistsInList(pPlayer))
		return;

	//pPlayer->SetTeam( this );
	teamunit_t Unit;
	Unit.idx = pPlayer->entindex();
	Unit.ID = (ulong)pPlayer;

	MemberList.add(Unit);

	//Update everyone
	for (int n = 1; n <= gpGlobals->maxClients; n++)
	{
		CBasePlayer *pSendPlayer = (CBasePlayer *)UTIL_PlayerByIndex(n);
		if (!pSendPlayer)
			continue;

		MSGSend_PlayerTeam(pSendPlayer, pPlayer);
	}
}
void CTeam ::RemoveFromTeam(CBasePlayer *pPlayer)
{
	if (!pPlayer || !ExistsInList(pPlayer))
		return;

	for (int i = 0; i < MemberList.size(); i++)
		if (GetPlayer(i) == pPlayer)
		{
			MemberList.erase(i);
			break;
		}

	//Update everyone
	for (int n = 1; n <= gpGlobals->maxClients; n++)
	{
		CBasePlayer *pSendPlayer = (CBasePlayer *)UTIL_PlayerByIndex(n);
		if (!pSendPlayer)
			continue;

		MSGSend_PlayerTeam(pSendPlayer, pPlayer);
	}

	//If no members are left, this team will be deleted in CHalfLifeMultiplay::Think()
}

BOOL CTeam ::ExistsInList(CBasePlayer *pPlayer)
{
	if (!pPlayer)
		return FALSE;
	ValidateUnits();
	for (int i = 0; i < MemberList.size(); i++)
		if (MemberList[i].idx == pPlayer->entindex() && MemberList[i].ID == (ulong)pPlayer)
			return TRUE;
	return FALSE;
}
CTeam::~CTeam()
{
	ValidateUnits();
	//Backwards, because SetTeam(NULL) erases the member from MemberList
	for (int i = (int)MemberList.size() - 1; i >= 0; i--)
	{
		CBasePlayer *pPlayer = (CBasePlayer *)UTIL_PlayerByIndex(MemberList[i].idx);
		if (!pPlayer || (ulong)pPlayer != MemberList[i].ID)
			continue;

		pPlayer->SetTeam(NULL);
	}
}

void MSGSend_PlayerTeam(CBasePlayer *pSendToPlayer, CBasePlayer *pPlayer)
{
	MESSAGE_BEGIN(MSG_ONE, gmsgTeamInfo, NULL, pSendToPlayer->edict());
	WRITE_BYTE(pPlayer->entindex());
	WRITE_STRING(pPlayer->TeamID());
	WRITE_LONG(pPlayer->m_pTeam ? pPlayer->m_pTeam->m_ID : 0);
	MESSAGE_END();
}

void CTeam ::UpdateClients()
{
}
void CTeam ::SetSolidMembers(BOOL fSolid)
{
}

BOOL SameTeam(CBaseEntity *pObject1, CBaseEntity *pObject2)
{
	//Original:
	/*if( !pObject1->TeamID()[0] || !pObject2->TeamID()[0] ) return FALSE;
	if( pObject1->TeamID() == pObject2->TeamID() ) return TRUE;
	return FALSE;*/

	if (!pObject1->TeamID()[0] || !pObject2->TeamID()[0])
		return FALSE;
	if (strcmp(pObject1->TeamID(), pObject2->TeamID()) == 0)
		return TRUE; //MiB AUG2007a: Comparing with == is bad. Bad, Dogg. Bad.
	return FALSE;
}
/*int CBaseEntity::TeamID( void ) { return m_pTeam ? m_pTeam->TeamID() : -1; }
void CBaseEntity::SetTeam( CTeam *pTeam ) {
	if( !pTeam && m_pTeam ) m_pTeam->RemoveFromTeam( edict() );
	m_pTeam = pTeam; 
	if( m_pTeam ) m_pTeam->AddToTeam( edict() ); 
}*/
