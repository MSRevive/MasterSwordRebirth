//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "CASVector.h"

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
 * @page Vector
 * @category Utilities
 *
 * @type class
 *
*/

//Vector
void ConstructVector3D( Vector* pMemory )
{
	// Doesn't work, since it breaks when used on other functions (returns 0, 0, 0)
	/*
	// Solokiller to Shepard on TWHL Discord when adding "opAddAssign" and co.
	//new ( pMemory ) Vector ();
	// that's placement new, not regular new
	// Angelscript provides a memory location, you then construct the object in place
	// it doesn't allocate memory
	// you could do this:
	byte byMemory[sizeof Vector];
	pMemory = new ( byMemory ) Vector();
	// you'd have to disregard memory alignment of course, but it's just an example
	// https://en.cppreference.com/w/cpp/language/new#Placement_new
	*/
	new ( pMemory ) Vector ();
}

void CopyConstructVector3D( void* pMemory, const Vector& vec )
{
	new ( pMemory ) Vector( vec );
}

void Float3ConstructVector3D( Vector* pMemory, float x, float y, float z )
{
	new ( pMemory ) Vector ( x, y, z );
}

void DestructVector3D( Vector* pMemory )
{
	pMemory->~Vector();
}

static bool Vector_Equals( const Vector& vec, Vector* pThis )
{
	return ( *pThis == vec ) != 0;
}

static Vector &Vector_opAddAssign( Vector *pVecThis, const Vector &vecOther )
{
	*pVecThis = *pVecThis + vecOther;
	return *pVecThis;
}

static Vector &Vector_opSubAssign( Vector *pVecThis, const Vector &vecOther )
{
	*pVecThis = *pVecThis - vecOther;
	return *pVecThis;
}

static Vector &Vector_opMulAssign_Float( Vector *pVecThis, const float &flOther )
{
	*pVecThis = *pVecThis * flOther;
	return *pVecThis;
}

static Vector &Vector_opDivAssign_Float( Vector *pVecThis, const float &flOther )
{
	*pVecThis = *pVecThis / flOther;
	return *pVecThis;
}

/**MARKDOWN
**Vector creation**
```cpp
Vector vecMyVector = Vector(10.0, -10.0);
```


**Vector copy creation**
```cpp
Vector vecCopy = Vector(0, 180, -32);
Vector vecMyVector = Vector(vecCopy);
```
 * @filename Vector_Construct
 */

void RegisterScriptVector( asIScriptEngine& engine )
{
	const char* const pszObjectName = "Vector";

	// Shepard - This object had "asOBJ_APP_CLASS_COPY_CONSTUCTOR" initially, the
	// reason it was removed is that it caused stack corruption on Linux but not
	// on Win32.
	engine.RegisterObjectType(
		pszObjectName, sizeof( Vector ),
		asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_APP_CLASS_CONSTRUCTOR | asOBJ_APP_CLASS_ALLFLOATS );

/**JSON
 * Constructs a Vector class.
 *
 * @type class
 * @name Vector
 * @desc_md true
 * @desc_file Vector_Construct
 *
 * @args %float% x # Sets the x vector
 * @args %float% y # Sets the y vector
 * @args %float% z # Sets the z vector
 *
 * @return Returns Vector class which can be manipulated
 */
	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void Vector()",
		asFUNCTION( ConstructVector3D ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void Vector(const Vector& in vec)",
		asFUNCTION( CopyConstructVector3D ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void Vector(float x, float y, float z)",
		asFUNCTION( Float3ConstructVector3D ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Destroys the Vector
 *
 * @type void
 * @name DestructVector
 * @child Vector
 * @isfunc true
 *
 */
	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_DESTRUCT, "void DestructVector()",
		asFUNCTION( DestructVector3D ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Grabs the x vector
 *
 * @type float
 * @name x
 * @child Vector
 *
 * @return Returns the X vector
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float x",
		asOFFSET( Vector, x ) );

/**JSON
 * Grabs the y vector
 *
 * @type float
 * @name y
 * @child Vector
 *
 * @return Returns the Y vector
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float y",
		asOFFSET( Vector, y ) );

/**JSON
 * Grabs the z vector
 *
 * @type float
 * @name z
 * @child Vector
 *
 * @return Returns the Z vector
 */
	engine.RegisterObjectProperty(
		pszObjectName, "float z",
		asOFFSET( Vector, z ) );

/**JSON
 * Checks if the vectors are the same
 *
 * @type bool
 * @name opEquals
 * @child Vector
 *
 * @args %Vector_in% other # The Vector we want to check
 *
 * @return Returns true if both Vectors are the same
 */
	engine.RegisterObjectMethod(
		pszObjectName, "bool opEquals(const Vector& in other) const",
		asFUNCTION( Vector_Equals ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Grabs the length of the Vector
 *
 * @type float
 * @name Length
 * @child Vector
 * @isfunc true
 *
 * @return Returns the length of our Vector
 */
	engine.RegisterObjectMethod(
		pszObjectName, "float Length() const",
		asMETHOD( Vector, Length ), asCALL_THISCALL );

/**JSON
 * Grabs the length of the Vector (Same as Vector2D)
 *
 * @type float
 * @name Length2D
 * @child Vector
 * @isfunc true
 *
 * @return Returns the length of our Vector in 2D
 */
	engine.RegisterObjectMethod(
		pszObjectName, "float Length2D() const",
		asMETHOD( Vector, Length2D ), asCALL_THISCALL );

/**JSON
 * Resets the Vector back to 0,0,0
 *
 * @type void
 * @name Zero
 * @child Vector
 * @isfunc true
 *
 */
	engine.RegisterObjectMethod(
		pszObjectName, "void Zero()",
		asMETHOD( Vector, Zero ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector& opAssign(const Vector& in other)",
		asMETHODPR( Vector, operator=, ( const Vector& ), Vector& ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector opNeg() const",
		asMETHODPR( Vector, operator-, ( ) const, Vector ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector opAdd(const Vector& in other) const",
		asMETHODPR( Vector, operator+, ( const Vector& ) const, Vector ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector opSub(const Vector& in other) const",
		asMETHODPR( Vector, operator-, ( const Vector& ) const, Vector ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector opMul(float fl) const",
		asMETHODPR( Vector, operator*, ( float ) const, Vector ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector opMul_r(float fl) const",
		asMETHODPR( Vector, operator*, ( float ) const, Vector ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector opDiv(float fl) const",
		asMETHODPR( Vector, operator/, ( float ) const, Vector ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector opDiv_r(float fl) const",
		asMETHODPR( Vector, operator/, ( float ) const, Vector ), asCALL_THISCALL );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector& opAddAssign(const Vector&in vecOther) const",
		asFUNCTION( Vector_opAddAssign ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector& opSubAssign(const Vector&in vecOther) const",
		asFUNCTION( Vector_opSubAssign ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector& opMulAssign(const float&in flOther) const",
		asFUNCTION( Vector_opMulAssign_Float ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "Vector& opDivAssign(const float&in flOther) const",
		asFUNCTION( Vector_opDivAssign_Float ), asCALL_CDECL_OBJFIRST );
}

