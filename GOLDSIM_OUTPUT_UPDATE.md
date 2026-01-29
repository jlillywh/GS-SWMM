# GoldSim Interface Update - Version 4.0

## Summary

The SwmmGoldSimBridge DLL has been updated to version 4.0 to include flow monitoring between storage nodes in the treatment train.

## Changes Made

### New Outputs Added

The DLL now provides **7 outputs** (previously 5):

| Index | Output Name | Units | Description |
|-------|-------------|-------|-------------|
| 0 | Catchment_Discharge | CFS | Runoff from subcatchment S1 |
| 1 | Bioswale_Volume | cu ft | Current volume in ST1 |
| 2 | Detention_Volume | cu ft | Current volume in ST2 |
| 3 | Retention_Volume | cu ft | Current volume in ST3 |
| 4 | **C1_Flow** | CFS | **NEW: Flow from Bioswale (ST1) to Detention (ST2)** |
| 5 | **C2_Flow** | CFS | **NEW: Flow from Detention (ST2) to Retention (ST3)** |
| 6 | C3_Flow | CFS | Final discharge to Outfall (J2) |

### Code Changes

**SwmmGoldSimBridge.cpp:**
- Added initialization for `conduit_c1_index` and `conduit_c2_index` in XF_INITIALIZE
- Added validation checks for links C1 and C2
- Added flow retrieval for C1 and C2 in XF_CALCULATE
- Updated output array to include C1 and C2 flows
- Updated XF_REP_ARGUMENTS to report 7 outputs
- Updated XF_REP_VERSION to 4.0
- Enhanced logging for all link flows

## GoldSim Configuration Update

### What You Need to Do

1. **Open your GoldSim model**

2. **Navigate to the SWMM External Element**

3. **Update the Interface tab:**
   - Go to the "Outputs" section
   - You should now have **7 outputs** instead of 5
   - Add two new outputs:
     - Output 5: `C1_Flow` (Scalar, CFS)
     - Output 6: `C2_Flow` (Scalar, CFS)
   - Rename Output 5 (was final discharge) to Output 7: `C3_Flow`

4. **Verify the complete output list:**
   ```
   Output 1: Catchment_Discharge (CFS)
   Output 2: Bioswale_Volume (cu ft)
   Output 3: Detention_Volume (cu ft)
   Output 4: Retention_Volume (cu ft)
   Output 5: C1_Flow (CFS)          ← NEW
   Output 6: C2_Flow (CFS)          ← NEW
   Output 7: C3_Flow (CFS)
   ```

5. **Copy the new DLL:**
   - Copy `GSswmm.dll` from the project root to your GoldSim model directory
   - Overwrite the old version

6. **Test the model:**
   - Run a quick simulation
   - Verify all 7 outputs are populated
   - Check that C1_Flow and C2_Flow show realistic values

## Benefits

With these new outputs, you can now:
- Monitor flow between each stage of the treatment train
- Verify that orifices are controlling flow as expected
- See when weirs activate during high-flow events
- Better understand the hydraulic behavior of the system
- Create plots showing flow attenuation through the system

## SWMM Model Requirements

Your SWMM model must have:
- Links named **C1**, **C2**, and **C3** (can be orifices, conduits, or weirs)
- C1 connects ST1 → ST2
- C2 connects ST2 → ST3
- C3 connects ST3 → J2

The current configuration uses:
- Orifices (C1, C2, C3) for low-flow drainage
- Weirs (W1, W2, W3) for emergency overflow

## Files Updated

- `SwmmGoldSimBridge.cpp` - Main bridge code
- `README.md` - Documentation
- `BUILD_CONFIGURATION.md` - Build documentation
- `GSswmm.dll` - Compiled DLL (version 4.0)

## Backward Compatibility

**Important:** This is a breaking change. Models using version 3.0 will need to:
1. Update the GoldSim External Element to expect 7 outputs
2. Replace the DLL with version 4.0
3. Update any downstream calculations that reference output indices

## Questions?

If you encounter any issues:
1. Check that links C1, C2, C3 exist in your SWMM model
2. Verify the DLL version reports 4.0 (check GoldSim logs)
3. Review the bridge_debug.log file for detailed diagnostics
