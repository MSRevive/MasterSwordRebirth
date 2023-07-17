#ifndef GAME_SERVER_ANGELSCRIPT_ENTITIES_CCUSTOMCBASEENTITY_H
#define GAME_SERVER_ANGELSCRIPT_ENTITIES_CCUSTOMCBASEENTITY_H

#include <angelscript.h>
#include <AngelscriptUtils/util/CASExtendAdapter.h>

#include "angelscript/CASLoggerInterface.h"

// Include our ent utils
#include "CASCustomEntityUtils.h"

#undef GetObject

/**
*	Extension class for custom entities inheriting from BASECLASS.
*	Provides extension methods for CBaseEntity.
*/
template<typename BASECLASS>
class CBaseCustomCBaseEntity : public BASECLASS, public IASCustomEntity
{
public:
	CASObjPtr& GetObject() const override final
	{
		//This cast is needed because you can't return non-const references to class members.
		return const_cast<CBaseCustomCBaseEntity*>( this )->CBaseCustomCBaseEntity::m_Instance;
	}

	void SetObject( CASObjPtr&& instance ) override final
	{
		CBaseCustomCBaseEntity::m_Instance = std::move( instance );
	}

	void SetClass( const CCustomEntityClass* pClass ) override final
	{
		m_pClass = pClass;
	}

	/**
	*	Sets a function. Can be either a function of the CBaseEntity class or the script class.
	*	@param function Function to set.
	*	@param pFunction Function to set. Can be null.
	*	@param pszName Name of the fundamental function being set.
	*/
	void SetFunction( CASRefPtr<asIScriptFunction>& function, asIScriptFunction* pFunction, const char* const pszName )
	{
		if( !pFunction )
		{
			function = nullptr;
			return;
		}

		bool bSuccess = false;

		if( pFunction->GetDelegateFunction() )
		{
			auto pDelegateType = pFunction->GetDelegateObjectType();

			ASSERT( pDelegateType );

			//Note: when checking for inheritance, check if our concrete type inherits from the given type.
			//When checking if our C++ type matches, check if the given type inherits from it.
			//This prevents subclasses from being treated as incompatible
			const bool bIsScriptFunction = m_Instance.GetTypeInfo()->DerivesFrom( pDelegateType );

			//TODO: can probably cache the C++ type pointer - Solokiller
			auto pCPPType = pDelegateType->GetEngine()->GetTypeInfoByName( m_pClass->GetBaseClassData().szCPPClassName.c_str() );

			if( bIsScriptFunction ||
				( pCPPType && pCPPType->DerivesFrom( pDelegateType ) ) )
			{
				bool bIsValid = false;

				if( bIsScriptFunction )
				{
					bIsValid = pFunction->GetDelegateObject() == m_Instance.Get();
				}
				else
				{
					bIsValid = pFunction->GetDelegateObject() == this;
				}

				if( bIsValid )
				{
					function = pFunction;

					bSuccess = true;
				}
			}
		}

		if( !bSuccess )
		{
			auto pActualFunction = pFunction->GetDelegateFunction();
			auto pActualObject = pFunction->GetDelegateObjectType();

			if( !pActualFunction )
				pActualFunction = pFunction;

			const std::string szCompleteName = pActualObject ? 
				std::string( pActualObject->GetNamespace() ) + "::" + pActualObject->GetName() + "::" + pActualFunction->GetName() : 
				std::string( pActualFunction->GetNamespace() ) + "::" + pActualFunction->GetName();

			as::GameLog( as::LogType::LOG_WRN, "Couldn't set custom entity \"%s\" (class %s) function %s to \"%s\"\n", 
				   CBaseCustomCBaseEntity::GetTargetName().ToCStr(), CBaseCustomCBaseEntity::GetClassname(), pszName, szCompleteName.c_str() );
		}

		pFunction->Release();
	}

	void OnCreate() override
	{
		CALL_EXTEND_FUNC( OnCreate, "()" );
	}

	void OnDestroy() override
	{
		CALL_EXTEND_FUNC( OnDestroy, "()" );

		m_Instance.Reset();
	}

	void UpdateOnRemove()
	{
		CALL_EXTEND_FUNC( UpdateOnRemove, "()" );
	}

	void Precache() override
	{
		CALL_EXTEND_FUNC( Precache, "()" );
	}

	void Spawn() override
	{
		CALL_EXTEND_FUNC( Spawn, "()" );
	}

	void Activate() override
	{
		CALL_EXTEND_FUNC( Activate, "()" );
	}

	int ObjectCaps( void ) override
	{
		CALL_EXTEND_FUNC_RET( int, ObjectCaps, "()" );
	}

