//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#ifndef GAME_SHARED_ZPS_ANGELSCRIPT_CASCLASSWRITER_H
#define GAME_SHARED_ZPS_ANGELSCRIPT_CASCLASSWRITER_H

#include <sstream>
#include <string>

#undef GetClassName

/**
*	Can write class declarations for Angelscript classes.
*/
class CASClassWriter final
{
public:
	/**
	*	Visibility level for member variables, methods and property accessors.
	*/
	enum class Visibility
	{
		PRIVATE,
		PROTECTED,
		PUBLIC
	};

	/**
	*	Returns the string representation of a visibility token.
	*/
	static const char* VisibilityToString( const Visibility visibility )
	{
		switch ( visibility )
		{
		case Visibility::PRIVATE:	return "private";
		case Visibility::PROTECTED:	return "protected";
		case Visibility::PUBLIC:	return "";
		}

		return "";
	}

public:
	/**
	*	@param szClassName Name of the class to declare.
	*/
	CASClassWriter( std::string&& szClassName )
		: m_szClassName( std::move( szClassName ) )
	{
	}

	/**
	*	@return Name of the class that is being declared.
	*/
	const std::string& GetClassName() const
	{
		return m_szClassName;
	}

	/**
	*	Adds a tab level.
	*/
	void AddTabLevel()
	{
		++m_uiTabs;
	}

	/**
	*	Removes a tab level.
	*/
	void RemoveTabLevel()
	{
		if ( m_uiTabs > 0 )
			--m_uiTabs;
	}

	/**
	*	Writes the given number of tabs.
	*/
	void WriteTabs( const unsigned int uiTabs )
	{
		for ( unsigned int uiTab = 0; uiTab < uiTabs; ++uiTab )
			m_Stream << '\t';
	}

	/**
	*	Writes the current number of tabs.
	*/
	void WriteTabs()
	{
		WriteTabs( m_uiTabs );
	}

	/**
	*	Writes a newline.
	*/
	void NewLine()
	{
		m_Stream << '\n';
	}

	/**
	*	Starts a new scope and adds an indent level.
	*/
	void StartBracket()
	{
		WriteTabs();

		m_Stream << '{';

		NewLine();

		AddTabLevel();
	}

	/**
	*	Ends a scope and removes an indent level.
	*/
	void EndBracket()
	{
		RemoveTabLevel();

		WriteTabs();

		m_Stream << '}';

		NewLine();
	}

	/**
	*	Starts a class declaration. Also starts a new scope.
	*	@param bAbstract Whether this is an abstract class
	*	@param pszInterfaces Optional. Interfaces to inherit from.
	*/
	void StartClassDeclaration( bool bAbstract = false, const char* pszInterfaces = nullptr )
	{
		WriteTabs();

		if ( bAbstract )
			m_Stream << "abstract ";

		m_Stream << "class " << m_szClassName;

		if ( pszInterfaces )
			m_Stream << " : " << pszInterfaces;

		NewLine();

		StartBracket();
	}

	/**
	*	Ends a class declaration.
	*/
	void EndClassDeclaration()
	{
		EndBracket();
	}

	/**
	*	Writes a property declaration.
	*	@param visibility Visibility level of the property.
	*	@param pszType Property type.
	*	@param pszName Property name.
	*	@param pszDefault Optional. The default value for this property.
	*/
	void WriteProperty( const Visibility visibility, const char* const pszType, const char* const pszName, const char* const pszDefault = nullptr )
	{
		WriteTabs();

		const auto pszVisibility = VisibilityToString( visibility );

		if ( *pszVisibility )
			m_Stream << pszVisibility << ' ';

		m_Stream << pszType << ' ' << pszName;

		if ( pszDefault )
			m_Stream << " = " << pszDefault;

		m_Stream << ';';

		NewLine();
	}

	/**
	*	Writes a method header.
	*	@param visibility The visibility of the method.
	*	@param pszReturnType Method return type.
	*	@param pszName Method name.
	*	@param pszParameters Method parameters.
	*	@param bIsConst Whether the method is const.
	*	@param bIsFinal Whether the method is final.
	*/
	void WriteMethodHeader( const Visibility visibility, 
							const char* const pszReturnType, const char* const pszName, const char* const pszParameters, 
							const bool bIsConst, const bool bIsFinal = false )
	{
		WriteTabs();

		const auto pszVisibility = VisibilityToString( visibility );

		if ( *pszVisibility )
			m_Stream << pszVisibility << ' ';

		m_Stream << pszReturnType << ' ' << pszName << '(' << pszParameters << ')';

		if ( bIsConst )
			m_Stream << " const";

		if ( bIsFinal )
			m_Stream << " final";

		NewLine();
	}

	/**
	*	Starts a property declaration. Starts a new scope.
	*	@param visibility Visibility of the property.
	*	@param pszType Property type.
	*	@param pszName Property name.
	*/
	void StartPropDeclaration( const Visibility visibility, const char* const pszType, const char* const pszName )
	{
		WriteTabs();

		const auto pszVisibility = VisibilityToString( visibility );

		if ( *pszVisibility )
			m_Stream << pszVisibility << ' ';

		m_Stream << pszType << ' ' << pszName;

		NewLine();

		StartBracket();
	}

	/**
	*	Ends a property declaration.
	*/
	void EndPropDeclaration()
	{
		EndBracket();
	}

	/**
	*	Writes a property getter.
	*	@param pszContents Getter contents.
	*	@param bIsConst Whether the getter is const.
	*/
	void WritePropGetter( const char* const pszContents, const bool bIsConst )
	{
		WriteTabs();

		m_Stream << "get";

		if ( bIsConst )
			m_Stream << " const";

		NewLine();

		StartBracket();

		Write( pszContents );

		EndBracket();
	}

	/**
	*	Writes a property setter.
	*	@param pszContents Setter contents.
	*/
	void WritePropSetter( const char* const pszContents )
	{
		WriteTabs();

		m_Stream << "set";

		NewLine();

		StartBracket();

		Write( pszContents );

		EndBracket();
	}

	/**
	*	Writes a constructor header.
	*	@param pszParameters Constructor parameters.
	*/
	void WriteConstructorHeader( const char* const pszParameters = "" )
	{
		WriteTabs();

		m_Stream << m_szClassName << '(' << pszParameters << ')';

		NewLine();
	}

	/**
	*	Writes a number of lines of code, intended to the current tab level.
	*/
	void Write( const char* const pszString )
	{
		std::stringstream stream( pszString );

		std::string szLine;

		while ( std::getline( stream, szLine ) )
		{
			WriteTabs();
			m_Stream << szLine;
			NewLine();
		}
	}

	/**
	*	@return The string representation of the declaration.
	*/
	std::string ToString() const
	{
		return m_Stream.str();
	}

private:
	const std::string m_szClassName;
	std::stringstream m_Stream;

	unsigned int m_uiTabs = 0;

public:
	CASClassWriter( const CASClassWriter& ) = delete;
	CASClassWriter& operator=( const CASClassWriter& ) = delete;
};

#endif // GAME_SHARED_ZPS_ANGELSCRIPT_CASCLASSWRITER_H
