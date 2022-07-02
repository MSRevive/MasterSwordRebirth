// 
// Scriptpack.cpp : Defines the entry point for the console application.
//

#include <direct.h>
#include <string>
#include <sys/types.h>
#include <sys/stat.h>

#include "cbase.h"
#include "tclap/CmdLine.h"

HANDLE g_resHandle;
bool verbose;

int main(int argc, char** argv)
{
	try {
		//Program description
		TCLAP::CmdLine cmd("Packs via scripts for use with MSR", ' ', "0.9");
		
		//String arguements
		char buffer[MAX_PATH];
		char* dirStr = _getcwd(buffer, sizeof(buffer));
		TCLAP::ValueArg<char*> dirArg("d", "dir", "Current Directory", false, dirStr, "Set the working directory");
		cmd.add(dirArg);
		
		TCLAP::SwitchArg relSwitch("r", "release", "Release build", cmd, false);
		TCLAP::SwitchArg verboseSwitch("v", "verbose", "Turn on/off verbose.", cmd, false);
		
		//Parse command line arguements
		cmd.parse(argc, argv);
		
		char *currentDir = dirArg.getValue();
		bool release = relSwitch.getValue();
		verbose = verboseSwitch.getValue();
		
		struct stat info;
		if(stat(currentDir, &info) != 0)
		{
			printf("Error: directory %s not found!\n", currentDir);
			exit(-1);
		}
		
		printf("Packing %s...\n\n", currentDir);
		PackScriptDir(currentDir);
		printf("Wrote changes to the script dll. Hash %u\n\n", GetFileCheckSum("./sc.dll"));
	} catch (TCLAP::ArgException &err)
	{
		std::cout << "Error: " << err.error() << "for arg " << err.argId() << std::endl;
		exit(-1);
	}

	return 0;
}