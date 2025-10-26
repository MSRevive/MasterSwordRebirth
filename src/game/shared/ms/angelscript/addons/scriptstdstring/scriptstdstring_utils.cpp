#include <assert.h>
#include "scriptstdstring.h"
#include "../scriptarray/scriptarray.h"
#include <stdio.h>
#include <string.h>

using namespace std;

BEGIN_AS_NAMESPACE

// This function takes an input string and splits it into parts by looking
// for a specified delimiter. Example:
//
// string str = "A|B||D";
// array<string>@ array = str.split("|");
//
// The resulting array has the following elements:
//
// {"A", "B", "", "D"}
//
// AngelScript signature:
// array<string>@ string::split(const string &in delim) const
static CScriptArray *StringSplit(const string &delim, const string &str)
{
	// Obtain a pointer to the engine
	asIScriptContext *ctx = asGetActiveContext();
	asIScriptEngine *engine = ctx->GetEngine();

	// TODO: This should only be done once
	// TODO: This assumes that CScriptArray was already registered
	asITypeInfo *arrayType = engine->GetTypeInfoByDecl("array<string>");

	// Create the array object
	CScriptArray *array = CScriptArray::Create(arrayType);

	// Find the existence of the delimiter in the input string
	size_t pos = 0, prev = 0;
	asUINT count = 0;
	while( (pos = str.find(delim, prev)) != string::npos )
	{
		// Add the part to the array
		array->Resize(array->GetSize()+1);
		((string*)array->At(count))->assign(&str[prev], pos-prev);

		// Find the next part
		count++;
		prev = pos + delim.length();
	}

	// Add the remaining part
	array->Resize(array->GetSize()+1);
	((string*)array->At(count))->assign(&str[prev]);

	return array;
}

static void StringSplit_Generic(asIScriptGeneric *gen)
{
	// Get the arguments
	string *str   = (string*)gen->GetObject();
	string *delim = *(string**)gen->GetAddressOfArg(0);

	// Return the array by handle
	*(CScriptArray**)gen->GetAddressOfReturnLocation() = StringSplit(*delim, *str);
}



// This function takes as input an array of string handles as well as a
// delimiter and concatenates the array elements into one delimited string.
// Example:
//
// array<string> array = {"A", "B", "", "D"};
// string str = join(array, "|");
//
// The resulting string is:
//
// "A|B||D"
//
// AngelScript signature:
// string join(const array<string> &in array, const string &in delim)
static string StringJoin(const CScriptArray &array, const string &delim)
{
	// Create the new string
	string str = "";
	if( array.GetSize() )
	{
		int n;
		for( n = 0; n < (int)array.GetSize() - 1; n++ )
		{
			str += *(string*)array.At(n);
			str += delim;
		}

		// Add the last part
		str += *(string*)array.At(n);
	}

	return str;
}

static void StringJoin_Generic(asIScriptGeneric *gen)
{
	// Get the arguments
	CScriptArray  *array = *(CScriptArray**)gen->GetAddressOfArg(0);
	string *delim = *(string**)gen->GetAddressOfArg(1);

	// Return the string
	new(gen->GetAddressOfReturnLocation()) string(StringJoin(*array, *delim));
}

// Check if a string starts with a given prefix
// AngelScript signature:
// bool string::starts_with(const string &in prefix) const
static bool String_StartsWith(const string &prefix, const string &str)
{
	if (str.length() < prefix.length())
		return false;
	return str.compare(0, prefix.length(), prefix) == 0;
}

static void String_StartsWith_Generic(asIScriptGeneric *gen)
{
	string *str = (string*)gen->GetObject();
	string *prefix = *(string**)gen->GetAddressOfArg(0);
	*(bool*)gen->GetAddressOfReturnLocation() = String_StartsWith(*prefix, *str);
}

// Check if a string ends with a given suffix
// AngelScript signature:
// bool string::ends_with(const string &in suffix) const
static bool String_EndsWith(const string &suffix, const string &str)
{
	if (str.length() < suffix.length())
		return false;
	return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}

