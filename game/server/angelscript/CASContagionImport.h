#ifndef GAME_ANGELSCRIPT_CONTAGION_IMPORT_H
#define GAME_ANGELSCRIPT_CONTAGION_IMPORT_H

#include <string>
#include <angelscript/angelscript.h>
#include "extdll.h"
#include <progdefs.h>
#include <vector.h>
#include <eiface.h>
#include "hl/util.h"
#include "msdllheaders.h"
#include <FileSystem.h>
#include "ms/filesystem_shared.h"

#define FORCEINLINE __forceinline
#define MAX_PATH 260
#define COPY_ALL_CHARACTERS -1
#define Valve_Min( a, b ) ( ( ( a ) < ( b ) ) ? ( a ) : ( b ) )
#define Valve_Max( a, b ) ( ( ( a ) > ( b ) ) ? ( a ) : ( b ) )

// Imported crap from Contagion & SDK 2013.
// Though, most of the code has been modified and changed for thie engine branch.
namespace ContagionImport
{
	FORCEINLINE void UTIL_STDReplaceString( std::string &path, std::string search, std::string replace )
	{
		size_t pos = 0;
		while ((pos = path.find(search, pos)) != std::string::npos) {
			path.replace(pos, search.length(), replace);
			pos += replace.length();
		}
	}

	FORCEINLINE unsigned char tolower_fast(unsigned char c)
	{
		if ( (c >= 'A') && (c <= 'Z') )
			return c + ('a' - 'A');
		return c;
	}

	FORCEINLINE char const* Q_stristr( char const* pStr, char const* pSearch )
	{
		if (!pStr || !pSearch) 
		return 0;

		char const* pLetter = pStr;

		// Check the entire string
		while (*pLetter != 0)
		{
			// Skip over non-matches
			if (tolower_fast((unsigned char)*pLetter) == tolower_fast((unsigned char)*pSearch))
			{
				// Check for match
				char const* pMatch = pLetter + 1;
				char const* pTest = pSearch + 1;
				while (*pTest != 0)
				{
					// We've run off the end; don't bother.
					if (*pMatch == 0)
						return 0;

					if (tolower_fast((unsigned char)*pMatch) != tolower_fast((unsigned char)*pTest))
						break;

					++pMatch;
					++pTest;
				}

				// Found a match!
				if (*pTest == 0)
					return pLetter;
			}

			++pLetter;
		}

		return 0;
	}

	FORCEINLINE char* Q_str( char* pStr, char const* pSearch )
	{
		return (char*)Q_stristr( (char const*)pStr, pSearch );
	}

	//-----------------------------------------------------------------------------
	// Purpose: If COPY_ALL_CHARACTERS == max_chars_to_copy then we try to add the whole pSrc to the end of pDest, otherwise
	//  we copy only as many characters as are specified in max_chars_to_copy (or the # of characters in pSrc if thats's less).
	// Input  : *pDest - destination buffer
	//			*pSrc - string to append
	//			destBufferSize - sizeof the buffer pointed to by pDest
	//			max_chars_to_copy - COPY_ALL_CHARACTERS in pSrc or max # to copy
	// Output : char * the copied buffer
	//-----------------------------------------------------------------------------
	FORCEINLINE char *V_strncat(char *pDest, const char *pSrc, size_t destBufferSize, int max_chars_to_copy )
	{
		size_t charstocopy = (size_t)0;

		size_t len = strlen(pDest);
		size_t srclen = strlen( pSrc );
		if ( max_chars_to_copy <= COPY_ALL_CHARACTERS )
			charstocopy = srclen;
		else
			charstocopy = (size_t)Valve_Min( max_chars_to_copy, (int)srclen );

		if ( len + charstocopy >= destBufferSize )
			charstocopy = destBufferSize - len - 1;

		if ( !charstocopy )
			return pDest;

		char *pOut = strncat( pDest, pSrc, charstocopy );
		pOut[destBufferSize-1] = 0;
		return pOut;
	}

	FORCEINLINE void V_strcat( char *dest, const char *src, int cchDest )
	{
		V_strncat( dest, src, cchDest, COPY_ALL_CHARACTERS );
	}

	// Copies at most nCharsToCopy bytes from pIn into pOut.
	// Returns false if it would have overflowed pOut's buffer.
	FORCEINLINE static bool CopyToMaxChars( char *pOut, int outSize, const char *pIn, int nCharsToCopy )
	{
		if ( outSize == 0 )
			return false;

		int iOut = 0;
		while ( *pIn && nCharsToCopy > 0 )
		{
			if ( iOut == (outSize-1) )
			{
				pOut[iOut] = 0;
				return false;
			}
			pOut[iOut] = *pIn;
			++iOut;
			++pIn;
			--nCharsToCopy;
		}
	
		pOut[iOut] = 0;
		return true;
	}

