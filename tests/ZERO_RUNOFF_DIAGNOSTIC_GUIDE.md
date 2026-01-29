# Zero Runoff Diagnostic Guide

## Problem Description

You're experiencing zero runoff output from the GoldSim-SWMM Bridge despite:
- Simulation running to completion
- High rainfall inputs (up to 100 in/hr)
- No error messages

This guide will help you diagnose and fix the issue.

## Quick Diagnostic Steps

### Step 1: Run Python Diagnostic Scripts

We've created two diagnostic scripts to help identify the problem:

#### Script 1: DLL Diagnostic (`diagnose_dll.py`)

This script mimics what GoldSim does and tests the DLL directly.

```bash
cd tests
python diagnose_dll.py
```

**What it checks:**
- DLL loads correctly
- Version and argument reporting work
- SWMM initializes successfully
- Calculate function is called correctly
- Runoff values at various rainfall rates
- SWMM report file for errors

#### Script 2: SWMM Model Diagnostic (`diagnose_swmm_model.py`)

This script analyzes your SWMM input file for common issues.

```bash
cd tests
python diagnose_swmm_model.py model.inp
```

**What it checks:**
- Subcatchments are defined
- Subcatchment parameters are valid (area > 0, etc.)
- Infiltration rates aren't too high
- Time steps are configured
- Rain gages are defined

### Step 2: Check SWMM Report File

Open `model.rpt` in a text editor and look for:

1. **Errors or Warnings**
   - Search for "ERROR" or "WARNING"
   - These indicate SWMM-specific problems

2. **Subcatchment Runoff Summary**
   - Should show non-zero runoff values
   - If all zeros, SWMM isn't generating runoff

3. **Continuity Errors**
   - Check "Runoff Quantity Continuity"
   - Should show rainfall and runoff volumes

## Common Causes of Zero Runoff

### Cause 1: No Subcatchments Defined

**Symptom**: SWMM has nothing to simulate

**Check**:
```bash
# Look for [SUBCATCHMENTS] section in model.inp
grep -A 10 "\[SUBCATCHMENTS\]" model.inp
```

**Fix**: Add at least one subcatchment to model.inp

**Example**:
```
[SUBCATCHMENTS]
;;Name           Rain Gage        Outlet           Area     %Imperv  Width    %Slope
S1               RG1              J1               10       50       500      0.5
```

### Cause 2: Invalid Subcatchment Parameters

**Symptom**: Subcatchment exists but has zero/negative values

**Check**: Run `diagnose_swmm_model.py` - it will flag invalid parameters

**Common issues**:
- Area = 0 (no drainage area)
- Width = 0 (infinite time of concentration)
- Slope = 0 (no flow)

**Fix**: Set valid positive values:
```
Area:  10 acres (or appropriate size)
Width: 500 ft (or calculate from area/length)
Slope: 0.5% (or site-specific value)
```

### Cause 3: Time Step Mismatch

**Symptom**: SWMM doesn't advance through time

**Check**:
1. Open `model.inp` and find ROUTING_STEP:
   ```
   [OPTIONS]
   ROUTING_STEP         0:05:00
   ```

2. Check GoldSim time step matches exactly:
   - SWMM: `0:05:00` = GoldSim: 5 minutes
   - SWMM: `0:01:00` = GoldSim: 1 minute

**Fix**: Make GoldSim time step equal to SWMM ROUTING_STEP

### Cause 4: Simulation Duration Too Short

**Symptom**: SWMM ends before runoff can develop

**Check**: Compare simulation duration to time of concentration

**Fix**: 
- Increase GoldSim simulation duration
- Or decrease SWMM routing step for faster response

### Cause 5: Extremely High Infiltration

**Symptom**: All rainfall infiltrates, no runoff generated

**Check**: Look at [INFILTRATION] section in model.inp

**Example of problem**:
```
[INFILTRATION]
;;Subcatchment   MaxRate    MinRate    Decay      DryTime
S1               100.0      10.0       4          7
```
If MaxRate > rainfall rate, all water infiltrates.

