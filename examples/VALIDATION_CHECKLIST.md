# Validation Checklist - Complete Control Example

Use this checklist to verify the example is working correctly before using it in production or presenting it.

## Pre-Flight Checks

### File Validation

- [ ] `complete_control_example.inp` exists in `examples/` directory
- [ ] File size is reasonable (should be ~8-10 KB)
- [ ] File opens in text editor without errors
- [ ] No special characters or encoding issues

### Parser Validation

Run: `python generate_mapping.py examples\complete_control_example.inp`

- [ ] Script completes without errors
- [ ] "✓ No validation issues found" message appears
- [ ] Discovers exactly 6 inputs (ElapsedTime + 5 controls)
- [ ] Discovers exactly 5 outputs (POND, OUT1, OR1, W1, S1)
- [ ] Input order is: ElapsedTime, RG1, P1, OR1, W1, J2
- [ ] Output order is: POND, OUT1, OR1, W1, S1
- [ ] `SwmmGoldSimBridge.json` is created
- [ ] JSON file is valid (can be opened in text editor)

### Mapping File Validation

Open `SwmmGoldSimBridge.json` and verify:

- [ ] `"version": "1.0"` is present
- [ ] `"input_count": 6` is correct
- [ ] `"output_count": 5` is correct
- [ ] `"inp_file_hash"` is present (32-character hex string)
- [ ] All 6 inputs are listed with correct properties:
  - [ ] ElapsedTime (SYSTEM, ELAPSEDTIME)
  - [ ] RG1 (GAGE, RAINFALL)
  - [ ] P1 (PUMP, SETTING)
  - [ ] OR1 (ORIFICE, SETTING)
  - [ ] W1 (WEIR, SETTING)
  - [ ] J2 (NODE, LATFLOW)
- [ ] All 5 outputs are listed with correct properties:
  - [ ] POND (STORAGE, VOLUME)
  - [ ] OUT1 (OUTFALL, FLOW)
  - [ ] OR1 (ORIFICE, FLOW)
  - [ ] W1 (WEIR, FLOW)
  - [ ] S1 (SUBCATCH, RUNOFF)

### SWMM Model Validation (Optional but Recommended)

If you have SWMM GUI installed:

- [ ] Open `complete_control_example.inp` in SWMM GUI
- [ ] Model loads without errors
- [ ] Run simulation with default settings
- [ ] Simulation completes successfully
- [ ] Check report file for warnings/errors
- [ ] Verify all elements are present in model

## GoldSim Setup Validation

### External Element Configuration

- [ ] External element created in GoldSim
- [ ] DLL path points to `GSswmm.dll`
- [ ] Function name is exactly `SwmmGoldSimBridge` (case-sensitive)
- [ ] Number of inputs set to 6
- [ ] Number of outputs set to 5
- [ ] "Run Cleanup after each realization" is checked
- [ ] "Run in separate process space" is checked

### Input Configuration

Verify each input is connected:

- [ ] Input 0: Connected to `ETime` element
- [ ] Input 1: Connected to rainfall source (RG1)
- [ ] Input 2: Connected to pump control (P1)
- [ ] Input 3: Connected to orifice control (OR1)
- [ ] Input 4: Connected to weir control (W1)
- [ ] Input 5: Connected to node inflow (J2)

### Output Configuration

Verify each output is captured:

- [ ] Output 0: POND volume element created
- [ ] Output 1: OUT1 flow element created
- [ ] Output 2: OR1 flow element created
- [ ] Output 3: W1 flow element created
- [ ] Output 4: S1 runoff element created

### Simulation Settings

- [ ] Time step = 300 seconds (5 minutes)
- [ ] Duration = 21600 seconds (6 hours) or longer
- [ ] Start time = 0
- [ ] Number of realizations = 1 (for testing)

### File Deployment

All required files in GoldSim model directory:

