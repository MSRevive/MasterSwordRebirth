#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#if !defined(MAX_PATH)
#define MAX_PATH PATH_MAX
#endif

#include <algorithm>
#include <iterator>
#include <string>
#include <filesystem>
#include <iostream>