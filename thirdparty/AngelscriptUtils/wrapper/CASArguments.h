#ifndef ANGELSCRIPT_CASARGUMENTS_H
#define ANGELSCRIPT_CASARGUMENTS_H

#include <vector>

#include "AngelscriptUtils/util/CASBaseClass.h"

class asIScriptEngine;

/**
*	@defgroup ASArguments Angelscript Arguments Utils
*	Everything related to handling arguments is in here.
*
*	@{
*/

namespace ArgType
{
/**
*	Possible types that an argument can be.
*	TODO: shouldn't be a bit field.
*	TODO: shouldn't this be trivial to determine using helper functions?
*	TODO: move to ASUtil.h
*	TODO: probably obsolete due to ITypeInfo's existence
*/
enum ArgType
{
	/**
	*	No type set.
	*/
	NONE		= 1 << 0,

	/**
	*	No return type.
	*/
	AS_VOID		= 1 << 1,
	PRIMITIVE	= 1 << 2,
	VALUE		= 1 << 3,
	REF			= 1 << 4,

	/**
	*	It's an enum, treat as dword.
	*/
	ENUM		= 1 << 5,
};
}

/**
*	Can store the value of an argument.
*/
struct ArgumentValue final
{
	union
	{
		asBYTE		byte;
		asWORD		word;
		asDWORD		dword;
		asQWORD		qword;

		float		flValue;
		double		dValue;

		void*		pValue;
	};

	ArgumentValue()
		: qword()
	{
	}

};

/**
*	Represents a single argument.
*/
class CASArgument final
{
public:

	/**
	*	Default constructor.
	*/
	CASArgument() = default;

	/**
	*	Destructor.
	*/
	~CASArgument();

	/**
	*	Copy constructor.
	*/
	CASArgument( const CASArgument& other );

	/**
	*	Assignment operator.
	*/
	CASArgument& operator=( const CASArgument& other );

	/**
	*	@return The type id.
	*/
	inline int GetTypeId() const { return m_iTypeId; }

	/**
	*	@return The argument type.
	*/
	inline ArgType::ArgType GetArgumentType() const { return m_ArgType; }

	/**
	*	@return The argument value.
	*/
	inline const ArgumentValue& GetArgumentValue() const { return m_Value; }

	/**
	*	@copydoc GetArgumentValue() const
	*/
	inline ArgumentValue& GetArgumentValue() { return m_Value; }

	/**
	*	@return Whether this argument has a value.
	*/
	bool HasValue() const { return !( m_ArgType & ( ArgType::AS_VOID | ArgType::NONE ) ); }

	/**
	*	Gets the address of the argument.
	*	For read access only!
	*	@return The address of the argument.
	*/
	void* GetArgumentAsPointer() const;

	/**
	*	Sets the argument to the given value and type.
	*	@param engine Engine to use.
	*	@param iTypeId Type Id.
	*	@param type Argument type.
	*	@param value Value to assign.
	*	@param bCopy Whether to copy the value, or point to the same instance.
	*	@return true on success, false otherwise.
	*/
	bool Set( asIScriptEngine& engine, const int iTypeId, const ArgType::ArgType type, const ArgumentValue& value, const bool bCopy = false );

	/**
	*	Same as the other version, but will retrieve the engine from the active manager instead.
	*	@see Set( asIScriptEngine& engine, const int iTypeId, const ArgType::ArgType type, const ArgumentValue& value, const bool bCopy = false )
	*/
	bool Set( const int iTypeId, const ArgType::ArgType type, const ArgumentValue& value, const bool bCopy = false );

	/**
	*	Sets the argument to that of the given argument. The value is copy constructed, or in the case of ref types, a reference is added.
	*	@param other Argument to copy.
	*	@return true on success, false otherwise.
	*/
	bool Set( const CASArgument& other );

	/**
	*	Resets the argument. It is left with no value.
	*/
	void Reset();

private:

	//Object type id
	int m_iTypeId = -1;

	//Used to prevent looking up the type again if it involves expensive lookup
	ArgType::ArgType m_ArgType = ArgType::NONE;

	//The actual value.
	ArgumentValue m_Value = ArgumentValue();
};

/**
*	This class can store a variable number of arguments.
*/
class CASArguments final : public CASRefCountedBaseClass
{
public:
	typedef std::vector<CASArgument> Arguments_t;

public:

	/**
	*	Default constructor. Creates an empty argument list.
	*/
	CASArguments() = default;

	/**
	*	Constructor. Creates a list of arguments based on the given generic call instance.
	*	@param arguments Generic call instance.
	*	@param uiStartIndex The index of the first argument to use.
	*	@see SetArguments( asIScriptGeneric& arguments, size_t uiStartIndex )
	*/
	CASArguments( asIScriptGeneric& arguments, size_t uiStartIndex = 0 );

	/**
	*	Constructor. Creates a list of arguments based on the given function, and the given varargs pointer.
	*	@param targetFunc Function whose arguments will be used for type info.
	*	@param list Pointer to the arguments to use.
	*	@see SetArguments( asIScriptFunction& targetFunc, va_list list )
	*/
	CASArguments( asIScriptFunction& targetFunc, va_list list );

	/**
	*	Destructor.
	*/
	~CASArguments();

	void Release() const;

	/**
	*	Copy constructor.
	*/
	CASArguments( const CASArguments& other );

	/**
	*	Assignment operator.
	*/
	CASArguments& operator=( const CASArguments& other );

	/**
	*	Copies the arguments from the given arguments object to this one.
	*/
	void Assign( const CASArguments& other );

	/**
	*	Clears the list of arguments.
	*/
	void Clear();

	/**
	*	@return The list of arguments.
	*/
	const Arguments_t& GetArgumentList() const { return m_Arguments; }

	/**
	*	@return The number of arguments.
	*/
	size_t GetArgumentCount() const { return m_Arguments.size(); }

	/**
	*	Gets the argument at the given index.
	*	@return The requested argument, or null if the index is invalid.
	*/
	const CASArgument* GetArgument( const size_t uiIndex ) const;

	/**
	*	@return Whether there are any arguments in this object.
	*/
	bool HasArguments() const { return !m_Arguments.empty(); }

	/**
	*	Sets the list of arguments to that of the given generic call instance.
	*	@param arguments Generic call instance.
	*	@param uiStartIndex The index of the first argument to use.
	*	@return true on success, false otherwise.
	*/
	bool SetArguments( asIScriptGeneric& arguments, size_t uiStartIndex = 0 );

	/**
	*	Sets the list of arguments to that of the given function, and the given varargs pointer.
	*	@param targetFunc Function whose arguments will be used for type info.
	*	@param list Pointer to the arguments to use.
	*	@return true on success, false otherwise.
	*/
	bool SetArguments( asIScriptFunction& targetFunc, va_list list );

private:
	Arguments_t m_Arguments;
};

/** @} */

#endif //ANGELSCRIPT_CASARGUMENTS_H
