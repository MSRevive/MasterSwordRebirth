//========= Copyright (c) 2018 Zombie Panic! Team, All rights reserved. ============//

#include "CASColor.h"

#if !defined( ASCOMPILER )
#include "angelscript/CHL2ASServerManager.h"
#else
#include "angelscript/CASCManager.h"
#endif

/**PAGE
 *
 * Class used for color related purposes.
 *
 * @page Color
 * @category Utilities
 *
 * @desc_md false
 * @type class
 *
*/

// Converts RGB to HEX
std::string ConvertRGBtoHex( int num )
{
	static std::string hexDigits = "0123456789ABCDEF";
	std::string rgb;

	for ( int i = (3 * 2) - 1; i >= 0; i-- )
		rgb += hexDigits[((num >> i * 4) & 0xF)];

	return rgb;
}


// Convert HEX to RGB
static int HexadecimalToDecimal( std::string hex )
{
	int hexLength = hex.length();
	double dec = 0;

	for ( int i = 0; i < hexLength; ++i )
	{
		char b = hex[i];

		if ( b >= 48 && b <= 57 )
			b -= 48;
		else if ( b >= 65 && b <= 70 )
			b -= 55;

		dec += b * pow(16, ((hexLength - i) - 1));
	}

	return (int)dec;
}

void ConstructColor( Color* pMemory )
{
	new ( pMemory ) Color();
}


void CopyConstructColor( Color* pMemory, const Color& conColor )
{
	new ( pMemory ) Color( conColor );
}


void Int2ConstructColor( Color* pMemory, int r, int g, int b )
{
	new ( pMemory ) Color( r, g, b, 255 );
}

void Int2ConstructColor_Alpha( Color* pMemory, int r, int g, int b, int a )
{
	new ( pMemory ) Color( r, g, b, a );
}

void DestroyColor( Color* pMemory )
{
	pMemory->~Color();
}

void Hex2ConstructColor( Color* pMemory, const std::string& szHexColor )
{
	// Convert to normal std::string, because we can't use the const std::string&
	std::string hex = szHexColor;

	if ( hex[0] == '#' )
		hex = hex.erase(0, 1);

	unsigned char r = (unsigned char)HexadecimalToDecimal( hex.substr(0, 2) );
	unsigned char g = (unsigned char)HexadecimalToDecimal( hex.substr(2, 2) );
	unsigned char b = (unsigned char)HexadecimalToDecimal( hex.substr(4, 2) );

	new ( pMemory ) Color( r, g, b );
}

std::string Color_ToHEX( Color* pMemory )
{
	int r, g, b;

	r = pMemory->r();
	g = pMemory->g();
	b = pMemory->b();

	int rgbNum = ((r & 0xff) << 16)
		| ((g & 0xff) << 8)
		| (b & 0xff);

	return ConvertRGBtoHex( rgbNum );
}


std::string Color_ToHEXInt( Color* pMemory, int r, int g, int b )
{
	int rgbNum = ((r & 0xff) << 16)
		| ((g & 0xff) << 8)
		| (b & 0xff);

	return ConvertRGBtoHex( rgbNum );
}


Color Color_ToRGB( Color* pMemory, const std::string& szHexColor )
{
	// Convert to normal std::string, because we can't use the const std::string&
	std::string hex = szHexColor;

	if ( hex[0] == '#' )
		hex = hex.erase(0, 1);

	unsigned char r = (unsigned char)HexadecimalToDecimal( hex.substr(0, 2) );
	unsigned char g = (unsigned char)HexadecimalToDecimal( hex.substr(2, 2) );
	unsigned char b = (unsigned char)HexadecimalToDecimal( hex.substr(4, 2) );

	return Color( r, g, b );
}


/**MARKDOWN
**RGB color creation**
```cpp
Color clrMyColor = Color(255, 150, 25);
Color clrMyColor2 = Color(255, 150, 25, 255);
```

**HEX color creation**
```cpp
Color clrMyColor = Color("#003366");
```

**Color copy creation**
```cpp
Color clrCopy = Color("#111266");
Color clrMyColor = Color(clrCopy);
```
 * @filename Color_Construct
 */