	CBaseEntity* Respawn() override
	{
		//TODO: need helper macros for different return types.
		CBaseEntity* pResult = nullptr;

		if( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "CBaseEntity@ Respawn()" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			if( method.Call( CallFlag::NONE ) )
			{
				method.GetReturnValue( &pResult );
			}
		}
		else
		{
			pResult = BaseClass::Respawn();
		}

		return pResult;
	}

	virtual void SetScriptThink( const std::string &szFunction )
	{
		m_szThinkFunc = szFunction;
	}

	virtual void OnThink()
	{
#if !defined( ASCOMPILER )
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( UTIL_VarArgs( "void %s()", m_szThinkFunc.c_str() ) ) )
		{
			as::Call( GetObject().Get(), pFunction );
		}
#endif
	}

	void SetScriptTouch( const std::string &szFunction )
	{
		m_szTouchFunc = szFunction;
	}

	virtual void OnTouch( CBaseEntity* pOther )
	{
#if !defined( ASCOMPILER )
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( UTIL_VarArgs( "void %s(CBaseEntity@)", m_szTouchFunc.c_str() ) ) )
		{
			as::Call( GetObject().Get(), pFunction, pOther );
		}
#endif
	}

	void SetScriptUse( const std::string &szFunction )
	{
		m_szUseFunc = szFunction;
	}

	virtual void OnUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
	{
#if !defined( ASCOMPILER )
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( UTIL_VarArgs( "void %s(CBaseEntity@, CBaseEntity@, USE_TYPE, float)", m_szUseFunc.c_str() ) ) )
		{
			as::Call( GetObject().Get(), pFunction, pActivator, pCaller, useType, value );
		}
#endif
	}

	virtual void SetScriptBlocked( const std::string &szFunction )
	{
		m_szBlockedFunc = szFunction;
	}

	virtual void OnBlocked( CBaseEntity* pOther )
	{
#if !defined( ASCOMPILER )
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( UTIL_VarArgs( "void %s(CBaseEntity@)", m_szBlockedFunc.c_str() ) ) )
		{
			as::Call( GetObject().Get(), pFunction, pOther );
		}
#endif
	}

	int BloodColor( void ) override
	{
		CALL_EXTEND_FUNC_RET( int, BloodColor, "()" );
	}

#if !defined( ASCOMPILER )
	int TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType ) override
	{
		int iRes = false;
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "int TakeDamage(entvars_t &in, entvars_t &in, float &in, int &in)" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			float dmg = flDamage;
			int bits = bitsDamageType;
			if ( method.Call( CallFlag::NONE, &pevInflictor, &pevAttacker, &dmg, &bits ) )
				method.GetReturnValue( &iRes );
		}
		else
			iRes = BaseClass::TakeDamage( pevInflictor, pevAttacker, flDamage, bitsDamageType );
		return iRes;
	}

	BOOL IsTriggered( CBaseEntity *pActivator ) override
	{
		CALL_EXTEND_FUNC_RET( BOOL, IsTriggered, "(CBaseEntity@)", pActivator );
	}

	int IsMoving( void ) override
	{
		CALL_EXTEND_FUNC_RET( int, IsMoving, "()" );
	}

	BOOL OnControls( entvars_t *pev ) override
	{
		CALL_EXTEND_FUNC_RET( BOOL, OnControls, "(entvars_t@)", pev );
	}

	BOOL IsAlive() override
	{
		CALL_EXTEND_FUNC_RET( BOOL, IsAlive, "() const" );
	}

	BOOL IsBSPModel() override
	{
		CALL_EXTEND_FUNC_RET( BOOL, IsBSPModel, "() const" );
	}

	BOOL IsInWorld() override
	{
		CALL_EXTEND_FUNC_RET( BOOL, IsInWorld, "() const" );
	}

	Vector EyePosition( void ) override
	{
		CALL_EXTEND_FUNC_RET( Vector, EyePosition, "()" );
	}

	Vector EarPosition( void ) override
	{
		CALL_EXTEND_FUNC_RET( Vector, EarPosition, "()" );
	}
#endif

	std::string m_szThinkFunc = "Think";
	std::string m_szUseFunc = "Use";
	std::string m_szTouchFunc = "Touch";
	std::string m_szBlockedFunc = "Blocked";

private:
	CASObjPtr m_Instance;

	const CCustomEntityClass* m_pClass = nullptr;
};


/**
*	C++ Class type for custom CBaseEntity entities.
*/
using ScriptBase_Entity = CBaseCustomCBaseEntity<CBaseEntity>;


/// <summary>
/// Angelscript related entities. Does nothing by default.
/// </summary>

class CASScript_Entity : public CBaseCustomCBaseEntity<CBaseEntity>
{
public:
	void Spawn() override;
	void OnThink() override;
	void SetScriptThink( const std::string &szFunction ) override;
};

#endif //GAME_SERVER_ANGELSCRIPT_ENTITIES_CCUSTOMCBASEENTITY_H