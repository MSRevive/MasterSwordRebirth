//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "mathlib.h"

#include "angelscript/add_on/random.h"

#include "CASMath.h"
#include "CASQAngle.h"
#include "CASVector.h"
#include "CASVector2D.h"

#include "angelscript/HL2ASConstants.h"
#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#else
#include "angelscript/CASCManager.h"
#endif

/**PAGE
 *
 * Used to make simple to complex math related equations.
 *
 * @page Math
 * @category Utilities
 *
 * @type namespace
 *
*/

//Angelscript expects actual variables to back up exposed variables.
//Using different names to avoid the preprocessor replacing them with constants.
static const int8_t CONST_INT8_MIN		= INT8_MIN;
static const int16_t CONST_INT16_MIN	= INT16_MIN;
static const int32_t CONST_INT32_MIN	= INT32_MIN;
static const int64_t CONST_INT64_MIN	= INT64_MIN;

static const int8_t CONST_INT8_MAX		= INT8_MAX;
static const int16_t CONST_INT16_MAX	= INT16_MAX;
static const int32_t CONST_INT32_MAX	= INT32_MAX;
static const int64_t CONST_INT64_MAX	= INT64_MAX;

static const uint8_t CONST_UINT8_MAX	= UINT8_MAX;
static const uint16_t CONST_UINT16_MAX	= UINT16_MAX;
static const uint32_t CONST_UINT32_MAX	= UINT32_MAX;
static const uint64_t CONST_UINT64_MAX	= UINT64_MAX;

static const float CONST_FLOAT_MIN		= FLT_MIN;
static const double CONST_DOUBLE_MIN	= DBL_MIN;

static const float CONST_FLOAT_MAX		= FLT_MAX;
static const double CONST_DOUBLE_MAX	= DBL_MAX;

static const double CONST_PI			= M_PI;

static void RegisterScriptMathConstants( asIScriptEngine& engine )
{
	const std::string szOldNS = engine.GetDefaultNamespace();

	engine.SetDefaultNamespace( "Math" );

	engine.RegisterGlobalProperty(
		"const int8 INT8_MIN",
		const_cast<int8_t*>( &CONST_INT8_MIN ) );

	engine.RegisterGlobalProperty(
		"const int16 INT16_MIN",
		const_cast<int16_t*>( &CONST_INT16_MIN ) );

	engine.RegisterGlobalProperty(
		"const int32 INT32_MIN",
		const_cast<int32_t*>( &CONST_INT32_MIN ) );

	engine.RegisterGlobalProperty(
		"const int64 INT64_MIN",
		const_cast<int64_t*>( &CONST_INT64_MIN ) );

	engine.RegisterGlobalProperty(
		"const int8 INT8_MAX",
		const_cast<int8_t*>( &CONST_INT8_MAX ) );

	engine.RegisterGlobalProperty(
		"const int16 INT16_MAX",
		const_cast<int16_t*>( &CONST_INT16_MAX ) );

	engine.RegisterGlobalProperty(
		"const int32 INT32_MAX",
		const_cast<int32_t*>( &CONST_INT32_MAX ) );

	engine.RegisterGlobalProperty(
		"const int64 INT64_MAX",
		const_cast<int64_t*>( &CONST_INT64_MAX ) );

	engine.RegisterGlobalProperty(
		"const uint8 UINT8_MAX",
		const_cast<uint8_t*>( &CONST_UINT8_MAX ) );

	engine.RegisterGlobalProperty(
		"const uint16 UINT16_MAX",
		const_cast<uint16_t*>( &CONST_UINT16_MAX ) );

	engine.RegisterGlobalProperty(
		"const uint32 UINT32_MAX",
		const_cast<uint32_t*>( &CONST_UINT32_MAX ) );

	engine.RegisterGlobalProperty(
		"const uint64 UINT64_MAX",
		const_cast<uint64_t*>( &CONST_UINT64_MAX ) );

	engine.RegisterGlobalProperty(
		"const float FLOAT_MIN",
		const_cast<float*>( &CONST_FLOAT_MIN ) );

	engine.RegisterGlobalProperty(
		"const double DOUBLE_MIN",
		const_cast<double*>( &CONST_DOUBLE_MIN ) );

	engine.RegisterGlobalProperty(
		"const float FLOAT_MAX",
		const_cast<float*>( &CONST_FLOAT_MAX ) );

	engine.RegisterGlobalProperty(
		"const double DOUBLE_MAX",
		const_cast<double*>( &CONST_DOUBLE_MAX ) );

	engine.RegisterGlobalProperty(
		"const double PI",
		const_cast<double*>( &CONST_PI ) );

	engine.SetDefaultNamespace( szOldNS.c_str() );
}