/**MARKDOWN
**ToHEX** can also be used to convert RGB to HEX to

```cpp
Color clrMyColor = Color();
string HexFormat = clrMyColor.ToHEX(125, 255, 25);
```
 * @filename Color_ToHEX
 */
void RegisterASColor( asIScriptEngine &engine )
{
	const char* const pszObjectName = "Color";

	engine.RegisterObjectType(
		pszObjectName, sizeof( Color ),
		asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_APP_CLASS_CONSTRUCTOR | asOBJ_APP_CLASS_COPY_CONSTRUCTOR | asOBJ_APP_CLASS_ALLFLOATS );

	engine.RegisterObjectMethod(
		pszObjectName, "Color& opAssign(const Color& in other)",
		asMETHODPR( Color, operator=, ( const Color& ), Color& ), asCALL_THISCALL );

/**JSON
 * Constructs a Color class.
 *
 * @type class
 * @name Color
 * @desc_md true
 * @desc_file Color_Construct
 *
 * @args %int% r # Sets the red color
 * @args %int% g # Sets the green color
 * @args %int% b # Sets the blue color
 *
 * @return Returns Color class which can be manipulated
 */
	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void Color()",
		asFUNCTION( ConstructColor ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void Color(const Color& in other)",
		asFUNCTION( CopyConstructColor ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void Color(int r, int g, int b)",
		asFUNCTION( Int2ConstructColor ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void Color(int r, int g, int b, int a)",
		asFUNCTION( Int2ConstructColor_Alpha ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_CONSTRUCT, "void Color(const string& in szHexColor)",
		asFUNCTION( Hex2ConstructColor ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Destroys the color
 *
 * @type void
 * @name DestroyColor
 * @isfunc true
 * @child Color
 *
 */
	engine.RegisterObjectBehaviour(
		pszObjectName, asBEHAVE_DESTRUCT, "void DestroyColor()",
		asFUNCTION( DestroyColor ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Grabs the red color value
 *
 * @type int
 * @name r
 * @child Color
 * @isfunc true
 *
 * @return Returns the color value (0-255)
 */
	engine.RegisterObjectMethod(
		pszObjectName, "int r() const",
		asMETHOD( Color, r ), asCALL_THISCALL );

/**JSON
 * Grabs the green color value
 *
 * @type int
 * @name g
 * @child Color
 * @isfunc true
 *
 * @return Returns the color value (0-255)
 */
	engine.RegisterObjectMethod(
		pszObjectName, "int g() const",
		asMETHOD( Color, g ), asCALL_THISCALL );

/**JSON
 * Grabs the blue color value
 *
 * @type int
 * @name b
 * @child Color
 * @isfunc true
 *
 * @return Returns the color value (0-255)
 */
	engine.RegisterObjectMethod(
		pszObjectName, "int b() const",
		asMETHOD( Color, b ), asCALL_THISCALL );

/**JSON
 * Grabs the alpha value
 *
 * @type int
 * @name a
 * @child Color
 * @isfunc true
 *
 * @return Returns the alpha value (0-255)
 */
	engine.RegisterObjectMethod(
		pszObjectName, "int a() const",
		asMETHOD( Color, a ), asCALL_THISCALL );

/**JSON
 * Grabs the color code in HEX format
 *
 * @type string
 * @name ToHEX
 * @child Color
 * @isfunc true
 * @desc_md true
 * @desc_file Color_ToHEX
 *
 * @return Returns the HEX value of the color code
 */
	engine.RegisterObjectMethod(
		pszObjectName, "string ToHEX()",
		asFUNCTION( Color_ToHEX ), asCALL_CDECL_OBJFIRST );

	engine.RegisterObjectMethod(
		pszObjectName, "string ToHEX(int r, int g, int b)",
		asFUNCTION( Color_ToHEXInt ), asCALL_CDECL_OBJFIRST );

/**JSON
 * Converts HEX color to RGB
 *
 * @type class
 * @name ToRGB
 * @child Color
 *
 * @args %string_in% szHexColor # Our HEX value
 *
 * @return Returns Color class which can be manipulated
 */
	engine.RegisterObjectMethod(
		pszObjectName, "Color ToRGB(const string& in szHexColor)",
		asFUNCTION( Color_ToRGB ), asCALL_CDECL_OBJFIRST );
}
