# GoldSim External DLL Interface Setup

## Quick Reference

**DLL Name**: `GSswmm.dll`  
**Function Name**: `SwmmGoldSimBridge`  
**Version**: 3.0 (Treatment Train)  
**Inputs**: 2  
**Outputs**: 5

---

## Files Required

Copy these files to your GoldSim model directory:

1. **`x64/Release/GSswmm.dll`** - The bridge DLL (64-bit)
2. **`swmm5.dll`** - The SWMM 5.2 engine DLL (64-bit)
3. **`model.inp`** - Your SWMM input file (use `tests/model_backup.inp` as template)

---

## GoldSim External Element Configuration

### Basic Settings

- **DLL File**: `GSswmm.dll`
- **Function Name**: `SwmmGoldSimBridge` (case-sensitive!)
- **DLL Type**: Standard External DLL

### Input Interface (2 inputs)

| Index | Name | Description | Units | Source in GoldSim |
|-------|------|-------------|-------|-------------------|
| 1 | ETime | Elapsed simulation time | seconds | Use `ETime(s)` system variable |
| 2 | Rainfall | Rainfall intensity | in/hr | Your rainfall time series or expression |

**Why ETime is needed**: GoldSim only calls the DLL when inputs change. Since ETime always changes, this ensures the DLL is called every time step, keeping SWMM synchronized with GoldSim.

### Output Interface (5 outputs)

| Index | Name | Description | Units | Use in GoldSim |
|-------|------|-------------|-------|----------------|
| 1 | Catchment_Discharge | Runoff from subcatchment S1 | CFS | Surface runoff to treatment train |
| 2 | Bioswale_Volume | Water stored in ST1 | cu.ft | First treatment stage volume |
| 3 | Detention_Volume | Water stored in ST2 | cu.ft | Second treatment stage volume |
| 4 | Retention_Volume | Water stored in ST3 | cu.ft | Third treatment stage volume |
| 5 | Final_Discharge | Flow from C3 to outfall J2 | CFS | Final discharge to environment |

---

## GoldSim Model Structure

### Recommended Setup

```
┌─────────────────────┐
│  System Variable    │
│  ETime(s)           │ ──┐
└─────────────────────┘   │
                          │
┌─────────────────────┐   │
│  Time Series or     │   │
│  Expression         │   │
│  Rainfall (in/hr)   │ ──┤
└─────────────────────┘   │
                          ▼
                  ┌───────────────────┐
                  │ External Element  │
                  │ GSswmm.dll        │
                  │ SwmmGoldSimBridge │
                  └───────────────────┘
                          │
        ┌─────────────────┼─────────────────┬─────────────────┬─────────────────┐
        ▼                 ▼                 ▼                 ▼                 ▼
┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐
│ Data Element │  │ Data Element │  │ Data Element │  │ Data Element │  │ Data Element │
│ Catchment    │  │ Bioswale     │  │ Detention    │  │ Retention    │  │ Final        │
│ Discharge    │  │ Volume       │  │ Volume       │  │ Volume       │  │ Discharge    │
└──────────────┘  └──────────────┘  └──────────────┘  └──────────────┘  └──────────────┘
```

### Step-by-Step Configuration

1. **Create External Element**
   - Right-click in GoldSim → Insert Element → External
   - Name it (e.g., "SWMM_Bridge")

2. **Configure DLL Settings**
   - DLL File: Browse to `GSswmm.dll`
   - Function Name: Type `SwmmGoldSimBridge` exactly
   - Click "Get Argument Info" to verify (should show 2 inputs, 5 outputs)

3. **Connect Inputs**
   - Input 1: Link to `ETime(s)` system variable
   - Input 2: Link to your rainfall time series/expression

4. **Connect Outputs**
   - Create 5 Data elements to receive the outputs
   - Link each output to its corresponding Data element

---

## Time Step Configuration

**CRITICAL**: Your GoldSim time step must match the SWMM ROUTING_STEP.

In the provided `model_backup.inp`:
```
ROUTING_STEP         00:01:00
```

This means **1 minute** time steps.

**GoldSim Settings**:
- Set your simulation time step to **1 minute**
- Or modify the SWMM model's ROUTING_STEP to match your desired GoldSim time step

---

## SWMM Model Requirements

Your SWMM input file (`model.inp`) must contain these elements:

### Required Elements

| Element Type | Name | Purpose |
|--------------|------|---------|
| Subcatchment | S1 | Receives rainfall, generates runoff |
| Storage Node | ST1 | Bioswale (first treatment stage) |
| Storage Node | ST2 | Detention (second treatment stage) |
| Storage Node | ST3 | Retention (third treatment stage) |
| Outfall | J2 | Final discharge point |
| Conduit | C1 | Connects ST1 to ST2 |
| Conduit | C2 | Connects ST2 to ST3 |
| Conduit | C3 | Connects ST3 to J2 (final discharge) |

### Model Configuration

- **Flow Units**: CFS (cubic feet per second)
- **Infiltration**: GREEN_AMPT (3 parameters: suction, conductivity, deficit)
- **Flow Routing**: DYNWAVE (for accurate hydraulics)
- **Routing Step**: Must match GoldSim time step

---

## Simulation Lifecycle

### Initialization (Automatic)
- Occurs on first time step of each realization
- DLL calls `swmm_open()` and `swmm_start()`
- Identifies all treatment train elements
- Returns error if any element is missing

