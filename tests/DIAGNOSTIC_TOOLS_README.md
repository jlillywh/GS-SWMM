# Diagnostic Tools for GoldSim-SWMM Bridge

## Overview

This directory contains diagnostic tools to help troubleshoot issues with the GoldSim-SWMM Bridge DLL, particularly the common problem of zero runoff output.

## Available Tools

### 1. `diagnose_dll.py` - DLL Diagnostic Script

**Purpose**: Test the bridge DLL directly, mimicking what GoldSim does

**Usage**:
```bash
cd tests
python diagnose_dll.py
```

**Requirements**:
- Python 3.6+
- GSswmm.dll in tests directory
- swmm5.dll in tests directory
- model.inp in tests directory

**What it does**:
1. Loads the bridge DLL
2. Tests version reporting
3. Tests argument reporting
4. Tests initialization
5. Tests calculate with various rainfall values (0 to 100 in/hr)
6. Tests cleanup
7. Analyzes SWMM report file
8. Checks SWMM input file

**Output**: Detailed diagnostic report showing where the problem occurs

### 2. `diagnose_swmm_model.py` - SWMM Model Diagnostic Script

**Purpose**: Analyze SWMM input file for common configuration issues

**Usage**:
```bash
cd tests
python diagnose_swmm_model.py [model.inp]
```

**Requirements**:
- Python 3.6+
- SWMM input file (model.inp)

