#pragma once

//if you want to profile memory, remove comment below
//#define PROFILER_MEMORY

#ifdef PROFILER_MEMORY
#include "Lib.Profiler/MemoryProfiler.h"
#endif



void showMemoryInfo(const char* info, const char* funcName, int line,bool bForce);
#define SHOWMEMORYINFOA(info)  showMemoryInfo(info,__FUNCTION__,__LINE__,false);
#define SHOWMEMORYINFOA_force(info)  showMemoryInfo(info,__FUNCTION__,__LINE__,true);
#define SHOWMEMORYINFO   SHOWMEMORYINFOA("null")
