#include <fstream>
#include <string>
#include <vector>

#include <angelscript.h>

class ByteCode : public asIBinaryStream
{
private:
	int readPos;
	int writePos;
	std::vector<asBYTE> code;

public:
	void Reset() { readPos = writePos = 0; code.resize(0); }

	std::vector<asBYTE>& GetCode() { return(code); }
	asUINT GetSize() { return( (asUINT)code.size() ); }

	// asIBinaryStream
	int Read( void* ptr, asUINT size );
	int Write( const void* ptr, asUINT size );

	// returns status of asIScriptModule::LoadByteCode() / asIScriptModule::SaveByteCode()
	// if module is NULL, returns asINVALID_ARG
	int Read( asIScriptModule* module );
	int Write( asIScriptModule* module );

public:
	ByteCode() { Reset(); }
};

class ByteCodeFile
{
private:
	asUINT                   code_size;
	std::vector<std::string> code_name;
	std::vector<ByteCode>    code_byte;

public:
	void Reset() { code_size = 0; code_name.resize(0); code_byte.resize(0); }

	/// Return number of stored modules
	asUINT GetCount() { return(code_size); }

	/// Returns name/code with specified index
	bool GetByIndex( asUINT idx, std::string& name, ByteCode& bytecode );
	bool GetNameByIndex( asUINT idx, std::string& name );
	bool GetCodeByIndex( asUINT idx, ByteCode& code );

	/// helper functions
	bool Load( std::string filename );
	bool Save( std::string filename );

	/// Loading/saving
	void Load( std::istream& stream );
	void Save( std::ostream& stream );

protected:
	// called before any reading/writing operations
	virtual void OnLoadStart( std::istream& stream ) {}
	virtual void OnSaveStart( std::ostream& stream ) {}

	// called after all reading/writing operations are finished
	virtual void OnLoadEnd( std::istream& stream ) {}
	virtual void OnSaveEnd( std::ostream& stream ) {}

	// called before reading/write module-specific data (name, bytecode)
	virtual void OnLoadCodeStart( asUINT idx ) {}
	virtual void OnSaveCodeStart( asUINT idx ) {}

	// called before reading/write module-specific data (name, bytecode)
	virtual void OnLoadCodeEnd( asUINT idx ) {}
	virtual void OnSaveCodeEnd( asUINT idx ) {}

	// called after successfuly added module's bytecode
	virtual void OnModuleAdded( asUINT idx, asIScriptModule* module ) {}

	// reading/writing helpers
	void LoadString( std::istream& stream, std::string& data );
	void SaveString( std::ostream& stream, const std::string& data );

	template<typename T> void LoadData( std::istream& stream, T& data )
	{
		stream.read( reinterpret_cast<char*>(&data), sizeof(T) );
	}

	template<typename T> void SaveData( std::ostream& stream, const T& data )
	{
		stream.write( reinterpret_cast<const char*>(&data), sizeof(T) );
	}

public:
	// add all modules currently present in engine
	bool Add( asIScriptEngine* engine );

	// add single module
	bool Add( asIScriptModule* module );

	// add collection of modules
	bool Add( std::vector<asIScriptModule*> modules );

public:
	ByteCodeFile() { Reset(); }
};