- [ ] `complete_control_example.inp`
- [ ] `SwmmGoldSimBridge.json`
- [ ] `GSswmm.dll`
- [ ] `swmm5.dll`
- [ ] Your GoldSim model file (`.gsm`)

## Test Scenario 1: Baseline (All Controls Off)

### Input Values
- ElapsedTime: `ETime`
- RG1_Rainfall: `0.0` in/hr
- P1_Pump: `0.0` (off)
- OR1_Orifice: `0.0` (closed)
- W1_Weir: `0.0` (closed)
- J2_Inflow: `0.0` CFS

### Expected Results
- [ ] Simulation runs to completion without errors
- [ ] All outputs remain at 0 (no inputs, no outputs)
- [ ] POND_Volume stays at 0
- [ ] No error messages in GoldSim log

## Test Scenario 2: Rainfall Only

### Input Values
- ElapsedTime: `ETime`
- RG1_Rainfall: `2.5` in/hr (constant)
- P1_Pump: `0.0` (off)
- OR1_Orifice: `0.5` (50% open)
- W1_Weir: `0.0` (closed)
- J2_Inflow: `0.0` CFS

### Expected Results
- [ ] Simulation runs to completion
- [ ] S1_Runoff increases from 0 (lag time ~5-10 minutes)
- [ ] POND_Volume increases over time
- [ ] OR1_Flow increases as pond fills
- [ ] OUT1_Flow equals OR1_Flow (only outlet active)
- [ ] W1_Flow remains 0 (weir closed)
- [ ] No negative values in any output

### Validation Checks
- [ ] S1_Runoff > 0 after ~10 minutes
- [ ] POND_Volume increases monotonically during rainfall
- [ ] OR1_Flow > 0 when POND_Volume > 0
- [ ] OUT1_Flow ≈ OR1_Flow (within 1%)
- [ ] Mass balance: Cumulative S1_Runoff ≈ POND_Volume + Cumulative OUT1_Flow

## Test Scenario 3: Pump Control

### Input Values
- ElapsedTime: `ETime`
- RG1_Rainfall: `5.0` in/hr (heavy rain)
- P1_Pump: `If(POND_Volume > 5000, 1.0, 0.0)`
- OR1_Orifice: `0.0` (closed)
- W1_Weir: `0.0` (closed)
- J2_Inflow: `0.0` CFS

### Expected Results
- [ ] Simulation runs to completion
- [ ] POND_Volume increases rapidly
- [ ] When POND_Volume crosses 5000 ft³, pump activates
- [ ] OUT1_Flow jumps when pump turns on
- [ ] POND_Volume growth slows or reverses after pump activation
- [ ] OR1_Flow remains 0 (orifice closed)
- [ ] W1_Flow remains 0 (weir closed)

### Validation Checks
- [ ] Pump activation occurs at POND_Volume ≈ 5000 ft³
- [ ] OUT1_Flow increases significantly when pump activates
- [ ] POND_Volume does not exceed maximum (15000 ft³)
- [ ] Pump flow is reasonable (check against pump curve)

## Test Scenario 4: All Controls Active

### Input Values
- ElapsedTime: `ETime`
- RG1_Rainfall: `10.0` in/hr (extreme)
- P1_Pump: `1.0` (full speed)
- OR1_Orifice: `1.0` (fully open)
- W1_Weir: `1.0` (fully open)
- J2_Inflow: `20.0` CFS

### Expected Results
- [ ] Simulation runs to completion
- [ ] All outputs are active (non-zero)
- [ ] OUT1_Flow is sum of pump + orifice + weir + lateral inflow
- [ ] POND_Volume increases but at slower rate (all outlets active)
- [ ] S1_Runoff is high due to extreme rainfall
- [ ] System operates at maximum capacity

### Validation Checks
- [ ] OUT1_Flow > OR1_Flow + W1_Flow (includes pump and lateral inflow)
- [ ] All flow values are positive
- [ ] POND_Volume stays within physical limits (0-15000 ft³)
- [ ] Mass balance maintained