#define valve_min( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )

//Pass by value wrappers for AS.
template<typename T>
static T AS_Min( T lhs, T rhs )
{
	return valve_min( lhs, rhs );
}

#define valve_max( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )

template<typename T>
static T AS_Max( T lhs, T rhs )
{
	return valve_max( lhs, rhs );
}

#define valve_clamp(val, min, max) (((val) > (max)) ? (max) : (((val) < (min)) ? (min) : (val)))

template<typename T>
static T AS_Clamp( T value, T min, T max )
{
	return valve_clamp( value, min, max );
}

//TODO: move to mathlib - Solokiller
float UTIL_DegreesToRadians( float flDegrees )
{
	return flDegrees * ( M_PI / 180.0 );
}

float UTIL_RadiansToDegrees( float flRadians )
{
	return flRadians * ( 180.0 / M_PI );
}

int UTIL_RandomInt(int imin, int imax)
{
	return RandomStream()->RandomInt( imin, imax );
}

float UTIL_RandomFloat(float fmin, float fmax)
{
	return RandomStream()->RandomFloat( fmin, fmax );
}

float AS_AngleMod(float val)
{
	return anglemod(val);
}

float AS_AngleDiff( float destAngle, float srcAngle )
{
	float delta;

	delta = fmodf(destAngle - srcAngle, 360.0f);
	if ( destAngle > srcAngle )
	{
		if ( delta >= 180 )
			delta -= 360;
	}
	else
	{
		if ( delta <= -180 )
			delta += 360;
	}
	return delta;
}

float AS_ApproachAngle( float target, float value, float speed )
{
	target = anglemod( target );
	value = anglemod( value );
	
	float delta = target - value;

	// Speed is assumed to be positive
	if ( speed < 0 )
		speed = -speed;

	if ( delta < -180 )
		delta += 360;
	else if ( delta > 180 )
		delta -= 360;

	if ( delta > speed )
		value += speed;
	else if ( delta < -speed )
		value -= speed;
	else 
		value = target;

	return value;
}

float AS_AngleDistance( float next, float cur )
{
	float delta = next - cur;

	if ( delta < -180 )
		delta += 360;
	else if ( delta > 180 )
		delta -= 360;

	return delta;
}

