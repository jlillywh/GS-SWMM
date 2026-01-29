# Logging Configuration Guide

## Overview

The GoldSim-SWMM Bridge DLL (v4.0) includes a configurable logging system that writes to `bridge_debug.log`. You can control the verbosity to reduce log file size and focus on important information.

## Log Levels

The DLL supports 4 log levels:

| Level | Name | What Gets Logged |
|-------|------|------------------|
| 0 | OFF | No logging at all |
| 1 | ERROR | Only errors (failed operations, missing files, invalid data) |
| 2 | INFO | Errors + Important events (initialization, cleanup, major milestones) |
| 3 | DEBUG | Everything (all function calls, values, calculations, time steps) |

## Current Configuration

**Default Log Level: 2 (INFO)**

This provides a good balance:
- ✅ Logs initialization and cleanup
- ✅ Logs errors and failures
- ✅ Shows version information
- ❌ Skips detailed time-step calculations
- ❌ Skips individual value retrievals

## How to Change Log Level

### Option 1: Edit Source Code (Requires Rebuild)

1. Open `SwmmGoldSimBridge.cpp` in a text editor
2. Find this line near the top (around line 30):
   ```cpp
   #define LOG_LEVEL 2  // Default: INFO level
   ```
3. Change the number to your desired level:
   ```cpp
   #define LOG_LEVEL 1  // For ERROR only
   #define LOG_LEVEL 0  // To disable logging
   #define LOG_LEVEL 3  // For full DEBUG logging
   ```
4. Rebuild the DLL:
   ```batch
   rebuild_and_test.bat
   ```
5. Copy the new DLL to your GoldSim model directory

### Option 2: Quick Reference

**For Production Use (Minimal Logging):**
```cpp
#define LOG_LEVEL 1  // Errors only
```
- Log file will only grow when problems occur
- Recommended for long simulations or Monte Carlo runs

**For Development/Debugging:**
```cpp
#define LOG_LEVEL 3  // Full debug
```
- Logs every operation and value
- Useful for troubleshooting issues
- Log file will grow quickly

**To Disable Logging:**
```cpp
#define LOG_LEVEL 0  // No logging
```
- No log file created
- Slightly faster performance
- Use only when everything is working perfectly

## Log File Format

### With LOG_LEVEL 2 (INFO) - Recommended

```
=======================================================
GoldSim-SWMM Bridge DLL v4.0
Log Level: 2 (0=OFF, 1=ERROR, 2=INFO, 3=DEBUG)
=======================================================

[10:15:23.456] [INFO ] XF_INITIALIZE: Input file: model.inp
[10:15:23.789] [INFO ] XF_INITIALIZE: Success - SWMM is now running
[10:15:24.012] [INFO ] XF_REP_VERSION: Returning version 4.0
[10:15:24.034] [INFO ] XF_REP_ARGUMENTS: Returning 2 inputs, 7 outputs
[10:17:45.678] [INFO ] XF_CALCULATE: Cleanup successful, simulation complete
```

### With LOG_LEVEL 3 (DEBUG) - Verbose

