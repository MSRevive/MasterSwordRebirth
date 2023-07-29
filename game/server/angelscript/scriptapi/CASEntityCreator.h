//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_CASENTITYCREATOR_H
#define GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_CASENTITYCREATOR_H

#include <cstdint>
#include <string>
#include <vector>

class asIScriptEngine;
class CEntityData;

class IEntityKeyData final
{
public:
	IEntityKeyData() = default;
	~IEntityKeyData() = default;

	std::string m_strInput = "";
	std::string m_strValue = "";

private:
	IEntityKeyData( const IEntityKeyData& vdata );
};

class CEntityData final
{
public:
	CEntityData() = default;
	~CEntityData() = default;

	void AddInput( const std::string &strInput, const std::string &strValue );
	int GetSize() { return m_hKeys.size(); }
	int GetSpawnFlags() { return m_nFlags; }
	bool HasCustomFlags() { return m_bCustomFlags; }
	const char *GetString( int index, bool type );

	std::vector<IEntityKeyData *> m_hKeys;
	int m_nFlags = 0;
	bool m_bCustomFlags = false;

private:
	CEntityData( const CEntityData & );
};

void RegisterASEntityCreator( asIScriptEngine &engine );
void RegisterCustomASEntities( asIScriptEngine &engine );

#endif // GAME_SERVER_ZPS_ANGELSCRIPT_SCRIPTAPI_CASENTITYCREATOR_H
