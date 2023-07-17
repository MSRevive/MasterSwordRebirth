#ifndef GAME_SHARED_ANGELSCRIPT_CUSTOMWEAPON_WEAPON_H
#define GAME_SHARED_ANGELSCRIPT_CUSTOMWEAPON_WEAPON_H

#if !defined( CLIENT_DLL )
// On server, we want to read all of the info
#include <angelscript/ScriptAPI/CASCustomEntity_Entity.h>
#include <angelscript/CASLoggerInterface.h>
#else
// On client, we only care about our interfaces
#include <angelscript.h>
#include <AngelscriptUtils/util/CASExtendAdapter.h>
#include <angelscript/ScriptAPI/IASCustomEntity.h>
#endif

#if !defined( ASCOMPILER )
#include "terror/weapons/TerrorWeapon.h"
#else
#include "angelscript/compile_fix/base_ent.h"
#include "angelscript/compile_fix/terrorweapon.h"
#endif

#if defined( CLIENT_DLL )
#define CBaseCustomCBaseWeapon C_BaseCustomCBaseWeapon
#else
void RegisterASCTerrorWeapon( asIScriptEngine &engine );
#endif

template<typename BASECLASS>
class CBaseCustomCBaseWeapon : public BASECLASS, public IASCustomEntity
{

public:
	DECLARE_CLASS( CBaseCustomCBaseWeapon, BASECLASS );

	CASObjPtr& GetObject() const override final
	{
		//This cast is needed because you can't return non-const references to class members.
		return const_cast<CBaseCustomCBaseWeapon*>( this )->CBaseCustomCBaseWeapon::m_Instance;
	}

	void SetObject( CASObjPtr&& instance ) override final
	{
		CBaseCustomCBaseWeapon::m_Instance = std::move( instance );
	}

	void SetClass( const CCustomEntityClass* pClass ) override final
	{
		m_pClass = pClass;
		m_pInf = new CTerrorWeaponInfo;
	}

	/**
	*	Sets a function. Can be either a function of the CBaseEntity class or the script class.
	*	@param function Function to set.
	*	@param pFunction Function to set. Can be null.
	*	@param pszName Name of the fundamental function being set.
	*/
	void SetFunction( CASRefPtr<asIScriptFunction>& function, asIScriptFunction* pFunction, const char* const pszName )
	{
		if ( !pFunction )
		{
			function = nullptr;
			return;
		}

		bool bSuccess = false;

		if ( pFunction->GetDelegateFunction() )
		{
			auto pDelegateType = pFunction->GetDelegateObjectType();

			//Note: when checking for inheritance, check if our concrete type inherits from the given type.
			//When checking if our C++ type matches, check if the given type inherits from it.
			//This prevents subclasses from being treated as incompatible
			const bool bIsScriptFunction = m_Instance.GetTypeInfo()->DerivesFrom( pDelegateType );

			//TODO: can probably cache the C++ type pointer - Solokiller
			auto pCPPType = pDelegateType->GetEngine()->GetTypeInfoByName( m_pClass->GetBaseClassData().szCPPClassName.c_str() );

			if ( bIsScriptFunction ||
				( pCPPType && pCPPType->DerivesFrom( pDelegateType ) ) )
			{
				bool bIsValid = false;

				if ( bIsScriptFunction )
				{
					bIsValid = pFunction->GetDelegateObject() == m_Instance.Get();
				}
				else
				{
					bIsValid = pFunction->GetDelegateObject() == this;
				}

				if ( bIsValid )
				{
					function = pFunction;

					bSuccess = true;
				}
			}
		}

		if ( !bSuccess )
		{
#if !defined( CLIENT_DLL )
			auto pActualFunction = pFunction->GetDelegateFunction();
			auto pActualObject = pFunction->GetDelegateObjectType();

			if ( !pActualFunction )
				pActualFunction = pFunction;

			const std::string szCompleteName = pActualObject ? 
				std::string( pActualObject->GetNamespace() ) + "::" + pActualObject->GetName() + "::" + pActualFunction->GetName() : 
				std::string( pActualFunction->GetNamespace() ) + "::" + pActualFunction->GetName();

			as::GameLog( as::LogType::LOG_WRN, "Couldn't set custom entity \"%s\" (class %s) function %s to \"%s\"\n", 
				   ThisClass::GetTargetName().ToCStr(), ThisClass::GetClassname(), pszName, szCompleteName.c_str() );
#endif
		}

		pFunction->Release();
	}

