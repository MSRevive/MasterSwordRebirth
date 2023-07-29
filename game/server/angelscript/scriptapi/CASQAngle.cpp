//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "CASQAngle.h"

#include "angelscript/HL2ASConstants.h"
#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#else
#include "angelscript/CASCManager.h"
#endif

#include "qangle.h"

/**PAGE
 *
 * Commonly used utility functions
 *
 * @page QAngle
 * @category Utilities
 *
 * @type class
 *
*/

//QAngle
void ConstructQAngle( QAngle* pMemory )
{
	new ( pMemory ) QAngle ();
}

void CopyConstructQAngle( void* pMemory, const QAngle& vec )
{
	new ( pMemory ) QAngle ( vec );
}

void Float3ConstructQAngle( QAngle* pMemory, float x, float y, float z )
{
	new ( pMemory ) QAngle ( x, y, z );
}

void DestructQAngle( QAngle* pMemory )
{
	pMemory->~QAngle();
}

static QAngle &QAngle_opAddAssign( QAngle *pVecThis, const QAngle &vecOther )
{
	*pVecThis = *pVecThis + vecOther;
	return *pVecThis;
}

static QAngle &QAngle_opSubAssign( QAngle *pVecThis, const QAngle &vecOther )
{
	*pVecThis = *pVecThis - vecOther;
	return *pVecThis;
}

static QAngle &QAngle_opMulAssign( QAngle *pVecThis, const QAngle &vecOther )
{
	*pVecThis = *pVecThis * vecOther;
	return *pVecThis;
}

static QAngle &QAngle_opDivAssign( QAngle *pVecThis, const QAngle &vecOther )
{
	*pVecThis = *pVecThis / vecOther;
	return *pVecThis;
}

static QAngle &QAngle_opMulAssign_Float( QAngle *pVecThis, const float &flOther )
{
	*pVecThis = *pVecThis * flOther;
	return *pVecThis;
}

static QAngle &QAngle_opDivAssign_Float( QAngle *pVecThis, const float &flOther )
{
	*pVecThis = *pVecThis / flOther;
	return *pVecThis;
}

static bool QAngle_Equals( const QAngle& vec, QAngle* pThis )
{
	return ( *pThis == vec ) != 0;
}

/**MARKDOWN
**QAngle creation**
```cpp
QAngle angleMyAngle = QAngle(0, 180, -32);
```


**QAngle copy creation**
```cpp
QAngle angleCopy = QAngle(0, 180, -32);
QAngle angleMyAngle = QAngle(angleCopy);
```
 * @filename QAngle_Construct
 */

void RegisterScriptQAngle( asIScriptEngine& engine )
{
	const char* const pszObjectName = "QAngle";

	// Shepard - This object had "asOBJ_APP_CLASS_COPY_CONSTUCTOR" initially, the
	// reason it was removed is that it caused stack corruption on Linux but not
	// on Win32.
	engine.RegisterObjectType(
		pszObjectName, sizeof( QAngle ),
		asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_APP_CLASS_CONSTRUCTOR | asOBJ_APP_CLASS_ALLFLOATS );

/**JSON
 * Constructs a QAngle class.
 *
 * @type class
 * @name QAngle
 * @desc_md true
 * @desc_file QAngle_Construct
 *
 * @args %float% x # Sets the x angle
 * @args %float% y # Sets the y angle
 * @args %float% z # Sets the z angle
 *
 * @return Returns QAngle class which can be manipulated
 */
	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void QAngle()",
		asFUNCTION( ConstructQAngle ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void QAngle(const QAngle& in vec)",
		asFUNCTION( CopyConstructQAngle ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void QAngle(float x, float y, float z)",
		asFUNCTION( Float3ConstructQAngle ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Destroys the QAngle
 *
 * @type void
 * @name DestructQAngle
 * @child QAngle
 * @isfunc true
 *
 */
	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_DESTRUCT, "void DestructQAngle()",
		asFUNCTION( DestructQAngle ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Grabs the x angle
 *
 * @type float
 * @name x
 * @child QAngle
 *
 * @return Returns the X angle
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float x",
		asOFFSET( QAngle, x ) );

/**JSON
 * Grabs the y angle
 *
 * @type float
 * @name y
 * @child QAngle
 *
 * @return Returns the Y angle
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float y",
		asOFFSET( QAngle, y ) );

/**JSON
 * Grabs the z angle
 *
 * @type float
 * @name z
 * @child QAngle
 *
 * @return Returns the Z angle
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float z",
		asOFFSET( QAngle, z ) );

/**JSON
 * Checks if the angles are the same
 *
 * @type bool
 * @name opEquals
 * @child QAngle
 *
 * @args %QAngle_in% other # The Vector we want to check
 *
 * @return Returns true if both QAngles are the same
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool opEquals(const QAngle& in other) const",
		asFUNCTION( QAngle_Equals ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Grabs the length of the QAngle
 *
 * @type float
 * @name Length
 * @child QAngle
 * @isfunc true
 *
 * @return Returns the length of our QAngle
 */
	engine.RegisterObjectMethod(
		pszObjectName, "float Length() const",
		asMETHOD( QAngle, Length ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle& opAssign(const QAngle& in other)",
		asMETHODPR( QAngle, operator=, ( const QAngle& ), QAngle& ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle opNeg() const",
		asMETHODPR( QAngle, operator-, ( ) const, QAngle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle opAdd(const QAngle& in other) const",
		asMETHODPR( QAngle, operator+, ( const QAngle& ) const, QAngle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle opSub(const QAngle& in other) const",
		asMETHODPR( QAngle, operator-, ( const QAngle& ) const, QAngle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle opMul(float fl) const",
		asMETHODPR( QAngle, operator*, ( float ) const, QAngle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle opMul_r(float fl) const",
		asMETHODPR( QAngle, operator*, ( float ) const, QAngle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle opDiv(float fl) const",
		asMETHODPR( QAngle, operator/, ( float ) const, QAngle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle opDiv_r(float fl) const",
		asMETHODPR( QAngle, operator/, ( float ) const, QAngle ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle& opAddAssign(const QAngle&in vecOther) const",
		asFUNCTION( QAngle_opAddAssign ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle& opSubAssign(const QAngle&in vecOther) const",
		asFUNCTION( QAngle_opSubAssign ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle& opMulAssign(const QAngle&in vecOther) const",
		asFUNCTION( QAngle_opMulAssign ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle& opDivAssign(const QAngle&in vecOther) const",
		asFUNCTION( QAngle_opDivAssign ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle& opMulAssign(const float&in flOther) const",
		asFUNCTION( QAngle_opMulAssign_Float ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "QAngle& opDivAssign(const float&in flOther) const",
		asFUNCTION( QAngle_opDivAssign_Float ), asCALL_CDECL_OBJFIRST );
}

