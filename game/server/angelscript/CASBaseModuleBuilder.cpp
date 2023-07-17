//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include <algorithm>
#include <experimental/filesystem>

#include "angelscript/CASContagionImport.h"
#include <Angelscript/add_on/scriptbuilder.h>

#include "CASBaseModuleBuilder.h"

#include "angelscript/CHL2ASServerManager.h"
#include <AngelscriptUtils/CASModule.h>
#include "CASLoggerInterface.h"

#include <angelscript/ScriptAPI/CASCustomEntityUtils.h>

#include <windows.h>
#include "global.h"

// Ported from Source Engine, hurrdidurr
#include "IceKey.H"

namespace fs = std::experimental::filesystem;

CASBaseModuleBuilder::CASBaseModuleBuilder( std::string&& szBasePath, std::string&& szModuleTypeName )
	: m_szBasePath( std::move( szBasePath ) )
	, m_szModuleTypeName( std::move( szModuleTypeName ) )
{
#ifndef CLIENT_DLL
	// Add in the custom entity base classes.
	// Client doesn't read these, as we already network the base entity over to the client (only the required stuff anyway)
	for ( const auto baseClass : g_CustomEntities.GetBaseClassList() )
	{
		AddInternalScript( "__" + baseClass.szClassName, std::string( baseClass.szClassDeclaration ) );
	}
#endif
	m_IgnorePrintError = false;
}

bool CASBaseModuleBuilder::AddInternalScript( std::string&& szName, std::string&& szContents )
{
	if ( HasInternalScript( szName.c_str() ) ) return false;
	m_InternalScripts.emplace_back( std::make_pair( std::move( szName ), std::move( szContents ) ) );
	return true;
}

bool CASBaseModuleBuilder::HasInternalScript( const char* const pszName ) const
{
	assert( pszName );
	if ( !pszName ) return false;
	auto it = std::find_if( m_InternalScripts.begin(), m_InternalScripts.end(), 
				  [ = ]( const std::pair<std::string, std::string> script )
	{
		return script.first == pszName;
	} );
	return it != m_InternalScripts.end();
}

void CASBaseModuleBuilder::DumpContents( const char* szFile, const char* szBuffer )
{
	// Dumps our content to a file.
	// ONLY use this for debugging!
	FILE *pFile = nullptr;
	fopen_s( &pFile, szFile, "wt" );
	if ( !pFile ) return;
	fprintf( pFile, "%s", szBuffer );
	fclose( pFile );
}

bool CASBaseModuleBuilder::IncludeCoreMSCScripts()
{
	// Make sure we include these first. But ignore the error first, if we don't find the asc format.
	m_IgnorePrintError = true;
	bool bRet = false;
	if ( MSGlobals::HasConnected )
		bRet = AddScript( "core/msc.asc" );
	else
		bRet = AddScript( "core/msc" );
	m_IgnorePrintError = false;
	return bRet;
}

bool CASBaseModuleBuilder::HasScript( const char* const pszName ) const
{
	if ( !pszName )
		return false;

	auto it = std::find_if( m_Scripts.begin(), m_Scripts.end(),
							[ = ]( const auto& script )
	{
		return script == pszName;
	} );

	return it != m_Scripts.end();
}

bool CASBaseModuleBuilder::AddScript( const char *szName )
{
	if ( HasScript( szName ) )
		return false;

	char szPlugin[128];
	strcpy( szPlugin, szName );
	if ( MSGlobals::HasConnected )
	{
		// Make sure we ONLY add ASC files.
		std::string strcheckFile = szName;
		ContagionImport::UTIL_STDReplaceString( strcheckFile, ".as", ".asc" );
	}

	m_Scripts.emplace_back( std::move( szPlugin ) );

	return true;
}