## Test Scenario 5: Dynamic Control

### Input Values
- ElapsedTime: `ETime`
- RG1_Rainfall: `If(ETime < 3600, 5.0, 0.0)` (1-hour storm)
- P1_Pump: `If(POND_Volume > 5000, 1.0, 0.0)`
- OR1_Orifice: `Min(POND_Volume / 10000, 1.0)`
- W1_Weir: `If(POND_Volume > 8000, 1.0, 0.0)`
- J2_Inflow: `5.0 * Sin(ETime / 3600)`

### Expected Results
- [ ] Simulation runs to completion
- [ ] Complex hydrograph response
- [ ] Multiple control activations
- [ ] Realistic storm simulation behavior
- [ ] Pond fills during storm, drains after
- [ ] Controls activate/deactivate at thresholds

### Validation Checks
- [ ] Rainfall stops at ETime = 3600 seconds
- [ ] S1_Runoff decreases after rainfall stops
- [ ] POND_Volume peaks after rainfall stops (lag time)
- [ ] Controls respond to pond level changes
- [ ] System returns to near-zero state after 6 hours

## Performance Validation

### Execution Time
- [ ] Single realization completes in < 30 seconds
- [ ] No excessive delays or hanging
- [ ] Progress indicator updates smoothly

### Memory Usage
- [ ] GoldSim memory usage is reasonable (< 500 MB)
- [ ] No memory leaks (usage stable across realizations)

### Stability
- [ ] Multiple realizations complete successfully
- [ ] Results are consistent across runs (deterministic)
- [ ] No crashes or error dialogs

## Output Validation

### Physical Reasonableness
- [ ] All volumes are non-negative
- [ ] All flows are non-negative
- [ ] POND_Volume ≤ 15000 ft³ (max capacity)
- [ ] Control settings stay in 0-1 range
- [ ] Runoff responds to rainfall with appropriate lag

### Mass Balance
Calculate for entire simulation:
- Total Inflow = Cumulative S1_Runoff + Cumulative J2_Inflow
- Total Outflow = Cumulative OUT1_Flow
- Storage Change = Final POND_Volume - Initial POND_Volume

- [ ] Total Inflow ≈ Total Outflow + Storage Change (within 5%)

### Control Response
- [ ] Pump flow appears only when P1_Pump > 0
- [ ] OR1_Flow scales with OR1_Orifice setting
- [ ] W1_Flow appears only when W1_Weir > 0
- [ ] Controls respond immediately (no lag)

## Documentation Validation

### README Files
- [ ] `examples/README.md` is complete and accurate
- [ ] `examples/QUICKSTART.md` is complete and accurate
- [ ] `examples/VALIDATION_CHECKLIST.md` (this file) is complete
- [ ] All file paths in documentation are correct
- [ ] All commands in documentation work as written

### Code Comments
- [ ] SWMM .inp file has clear section headers
- [ ] DUMMY references are clearly marked
- [ ] Control rules are documented

## Final Checks

### Before Sharing
- [ ] All validation checks above pass
- [ ] Example runs successfully on clean GoldSim installation
- [ ] Documentation is clear and complete
- [ ] No temporary or debug files in examples/ directory
- [ ] All file paths use relative paths (not absolute)

### Before Announcing
- [ ] Example tested by at least one other person
- [ ] All discovered issues documented and resolved
- [ ] Performance is acceptable for demonstration
- [ ] Results are reproducible

### Production Ready
- [ ] Example works with latest DLL version
- [ ] All 5 input types demonstrated successfully
- [ ] Documentation matches actual behavior
- [ ] No known bugs or issues

## Sign-Off

Date: _______________

Validated by: _______________

Notes:
_______________________________________________________________________
_______________________________________________________________________
_______________________________________________________________________

Issues Found:
_______________________________________________________________________
_______________________________________________________________________
_______________________________________________________________________

Resolution:
_______________________________________________________________________
_______________________________________________________________________
_______________________________________________________________________