	FORCEINLINE bool V_StrSubst(
		const char* pIn,
		const char* pMatch,
		const char* pReplaceWith,
		char* pOut,
		int outLen,
		bool bCaseSensitive
	)
	{
		int replaceFromLen = (int)strlen( pMatch );
		int replaceToLen = (int)strlen( pReplaceWith );

		const char *pInStart = pIn;
		char *pOutPos = pOut;
		pOutPos[0] = 0;

		while ( 1 )
		{
			int nRemainingOut = outLen - (pOutPos - pOut);

			const char *pTestPos = ( bCaseSensitive ? strstr( pInStart, pMatch ) : Q_stristr( pInStart, pMatch ) );
			if ( pTestPos )
			{
				// Found an occurence of pMatch. First, copy whatever leads up to the string.
				int copyLen = pTestPos - pInStart;
				if ( !CopyToMaxChars( pOutPos, nRemainingOut, pInStart, copyLen ) )
					return false;
			
				// Did we hit the end of the output string?
				if ( copyLen > nRemainingOut-1 )
					return false;

				pOutPos += strlen( pOutPos );
				nRemainingOut = outLen - (pOutPos - pOut);

				// Now add the replacement string.
				if ( !CopyToMaxChars( pOutPos, nRemainingOut, pReplaceWith, replaceToLen ) )
					return false;

				pInStart += copyLen + replaceFromLen;
				pOutPos += replaceToLen;			
			}
			else
			{
				// We're at the end of pIn. Copy whatever remains and get out.
				int copyLen = (int)strlen( pInStart );
				strncpy( pOutPos, pInStart, nRemainingOut );
				return ( copyLen <= nRemainingOut-1 );
			}
		}
	}

	FORCEINLINE const char *UTIL_LogFolder( const char *strFilename, bool bDate )
	{
		if ( !strFilename )
		{
			bDate = false;
			strFilename = "angelscript";
		}

		static char LogFile[ 1024 ];

		// Make sure this folder exist first
		if ( g_pFileSystem )
			g_pFileSystem->CreateDirHierarchy( "logs", "GAME" );

		// Setup the current time for our log name.
		time_t theTime = time(NULL);
		struct tm *aTime = localtime(&theTime);

		int day = aTime->tm_mday;
		int month = aTime->tm_mon + 1; // Month is 0 - 11, add 1 to get a jan-dec 1-12 concept
		int year = aTime->tm_year + 1900; // Year is # years since 1900

		// Grab our date
		char datefolder[ 512 ];
		snprintf( datefolder, sizeof( datefolder ), "%i-%i-%i", day, month, year );

		strcpy_s( LogFile, "msr/logs/" );

		// Use date folder?
		if ( bDate )
		{
			strcat_s( LogFile, datefolder );
			strcat_s( LogFile, "/" );
		}

		strcat_s( LogFile, strFilename );

		// Only add, if we don't have .log
		if ( Q_stristr( strFilename, ".log" ) == 0 )
			strcat_s( LogFile, ".log" );

		// Always make sure this directory exist!
		if ( bDate && g_pFileSystem )
		{
			char logsfolder[ 1024 ];
			snprintf( logsfolder, sizeof( logsfolder ), "logs/%s", datefolder );
			g_pFileSystem->CreateDirHierarchy( logsfolder, "GAME" );
		}

		return LogFile;
	}

	FORCEINLINE const char *UTIL_CurrentMapLog()
	{
		return UTIL_LogFolder( STRING( gpGlobals->mapname ), true );
	}

	// Ported from ZPS
	FORCEINLINE void ZPUTILS_ReplaceColorCode( std::string &stdstring, const char *strReplace, const char *strColor, bool bSpecial = false )
	{
		// Static character string
		static char sString[1024];
		static char sString_OutPut[1024];

		// Insert the new value
		strcpy( sString, stdstring.c_str() );

		char colorfind[250];
		colorfind[0] = 0;
		strcpy( colorfind, strReplace );
		strlwr( colorfind );

		// Find this value
		std::string strFind = "{";
		strFind += colorfind;
		strFind += "}";

		// Replace it with this color code
		std::string strColorCode = bSpecial ? "" : "\x07";
		strColorCode += strColor;

		// Now lets check for certain strings, and replace em!
		V_StrSubst( sString, strFind.c_str(), strColorCode.c_str(), sString_OutPut, sizeof( sString_OutPut ), false );

		stdstring = sString_OutPut;
	}

	struct __HexColorString
	{
		std::string _color;
		std::string _hex;
	};