	void OnCreate() override
	{
		if ( !GetObject().GetTypeInfo() ) return;
		CALL_EXTEND_FUNC( OnCreate, "()" );
	}

	void OnDestroy() override
	{
		if ( !GetObject().GetTypeInfo() ) return;
		CALL_EXTEND_FUNC( OnDestroy, "()" );

		m_Instance.Reset();
	}

	virtual void CreateAndPopulate()
	{
		BeginPopulate( "weapon_script" );
	}

	void BeginPopulate( const char *szClassname )
	{
		if ( m_pInf ) return;
		m_pInf = new CTerrorWeaponInfo;
		WeaponInfo info;
		GetWeaponInformation( info );
		m_pInf->PopulateFromCustomWeaponInfo( "", szClassname );
		LoadWeaponInformation();
	}

	virtual void LoadWeaponInformation() {} // For client only

	void Precache() override
	{
		BaseClass::Precache();
		if ( !GetObject().GetTypeInfo() ) return;
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "void Precache()" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			method.Call( CallFlag::NONE );
		}
	}

	void Spawn() override
	{
		BaseClass::Spawn();
		if ( !GetObject().GetTypeInfo() ) return;
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "void Spawn()" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			method.Call( CallFlag::NONE );
		}
	}

	void PlayAnimation( int iActivity )
	{
		SendWeaponAnim( iActivity );
	}

	void PlaySound( int iSound )
	{
		WeaponSound( iSound );
	}

	bool IsInIronsights() override
	{
		// Doesn't exist? Make sure it does!
		if ( !m_pInf )
			CreateAndPopulate();
		return BaseClass::IsInIronsights();
	}

	void SetScriptThink( const std::string &szFunction ) override final
	{
		m_szThinkFunc = szFunction;
	}

	bool CheckMeleeEvent( CTerrorPlayer *pTerror ) override
	{
		if ( GetObject().GetTypeInfo() )
		{
			bool bRes = false;
			if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "bool CheckMeleeEvent(CTerrorPlayer@)" ) )
			{
				CASOwningContext ctx( *pFunction->GetEngine() );

				CASMethod method( *pFunction, ctx, GetObject().Get() );

				if ( method.Call( CallFlag::NONE, pTerror ) )
					method.GetReturnValue( &bRes );
			}
			return bRes;
		}
		return BaseClass::CheckMeleeEvent( pTerror );
	}

	void ItemBusyFrame( void ) override
	{
		BaseClass::ItemPostFrame();
		if ( !GetObject().GetTypeInfo() ) return;
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "void ItemBusyFrame()" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			method.Call( CallFlag::NONE );
		}
	}

	void ItemPostFrame( void ) override
	{
		BaseClass::ItemPostFrame();
		if ( !GetObject().GetTypeInfo() ) return;
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "void ItemPostFrame()" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			method.Call( CallFlag::NONE );
		}
	}

	void WeaponIdle( void ) override
	{
		if ( !GetObject().GetTypeInfo() ) return;
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "void WeaponIdle()" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			method.Call( CallFlag::NONE );
		}
		else
			BaseClass::WeaponIdle();
	}

	void Think() override
	{
		if ( !GetObject().GetTypeInfo() ) return;
#if GAME_DLL
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( UTIL_VarArgs( "void %s()", m_szThinkFunc.c_str() ) ) )
#else
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( VarArgs( "void %s()", m_szThinkFunc.c_str() ) ) )
#endif
		{
			as::Call( GetObject().Get(), pFunction );
		}
	}

	void GetWeaponInformation( WeaponInfo& info ) override
	{
		if ( !GetObject().GetTypeInfo() ) return;
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "void GetWeaponInformation(WeaponInfo& out)" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			method.Call( CallFlag::NONE, &info );
		}
