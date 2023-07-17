#ifndef GAME_SHARED_ANGELSCRIPT_ICUSTOMENT_H
#define GAME_SHARED_ANGELSCRIPT_ICUSTOMENT_H

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <AngelscriptUtils/util/CASRefPtr.h>
#include <AngelscriptUtils/util/CASObjPtr.h>

using CustomEntCreateFn = void( *)( entvars_t* );

// Ported from Source Engine
struct BaseClassData_t final
{
	BaseClassData_t( std::string&& szClassName, std::string&& szCPPClassName, std::string&& szBaseClassName, std::string&& szClassDeclaration )
		: szClassName( std::move( szClassName ) )
		, szCPPClassName( std::move( szCPPClassName ) )
		, szBaseClassName( std::move( szBaseClassName ) )
		, szClassDeclaration( std::move( szClassDeclaration ) )
	{
	}

	const std::string szClassName;
	const std::string szCPPClassName;
	const std::string szBaseClassName;
	const std::string szClassDeclaration;
};

enum CustomEntityTypes_t
{
	k_eCustomWeapon = 0,				// << Create custom weapons
	k_eCustomEntity,					// << Clients will be able to interact and see this entity
};

/**
*	Represents a custom entity class.
*/
class CCustomEntityClass final
{
public:
	/**
	*	@param szMapName Name of this entity. This is the name that the fgd refers to.
	*	@param typeInfo Type info that represents the class.
	*	@param baseClassData Base class data that represents this custom entity's base class.
	*/
	CCustomEntityClass( std::string&& szMapName, CASRefPtr<asITypeInfo>&& typeInfo, const BaseClassData_t& baseClassData, CustomEntityTypes_t eClass )
		: m_szMapName( std::move( szMapName ) )
		, m_TypeInfo( std::move( typeInfo ) )
		, m_BaseClassData( baseClassData )
		, m_eClass( eClass )
	{
	}

	const std::string& GetMapName() const { return m_szMapName; }

	const CASRefPtr<asITypeInfo>& GetTypeInfo() const { return m_TypeInfo; }

	const BaseClassData_t& GetBaseClassData() const { return m_BaseClassData; }

	CustomEntityTypes_t GetClass() const { return m_eClass; }

private:
	CustomEntityTypes_t m_eClass;
	std::string m_szMapName;
	CASRefPtr<asITypeInfo> m_TypeInfo;

	const BaseClassData_t& m_BaseClassData;
};

/**
*	Interface for custom entity classes.
*/
class IASCustomEntity
{
public:
	virtual ~IASCustomEntity() = 0;

	/**
	*	@return The object pointer for this custom entity's script object.
	*/
	virtual CASObjPtr& GetObject() const = 0;

	/**
	*	Sets this entity's script object.
	*/
	virtual void SetObject( CASObjPtr&& instance ) = 0;

	/**
	*	Sets the class object.
	*/
	virtual void SetClass( const CCustomEntityClass* pClass ) = 0;

	/**
	*	Sets this entity's Think function.
	*/
	virtual void SetScriptThink( const std::string &szFunction ) = 0;

	/*
	*	Called when the script instance has fully initialized.
	*	Use this to communicate with the script instance, and decide if you're in a proper state.
	*	Return false if you want to be destroyed.
	*/
	virtual bool OnScriptClassReady();
};

inline IASCustomEntity::~IASCustomEntity()
{
}

inline bool IASCustomEntity::OnScriptClassReady()
{
	return true;
}

#endif