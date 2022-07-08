#include <iostream>
#include <fstream>

#include "cbase.h"
#include "packer.h"
#include "../stream_safe.h"
#include "parser.h"

extern bool verbose;
extern bool release;
extern bool errFile;

//we grab all the files in the scripts directory to get ready for packing.
void Packer::readDirectory(char *pszName, bool cooked)
{
	WIN32_FIND_DATA wfd;
	HANDLE findHandle;
	
	char cSearchString[MAX_PATH];
	_snprintf(cSearchString, MAX_PATH, "%s\\*.*", pszName);
	
	if ((findHandle = FindFirstFile(cSearchString, &wfd)) == INVALID_HANDLE_VALUE) 
		return;

	storeFile(pszName, wfd, cooked);

	while (FindNextFile(findHandle, &wfd))
		storeFile(pszName, wfd, cooked);

	FindClose(findHandle);
}

void Packer::cookScripts() 
{
	if(release == true && m_StoredFiles.size() > 0)
	{	
		CMemFile inFile;
		for(size_t i = 0; i < m_StoredFiles.size(); i++)
		{
			msstring &fullPath = m_StoredFiles[i];
			if(inFile.ReadFromFile(fullPath))
			{
				char cRelativePath[MAX_PATH];
				strncpy(cRelativePath, &fullPath[strlen(m_RootDir) + 1], MAX_PATH);
				
				char createFile[MAX_PATH];
				strncpy(createFile, m_CookedDir, MAX_PATH);
				strncat(createFile, cRelativePath, MAX_PATH);
				
				if (verbose == true)
					printf("Cleaning script: %s\n", cRelativePath);
				
				//convert char array to std::string for parser.
				char *cstr((char*)inFile.m_Buffer);
				std::string data(cstr);
				
				Parser parser(data, cRelativePath);
				parser.stripComments();
				
				//we check for errors here because comments were already replaced.
				parser.checkQuotes(); //check for quote errors
				parser.checkBrackets(); //check for closing errors
				
				parser.stripTabs();
				parser.stripDebug();
				parser.stripEmptyLines();
				
				std::string result = parser.getResult();
				
				std::ofstream o(createFile);
				o << result << std::endl;
				o.close();
				
				//do error print at the end
				parser.printErrors();
				if (errFile)
					parser.saveErrors();
					
				if (verbose == true)
					printf("End script cleaning: %s\n", cRelativePath);
			}
		}
		
		//read and store cooked directory scripts
		readDirectory(m_CookedDir, true);
	}
}

void Packer::packScripts()
{
	//we want to make sc.dll in via root dir.
	char cWriteFile[MAX_PATH];
	_snprintf(cWriteFile, MAX_PATH, "%s\\sc.dll", m_OutDir);
	
	struct stat info;
	if(stat(cWriteFile, &info) == 0)
		std::remove(cWriteFile);
	
	CGroupFile GroupFile;
	try {
		GroupFile.Open(cWriteFile);
	}
	catch(...)
	{
		printf("Failed to create %s\n", cWriteFile);
		exit(-1);
	}
	
	if(release == true && m_CookedFiles.size() > 0)
	{
		CMemFile InFile;
		for (size_t i = 0; i < m_CookedFiles.size(); i++)
		{
			msstring &FullPath = m_CookedFiles[i];
			if (InFile.ReadFromFile(FullPath))
			{
				char cRelativePath[MAX_PATH];
				strncpy(cRelativePath, &FullPath[strlen(m_RootDir) + 1], MAX_PATH);
				
				if (verbose == true)
					printf("Doing file: %s\n", cRelativePath);
	
				if (!GroupFile.WriteEntry(cRelativePath, InFile.m_Buffer, InFile.m_BufferSize))
					printf("Failed to write entry: %s\n", cRelativePath);
			}
		}
	}
	else if(m_StoredFiles.size() > 0)
	{
		std::cout << "CookedFiles is 0 or release turned off." << "\n" << std::endl;
	
		CMemFile InFile;
		for (size_t i = 0; i < m_StoredFiles.size(); i++)
		{
			msstring &FullPath = m_StoredFiles[i];
			if (InFile.ReadFromFile(FullPath))
			{
				char cRelativePath[MAX_PATH];
				strncpy(cRelativePath, &FullPath[strlen(m_RootDir) + 1], MAX_PATH);
				
				if (verbose == true)
					printf("Doing file: %s\n", cRelativePath);
				
				//perform error check
				doErrorCheck((char*)InFile.m_Buffer, cRelativePath);
	
				if (!GroupFile.WriteEntry(cRelativePath, InFile.m_Buffer, InFile.m_BufferSize))
					printf("Failed to write entry: %s\n", cRelativePath);
			}
		}
	}
	else
		std::cout << "Failed to pack scripts, exiting" << std::endl;
	
	//close and flush GroupFile
	GroupFile.Flush();
	GroupFile.Close();
}

//store files info in array so we can process them at a later time.
void Packer::storeFile(char *pszCurrentDir, WIN32_FIND_DATA &wfd, bool cooked)
{
	char cFullPath[MAX_PATH];
	_snprintf(cFullPath, MAX_PATH, "%s\\%s", pszCurrentDir, wfd.cFileName);
	
	if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
	{		
		if (wfd.cFileName[0] != '.') 
			readDirectory(cFullPath, cooked); // Braces are neccesary
	}
	else if (strstr(wfd.cFileName, ".script") || !stricmp(wfd.cFileName, "items.txt"))
	{
		if (cooked)
			m_CookedFiles.add(cFullPath);
		else
			m_StoredFiles.add(cFullPath);
	}
}

void Packer::doErrorCheck(char *file, char *name)
{
	//convert char array to std::string for parser.
	char *cstr(file);
	std::string data(cstr);
	
	//we create a Parser object to access error checking stuff.
	Parser parser(data, name);
	//we have to strip comments to ignore them, cause laziness
	parser.stripComments();
	
	//we check for errors here because comments were already replaced and tabs removed.
	parser.checkQuotes(); //check for quote errors
	parser.checkBrackets(); //check for closing errors
	
	//do error print at the end
	parser.printErrors();
	if (errFile)
		parser.saveErrors();
}