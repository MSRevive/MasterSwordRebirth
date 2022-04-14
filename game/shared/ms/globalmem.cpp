/*
	GlobalMem.cpp - Shared global memory control
*/

#include "msdllheaders.h"
#include "global.h"
#include "logger.h"

#ifdef VALVE_DLL
//#define TRACK_MEMORY		//Deterimines whether all memory allocations should be catalogued and dumped to file
#ifdef TRACK_MEMORY
memalloc_t* Allocations[500000];
int alloctotal = 0;
int allocid = 0;
int allochighest = 0;
memalloc_t MemAlloc;
bool FileDetails = false;
int AllocationStack = 0;
void DisableAllocateTrace() { AllocationStack++; }
void EnableAllocateTrace() { AllocationStack--; }
#endif
#endif

#define MS_FATAL_ERROR_MEM(MemErrMsg) Print("%s\n", MemErrMsg);

#ifdef DEV_BUILD
void* operator new(size_t size, const char* pszSourceFile, int LineNum)
{
#ifdef TRACK_MEMORY
	FileDetails = true;
	MemAlloc.SourceFile = pszSourceFile;
	MemAlloc.LineNum = LineNum;
#endif
	return operator new(size);
}
#endif

void* operator new(size_t size)
{
	try
	{
		void* pAddr = malloc(size);
		if (!pAddr)
		{
			MS_FATAL_ERROR_MEM("Out of Memory.  Couldn't Allocate New Block");
			return NULL;
		}

		memset(pAddr, 0, size); //Dogg: Initialize all new memory

#ifdef TRACK_MEMORY
		if (!AllocationStack)
		{
			if (!FileDetails)
			{
				MemAlloc.SourceFile = "No file";
				MemAlloc.LineNum = 0;
			}
			MemAlloc.pAddr = pAddr;
			MemAlloc.Used = true;

			//Use breakpoints here to track down leaking memory
			//Have them output to file, then trace them here by
			//their ID
			// if( allocid == 128027 )
			// 	int stop = 0;

			MemAlloc.Index = allocid++;
			if (MemAlloc.Index > allochighest)
				allochighest = MemAlloc.Index;
			if (allocid >= ARRAYSIZE(Allocations))
			{
				logfile << "Error: Alloc New Memory: Allocations exceed max debug size (" << ARRAYSIZE(Allocations) << ")" << endl;
				//int stop = 0;
				exit(1);
				return NULL;
			}

			DisableAllocateTrace();
			Allocations[alloctotal++] = new memalloc_t(MemAlloc);
			EnableAllocateTrace();
		}
		FileDetails = false;
#endif

		MemMgr::NewAllocation(pAddr, size);
		return pAddr;
	}
	catch (...)
	{
		//MS_FATAL_ERROR_MEM("Unhandled Exception While Allocating Memory")
		//MS_FATAL_ERROR_MEM( "Unhandled Exception While Allocating Memory" )
		//Thothie NOV2015_24 (post release) disable alloc memory pop-up
#ifndef VALVE_DLL
		msstring erloc = "client";
#else
		msstring erloc = "server";
		chatlog << "MSMEMORY: Unhandled Exception While Allocating Memory! (*operator new) " << erloc.c_str() << endl;
#endif
		Print("MSMEMORY: Unhandled Exception While Allocating Memory! (*operator new) [%s]\n", erloc.c_str());
		logfile << "MSMEMORY: Unhandled Exception While Allocating Memory! (*operator new) " << erloc.c_str() << endl;
	}
	return NULL;
}

#ifdef DEV_BUILD
void operator delete(void* ptr, const char* pszSourceFile, int LineNum) { delete ptr; }
#endif

void operator delete(void* ptr)
{
	try
	{
#ifdef TRACK_MEMORY
		if (!AllocationStack)
		{
			bool found = false;
			for (int i = 0; i < alloctotal; i++)
				if (Allocations[i]->pAddr == ptr)
				{
					found = true;
					DisableAllocateTrace();

					//Use this to track a certain memory deletion
					//if( Allocations[i]->Index == 127750 )
					// if( Allocations[i]->Index == 127750 )
					// 	int stop = 0;

					delete Allocations[i];
					if (i < alloctotal - 1)
						memcpy(&Allocations[i], &Allocations[i + 1], sizeof(void*) * (alloctotal - i - 1));
					alloctotal--;
					EnableAllocateTrace();
					break;
				}

			if (!found)
			{
				logfile << "Delete Memory: Tried to delete memory that wasn't allocated (" << Allocations[i]->SourceFile.c_str() << ":" << Allocations[i]->LineNum + ")" << endl;
				exit(1);
				return;
			}
		}
#endif

		MemMgr::NewDeallocation(ptr);
		free(ptr);
	}
	catch (...)
	{
		//MS_FATAL_ERROR_MEM( "Unhandled Exception While Deallocating Memory" )
		//Thothie NOV2015_24 (post release) disable alloc memory pop-up
#ifndef VALVE_DLL
		msstring erloc = "client";
#else
		msstring erloc = "server";
		chatlog << "MSMEMORY: Unhandled Exception While Deallocating Memory! (*operator delete) " << erloc.c_str() << endl;
#endif
		Print("MSMEMORY: Unhandled Exception While Deallocating Memory! (*operator delete) [%s]\n", erloc.c_str());
		logfile << "MSMEMORY: Unhandled Exception While Deallocating Memory! (*operator delete) " << erloc.c_str() << endl;
	}
}

void LogMemoryUsage(msstring_ref Title)
{
#ifdef TRACK_MEMORY
	logfile << Title << endl;
	logfile << "[Current Memory Allocations: " << alloctotal << "][Highest Ever: " << allochighest << "]" << endl;
	for (int i = 0; i < alloctotal; i++)
	{
		//if( Allocations[i]->Index == 124117 )
		logfile << "[Unfreed #" << i << "][" << Allocations[i]->Index << "] " << Allocations[i]->SourceFile.c_str() << " : " << Allocations[i]->LineNum << endl;
	}
#endif
}