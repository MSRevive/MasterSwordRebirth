// 
// Scriptpack.cpp : Defines the entry point for the console application.
//

#include "cbase.h"

HANDLE g_resHandle;
char *pszRoot = NULL;

int main(int argc, char* argv[])
{
	char CurrentDir[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, CurrentDir);

	if (argc >= 2)
	{
		if (!SetCurrentDirectory(argv[1]))
		{
			printf("Couldn't change to %s\n", argv[1]);
			return 1;
		}
		pszRoot = argv[1];
	}
	else pszRoot = CurrentDir;

	printf("Parsing %s...\n\n", pszRoot);
	PackScriptDir(pszRoot);
	printf("Wrote changes to the script dll.\n\n");

	return 0;
}