**What it checks**:
- OPTIONS section (flow units, routing step, etc.)
- SUBCATCHMENTS section (area, width, slope, etc.)
- SUBAREAS section (Manning's n, storage depth, etc.)
- INFILTRATION section (rates that might be too high)
- RAINGAGES section
- TIMESERIES section

**Output**: Detailed analysis with recommendations for fixes

### 3. `ZERO_RUNOFF_DIAGNOSTIC_GUIDE.md` - Comprehensive Guide

**Purpose**: Step-by-step guide to diagnose and fix zero runoff issues

**Contents**:
- Common causes of zero runoff
- Detailed diagnostic procedures
- Testing with minimal SWMM model
- Advanced debugging techniques
- Solutions summary

## Quick Start

If you're experiencing zero runoff:

### Step 1: Run DLL Diagnostic

```bash
cd tests
python diagnose_dll.py
```

Look for:
- ❌ Any failures in initialization
- ❌ All runoff values are zero
- ✓ What works correctly

### Step 2: Run SWMM Model Diagnostic

```bash
cd tests
python diagnose_swmm_model.py
```

Look for:
- ❌ No subcatchments defined
- ❌ Zero or negative parameters (area, width, slope)
- ❌ Extremely high infiltration rates
- ✓ Valid configuration

### Step 3: Check SWMM Report

Open `model.rpt` and look for:
- ERROR or WARNING messages
- Subcatchment Runoff Summary (should show runoff)
- Continuity errors

### Step 4: Follow the Guide

Read `ZERO_RUNOFF_DIAGNOSTIC_GUIDE.md` for detailed troubleshooting steps.

## Common Issues and Quick Fixes

### Issue 1: "No subcatchments defined"

**Fix**: Add subcatchment to model.inp:
```
[SUBCATCHMENTS]
;;Name           Rain Gage        Outlet           Area     %Imperv  Width    %Slope
S1               RG1              J1               10       50       500      0.5
```

### Issue 2: "Subcatchment area is zero"

**Fix**: Set area to positive value:
```
S1               RG1              J1               10       50       500      0.5
                                                   ^^
                                                   Set to 10 acres (or appropriate)
```

### Issue 3: "Time step mismatch"

**Fix**: Match GoldSim time step to SWMM ROUTING_STEP:
- SWMM: `ROUTING_STEP 0:05:00` → GoldSim: 5 minutes
- SWMM: `ROUTING_STEP 0:01:00` → GoldSim: 1 minute

### Issue 4: "High infiltration rate"

**Fix**: Reduce MaxRate in [INFILTRATION] section:
```
[INFILTRATION]
;;Subcatchment   MaxRate    MinRate    Decay      DryTime
S1               3.0        0.5        4          7
                 ^^^
                 Set to 3.0 in/hr (or lower for urban areas)
```

## Example Diagnostic Session

```bash
$ cd tests
$ python diagnose_dll.py

======================================================================
STEP 1: Loading DLL
======================================================================
✓ DLL loaded: GSswmm.dll
✓ Function 'SwmmGoldSimBridge' found

======================================================================
STEP 2: Testing Version Reporting
======================================================================
Method ID: 2 (XF_REP_VERSION)
Status: 0 ✓ SUCCESS
Version: 1.0

======================================================================
STEP 3: Testing Argument Reporting
======================================================================
Method ID: 3 (XF_REP_ARGUMENTS)
Status: 0 ✓ SUCCESS
Number of inputs: 1.0
Number of outputs: 1.0

======================================================================
STEP 4: Testing Initialization
======================================================================
✓ Found: model.inp
✓ Found: swmm5.dll

Calling XF_INITIALIZE...
Method ID: 0 (XF_INITIALIZE)
Status: 0 ✓ SUCCESS - SWMM initialized

======================================================================
STEP 5: Testing Calculate with Various Rainfall Values
======================================================================

Time Step 1:
  Rainfall:     0.00 in/hr
  Runoff:      0.0000 CFS
  Status:   0 ✓

Time Step 2:
  Rainfall:     1.00 in/hr
  Runoff:      0.0000 CFS
  Status:   0 ✓

Time Step 3:
  Rainfall:     5.00 in/hr
  Runoff:      0.0000 CFS
  Status:   0 ✓

...

======================================================================
RESULTS ANALYSIS
======================================================================
❌ PROBLEM IDENTIFIED: All runoff values are ZERO

Possible causes:
  1. SWMM model has no subcatchments
  2. Subcatchment parameters are invalid (zero area, etc.)
  3. Time step mismatch (SWMM not advancing)
  4. Infiltration rate too high (all rainfall infiltrates)
  5. SWMM simulation ended prematurely

Check the SWMM report file below for details...
```

## Interpreting Results

### DLL Diagnostic Results

**All tests pass, but runoff is zero**:
- Problem is in SWMM model configuration
- Run SWMM model diagnostic
- Check model.rpt for SWMM errors

**Initialize fails**:
- Check file paths (model.inp, swmm5.dll)
- Verify model.inp is valid SWMM file
- Check error message for details

**Calculate fails**:
- Check if SWMM is running
- Verify subcatchment index is valid
- Check SWMM report for errors

### SWMM Model Diagnostic Results

**No subcatchments found**:
- CRITICAL: Add subcatchments to model
- SWMM has nothing to simulate

**Zero or negative parameters**:
- CRITICAL: Fix area, width, or slope
- These must be positive values

**High infiltration rate**:
- WARNING: May cause zero runoff
- Reduce MaxRate in [INFILTRATION]

**No warnings**:
- Model configuration looks OK
- Problem may be in time synchronization
- Or in how bridge is being called

## Advanced Debugging

### Add Debug Output to DLL

Modify `SwmmGoldSimBridge.cpp`:

```cpp
case XF_CALCULATE:
    // Add debug logging
    FILE* debug_file;
    fopen_s(&debug_file, "bridge_debug.txt", "a");
    if (debug_file) {
        fprintf(debug_file, "Rainfall: %.4f, Runoff: %.4f\n", 
                inargs[0], runoff_value);
        fclose(debug_file);
    }
    // ... rest of code
```

Rebuild DLL and check `bridge_debug.txt` after running.

### Use Windows DebugView

1. Download DebugView from Sysinternals
2. Add OutputDebugString calls to DLL:
   ```cpp
   char msg[200];
   sprintf_s(msg, "DEBUG: Rainfall=%.2f\n", inargs[0]);
   OutputDebugStringA(msg);
   ```
3. Watch real-time output in DebugView

## Files in This Directory

```
tests/
├── diagnose_dll.py                    # DLL diagnostic script
├── diagnose_swmm_model.py             # SWMM model diagnostic script
├── ZERO_RUNOFF_DIAGNOSTIC_GUIDE.md    # Comprehensive troubleshooting guide
├── DIAGNOSTIC_TOOLS_README.md         # This file
├── GSswmm.dll                         # Bridge DLL (copy here)
├── swmm5.dll                          # SWMM engine (copy here)
└── model.inp                          # SWMM model (copy here)
```

## Support

If diagnostic tools don't resolve your issue:

1. **Collect information**:
   - Output from `diagnose_dll.py`
   - Output from `diagnose_swmm_model.py`
   - Contents of `model.rpt`
   - Your `model.inp` file

2. **Test SWMM independently**:
   - Open model.inp in SWMM GUI
   - Run simulation
   - Check if SWMM generates runoff

3. **Try minimal test model**:
   - Use example from ZERO_RUNOFF_DIAGNOSTIC_GUIDE.md
   - If minimal model works, problem is in your model
   - If minimal model fails, problem is in bridge/setup

## Tips

- **Start simple**: Use minimal SWMM model first
- **Test incrementally**: Add complexity one step at a time
- **Check SWMM first**: Verify model works in SWMM GUI
- **Match time steps**: Critical for proper synchronization
- **Check units**: Rainfall and runoff units must be consistent

## Version

**Diagnostic Tools Version**: 1.0  
**Compatible with**: GSswmm.dll v1.00  
**Last Updated**: 2025-01-16

---

**Need help?** Start with `diagnose_dll.py` and follow the output recommendations!
