//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SHARED_ZPS_ANGELSCRIPT_CASEBASEMODULEBUILDER_H
#define GAME_SHARED_ZPS_ANGELSCRIPT_CASEBASEMODULEBUILDER_H

#include <string>

#include <FileSystem.h>
#include <AngelscriptUtils/IASModuleBuilder.h>

enum ErrorBaseModuleLoader
{
	ER_SUCCESS,
	ER_FILE_NOT_FOUND,
	ER_FILE_CANT_BE_ADDED,
	ER_INCLUDE_LOADING,
	ER_INCLUDE_READING,
	ER_INCLUDE_OPEN,
	ER_INCLUDE_OPEN_SCRIPT,
	ER_BAD_COMPILE
};

/**
*	Base class for builders that handle internal and regular scripts.
*	Internal scripts are scripts that the application itself defines. These usually contain base classes for extension classes.
*	Regular scripts are loaded from disk.
*/
class CASBaseModuleBuilder : public IASModuleBuilder
{
public:
	// Internal scripts
	typedef std::vector<std::pair<std::string, std::string>> InternalScripts_t;
	typedef std::vector<std::string> Scripts_t;

	/**
	*	Constructor.
	*	@param szBasePath Path to prepend to scripts when loading them.
	*	@param szModuleTypeName Name of the module type being built.
	*/
	CASBaseModuleBuilder( std::string&& szBasePath, std::string&& szModuleTypeName );
	~CASBaseModuleBuilder() = default;

	/**
	*	@return The base path.
	*/
	const std::string& GetBasePath() const { return m_szBasePath; }

	/**
	*	@return List of scripts.
	*/
	const Scripts_t& GetScripts() const { return m_Scripts; }

	/**
	*	@param pszName Script name.
	*	@return Whether the given script is in the list.
	*/
	bool HasScript( const char* const pszName ) const;

	/**
	*	@param szName Script name.
	*	@return Whether the given script has been added.
	*/
	bool AddScript( const char *szName );

#if defined( SOURCE1 )
	// Same as AddScript, except it reads our game's filesystem instead.
	bool AddScriptFileSystem( CScriptBuilder& builder, const char *szName );
	bool IncludeScriptFileSystem( CScriptBuilder& builder, const char* const pszIncludeFileName, const char* const pszFromFileName );
	int FileSystemReading( CScriptBuilder& builder, const char *szFile, FileHandle_t hFile, bool encrypted );
#endif

	bool DefineWords( CScriptBuilder& builder ) override;

	bool AddScripts( CScriptBuilder& builder ) override;

	bool IncludeScript( CScriptBuilder& builder, const char* const pszIncludeFileName, const char* const pszFromFileName ) override;

	bool PreBuild( CScriptBuilder& builder ) override;

	bool PostBuild( CScriptBuilder& builder, const bool bSuccess, CASModule* pModule ) override;

	asIScriptFunction *ImportedFunction( CScriptBuilder& builder, asUINT index );

	void ImportFunctions( CScriptBuilder& builder );

	virtual void PrintError( ErrorBaseModuleLoader err, const char *strinput1, const char *strinput2 = "", const char *strinput3 = "", const char *strinput4 = "" );

	// For internal scripts
	bool AddInternalScript( std::string&& szName, std::string&& szContents );
	const InternalScripts_t& GetInternalScripts() const { return m_InternalScripts; }
	bool HasInternalScript( const char* const pszName ) const;

	void DumpContents( const char *szFile, const char *szBuffer );

	bool IncludeCoreMSCScripts();

private:
	bool m_IgnorePrintError = false;
	std::string m_szBasePath;
	std::string m_szModuleTypeName;
	std::vector<std::string> m_Scripts;
	InternalScripts_t m_InternalScripts;
};

#endif // GAME_SHARED_ZPS_ANGELSCRIPT_CASEBASEMODULEBUILDER_H