static void String_EndsWith_Generic(asIScriptGeneric *gen)
{
	string *str = (string*)gen->GetObject();
	string *suffix = *(string**)gen->GetAddressOfArg(0);
	*(bool*)gen->GetAddressOfReturnLocation() = String_EndsWith(*suffix, *str);
}

// Check if a string contains a substring
// AngelScript signature:
// bool string::contains(const string &in substr) const
static bool String_Contains(const string &substr, const string &str)
{
	return str.find(substr) != string::npos;
}

static void String_Contains_Generic(asIScriptGeneric *gen)
{
	string *str = (string*)gen->GetObject();
	string *substr = *(string**)gen->GetAddressOfArg(0);
	*(bool*)gen->GetAddressOfReturnLocation() = String_Contains(*substr, *str);
}

// Trim whitespace from both ends of a string
// AngelScript signature:
// string string::trim() const
static string String_Trim(const string &str)
{
	size_t first = str.find_first_not_of(" \t\r\n");
	if (first == string::npos)
		return "";
	size_t last = str.find_last_not_of(" \t\r\n");
	return str.substr(first, last - first + 1);
}

static void String_Trim_Generic(asIScriptGeneric *gen)
{
	string *str = (string*)gen->GetObject();
	new(gen->GetAddressOfReturnLocation()) string(String_Trim(*str));
}

// Convert string to lowercase
// AngelScript signature:
// string string::to_lower() const
static string String_ToLower(const string &str)
{
	string result = str;
	for (size_t i = 0; i < result.length(); i++)
	{
		if (result[i] >= 'A' && result[i] <= 'Z')
			result[i] += 32; // Convert to lowercase
	}
	return result;
}

static void String_ToLower_Generic(asIScriptGeneric *gen)
{
	string *str = (string*)gen->GetObject();
	new(gen->GetAddressOfReturnLocation()) string(String_ToLower(*str));
}

// Convert string to uppercase
// AngelScript signature:
// string string::to_upper() const
static string String_ToUpper(const string &str)
{
	string result = str;
	for (size_t i = 0; i < result.length(); i++)
	{
		if (result[i] >= 'a' && result[i] <= 'z')
			result[i] -= 32; // Convert to uppercase
	}
	return result;
}

static void String_ToUpper_Generic(asIScriptGeneric *gen)
{
	string *str = (string*)gen->GetObject();
	new(gen->GetAddressOfReturnLocation()) string(String_ToUpper(*str));
}

// This is where the utility functions are registered.
// The string type must have been registered first.
void RegisterStdStringUtils(asIScriptEngine *engine)
{
	int r;

	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		r = engine->RegisterObjectMethod("string", "array<string>@ split(const string &in) const", asFUNCTION(StringSplit_Generic), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("string join(const array<string> &in, const string &in)", asFUNCTION(StringJoin_Generic), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "bool starts_with(const string &in) const", asFUNCTION(String_StartsWith_Generic), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "bool ends_with(const string &in) const", asFUNCTION(String_EndsWith_Generic), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "bool contains(const string &in) const", asFUNCTION(String_Contains_Generic), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "string trim() const", asFUNCTION(String_Trim_Generic), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "string to_lower() const", asFUNCTION(String_ToLower_Generic), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "string to_upper() const", asFUNCTION(String_ToUpper_Generic), asCALL_GENERIC); assert(r >= 0);
	}
	else
	{
		r = engine->RegisterObjectMethod("string", "array<string>@ split(const string &in) const", asFUNCTION(StringSplit), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterGlobalFunction("string join(const array<string> &in, const string &in)", asFUNCTION(StringJoin), asCALL_CDECL); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "bool starts_with(const string &in) const", asFUNCTION(String_StartsWith), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "bool ends_with(const string &in) const", asFUNCTION(String_EndsWith), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "bool contains(const string &in) const", asFUNCTION(String_Contains), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "string trim() const", asFUNCTION(String_Trim), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "string to_lower() const", asFUNCTION(String_ToLower), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("string", "string to_upper() const", asFUNCTION(String_ToUpper), asCALL_CDECL_OBJLAST); assert(r >= 0);
	}
}

END_AS_NAMESPACE