#if defined( SOURCE1 )
bool CASBaseModuleBuilder::AddScriptFileSystem( CScriptBuilder& builder, const char* szName )
{
	FileHandle_t hFile = g_pFileSystem->Open( szName, "rb", "GAME" );
	bool bDecode = false;
	bool bLoaded = false;
	if ( !hFile )
	{
		// Try encrypted one
		std::string strcheckCompiled = szName;
		ContagionImport::UTIL_STDReplaceString( strcheckCompiled, ".as", ".asc" );
		ContagionImport::UTIL_STDReplaceString( strcheckCompiled, ".ascc", ".asc" );	// Don't make an oof
		// load file into a null-terminated buffer
		FileHandle_t hFile = g_pFileSystem->Open( strcheckCompiled.c_str(), "rb", "GAME");
		if ( !hFile )
			return false;
		bDecode = true;
		bLoaded = true;
	}

	// Add to memory
	const auto result = FileSystemReading( builder, szName, hFile, bDecode );
	if ( result < 0 )
	{
		PrintError( ER_FILE_CANT_BE_ADDED, szName );
		bLoaded = false;
	}

	// Close it after use
	g_pFileSystem->Close( hFile );

	// Return our state
	return bLoaded;
}

bool CASBaseModuleBuilder::IncludeScriptFileSystem( CScriptBuilder& builder, const char* const pszIncludeFileName, const char* const pszFromFileName )
{
	std::experimental::filesystem::path path( pszFromFileName );
	path.remove_filename();

	char szPath[ MAX_PATH ];
	strcpy( szPath, path.string().c_str() );
#ifdef WIN32
	ContagionImport::V_strcat( szPath, "\\", sizeof( szPath ) );
#else
	ContagionImport::V_strcat( szPath, "/", sizeof( szPath ) );
#endif
	ContagionImport::V_strcat( szPath, pszIncludeFileName, sizeof( szPath ) );

	bool bEncrypted = false;
	if ( ContagionImport::Q_stristr( szPath, ".asc" ) )
		bEncrypted = true;
	else
	{
		if ( !ContagionImport::Q_stristr( szPath, ".as" ) )
			ContagionImport::V_strcat( szPath, ".as", sizeof( szPath ) );
	}

	// Read our file, and grab it's content
	FileHandle_t hFile = g_pFileSystem->Open( szPath, "rb", "GAME" );
	if ( !hFile )
	{
		PrintError( ER_INCLUDE_OPEN, pszFromFileName, pszIncludeFileName );
		return false;
	}

	// Add to memory
	const auto result = FileSystemReading( builder, szPath, hFile, bEncrypted );
	bool bSuccess = false;
	if ( result >= 0 )
	{
		if ( result == 1 )
			g_ASManager.ASPrint( "Included script \"{lime}%s{white}\"", szPath );
		bSuccess = true;
	}
	else
	{
		if ( result == -1 )
			PrintError( ER_INCLUDE_READING, pszFromFileName, pszIncludeFileName );
		else
			PrintError( ER_INCLUDE_LOADING, pszFromFileName, pszIncludeFileName );
	}

	// Make sure we close it after use.
	g_pFileSystem->Close( hFile );

	return bSuccess;
}

const unsigned char* UTILS_EncryptionKey_AS( void )
{
	static const unsigned char key[32] = {
		0x4c, 0x66, 0x3d, 0x35, 0x6f, 0x2a, 0x63, 0x30, 0x45, 0x13, 0x84, 0x20,
		0xCF, 0x02, 0x47, 0xB9, 0x85, 0xEC, 0x78, 0xD7, 0x85, 0xEF, 0x07, 0xD7,
		0xA8, 0x15, 0x11, 0x6E, 0x11, 0xDF, 0xEE, 0x39
	};
	return (unsigned char*)key;
}

int CASBaseModuleBuilder::FileSystemReading( CScriptBuilder& builder, const char* szFile, FileHandle_t hFile, bool encrypted )
{
	const char* data = nullptr;

	// On source, this is more streamlined. But not here.
	int size = g_pFileSystem->Size( hFile );
	char* _pBuffer = new char[ size + 1 ];
	int filesize = g_pFileSystem->Read( _pBuffer, size, hFile );
	if ( filesize != size )
		return -1;
	data = _pBuffer;

	std::string strbuf = data;

	// If encrypted, decode it
	if ( encrypted )
	{
		IceKey ice( 0 ); // level 0 = 64bit key
		ice.set( (unsigned char*)UTILS_EncryptionKey_AS() ); // set key

		int blockSize = ice.blockSize();

		unsigned char *temp = (unsigned char *)_alloca( filesize );
		unsigned char *p1 = (unsigned char *)_pBuffer;
		unsigned char *p2 = temp;

		int bytesLeft = filesize;
		while ( bytesLeft >= blockSize )
		{
			ice.decrypt( p1, p2 );
			bytesLeft -= blockSize;
			p1+=blockSize;
			p2+=blockSize;
		}

		memcpy( p2, p1, bytesLeft );

		std::string strbuf = (char *)temp;
	}

	delete[] _pBuffer;

	return builder.AddSectionFromMemory( szFile, strbuf.c_str(), strbuf.size() );
}
#endif

