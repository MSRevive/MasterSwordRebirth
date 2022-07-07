// 
// Scriptpack.cpp : Defines the entry point for the console application.
//

#include <direct.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "cbase.h"
#include "packer.h"
#include "crc/crchash.h"
#include "tclap/CmdLine.h"

bool verbose;
bool release;
bool errFile;

int main(int argc, char** argv)
{
	try {
		//Program description
		TCLAP::CmdLine cmd("Packs via scripts for use with MSR", ' ', "0.9");
		
		char workDir[MAX_PATH];
		char _outDir[MAX_PATH];
		
		//String arguements
		char rootDir[MAX_PATH];
		_getcwd(rootDir, MAX_PATH);
		memcpy(workDir, rootDir, MAX_PATH);
		strncat(workDir, "\\scripts", MAX_PATH);
		memcpy(_outDir, rootDir, MAX_PATH);
		strncat(_outDir, "\\cooked", MAX_PATH);
		
		TCLAP::ValueArg<char*> oDirArg("o", "output", "Output Directory", false, _outDir, "The output directory for cleaned scripts");
		cmd.add(oDirArg);
		
		TCLAP::SwitchArg relSwitch("r", "release", "Release build", cmd, false);
		TCLAP::SwitchArg verboseSwitch("v", "verbose", "Turn on/off verbose.", cmd, false);
		TCLAP::SwitchArg errFileSwitch("e", "errfile", "Turn on/off error output to file.", cmd, false);
		
		//Parse command line arguements
		cmd.parse(argc, argv);
		
		char *outDir = oDirArg.getValue();
		release = relSwitch.getValue();
		verbose = verboseSwitch.getValue();
		errFile = errFileSwitch.getValue();
		
		struct stat info;
		if(stat(workDir, &info) != 0)
		{
			printf("Error: work directory %s not found!\n", workDir);
			exit(-1);
		}
		
		if(stat(outDir, &info) != 0)
		{
			printf("Error: output directory %s not found!\n", outDir);
		}
		
		printf("Packing %s...\n\n", workDir);
		Packer packer(workDir, rootDir, outDir);
		packer.cookScripts();
		//packer.packScripts();
		printf("Wrote changes to the script dll. Hash %u\n\n", GetFileCheckSum("./sc.dll"));
	} catch (TCLAP::ArgException &err)
	{
		std::cout << "Error: " << err.error() << "for arg " << err.argId() << std::endl;
		exit(-1);
	}

	return 0;
}