static void RegisterScriptMathFunctions( asIScriptEngine& engine )
{
	const std::string szOldNS = engine.GetDefaultNamespace();

	engine.SetDefaultNamespace( "Math" );

/**JSON
 * Randomizes the interger value.
 *
 * @type int
 * @name RandomInt
 *
 * @args %int_in% iLow # The lowest value it can reach
 * @args %int_in% iHigh # The highest value it can reach
 *
 * @return Returns the randomized interger value
 */
	engine.RegisterGlobalFunction(
		"int RandomInt(int low, int high)",
		asFUNCTION( UTIL_RandomInt ), asCALL_CDECL );

/**JSON
 * Randomizes the float value.
 *
 * @type float
 * @name RandomFloat
 *
 * @args %float_in% flLow # The lowest value it can reach
 * @args %float_in% flHigh # The highest value it can reach
 *
 * @return Returns the randomized float value
 */
	engine.RegisterGlobalFunction(
		"float RandomFloat(float low, float high)",
		asFUNCTION( UTIL_RandomFloat ), asCALL_CDECL );

/**JSON
 * Gets the angle modulation
 *
 * @type float
 * @name AngleMod
 *
 * @args %float_in% flAngle # Our angle input
 *
 * @return Returns the angle
 */
	engine.RegisterGlobalFunction(
		"float AngleMod(float flAngle)",
		asFUNCTION( AS_AngleMod ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"float AngleDiff(float flDestAngle, float flSrcAngle)",
		asFUNCTION( AS_AngleDiff ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"float ApproachAngle(float target, float value, float speed)",
		asFUNCTION( AS_ApproachAngle ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"float AngleDistance(float flNext, float flCur)",
		asFUNCTION( AS_AngleDistance ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"float DegreesToRadians(float flDegrees)",
		asFUNCTION( UTIL_DegreesToRadians ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"float RadiansToDegrees(float flRadians)",
		asFUNCTION( UTIL_RadiansToDegrees ), asCALL_CDECL );

/**JSON
 * Clamps the minimum value
 *
 * @type int
 * @name min
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc This value also allows for uint64, int64, int, float and double values.
 *
 * @args %int_in% iValue # Our raw input value
 * @args %int_in% iClamp # The min value it can reach
 *
 */
	engine.RegisterGlobalFunction(
		"uint64 min(uint64 lhs, uint64 rhs)",
		asFUNCTION( AS_Min<uint64_t> ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"int64 min(int64 lhs, int64 rhs)",
		asFUNCTION( AS_Min<int64_t> ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"double min(double lhs, double rhs)",
		asFUNCTION( AS_Min<double> ), asCALL_CDECL );

/**JSON
 * Clamps the maximum value
 *
 * @type int
 * @name max
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc This value also allows for uint64, int64, int, float and double values.
 *
 * @args %int_in% iClamp # The max value it can reach
 * @args %int_in% iValue # Our raw input value
 *
 */
	engine.RegisterGlobalFunction(
		"uint64 max(uint64 lhs, uint64 rhs)",
		asFUNCTION( AS_Max<uint64_t> ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"int64 max(int64 lhs, int64 rhs)",
		asFUNCTION( AS_Max<int64_t> ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"int64 max(int lhs, int rhs)",
		asFUNCTION( AS_Max<int> ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"int64 max(float lhs, float rhs)",
		asFUNCTION( AS_Max<float> ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"double max(double lhs, double rhs)",
		asFUNCTION( AS_Max<double> ), asCALL_CDECL );

/**JSON
 * Clamps the value, so it doesn't go beyond the min or max value
 *
 * @type int
 * @name clamp
 *
 * @infobox_enable true
 * @infobox_type info
 * @infobox_desc This value also allows for uint64, int64, int, float and double values.
 *
 * @args %int_in% iValue # Our raw input value
 * @args %int_in% iMin # The min value it can reach
 * @args %int_in% iMax # The max value it can reach
 *
 */
	engine.RegisterGlobalFunction(
		"uint64 clamp(uint64 value, uint64 min, uint64 max)",
		asFUNCTION( AS_Clamp<uint64_t> ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"int64 clamp(int64 value, int64 min, int64 max)",
		asFUNCTION( AS_Clamp<int64_t> ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"int64 clamp(int value, int min, int max)",
		asFUNCTION( AS_Clamp<int> ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"int64 clamp(float value, float min, float max)",
		asFUNCTION( AS_Clamp<float> ), asCALL_CDECL );

	engine.RegisterGlobalFunction(
		"double clamp(double value, double min, double max)",
		asFUNCTION( AS_Clamp<double> ), asCALL_CDECL );
	
	engine.SetDefaultNamespace( szOldNS.c_str() );
}

void RegisterScriptASMath( asIScriptEngine& engine )
{
	RegisterScriptQAngle( engine );
	RegisterScriptVector2D( engine );
	RegisterScriptVector( engine );
	RegisterScriptMathConstants( engine );
	RegisterScriptMathFunctions( engine );
}