bool CASBaseModuleBuilder::DefineWords( CScriptBuilder& builder )
{
#ifdef CLIENT_DLL
	builder.DefineWord( "CLIENT_DLL" );
#else
	builder.DefineWord( "SERVER_DLL" );
#endif

	return true;
}

bool CASBaseModuleBuilder::AddScripts( CScriptBuilder& builder )
{
	for ( auto& script : m_InternalScripts )
	{
		if ( builder.AddSectionFromMemory( script.first.c_str(), script.second.c_str() ) < 0 )
		{
			as::GameLog( as::LogType::LOG_ERR, "CASBaseModuleBuilder::AddScripts: Error adding internal script \"%s\"\n", script.first.c_str() );
			return false;
		}
	}

	for ( const auto& script : m_Scripts )
	{
		auto scriptName = m_szBasePath + script;
		char szRelativePath[ MAX_PATH ];
		// We are trying to load a precompiled file!!
		if ( ContagionImport::Q_stristr( scriptName.c_str(), ".asc" ) )
		{
			std::string strcheckCompiled = scriptName;
			ContagionImport::UTIL_STDReplaceString( strcheckCompiled, ".as", ".asc" );
			ContagionImport::UTIL_STDReplaceString( strcheckCompiled, ".ascc", ".asc" );	// Don't make an oof
			FileHandle_t fileHandle = g_pFileSystem->Open( strcheckCompiled.c_str(), "rb", "GAME" );
			// We found a compiled AS file!
			if ( fileHandle )
			{
				// load file into a null-terminated buffer
				int fileSize = g_pFileSystem->Size( fileHandle );
				char* buffer = new char[ fileSize + 1 ];

				g_pFileSystem->Read( buffer, fileSize, fileHandle ); // read into local buffer
				buffer[fileSize] = 0; // null terminate file as EOF
				g_pFileSystem->Close( fileHandle );	// close file after reading

				IceKey ice( 0 ); // level 0 = 64bit key
				ice.set( (unsigned char*)UTILS_EncryptionKey_AS() ); // set key

				int blockSize = ice.blockSize();

				unsigned char *temp = (unsigned char *)_alloca( fileSize );
				unsigned char *p1 = (unsigned char*)buffer;
				unsigned char *p2 = temp;

				int bytesLeft = fileSize;
				while ( bytesLeft >= blockSize )
				{
					ice.decrypt( p1, p2 );
					bytesLeft -= blockSize;
					p1+=blockSize;
					p2+=blockSize;
				}

				memcpy( p2, p1, bytesLeft );

				// Grab our full path
				const char *szFullPath = g_pFileSystem->GetLocalPath( strcheckCompiled.c_str(), szRelativePath, sizeof( szRelativePath ) );
				if ( !szFullPath )
				{
					PrintError( ER_FILE_NOT_FOUND, m_szModuleTypeName.c_str(), szRelativePath );
					return false;
				}

				// We now have a decoded buffer, lets load it!
				const auto result = builder.AddSectionFromMemory( szFullPath, buffer, fileSize );

				delete[] buffer;

				if ( result >= 0 )
				{
					return true;
				}
				else
				{
					PrintError( ER_BAD_COMPILE, strcheckCompiled.c_str(), m_szBasePath.c_str() );
					return false;
				}
			}
		}
		else
		{
			const int iResult = snprintf( szRelativePath, sizeof( szRelativePath ), "%s%s.as", m_szBasePath.c_str(), script.c_str() );
			if ( iResult < 0 || static_cast<size_t>( iResult ) >= sizeof( szRelativePath ) )
			{
				as::GameLog( as::LogType::LOG_ERR, "CASBaseModuleBuilder::AddScripts: Error formatting %s script \"%s\" path!\n", m_szModuleTypeName.c_str(), script.c_str() );
				return false;
			}

			if ( g_pFileSystem->GetLocalPath( szRelativePath, szRelativePath, sizeof( szRelativePath ) ) )
			{
				if ( int res = builder.AddSectionFromFile( szRelativePath ) <= 0 )
				{
					if ( res < 0 )
						PrintError( ER_FILE_CANT_BE_ADDED, szRelativePath );
					return false;
				}
			}
			else
			{
				PrintError( ER_FILE_NOT_FOUND, m_szModuleTypeName.c_str(), szRelativePath );
				return false;
			}
		}
	}
	return true;
}