### Calculate (Every Time Step)
- DLL receives ETime and Rainfall from GoldSim
- Sets rainfall in SWMM rain gage
- Calls `swmm_step()` to advance SWMM one time step
- Retrieves all 5 hydraulic values
- Returns values to GoldSim

### Cleanup (Automatic)
- Occurs at end of each realization
- DLL calls `swmm_end()` and `swmm_close()`
- Releases all SWMM resources
- Prepares for next realization

---

## Monte Carlo Simulation Support

The bridge fully supports Monte Carlo simulation:

- Each realization initializes SWMM independently
- State is completely reset between realizations
- No memory leaks or resource accumulation
- Tested with multiple realizations

**Example**: Run 1000 realizations with different rainfall patterns to analyze treatment train performance under uncertainty.

---

## Troubleshooting

### DLL Not Found
**Error**: "Cannot load DLL"  
**Solution**: 
- Verify `GSswmm.dll` is in the same directory as your GoldSim model
- Ensure you're using 64-bit GoldSim
- Check that `swmm5.dll` is also present

### Function Not Found
**Error**: "Cannot find function SwmmGoldSimBridge"  
**Solution**:
- Function name is case-sensitive: `SwmmGoldSimBridge`
- Verify DLL exports using: `dumpbin /exports GSswmm.dll`

### SWMM Initialization Fails
**Error**: "ERROR 200: one or more errors in input file"  
**Solution**:
- Check that `model.inp` is in the same directory
- Verify all required elements exist (S1, ST1, ST2, ST3, J2, C1, C2, C3)
- Check SWMM report file (`model.rpt`) for specific errors

### Element Not Found
**Error**: "Storage node ST1 (bioswale) not found"  
**Solution**:
- Element names are case-sensitive
- Verify your SWMM model has all required elements
- Check element names match exactly: S1, ST1, ST2, ST3, J2, C3

### Time Step Mismatch
**Symptom**: Results don't make sense, simulation unstable  
**Solution**:
- Ensure GoldSim time step = SWMM ROUTING_STEP
- Check both are in the same units (minutes, seconds, etc.)

### No Runoff Generated
**Symptom**: All outputs are zero  
**Solution**:
- Check subcatchment parameters (area, imperviousness, slope)
- Verify rainfall is being passed correctly
- Check infiltration parameters
- Review SWMM report file for warnings

---

## Verification Checklist

Before running your GoldSim model:

- [ ] GSswmm.dll is in GoldSim model directory
- [ ] swmm5.dll is in GoldSim model directory
- [ ] model.inp is in GoldSim model directory
- [ ] GoldSim is 64-bit version
- [ ] External element configured with 2 inputs, 5 outputs
- [ ] Function name is exactly: `SwmmGoldSimBridge`
- [ ] Input 1 connected to `ETime(s)`
- [ ] Input 2 connected to rainfall source
- [ ] All 5 outputs connected to Data elements
- [ ] GoldSim time step matches SWMM ROUTING_STEP
- [ ] SWMM model contains all required elements

---

## Example Rainfall Input

You can use various rainfall patterns in GoldSim:

### Constant Rainfall
```
Rainfall = 1.0  // 1 inch/hour constant
```

### Time-Varying Rainfall
```
Rainfall = if(ETime(hr) < 1, 2.0, 0.5)  // 2 in/hr first hour, then 0.5 in/hr
```

### Stochastic Rainfall (Monte Carlo)
```
Rainfall = RandNormal(1.0, 0.3)  // Mean 1.0 in/hr, StdDev 0.3
```

### Time Series
Import rainfall data from file or use GoldSim Time Series element

---

## Advanced Usage

### Accessing SWMM Report File
After simulation, check `model.rpt` in your GoldSim directory for:
- Detailed SWMM results
- Mass balance information
- Warnings and errors
- Continuity checks

### Accessing SWMM Output File
The `model.out` binary file contains complete time series data that can be viewed with:
- EPA SWMM GUI
- SWMM output viewing tools
- Custom post-processing scripts

### Debugging
Enable detailed logging by checking `bridge_debug.log` in your GoldSim directory. This file contains:
- All method calls
- Input/output values
- SWMM API calls
- Error messages

---

## Support and Documentation

- **Bridge Source**: `SwmmGoldSimBridge.cpp`
- **Build Guide**: `BUILD_CONFIGURATION.md`
- **Integration Tests**: `tests/INTEGRATION_TEST_README.md`
- **SWMM Documentation**: EPA SWMM 5.2 User Manual

---

## Version History

**Version 3.0** (Current)
- Treatment train support with 5 outputs
- 2 inputs: ETime and Rainfall
- Elements: S1, ST1, ST2, ST3, J2, C1, C2, C3
- Full Monte Carlo support

---

## Quick Start Example

1. Copy files to GoldSim directory:
   - `GSswmm.dll`
   - `swmm5.dll`
   - `model.inp` (from `tests/model_backup.inp`)

2. In GoldSim:
   - Insert External element
   - Set DLL: `GSswmm.dll`
   - Set Function: `SwmmGoldSimBridge`
   - Connect Input 1 to `ETime(s)`
   - Connect Input 2 to rainfall (e.g., `1.0` for constant 1 in/hr)
   - Create 5 Data elements for outputs

3. Set simulation:
   - Time step: 1 minute
   - Duration: 2 hours (or as needed)

4. Run and view results in the 5 output Data elements!