**Fix**: Use realistic infiltration parameters:
- Urban area: MaxRate = 3-5 in/hr
- Pervious area: MaxRate = 0.5-2 in/hr

### Cause 6: Rainfall Not Being Set

**Symptom**: Bridge isn't passing rainfall to SWMM

**Check**: Add debug output to bridge DLL

**In SwmmGoldSimBridge.cpp, add**:
```cpp
case XF_CALCULATE:
    // Add this debug line
    char debug[200];
    sprintf_s(debug, "DEBUG: Rainfall = %.2f, Subcatch = %d\n", 
              inargs[0], subcatchment_index);
    OutputDebugStringA(debug);
    
    // ... rest of calculate code
```

**View debug output**: Use DebugView from Sysinternals

### Cause 7: Wrong Subcatchment Index

**Symptom**: Bridge is setting rainfall on wrong subcatchment

**Check**: 
- Bridge uses subcatchment index 0 by default
- Verify your SWMM model has subcatchment at index 0

**Fix**: 
- Ensure first subcatchment in [SUBCATCHMENTS] is the one you want
- Or modify bridge to use correct index

## Detailed Diagnostic Procedure

### Phase 1: Verify SWMM Model Works Independently

1. **Open model.inp in SWMM GUI**
   - File → Open → Select model.inp
   
2. **Run simulation in SWMM**
   - Project → Run Simulation
   
3. **Check Status Report**
   - Report → Status
   - Look for errors or warnings
   
4. **View Subcatchment Results**
   - Report → Subcatchment → Runoff Summary
   - Should show non-zero runoff

**If SWMM GUI shows zero runoff**: Problem is in SWMM model, not bridge

**If SWMM GUI shows runoff**: Problem is in bridge or GoldSim setup

### Phase 2: Test Bridge DLL Directly

1. **Run diagnose_dll.py**:
   ```bash
   cd tests
   python diagnose_dll.py
   ```

2. **Check output for**:
   - ✓ DLL loads successfully
   - ✓ Initialize succeeds
   - ✓ Calculate returns non-zero runoff
   - ❌ Any failures or zero runoff

3. **If all tests pass but runoff is zero**:
   - Problem is likely in SWMM model
   - Check Phase 1 again

4. **If initialize fails**:
   - Check file paths
   - Verify swmm5.dll is present
   - Check model.inp is valid

### Phase 3: Check GoldSim Configuration

1. **Verify External Element Settings**:
   - Function name: `SwmmGoldSimBridge` (exact spelling!)
   - 1 input (rainfall)
   - 1 output (runoff)
   - "Run Cleanup after each realization" is checked

2. **Verify Time Step**:
   - GoldSim time step = SWMM ROUTING_STEP
   - Check units match (minutes, hours, etc.)

3. **Verify Rainfall Input**:
   - Rainfall values are non-zero
   - Rainfall is linked to External element input
   - Units are correct (inches/hour or mm/hour)

### Phase 4: Add Instrumentation

If problem persists, add debug output:

1. **Modify SwmmGoldSimBridge.cpp**:

```cpp
case XF_CALCULATE:
    // Log to file for debugging
    FILE* debug_file;
    fopen_s(&debug_file, "bridge_debug.txt", "a");
    if (debug_file) {
        fprintf(debug_file, "Calculate: rainfall=%.4f, runoff=%.4f\n", 
                inargs[0], runoff_value);
        fclose(debug_file);
    }
```

2. **Rebuild DLL**

3. **Run simulation**

4. **Check bridge_debug.txt**:
   - Verify rainfall values are being received
   - Check if runoff values are zero

## Testing with Minimal SWMM Model

Create a minimal test model to isolate the problem:

### Minimal model.inp

