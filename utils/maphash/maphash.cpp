// map_hash.cpp : Defines the entry point for the console application.
// Program is a modified version of https://gist.github.com/xandout/8443812
//
#include <stdio.h>
#include <string>
#include <iostream>
#include <windows.h>
#include <vector>
#include <ostream>
#include <fstream>

#include "../../MSShared/crc/crchash.h"

using namespace std;

//Simple struct to return from lsfiles
struct List {
	vector<string> files;
	vector<string> folders;
};

string getFileExt(string fileName)
{
	size_t perd = fileName.find_last_of(".");
	string ext = fileName.substr(perd + 1);

	return ext;
}

//All of the hard work
struct List lsfiles(string folder)  //(c) http://stackoverflow.com/a/20847429/1009816
{
  vector<string> files; //Will be added to List
	vector<string> folders; //Will be added to List
  char search_path[200];
  sprintf_s(search_path, "%s*.*", folder.c_str());
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
      } else 
			{
				//Put folders into vector
				folders.push_back(fd.cFileName);
			}
     }while(::FindNextFile(hFind, &fd)); 
      ::FindClose(hFind); 
  } 
	List me;
	me.files = files;
	me.folders = folders;
	
  return me;
}

int main(int argc, char *argv[])
{
	string where;
	if(argc > 1)
	{
		where = argv[1] + string("\\");
	}else
	{
		where = "";
	}
	List you = lsfiles(where); //Get contents of directory
	
	ofstream hashFile("maphash.txt"); //we open the maphash.txt file to write to.

	vector<string>::iterator files_begin = you.files.begin();
	vector<string>::iterator files_end = you.files.end();
	for(; files_begin != files_end; files_begin++)
	{
		if(getFileExt(*files_begin) == "bsp")
		{
			string s(*files_begin);
			cout << "Writing file hash for: " << *files_begin << endl; //we write to via text file our file hashes.
			hashFile << *files_begin << " " << GetFileCheckSumSize(s.c_str()) << endl;
		}
	}
	
	hashFile.close(); //we close the maphash.txt file since we no longer need to write to it.
	
	cout << "\n";
	cout << "Operation has finished. It is safe to close now." << endl;
	cout << "\n";
	cout << "Press 'enter' to close." << endl;
	cin.ignore();
	return 0;
}