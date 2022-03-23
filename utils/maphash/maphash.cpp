// map_hash.cpp : Defines the entry point for the console application.
// Program is a modified version of https://gist.github.com/xandout/8443812
//
#include <stdio.h>
#include <string>
#include <windows.h>
#include <vector>
#include <cstdio>

#include "../stream_safe.h"
#include "crc/crchash.h"
#include "rapidjson/document_safe.h"

using namespace std;
using namespace rapidjson;

string getFileExt(string fileName)
{
	size_t perd = fileName.find_last_of(".");
	string ext = fileName.substr(perd + 1);

	return ext;
}

//All of the hard work
vector<string> lsfiles(string folder)  //(c) http://stackoverflow.com/a/20847429/1009816
{
  char search_path[200];
  sprintf_s(search_path, "%s*.*", folder.c_str());
	
	vector<string> files;
  WIN32_FIND_DATA fd; 
  HANDLE hFind = ::FindFirstFile(search_path, &fd);
	
  if(hFind != INVALID_HANDLE_VALUE) 
  { 
    do 
    { 
      // read all (real) files in current folder, delete '!' read other 2 default folder . and ..
      if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) 
      {
      	files.push_back(fd.cFileName);
      }
     }while(::FindNextFile(hFind, &fd)); 
      ::FindClose(hFind); 
  }
	
  return files;
}

int main(int argc, char *argv[])
{
	string where;
	string output;
	
	for (int i = 0; i < argc; i++)
	{
		if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--dir"))
			where = argv[i+1] + string("\\");
			
		if (!strcmp(argv[i], "-o") || !strcmp(argv[i], "--out"))
			output = argv[i+1] + string("\\");
	}
	
	if (where.empty())
		where = "";
		
	if (output.empty())
		output = "";
	
	vector<string> list = lsfiles(where); //Get contents of directory
	
	Document doc;
	doc.SetObject();
	
	string fOutput = output+"maps.json";
	
	FILE* fp = fopen(fOutput.c_str(), "wb"); // non-Windows use "w"
	
	char writeBuffer[65536];
	FileWriteStream os(fp, writeBuffer, sizeof(writeBuffer));
	
	for(vector<string>::iterator t = list.begin(); t != list.end(); ++t)
	{
		if(getFileExt(t->c_str()) == "bsp")
		{
			string fullpath = where+t->c_str();
			cout << "Writing file hash for: " << t->c_str() << endl;
			doc.AddMember(Value(t->c_str(), doc.GetAllocator()), GetFileCheckSum(fullpath.c_str()), doc.GetAllocator());
		}
	}
	
	Writer<FileWriteStream> writer(os);
	doc.Accept(writer);
	
	fclose(fp);
	
	cout << "\n";
	cout << "Operation has finished. It is safe to close now." << endl;
	cout << "File Located in: " << fOutput.c_str() << endl;
	cout << "\n";
	cout << "Press 'enter' to close." << endl;
	cin.ignore();
	return 0;
}