bool CASBaseModuleBuilder::IncludeScript( CScriptBuilder& builder, const char* const pszIncludeFileName, const char* const pszFromFileName )
{
	// The following code is from SoloKiller's HLEnchanched, though with very minor edits.
	// We cannot use Contagion's IncludeScript code here, as it heavily relies on a heavily modified Source Filesystem.

	std::error_code error;
	std::experimental::filesystem::path path( pszFromFileName );

	path.remove_filename();

	path /= pszIncludeFileName;

	auto current = fs::current_path( error );
	if ( error )
	{
		PrintError( ER_INCLUDE_LOADING, pszFromFileName, pszIncludeFileName );
		return false;
	}

	current = fs::canonical( current, current, error );
	if ( error )
	{
		PrintError( ER_INCLUDE_LOADING, pszFromFileName, pszIncludeFileName );
		return false;
	}

	path = fs::canonical( path, current, error );
	if ( error )
	{
		PrintError( ER_INCLUDE_LOADING, pszFromFileName, pszIncludeFileName );
		return false;
	}

	auto it = path.begin();
	auto end = path.end();

	for ( auto component : current )
	{
		//operator== is case sensitive so C:\ and c:\ are considered to be different.
		if ( stricmp( it->u8string().c_str(), component.u8string().c_str() ) != 0 )
		{
			PrintError( ER_INCLUDE_LOADING, pszFromFileName, pszIncludeFileName );
			return false;
		}
		if ( it != end )
			++it;
	}

	//The current directory is the exe directory, but filesystem calls start in exedir/moddir, so advance once more.
	if ( it != end )
		++it;

	fs::path relativePath;

	for ( ; it != end; ++it )
	{
		relativePath /= *it;
	}

	const auto szRelativePath = relativePath.u8string();

	CFile file( szRelativePath.c_str(), "rb" );

	bool bSuccess = false;

	if ( file.IsOpen() )
	{
		const auto size = file.Size();

		auto data = std::make_unique<char[]>( size + 1 );

		if ( file.Read( data.get(), size ) == static_cast<int>( size ) )
		{
			const auto result = builder.AddSectionFromMemory( szRelativePath.c_str(), data.get(), size );

			if ( result >= 0 )
			{
				if ( result == 1 )
					g_ASManager.ASPrint( "Included script \"{lime}%s{white}\"", szRelativePath.c_str() );

				bSuccess = true;
			}
			else
				PrintError( ER_INCLUDE_LOADING, pszFromFileName, pszIncludeFileName );
		}
		else
			PrintError( ER_INCLUDE_READING, pszFromFileName, pszIncludeFileName );
	}
	else
	{
		bSuccess = IncludeScriptFileSystem( builder, pszIncludeFileName, pszFromFileName );
		if ( !bSuccess )
			PrintError( ER_INCLUDE_OPEN_SCRIPT, pszFromFileName, pszIncludeFileName );
	}

	return bSuccess;
}

bool CASBaseModuleBuilder::PreBuild( CScriptBuilder& builder )
{
#if defined( AS_DEBUG )
	const auto& scripts = GetScripts();
	g_ASManager.ASPrint( "{lime}%u{default} script%s\n\t{yellow}Compiling...\n", scripts.Count(), scripts.Count() == 1 ? "" : "s" );
#endif
	return true;
}

bool CASBaseModuleBuilder::PostBuild( CScriptBuilder& builder, const bool bSuccess, CASModule* pModule )
{
#if defined( AS_DEBUG )
	g_ASManager.ASPrint( "{lime}Done{default}!\n%s script compilation %s\n", m_szModuleTypeName.c_str(), bSuccess ? "{green}succeeded" : "{red}failed" );
#endif
	if ( bSuccess )
		ImportFunctions( builder );
	return true;
}

