//=============================================================================
//
// ms.dll - Master Sword server .dll/.so
//
// https://msrebirth.net/
//
// Author: Tom 'tschumann' Schumann
// Notes: load entities from .ent file
//
//=============================================================================

struct entity_t
{
	char classname[32];
	float angles[3];
	float origin[3];
};

//=========================================================
// CEntity 
//=========================================================

class CEntity
{
public:
	void	InitEntity( void );
	
	int		CheckENTFile(char *szMapName);
	int		FLoadEntity(char *szMapName);
private:
	void	FixKeyName( char *key );
	char	*aMemFile;
	char	*pMemFile;
	int	length;
};

extern CEntity WorldEntity;
