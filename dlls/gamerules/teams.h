#ifndef TEAM_H
#define TEAM_H

#define MAX_TEAMNAME_LEN 12

#ifdef VALVE_DLL
#ifdef CBASE_H
class CBasePlayer;

typedef EHANDLE TeamUnit;
struct teamunit_t
{
	int idx;  //Index of unit in world
	ulong ID; //Verification ID, to check if unit no longer exists or has been replaced
};

class CTeam
{
public:
	CTeam() { memset(this, 0, sizeof(CTeam)); }
	~CTeam();

	int m_ID;

	char m_TeamName[MAX_TEAMNAME_LEN + 1];

	vector<teamunit_t> MemberList;

	void AddToTeam(CBasePlayer *pNewPlayer);
	void RemoveFromTeam(CBasePlayer *pPlayer);
	BOOL ExistsInList(CBasePlayer *pPlayer);
	void ValidateUnits();
	void UpdateClients();
	void SetSolidMembers(BOOL fSolid);

	const char *TeamID() { return &m_TeamName[0]; }
	const char *TeamName() { return &m_TeamName[0]; }
	CBasePlayer *GetPlayer(int idx);

	static vector<CTeam *> Teams;
	static CTeam *CreateTeam(const char *pszName, ulong ID);
	static CTeam *GetTeam(const char *pszName);
	static CTeam *GetTeam(ulong ID);
};

BOOL SameTeam(CBaseEntity *pObject1, CBaseEntity *pObject2);
void MSGSend_PlayerTeam(CBasePlayer *pSendToPlayer, CBasePlayer *pPlayer);

#endif //EXTDLL_H
#endif //VALVE_DLL

#endif //TEAM_H
