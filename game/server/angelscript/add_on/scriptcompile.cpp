#include "cbase.h"

#include "scriptcompile.h"

#ifndef AS_NO_MEMORY_H
#include <memory.h>
#endif
#include <string.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

int ByteCode::Read( void* ptr, asUINT size ) // asIBinaryStream
{
	if( !ptr || !size )
		return 0;
	memcpy( ptr, &code[readPos], size );
	readPos += size;
	return 1;
}

int ByteCode::Write( const void* ptr, asUINT size ) // asIBinaryStream
{
	if( !ptr || !size )
		return 0;
	code.resize( code.size() + size );
	memcpy( &code[writePos], ptr, size );
	writePos += size;
	return 1;
}

int ByteCode::Read( asIScriptModule* module )
{
	if( !module )
		return( asINVALID_ARG );

	return( module->LoadByteCode( this ));
}

int ByteCode::Write( asIScriptModule* module )
{
	if( !module )
		return( asINVALID_ARG );

	return( module->SaveByteCode( this ));
}

/*
 * ByteCodeFile
 */

bool ByteCodeFile::GetByIndex( asUINT idx, std::string& name, ByteCode& code )
{
	if( idx >= code_size )
		return( false );

	name = code_name[idx];
	code = code_byte[idx];

	return( true );
}

bool ByteCodeFile::GetNameByIndex( asUINT idx, std::string& name )
{
	if( idx >= code_size )
		return( false );

	name = code_name[idx];

	return( true );
}

bool ByteCodeFile::GetCodeByIndex( asUINT idx, ByteCode& code )
{
	if( idx >= code_size )
		return( false );

	code = code_byte[idx];
	return( true );
}

bool ByteCodeFile::Load( std::string filename )
{
	std::ifstream file( filename, std::ios_base::in | std::ios_base::binary );
	if( !file )
		return( false );

	Load( file );
	file.close();

	return( true );
}

bool ByteCodeFile::Save( std::string filename )
{
	std::ofstream file( filename, std::ios_base::out | std::ios_base::binary );
	if( !file )
		return( false );

	Save( file );
	file.close();

	return( true );
}

void ByteCodeFile::Load( std::istream& stream )
{
	OnLoadStart( stream );

	asUINT count = 0;
	LoadData( stream, count );

	for( asUINT c=0; c<count; c++ )
	{
		OnLoadCodeStart( c );

		std::string name;
		asUINT size = 0;

		LoadString( stream, name );
		LoadData( stream, size );

		ByteCode bytecode;
		char* bytes = new char[size];
		stream.read( bytes, size );
		bytecode.Write( bytes, size );
		delete[] bytes;

		code_name.push_back( name );
		code_byte.push_back( bytecode );
		code_size++;

		OnLoadCodeEnd( c );
	}

	OnLoadEnd( stream );
}

void ByteCodeFile::Save( std::ostream& stream )
{
	OnSaveStart( stream );
	SaveData( stream, code_size );

	for( asUINT c=0; c<code_size; c++ )
	{
		OnSaveCodeStart( c );

		std::string name  = code_name[c];
		std::vector<asBYTE>& code = code_byte[c].GetCode();

		SaveString( stream, name );
		SaveData( stream, (asUINT)code.size() );

		for( std::vector<asBYTE>::const_iterator byte = code.begin(); byte != code.end(); ++byte )
			stream << *byte;

		OnSaveCodeEnd( c );
	}

	OnSaveEnd( stream );
}

void ByteCodeFile::LoadString( std::istream& stream, std::string& data )
{
	asUINT len = 0;
	LoadData( stream, len );
	if( len > 0 )
	{
		data = std::string( len, '\0' );
		stream.read( &data[0], len );
	}
}

void ByteCodeFile::SaveString( std::ostream& stream, const std::string& data )
{
	asUINT len = data.length();
	SaveData( stream, len );
	if( len > 0 )
		stream.write( data.c_str(), data.length() );
}

bool ByteCodeFile::Add( asIScriptEngine* engine )
{
	asUINT mLen = engine->GetModuleCount();
	for( uint m=0; m<mLen; m++ )
	{
		asIScriptModule* module = engine->GetModuleByIndex( m );
		if( !module )
			return( false );

		if( !Add( module ))
			return( false );
	}

	return( true );
}

bool ByteCodeFile::Add( asIScriptModule* module )
{
	if( !module )
		return( false );

	std::string name = module->GetName();

	ByteCode code;
	int temp = code.Write( module );
	if( temp != asSUCCESS )
		return( false );

	code_name.push_back( name );
	code_byte.push_back( code );
	code_size++;

	OnModuleAdded( code_size-1, module );

	return( true );
}

bool ByteCodeFile::Add( std::vector<asIScriptModule*> modules )
{
	bool result = true;
	for( auto module : modules )
	{
		if( !Add( module ))
			return( false );
	}

	return( result );
}
