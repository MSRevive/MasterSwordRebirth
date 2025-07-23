# Build and Debug Guide for MasterSwordRebirth

## Project Overview
This is a Half-Life 1 mod (Master Sword: Rebirth) that uses:
- CMake build system
- Visual Studio 2022 on Windows
- AngelScript for scripting (recently migrated to asbind20)
- Custom logging system (MSLogger)

## Building the Project

### Prerequisites
- Visual Studio 2022 with C++ development tools
- CMake (comes with VS2022)
- Windows SDK
- PowerShell

### Build Commands

#### Quick Build (Recommended)
```powershell
# From project root, run the build script:
powershell.exe -File run_server.ps1
```

This script:
1. Kills existing HLDS processes
2. Cleans build directories
3. Regenerates CMake files
4. Builds server and client DLLs
5. Copies to bin folder
6. Attempts to run the server

#### Manual Build
```powershell
# Generate build files
cmake -S . -B build -G "Visual Studio 17 2022" -A Win32

# Build server only
cmake --build build --config Release --target server

# Build client only  
cmake --build build --config Release --target client

# Build everything
cmake --build build --config Release
```

#### Build Output
- Server DLL: `build/src/game/server/Release/server.dll`
- Client DLL: `build/src/game/client/Release/client.dll`
- Final location: `bin/` directory

## Common Build Errors and Solutions

### 1. String Literal to char* Conversion Errors
**Error**: `cannot convert from 'const char [X]' to 'char *'`

**Solution**: Update function signatures to accept `const char*`:
```cpp
// Before
void SomeFunction(char* str);

// After  
void SomeFunction(const char* str);
```

Common locations:
- `TYPEDESCRIPTION.fieldName`
- `activity_map_t.name`
- `CBaseEntity::ThinkSet/TouchSet/UseSet`

### 2. asbind20 Type Registration Errors
**Error**: `use of undefined type 'CBaseEntity'`

**Solution**: Use dummy types for registration:
```cpp
// Create dummy types
namespace {
    struct CBaseEntity_Dummy { void* _dummy; };
    struct CBasePlayer_Dummy { void* _dummy; };
}

// Use macros to substitute
#define CBaseEntity CBaseEntity_Dummy
#define CBasePlayer CBasePlayer_Dummy
```

### 3. Method Signature Mismatches
**Error**: `cannot convert from 'Vector (__thiscall Vector::*)(const float)' to 'Vector &(__thiscall Vector::*)(float)'`

**Solution**: Fix return types for operators:
```cpp
// Before
inline Vector operator+=(const Vector &v) { return Vector(x += v.x, y += v.y, z += v.z); }

// After
inline Vector& operator+=(const Vector &v) { x += v.x; y += v.y; z += v.z; return *this; }
```

### 4. msstring Operator Ambiguity
**Error**: `'msstring::operator +': overloaded functions have similar conversions`

**Solution**: Explicitly cast or use proper msstring methods:
```cpp
// Instead of
msstring result = someString + charArray;

// Use
msstring result = someString;
result += charArray;
```

## Debugging Tips

### 1. Build Output Analysis
```powershell
# Save full build output
cmake --build build --config Release > build_output.txt 2>&1

# Filter for errors only
cmake --build build --config Release 2>&1 | findstr /i "error"

# Count errors
cmake --build build --config Release 2>&1 | findstr /c:"error C" | find /c /v ""
```

### 2. Incremental Debugging
When fixing multiple errors:
1. Fix one error type at a time
2. Focus on the first error in each file
3. Recompile frequently to verify fixes

### 3. AngelScript Debugging
Enable verbose logging in ASEntityBindings.cpp:
```cpp
#define MS_ANGEL_DEBUG(fmt, ...) // Already enabled
#define MS_ANGEL_INFO(fmt, ...)  // Already enabled
#define MS_ANGEL_ERROR(fmt, ...) // Already enabled
```

Check logs for:
- Type registration success/failure
- Method binding issues
- Cast operation results

### 4. Common File Locations
- **AngelScript bindings**: `src/game/shared/ms/angelscript/`
- **Entity system**: `src/game/server/hl/cbase.h`
- **Vector math**: `src/game/server/hl/vector.h`
- **String utilities**: `src/game/shared/ms/stackstring.h`
- **Build scripts**: Project root (`run_server.ps1`, `CMakeLists.txt`)

### 5. Parallel Build Issues
If parallel builds fail, force sequential:
```powershell
cmake --build build --config Release -j 1
```

### 6. Clean Rebuilds
When in doubt, clean everything:
```powershell
Remove-Item -Recurse -Force build
Remove-Item -Recurse -Force utils/build
Remove-Item -Path "bin/*.dll" -Force
```

## Project-Specific Notes

### Character Encoding Issues
Warning: `The file contains a character starting at offset 0x16 that is illegal`
- These are usually non-critical
- Often in edict.h or other engine headers
- Can be ignored unless they cause actual compilation failures

### Platform-Specific Code
Look for these preprocessor directives:
- `#ifdef VALVE_DLL` - Server-only code
- `#ifndef VALVE_DLL` - Client-only code
- `#ifdef _WIN32` - Windows-specific code

### Build System Files
- `CMakeLists.txt` - Main build configuration
- `src/game/server/CMakeLists.txt` - Server build config
- `src/game/client/CMakeLists.txt` - Client build config
- `cmake/*.cmake` - Platform toolchain files

## Quick Troubleshooting Checklist

1. ✓ Are you using the correct Visual Studio version? (2022)
2. ✓ Is the architecture set to Win32/x86? (Not x64)
3. ✓ Did you clean old build artifacts?
4. ✓ Are all string literals properly const-qualified?
5. ✓ Are dummy types defined for asbind20 registration?
6. ✓ Do all operator overloads return correct types?
7. ✓ Are forward declarations properly handled?

## Useful Commands for Development

```powershell
# Find all occurrences of an error pattern
Get-ChildItem -Recurse -Filter "*.cpp" | Select-String "pattern"

# Check which files include a header
Get-ChildItem -Recurse -Filter "*.cpp" | Select-String "#include.*headerfile"

# Find function definitions
Get-ChildItem -Recurse -Filter "*.cpp" | Select-String "functionName.*\{"

# List recently modified files
Get-ChildItem -Recurse | Where-Object {$_.LastWriteTime -gt (Get-Date).AddHours(-1)}
```