	// Static const colors. These will never change, and shouldn't change.
	static const std::vector<__HexColorString> __colors = {
		{ "aliceblue", "F0F8FF" },
		{ "allies", "4D7942" },
		{ "ancient", "EB4B4B" },
		{ "antiquewhite", "FAEBD7" },
		{ "aqua", "00FFFF" },
		{ "aquamarine", "7FFFD4" },
		{ "arcana", "ADE55C" },
		{ "axis", "FF4040" },
		{ "azure", "007FFF" },
		{ "beige", "F5F5DC" },
		{ "bisque", "FFE4C4" },
		{ "black", "000000" },
		{ "blanchedalmond", "FFEBCD" },
		{ "blue", "99CCFF" },
		{ "blueviolet", "8A2BE2" },
		{ "brown", "A52A2A" },
		{ "burlywood", "DEB887" },
		{ "cadetblue", "5F9EA0" },
		{ "chartreuse", "7FFF00" },
		{ "chocolate", "D2691E" },
		{ "collectors", "AA0000" },
		{ "common", "B0C3D9" },
		{ "community", "70B04A" },
		{ "coral", "FF7F50" },
		{ "cornflowerblue", "6495ED" },
		{ "cornsilk", "FFF8DC" },
		{ "corrupted", "A32C2E" },
		{ "crimson", "DC143C" },
		{ "cyan", "00FFFF" },
		{ "darkblue", "00008B" },
		{ "darkcyan", "008B8B" },
		{ "darkgoldenrod", "B8860B" },
		{ "darkgray", "A9A9A9" },
		{ "darkgrey", "A9A9A9" },
		{ "darkgreen", "006400" },
		{ "darkkhaki", "BDB76B" },
		{ "darkmagenta", "8B008B" },
		{ "darkolivegreen", "556B2F" },
		{ "darkorange", "FF8C00" },
		{ "darkorchid", "9932CC" },
		{ "darkred", "8B0000" },
		{ "darksalmon", "E9967A" },
		{ "darkseagreen", "8FBC8F" },
		{ "darkslateblue", "483D8B" },
		{ "darkslategray", "2F4F4F" },
		{ "darkslategrey", "2F4F4F" },
		{ "darkturquoise", "00CED1" },
		{ "darkviolet", "9400D3" },
		{ "deeppink", "FF1493" },
		{ "deepskyblue", "00BFFF" },
		{ "dimgray", "696969" },
		{ "dimgrey", "696969" },
		{ "dodgerblue", "1E90FF" },
		{ "exalted", "CCCCCD" },
		{ "firebrick", "B22222" },
		{ "floralwhite", "FFFAF0" },
		{ "forestgreen", "228B22" },
		{ "frozen", "4983B3" },
		{ "fuchsia", "FF00FF" },
		{ "fullblue", "0000FF" },
		{ "fullred", "FF0000" },
		{ "gainsboro", "DCDCDC" },
		{ "genuine", "4D7455" },
		{ "ghostwhite", "F8F8FF" },
		{ "gold", "FFD700" },
		{ "goldenrod", "DAA520" },
		{ "gray", "CCCCCC" },
		{ "grey", "CCCCCC" },
		{ "green", "3EFF3E" },
		{ "greenyellow", "ADFF2F" },
		{ "haunted", "38F3AB" },
		{ "honeydew", "F0FFF0" },
		{ "hotpink", "FF69B4" },
		{ "immortal", "E4AE33" },
		{ "indianred", "CD5C5C" },
		{ "indigo", "4B0082" },
		{ "ivory", "FFFFF0" },
		{ "khaki", "F0E68C" },
		{ "lavender", "E6E6FA" },
		{ "lavenderblush", "FFF0F5" },
		{ "lawngreen", "7CFC00" },
		{ "legendary", "D32CE6" },
		{ "lemonchiffon", "FFFACD" },
		{ "lightblue", "ADD8E6" },
		{ "lightcoral", "F08080" },
		{ "lightcyan", "E0FFFF" },
		{ "lightgoldenrodyellow", "FAFAD2" },
		{ "lightgray", "D3D3D3" },
		{ "lightgrey", "D3D3D3" },
		{ "lightgreen", "99FF99" },
		{ "lightpink", "FFB6C1" },
		{ "lightsalmon", "FFA07A" },
		{ "lightseagreen", "20B2AA" },
		{ "lightskyblue", "87CEFA" },
		{ "lightslategray", "778899" },
		{ "lightslategrey", "778899" },
		{ "lightsteelblue", "B0C4DE" },
		{ "lightyellow", "FFFFE0" },
		{ "lime", "00FF00" },
		{ "limegreen", "32CD32" },
		{ "linen", "FAF0E6" },
		{ "magenta", "FF00FF" },
		{ "maroon", "800000" },
		{ "mediumaquamarine", "66CDAA" },
		{ "mediumblue", "0000CD" },
		{ "mediumorchid", "BA55D3" },
		{ "mediumpurple", "9370D8" },
		{ "mediumseagreen", "3CB371" },
		{ "mediumslateblue", "7B68EE" },
		{ "mediumspringgreen", "00FA9A" },
		{ "mediumturquoise", "48D1CC" },
		{ "mediumvioletred", "C71585" },
		{ "midnightblue", "191970" },
		{ "mintcream", "F5FFFA" },
		{ "mistyrose", "FFE4E1" },
		{ "moccasin", "FFE4B5" },
		{ "mythical", "8847FF" },
		{ "navajowhite", "FFDEAD" },
		{ "navy", "000080" },
		{ "normal", "B2B2B2" },
		{ "oldlace", "FDF5E6" },
		{ "olive", "9EC34F" },
		{ "olivedrab", "6B8E23" },
		{ "orange", "FFA500" },
		{ "orangered", "FF4500" },
		{ "orchid", "DA70D6" },
		{ "palegoldenrod", "EEE8AA" },
		{ "palegreen", "98FB98" },
		{ "paleturquoise", "AFEEEE" },
		{ "palevioletred", "D87093" },
		{ "papayawhip", "FFEFD5" },
		{ "peachpuff", "FFDAB9" },
		{ "peru", "CD853F" },
		{ "pink", "FFC0CB" },
		{ "plum", "DDA0DD" },
		{ "powderblue", "B0E0E6" },
		{ "purple", "800080" },
		{ "rare", "4B69FF" },
		{ "red", "FF4040" },
		{ "rosybrown", "BC8F8F" },
		{ "royalblue", "4169E1" },
		{ "saddlebrown", "8B4513" },
		{ "salmon", "FA8072" },
		{ "sandybrown", "F4A460" },
		{ "seagreen", "2E8B57" },
		{ "seashell", "FFF5EE" },
		{ "selfmade", "70B04A" },
		{ "sienna", "A0522D" },
		{ "silver", "C0C0C0" },
		{ "skyblue", "87CEEB" },
		{ "slateblue", "6A5ACD" },
		{ "slategray", "708090" },
		{ "slategrey", "708090" },
		{ "snow", "FFFAFA" },
		{ "springgreen", "00FF7F" },
		{ "steelblue", "4682B4" },
		{ "strange", "CF6A32" },
		{ "tan", "D2B48C" },
		{ "teal", "008080" },
		{ "thistle", "D8BFD8" },
		{ "tomato", "FF6347" },
		{ "turquoise", "40E0D0" },
		{ "uncommon", "B0C3D9" },
		{ "unique", "FFD700" },
		{ "unusual", "8650AC" },
		{ "valve", "A50F79" },
		{ "vintage", "476291" },
		{ "violet", "EE82EE" },
		{ "wheat", "F5DEB3" },
		{ "white", "FFFFFF" },
		{ "whitesmoke", "F5F5F5" },
		{ "yellow", "FFFF00" },
		{ "yellowgreen", "9ACD32" }
	};