```
=======================================================
GoldSim-SWMM Bridge DLL v4.0
Log Level: 3 (0=OFF, 1=ERROR, 2=INFO, 3=DEBUG)
=======================================================

[10:15:23.456] [DEBUG] === Method Called: 0 ===
[10:15:23.457] [DEBUG] XF_INITIALIZE: Starting initialization
[10:15:23.458] [INFO ] XF_INITIALIZE: Input file: model.inp
[10:15:23.459] [DEBUG] XF_INITIALIZE: Calling swmm_open()
[10:15:23.789] [DEBUG] XF_INITIALIZE: swmm_open() succeeded
[10:15:23.790] [DEBUG] XF_INITIALIZE: Subcatchment count = 1, using index = 0
[10:15:23.791] [DEBUG] XF_INITIALIZE: Calling swmm_start()
[10:15:23.850] [DEBUG] XF_INITIALIZE: Subcatchment S1 index = 0
[10:15:23.851] [DEBUG] XF_INITIALIZE: Bioswale ST1 index = 0
[10:15:23.852] [DEBUG] XF_INITIALIZE: Detention ST2 index = 1
[10:15:23.853] [DEBUG] XF_INITIALIZE: Retention ST3 index = 2
[10:15:23.854] [DEBUG] XF_INITIALIZE: Outfall J2 index = 3
[10:15:23.855] [DEBUG] XF_INITIALIZE: Link C1 index = 0
[10:15:23.856] [DEBUG] XF_INITIALIZE: Link C2 index = 1
[10:15:23.857] [DEBUG] XF_INITIALIZE: Link C3 index = 2
[10:15:23.858] [INFO ] XF_INITIALIZE: Success - SWMM is now running
[10:15:24.012] [DEBUG] === Method Called: 1 ===
[10:15:24.013] [DEBUG] XF_CALCULATE: Called
[10:15:24.014] [DEBUG] XF_CALCULATE: GoldSim ETime = 300.000000 seconds (0.003472 days), Rainfall input = 1.500000 in/hr
[10:15:24.015] [DEBUG] XF_CALCULATE: Calling swmm_setValue(GAGE_RAINFALL, 0, 1.500000)
[10:15:24.016] [DEBUG] XF_CALCULATE: Calling swmm_step()
[10:15:24.050] [DEBUG] XF_CALCULATE: swmm_step() returned 0, SWMM elapsed_time = 0.003472 days (5.00 minutes since last step)
[10:15:24.051] [DEBUG] XF_CALCULATE: Subcatchment S1 runoff discharge = 2.345678 CFS
[10:15:24.052] [DEBUG] XF_CALCULATE: Bioswale volume = 1234.567890 cu.ft
[10:15:24.053] [DEBUG] XF_CALCULATE: Detention volume = 2345.678901 cu.ft
[10:15:24.054] [DEBUG] XF_CALCULATE: Retention volume = 3456.789012 cu.ft
[10:15:24.055] [DEBUG] XF_CALCULATE: Link C1 flow (Bioswale to Detention) = 0.123456 CFS
[10:15:24.056] [DEBUG] XF_CALCULATE: Link C2 flow (Detention to Retention) = 0.234567 CFS
[10:15:24.057] [DEBUG] XF_CALCULATE: Link C3 flow (final discharge) = 0.345678 CFS
... (continues for every time step)
```

### With LOG_LEVEL 1 (ERROR) - Minimal

```
=======================================================
GoldSim-SWMM Bridge DLL v4.0
Log Level: 1 (0=OFF, 1=ERROR, 2=INFO, 3=DEBUG)
=======================================================

(No entries unless an error occurs)
```

If an error occurs:
```
[10:15:23.456] [ERROR] XF_INITIALIZE: swmm_open() failed with code 200
```

## Log File Location

The log file is created in the **same directory as the DLL**:
- If running from GoldSim: `YourModelDirectory/bridge_debug.log`
- If running tests: `tests/bridge_debug.log`

## Log File Management

### Automatic Cleanup

The log file is **overwritten** each time you start a new GoldSim simulation (when XF_INITIALIZE is called). This prevents unlimited growth.

### Manual Cleanup

If you want to clear the log manually:
1. Close GoldSim
2. Delete `bridge_debug.log`
3. Restart GoldSim

### For Long Simulations

If running very long simulations or Monte Carlo with many realizations:
- Use `LOG_LEVEL 1` (ERROR only)
- Or use `LOG_LEVEL 0` (no logging)
- This prevents the log file from growing too large

## Troubleshooting with Logs

### When to Use Each Level

**Use LOG_LEVEL 1 (ERROR):**
- Production runs
- Monte Carlo simulations
- When everything is working
- When you only want to know if something breaks

**Use LOG_LEVEL 2 (INFO):**
- Normal development
- Verifying model setup
- Confirming initialization
- Checking version information
- **This is the recommended default**

**Use LOG_LEVEL 3 (DEBUG):**
- Investigating calculation issues
- Debugging time synchronization
- Verifying flow values
- Understanding detailed behavior
- Troubleshooting with support

## Version Information in Logs

The log file header always shows:
- DLL version number (4.0)
- Current log level setting
- Helps identify which DLL version you're running

Example:
```
=======================================================
GoldSim-SWMM Bridge DLL v4.0
Log Level: 2 (0=OFF, 1=ERROR, 2=INFO, 3=DEBUG)
=======================================================
```

## Performance Impact

| Log Level | Performance Impact | File Size (2-hour simulation) |
|-----------|-------------------|-------------------------------|
| 0 (OFF) | None | 0 bytes |
| 1 (ERROR) | Negligible | < 1 KB (if no errors) |
| 2 (INFO) | Minimal | ~5-10 KB |
| 3 (DEBUG) | Small (~1-2%) | 500 KB - 5 MB |

For most applications, the performance difference is negligible. Use the level that gives you the information you need.

## Summary

**Quick Setup for Production:**
1. Set `LOG_LEVEL 2` (INFO) - this is the default
2. Run your model
3. Check `bridge_debug.log` if issues occur
4. Switch to `LOG_LEVEL 3` (DEBUG) only if you need detailed troubleshooting

**The log file now includes the DLL version (4.0) in the header, making it easy to verify which version you're running.**
