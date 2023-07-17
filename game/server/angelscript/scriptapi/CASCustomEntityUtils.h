#ifndef GAME_SERVER_ANGELSCRIPT_ASCUSTOMENTITIESUTILS_H
#define GAME_SERVER_ANGELSCRIPT_ASCUSTOMENTITIESUTILS_H

// Include our custom ent
#include "CASCustomEntity.h"
#include "angelscript/CASClassWriter.h"

/**
*	Helper class to easily create new base classes for Angelscript custom entities.
*/
class CASBaseClassCreator final
{
public:
	/**
	*	Generates the common base class contents.
	*	@param writer Writer to use to write the class.
	*	@param pszEntityClass Name of the entity class to use for this base class.
	*	@param pszBaseClass Name of the base class to use for base class calls.
	*/
	static void GenerateCommonBaseClassContents( CASClassWriter& writer, const char* const pszEntityClass, const char* const pszBaseClass );

	/**
	*	@param pszClassName Name of the class to declare.
	*	@param pszEntityClass Name of the entity class to use for this base class.
	*	@param pszBaseClass Name of the base class to use for base class calls.
	*	@param createFn Function that creates the C++ class for this entity.
	*	@param baseClassList List to append the base class data to.
	*/
	CASBaseClassCreator( const char* const pszClassName,
						 const char* const pszEntityClass, const char* const pszBaseClass,
						 CASCustomEntities::BaseClassList_t& baseClassList )
		: m_Writer( pszClassName )
		, m_pszEntityClass( pszEntityClass )
		, m_pszBaseClass( pszBaseClass )
		, m_BaseClassList( baseClassList )
	{
		GenerateCommonBaseClassContents( m_Writer, m_pszEntityClass, m_pszBaseClass );
	}

	~CASBaseClassCreator()
	{
		m_Writer.EndClassDeclaration();

		m_BaseClassList.emplace_back( std::string( m_Writer.GetClassName() ), m_pszEntityClass, m_pszBaseClass, m_Writer.ToString() );
	}

	CASClassWriter& GetWriter() { return m_Writer; }

	const char* GetEntityClass() const { return m_pszEntityClass; }

	const char* GetBaseClass() const { return m_pszBaseClass; }

private:
	CASClassWriter m_Writer;

	const char* const m_pszEntityClass;
	const char* const m_pszBaseClass;

	CASCustomEntities::BaseClassList_t& m_BaseClassList;

private:
	CASBaseClassCreator( const CASBaseClassCreator& ) = delete;
	CASBaseClassCreator& operator=( const CASBaseClassCreator& ) = delete;
};

#endif //GAME_SERVER_ANGELSCRIPT_ASCUSTOMENTITIESUTILS_H