asIScriptFunction* CASBaseModuleBuilder::ImportedFunction( CScriptBuilder& builder, asUINT index )
{
	// MSC does not support plugins. It only support base script(s) and map scripts
#if 0
	auto find_func = builder.GetModule()->GetImportedFunctionDeclaration( index );
	auto find_module = builder.GetModule()->GetImportedFunctionSourceModule( index );
	asIScriptFunction *importfunc = nullptr;

	// Find our module
	// NOTE: Do NOT use GetModule(), as it will FAIL to read the name even if we find it in the index count.
	// To fix this, I simply used the plugin list count that we already created, and utilised that instead.
	for ( size_t uiIndex = 0; uiIndex < g_ASManager.GetPluginManager().GetPluginCount(); ++uiIndex )
	{
		const auto pModule = g_ASManager.GetPluginManager().GetPluginByIndex( uiIndex );
		auto pData = CASModule_GetPluginData( pModule );
		if ( !stricmp( pData->GetName(), find_module ) )
		{
			if ( zps_angelscript_debug.GetBool() )
				g_ASManager.ASPrint( "Found module [{gold}%s{white}]\n", pData->GetName() );
			auto srcMod = pModule->GetModule();
			importfunc = srcMod->GetFunctionByDecl( find_func );
			break;
		}
	}

	if ( importfunc && zps_angelscript_debug.GetBool() )
		g_ASManager.ASPrint( "Found function [{gold}%s{white}]!\n", find_func );

	return importfunc;
#else
	return nullptr;
#endif
}

void CASBaseModuleBuilder::ImportFunctions( CScriptBuilder& builder )
{
	for ( asUINT i = 0; i < builder.GetModule()->GetImportedFunctionCount(); i++ )
	{
		asIScriptFunction *func = ImportedFunction( builder, i );
		if ( func )
		{
#if defined( AS_DEBUG )
			int res = builder.GetModule()->BindImportedFunction( i, func );
			g_ASManager.ASPrint( "\t{gold}BindImportedFunction {white}>> {green}%i\n", res );
#else
			builder.GetModule()->BindImportedFunction( i, func );
#endif
		}
	}
}

void CASBaseModuleBuilder::PrintError( ErrorBaseModuleLoader err, const char *strinput1, const char *strinput2, const char *strinput3, const char *strinput4 )
{
	if ( m_IgnorePrintError ) return;

	const char *strErr = "";
	switch ( err )
	{
		case ER_FILE_NOT_FOUND:
			strErr = "{rare}ER_FILE_NOT_FOUND{default}: Couldn't find %s script \"{red}%s{default}\"!";
		break;

		case ER_FILE_CANT_BE_ADDED:
			strErr = "{rare}ER_FILE_CANT_BE_ADDED{default}: Error adding script \"{red}%s{default}\"!";
		break;

		case ER_INCLUDE_LOADING:
			strErr = "{rare}ER_INCLUDE_LOADING{default}: Error including script \"{red}%s{default}\" in \"{yellow}%s{default}\"!";
		break;

		case ER_INCLUDE_READING:
			strErr = "{rare}ER_INCLUDE_READING{default}: Error reading \"{red}%s{default}\" to include in \"{yellow}%s{default}\"!";
		break;

		case ER_INCLUDE_OPEN:
			strErr = "{rare}ER_INCLUDE_OPEN{default}: Error opening \"{red}%s{default}\" to include in \"{yellow}%s{default}\"!";
		break;

		case ER_INCLUDE_OPEN_SCRIPT:
			strErr = "{rare}ER_INCLUDE_OPEN_SCRIPT{default}: Couldn't open script \"{red}%s{default}\" to include in \"{yellow}%s{default}\"!";
		break;

		case ER_BAD_COMPILE:
			strErr = "{rare}ER_BAD_COMPILE{default}: The script \"{red}%s{default}\" was compiled incorrectly! (from section name \"{yellow}%s{default}\")";
		break;
	}

	g_ASManager.ASLogPrint(
		"Angelscript Base Module Builder",
		ContagionImport::UTIL_LogFolder( LOG_ANGELSCRIPT, false ),
		err == ER_SUCCESS ? as::LogType::LOG_INF : as::LogType::LOG_WRN,
		strErr,
		strinput1,
		strinput2,
		strinput3,
		strinput4
	);
}
