#ifndef ANGELSCRIPT_IASMODULEBUILDER_H
#define ANGELSCRIPT_IASMODULEBUILDER_H

#include "util/ASPlatform.h"

class CScriptBuilder;
class CASModule;

/**
*	@addtogroup ASModule
*
*	@{
*/

/**
*	Interface used to provide necessary information to build a module.
*/
class IASModuleBuilder
{
public:
	/**
	*	Destructor.
	*/
	virtual ~IASModuleBuilder() = 0;

	/**
	*	Allows preprocessor words to be defined.
	*	@param builder Builder to define words for.
	*	@return true on success, false on failure.
	*/
	virtual bool DefineWords( CScriptBuilder& builder );

	/**
	*	Allows scripts to be added.
	*	@param builder Builder to add scripts to.
	*	@return true on success, false on failure.
	*/
	virtual bool AddScripts( CScriptBuilder& builder ) = 0;

	/**
	*	Allows \#include statements to be processed.
	*	@param builder Builder to add scripts to.
	*	@param pszIncludeFileName Name of the file that is being included.
	*	@param pszFromFileName Name of the file that contains the include statement.
	*	@return true on success, false on failure.
	*/
	virtual bool IncludeScript( CScriptBuilder& builder, 
								const char* const pszIncludeFileName, 
								const char* const pszFromFileName );

	/**
	*	Called right before the module is built. Gives the builder a chance to evaluate the module.
	*	@param builder Builder.
	*	@return true if the module should be built, false otherwise.
	*/
	virtual bool PreBuild( CScriptBuilder& builder );

	/**
	*	Called after the build has finished.
	*	@param builder Builder.
	*	@param bSuccess Whether the build succeeded or failed.
	*	@param pModule If bSuccess is true, the module. Otherwise, null.
	*	@return true if the module should be kept, false if it should be discarded.
	*/
	virtual bool PostBuild( CScriptBuilder& builder, const bool bSuccess, CASModule* pModule );
};

inline IASModuleBuilder::~IASModuleBuilder()
{
}

inline bool IASModuleBuilder::DefineWords( CScriptBuilder& )
{
	return true;
}

inline bool IASModuleBuilder::IncludeScript( CScriptBuilder&, const char* const, const char* const )
{
	return false;
}

inline bool IASModuleBuilder::PreBuild( CScriptBuilder& )
{
	return true;
}

inline bool IASModuleBuilder::PostBuild( CScriptBuilder&, const bool, CASModule* )
{
	return true;
}

/** @} */

#endif //ANGELSCRIPT_IASMODULEBUILDER_H