# Build Configuration for GoldSim-SWMM Bridge DLL

## Overview

This document describes the build configuration for the GoldSim-SWMM Bridge DLL (GSswmm.dll). The project is configured to build a 64-bit Windows DLL that exports the `SwmmGoldSimBridge` function for use with GoldSim.

## Build Requirements

### Software Requirements

- **Visual Studio 2022** (Community, Professional, or BuildTools)
  - C++ Desktop Development workload
  - Windows 10 SDK
  - MSVC v145 or later build tools
  - x64 build tools

### Dependencies

- **SWMM 5.2 Library** (`swmm5.lib`)
  - Located in: `lib/swmm5.lib`
  - x64 version required
  
- **SWMM 5.2 Header** (`swmm5.h`)
  - Located in: `include/swmm5.h`
  
- **SWMM 5.2 DLL** (`swmm5.dll`)
  - Must be present at runtime
  - x64 version required

## Project Configuration

### Platform and Architecture

- **Platform**: x64 (64-bit)
- **Configuration**: Release (optimized for production)
- **Output Type**: Dynamic Link Library (DLL)
- **Character Set**: Unicode

### Build Settings

#### Compiler Settings (Release|x64)

```
Warning Level: Level3 (/W3)
Optimization: Full Optimization (/Ox)
Function-Level Linking: Yes (/Gy)
Intrinsic Functions: Yes (/Oi)
SDL Checks: Yes (/sdl)
Preprocessor Definitions: NDEBUG;_CONSOLE
Conformance Mode: Yes (/permissive-)
Additional Include Directories: $(ProjectDir);%(AdditionalIncludeDirectories)
```

#### Linker Settings (Release|x64)

```
Subsystem: Console
Generate Debug Information: Yes (for debugging if needed)
Additional Library Directories: $(ProjectDir)lib;%(AdditionalLibraryDirectories)
Additional Dependencies: swmm5.lib;%(AdditionalDependencies)
```

### File Structure

```
GSswmm/
├── SwmmGoldSimBridge.cpp    # Main source file
├── include/
│   └── swmm5.h              # SWMM API header
├── lib/
│   └── swmm5.lib            # SWMM import library (x64)
├── GSswmm.vcxproj           # Visual Studio project file
├── GSswmm.sln               # Visual Studio solution file
└── x64/
    └── Release/
        ├── GSswmm.dll       # Output DLL
        ├── GSswmm.lib       # Import library
        └── GSswmm.pdb       # Debug symbols
```

## Building the DLL

### Method 1: Using Visual Studio IDE

1. Open `GSswmm.sln` in Visual Studio 2022
2. Select **Release** configuration
3. Select **x64** platform
4. Build → Build Solution (or press Ctrl+Shift+B)
5. Output DLL will be in `x64/Release/GSswmm.dll`

### Method 2: Using MSBuild Command Line

```batch
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" ^
    GSswmm.sln ^
    /p:Configuration=Release ^
    /p:Platform=x64 ^
    /t:Rebuild ^
    /v:minimal
```

### Method 3: Using Developer Command Prompt

1. Open "x64 Native Tools Command Prompt for VS 2022"
2. Navigate to project directory
3. Run:
   ```
   msbuild GSswmm.sln /p:Configuration=Release /p:Platform=x64 /t:Rebuild
   ```

## Verifying the Build

### Check DLL Architecture

Use `dumpbin` to verify the DLL is 64-bit:

```batch
dumpbin /headers x64\Release\GSswmm.dll | findstr "machine"
```

Expected output:
```
            8664 machine (x64)
```

### Check DLL Exports

Use `dumpbin` to verify the exported function:

```batch
dumpbin /exports x64\Release\GSswmm.dll
```

Expected output should include:
```
    ordinal hint RVA      name
          1    0 00001234 SwmmGoldSimBridge
```

### Test the DLL

Run the integration test to verify functionality:

```batch
cd tests
copy ..\x64\Release\GSswmm.dll .
test_integration_treatment_train.exe
```

## Deployment

### Files Required for Deployment

1. **GSswmm.dll** - The bridge DLL
2. **swmm5.dll** - The SWMM engine DLL (x64)
3. **model.inp** - SWMM input file (user-provided)

### Deployment Steps

1. Copy `x64/Release/GSswmm.dll` to the GoldSim model directory
2. Copy `swmm5.dll` to the same directory
3. Ensure `model.inp` is in the same directory
4. Configure GoldSim External element to call `SwmmGoldSimBridge`

## Troubleshooting

### Build Errors

**Error: Cannot open include file 'swmm5.h'**
- Solution: Verify `include/swmm5.h` exists
- Check Additional Include Directories in project settings

**Error: Cannot open file 'swmm5.lib'**
- Solution: Verify `lib/swmm5.lib` exists and is x64
- Check Additional Library Directories in project settings

**Error: LNK2019: unresolved external symbol**
- Solution: Ensure swmm5.lib is x64 and matches swmm5.dll version
- Verify all SWMM API functions are available in the library

### Runtime Errors

**Error: The specified module could not be found**
- Solution: Ensure swmm5.dll is in the same directory as GSswmm.dll
- Or add swmm5.dll location to system PATH

**Error: The application was unable to start correctly (0xc000007b)**
- Solution: This indicates 32-bit/64-bit mismatch
- Verify all DLLs (GSswmm.dll, swmm5.dll) are x64
- Verify GoldSim is 64-bit version

## Version Information

- **DLL Version**: 4.0 (Treatment Train with Flow Monitoring)
- **SWMM Version**: 5.2 (Build 5.2.4)
- **Platform**: Windows x64
- **Compiler**: MSVC v145 (Visual Studio 2022)

## Build Validation Checklist

- [ ] Project builds without errors
- [ ] Project builds without warnings
- [ ] Output DLL is x64 architecture
- [ ] SwmmGoldSimBridge function is exported
- [ ] DLL links correctly with swmm5.lib
- [ ] Integration test passes
- [ ] DLL loads in GoldSim
- [ ] All 7 outputs are returned correctly

## Continuous Integration

For automated builds, use MSBuild with the following command:

```batch
msbuild GSswmm.sln ^
    /p:Configuration=Release ^
    /p:Platform=x64 ^
    /t:Rebuild ^
    /v:minimal ^
    /m ^
    /nologo
```

Exit code 0 indicates successful build.

## Additional Notes

- The project uses static linking to the C runtime (/MT) to avoid runtime dependencies
- Debug symbols (PDB) are generated even in Release mode for troubleshooting
- The DLL is optimized for speed (/Ox) rather than size
- All SWMM API calls are made through the swmm5.lib import library
- The bridge maintains no global state between GoldSim realizations except during active simulation