```
[TITLE]
Minimal Test Model

[OPTIONS]
FLOW_UNITS           CFS
INFILTRATION         HORTON
FLOW_ROUTING         KINWAVE
START_DATE           01/01/2024
START_TIME           00:00:00
END_DATE             01/01/2024
END_TIME             02:00:00
ROUTING_STEP         0:05:00
REPORT_STEP          0:05:00

[RAINGAGES]
RG1              INTENSITY  0:05     1.0      TIMESERIES TS1

[SUBCATCHMENTS]
;;Name           Rain Gage        Outlet           Area     %Imperv  Width    %Slope
S1               RG1              OUT1             10       50       500      0.5

[SUBAREAS]
;;Subcatchment   N-Imperv   N-Perv     S-Imperv   S-Perv     PctZero    RouteTo
S1               0.01       0.1        0.05       0.05       25         OUTLET

[INFILTRATION]
;;Subcatchment   MaxRate    MinRate    Decay      DryTime
S1               3.0        0.5        4          7

[OUTFALLS]
;;Name           Elevation  Type       
OUT1             0          FREE

[TIMESERIES]
;;Name           Date       Time       Value
TS1                         0:00       0.0
TS1                         0:30       5.0
TS1                         1:30       0.0
```

**Test this model**:
1. Save as `test_minimal.inp`
2. Run in SWMM GUI - should show runoff
3. Use with bridge - should show runoff
4. If still zero, problem is in bridge code

## Expected Runoff Values

For the minimal model above with 5 in/hr rainfall:

**Approximate runoff**:
- 10 acre catchment
- 50% impervious
- 5 in/hr rainfall for 1 hour
- Expected peak runoff: 5-15 CFS

**Runoff coefficient**:
- Impervious areas: ~0.9 (90% of rainfall becomes runoff)
- Pervious areas: ~0.1-0.3 (10-30% becomes runoff)
- Combined 50/50: ~0.5 (50% becomes runoff)

**If you see**:
- Runoff = 0: Problem exists
- Runoff = 0.1-1 CFS: Possible unit conversion issue
- Runoff = 5-15 CFS: Correct!
- Runoff > 50 CFS: Possible parameter error

## Advanced Debugging

### Use Windows DebugView

1. Download DebugView from Sysinternals
2. Run DebugView
3. Add OutputDebugString calls to bridge DLL
4. Watch real-time debug output

### Check SWMM API Calls

Add logging to verify SWMM API is being called:

```cpp
int error_code = swmm_setValue(swmm_SUBCATCH_RAINFALL, subcatchment_index, rainfall);
char msg[200];
sprintf_s(msg, "swmm_setValue returned: %d\n", error_code);
OutputDebugStringA(msg);
```

### Verify swmm_getValue Returns Data

```cpp
double runoff = swmm_getValue(swmm_SUBCATCH_RUNOFF, subcatchment_index);
char msg[200];
sprintf_s(msg, "swmm_getValue returned: %.6f\n", runoff);
OutputDebugStringA(msg);
```

## Solutions Summary

| Problem | Solution |
|---------|----------|
| No subcatchments | Add subcatchment to model.inp |
| Zero area | Set area > 0 (e.g., 10 acres) |
| Zero width | Set width > 0 (e.g., 500 ft) |
| Zero slope | Set slope > 0 (e.g., 0.5%) |
| Time step mismatch | Match GoldSim to SWMM ROUTING_STEP |
| High infiltration | Reduce MaxRate in [INFILTRATION] |
| Wrong subcatchment | Use index 0 or modify bridge |
| Rainfall not set | Check GoldSim input is linked |

## Next Steps

1. **Run both diagnostic scripts**
2. **Check SWMM report file**
3. **Test SWMM model in GUI**
4. **Try minimal test model**
5. **Add debug output if needed**

## Getting Help

If problem persists after trying these steps:

**Provide**:
1. Output from `diagnose_dll.py`
2. Output from `diagnose_swmm_model.py`
3. Contents of `model.rpt` (SWMM report)
4. GoldSim log file
5. Your `model.inp` file

This information will help identify the root cause.

---

**Remember**: The most common cause is invalid SWMM model parameters (zero area, width, or slope). Check these first!
