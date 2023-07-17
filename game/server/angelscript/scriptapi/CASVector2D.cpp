//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "CASMath.h"

#include "angelscript/HL2ASConstants.h"
#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#else
#include "angelscript/CASCManager.h"
#endif

#include "vector.h"


/**PAGE
 *
 * Commonly used utility functions
 *
 * @page Vector2D
 * @category Utilities
 *
 * @type class
 *
*/

//Vector2D
void ConstructVector2D( Vector2D* pMemory )
{
	new ( pMemory ) Vector2D ();
}

void CopyConstructVector2D( Vector2D* pMemory, const Vector2D& vec )
{
	new ( pMemory ) Vector2D ( vec );
}

void Float2ConstructVector2D( Vector2D* pMemory, float x, float y )
{
	new ( pMemory ) Vector2D ( x, y );
}

void DestructVector2D( Vector2D* pMemory )
{
	pMemory->~Vector2D();
}

static Vector2D &Vector2D_opAddAssign( Vector2D *pVecThis, const Vector2D &vecOther )
{
	*pVecThis = *pVecThis + vecOther;
	return *pVecThis;
}

static Vector2D &Vector2D_opSubAssign( Vector2D *pVecThis, const Vector2D &vecOther )
{
	*pVecThis = *pVecThis - vecOther;
	return *pVecThis;
}

static Vector2D &Vector2D_opMulAssign_Float( Vector2D *pVecThis, const float &flOther )
{
	*pVecThis = *pVecThis * flOther;
	return *pVecThis;
}

static Vector2D &Vector2D_opDivAssign_Float( Vector2D *pVecThis, const float&flOther )
{
	*pVecThis = *pVecThis / flOther;
	return *pVecThis;
}

/**MARKDOWN
**Vector2D creation**
```cpp
Vector2D vecMyVector2D = Vector2D(10.0, -10.0);
```


**Vector2D copy creation**
```cpp
Vector2D vecCopy = Vector2D(10.0, -10.0);
Vector2D vecMyVector2D = Vector2D(vecCopy);
```
 * @filename Vector2D_Construct
 */

void RegisterScriptVector2D( asIScriptEngine& engine )
{
	const char* const pszObjectName = "Vector2D";

	// Shepard - This object had "asOBJ_APP_CLASS_COPY_CONSTUCTOR" initially, the
	// reason it was removed is that it caused stack corruption on Linux but not
	// on Win32.
	engine.RegisterObjectType(
		pszObjectName, sizeof( Vector2D ),
		asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_APP_CLASS_CONSTRUCTOR | asOBJ_APP_CLASS_ALLFLOATS );

/**JSON
 * Constructs a Vector2D class.
 *
 * @type class
 * @name Vector2D
 * @desc_md true
 * @desc_file Vector2D_Construct
 *
 * @args %float% x # Sets the x vector
 * @args %float% y # Sets the y vector
 *
 * @return Returns Vector2D class which can be manipulated
 */
	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void Vector2D()",
		asFUNCTION( ConstructVector2D ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void Vector2D(const Vector2D& in other)",
		asFUNCTION( CopyConstructVector2D ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void Vector2D(float x, float y)",
		asFUNCTION( Float2ConstructVector2D ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Destroys the Vector2D
 *
 * @type void
 * @name DestructVector2D
 * @child Vector2D
 * @isfunc true
 *
 */
	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_DESTRUCT, "void DestructVector2D()",
		asFUNCTION( DestructVector2D ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Grabs the x vector
 *
 * @type float
 * @name x
 * @child Vector2D
 *
 * @return Returns the X vector
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float x",
		asOFFSET( Vector2D, x ) );

/**JSON
 * Grabs the y vector
 *
 * @type float
 * @name y
 * @child Vector2D
 *
 * @return Returns the Y vector
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float y",
		asOFFSET( Vector2D, y ) );

/**JSON
 * Grabs the length of the 2D Vector
 *
 * @type float
 * @name Length
 * @child Vector2D
 * @isfunc true
 *
 * @return Returns the length of our 2D Vector
 */
	engine.RegisterObjectMethod(
		pszObjectName, "float Length() const",
		asMETHOD( Vector2D, Length ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector2D& opAssign(const Vector2D& in other)",
		asMETHODPR( Vector2D, operator=, ( const Vector2D& ), Vector2D& ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector2D opAdd(const Vector2D& in other) const",
		asMETHODPR( Vector2D, operator+, ( const Vector2D& ) const, Vector2D ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector2D opSub(const Vector2D& in other) const",
		asMETHODPR( Vector2D, operator-, ( const Vector2D& ) const, Vector2D ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector2D opMul(float fl) const",
		asMETHODPR( Vector2D, operator*, ( float ) const, Vector2D ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector2D opMul_r(float fl) const",
		asMETHODPR( Vector2D, operator*, ( float ) const, Vector2D ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector2D opDiv(float fl) const",
		asMETHODPR( Vector2D, operator/, ( float ) const, Vector2D ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector2D opDiv_r(float fl) const",
		asMETHODPR( Vector2D, operator/, ( float ) const, Vector2D ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector2D& opAddAssign(const Vector2D&in vecOther) const",
		asFUNCTION( Vector2D_opAddAssign ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector2D& opSubAssign(const Vector2D&in vecOther) const",
		asFUNCTION( Vector2D_opSubAssign ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector2D& opMulAssign(const float&in flOther) const",
		asFUNCTION( Vector2D_opMulAssign_Float ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector2D& opDivAssign(const float&in flOther) const",
		asFUNCTION( Vector2D_opDivAssign_Float ), asCALL_CDECL_OBJFIRST );
}
