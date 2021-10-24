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
			Print("Couldn't change to %s\n", argv[1]);
			return 1;
		}
		pszRoot = argv[1];
	}
	else pszRoot = CurrentDir;

	Print("Parsing %s...\n\n", pszRoot);

	PackScriptDir(pszRoot);
	Print("Wrote changes to the script dll.\n\n");

	return 0;
}

void Print(char *szFmt, ...)
{
	va_list		argptr;
	static char	string[1024];

	va_start(argptr, szFmt);
	vsprintf(string, szFmt, argptr);
	va_end(argptr);

	printf(string);
}