#if !defined( CLIENT_DLL )
		else
			as::GameLog( as::LogType::LOG_WRN, "The weapon \"%s\" will output the wrong weapon information since GetWeaponInformation was not found!\n", GetClassname() );
#endif
	}

	void PrimaryAttack() override
	{
		// Don't do any tracers etc for clients, we only do such if we fire our bullets.
#if !defined( CLIENT_DLL )
		if ( !GetObject().GetTypeInfo() )
		{
			BaseClass::PrimaryAttack();
			return;
		}

		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "void PrimaryAttack()" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			method.Call( CallFlag::NONE );
		}
		else
			BaseClass::PrimaryAttack();
#endif
	}

	float GetFireRate() override
	{
		if ( !GetObject().GetTypeInfo() ) return 0.0f;
		float flRet = 0.0f;
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "float GetFireRate()" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			if ( method.Call( CallFlag::NONE ) )
				method.GetReturnValue( &flRet );
		}
		else
			flRet = GetTerrorWeaponData().m_flCycleTime;
		return flRet;
	}

#if !defined( ASCOMPILER )
	CSWeaponID GetWeaponID( void ) const override
	{
		return m_wepID;
	}

	CTerrorWeaponInfo const	&GetTerrorWeaponData( void ) const override
	{
		// Custom weapons have their own custom weapon info.
		return *m_pInf;
	}

	virtual void SaveWeaponInformation( WeaponInfo &info ) override
	{
		m_wepID = WEAPON_SCRIPT;
		m_pInf->PopulateFromCustomWeaponInfo( info.szPath.c_str(), info.szClassname.c_str() );
	}

	void SetWeaponID( CSWeaponID id ) { m_wepID = id; }
#endif

	void SecondaryAttack() override
	{
		if ( !GetObject().GetTypeInfo() ) return;
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "void SecondaryAttack()" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			method.Call( CallFlag::NONE );
		}
		else
			BaseClass::SecondaryAttack();
	}

	void MeleeAttack() override
	{
		if ( !GetObject().GetTypeInfo() ) return;
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "void MeleeAttack()" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			method.Call( CallFlag::NONE );
		}
		else
			BaseClass::MeleeAttack();
	}

	CTerrorPlayer* GrabOwner() override
	{
		if ( !GetObject().GetTypeInfo() ) return nullptr;
		CTerrorPlayer* pResult = nullptr;
		if ( auto pFunction = GetObject().GetTypeInfo()->GetMethodByDecl( "CTerrorPlayer@ GetPlayerOwner()" ) )
		{
			CASOwningContext ctx( *pFunction->GetEngine() );

			CASMethod method( *pFunction, ctx, GetObject().Get() );

			if ( method.Call( CallFlag::NONE ) )
				method.GetReturnValue( &pResult );
		}
		else
			pResult = BaseClass::GetPlayerOwner();
		return pResult;
	}

private:

#if !defined( ASCOMPILER )
	CSWeaponID m_wepID = WEAPON_SCRIPT;

	CTerrorWeaponInfo *m_pInf = nullptr;
#endif

	CASObjPtr m_Instance;

	const CCustomEntityClass* m_pClass = nullptr;

	std::string m_szThinkFunc = "Think";
};

/**
*	C++ Class type for custom CTerrorWeapon entities.
*/
using ScriptBase_Weapon = CBaseCustomCBaseWeapon<CTerrorWeapon>;

#endif //GAME_SHARED_ANGELSCRIPT_CUSTOMWEAPON_WEAPON_H