	// Gold Source does not support SDK2013 HEX style coloring for the chat or console.
	// So instead, we will simply get rid of it all.
	FORCEINLINE const char* UTIL_StripColors( const char* sString )
	{
		// Convert to std::string
		std::string strString = sString;

		// MSC does not support JSON, which the original code uses.
		// So instead, we will use a static const std::vector instead.
		for ( auto colornames : __colors )
			ZPUTILS_ReplaceColorCode( strString, colornames._color.c_str(), "", true );

		// Now do it for "team" and "default"
		ZPUTILS_ReplaceColorCode( strString, "default", "", true );
		ZPUTILS_ReplaceColorCode( strString, "team", "", true );

		// Now strip special chars
		UTIL_STDReplaceString( strString, "\x01", "" );

		// Static character string
		static char sString_OutPut[1024];
		sString_OutPut[0] = 0;

		// Insert the new value
		strcpy( sString_OutPut, strString.c_str() );

		return sString_OutPut;
	}

	// Ported from Source Engine
	FORCEINLINE CBaseEntity	*UTIL_EntityByIndex( int entityIndex )
	{
		CBaseEntity *entity = NULL;
		if ( entityIndex > 0 )
		{
			edict_t *edict = INDEXENT( entityIndex );
			if ( edict && !edict->free )
				entity = CBaseEntity::Instance( edict );
		}
		return entity;
	}
}


#endif