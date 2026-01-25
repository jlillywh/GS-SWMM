# Quick Start Guide - Complete Control Example

This guide walks you through testing the complete control example that demonstrates all 5 input types.

## Step 1: Validate the SWMM Model

First, verify the model is valid:

```batch
python generate_mapping.py examples\complete_control_example.inp
```

**Expected Output:**
```
Processing: examples\complete_control_example.inp

Validating .inp file...
✓ No validation issues found

Discovered 6 input(s):
  [0] ElapsedTime (SYSTEM)
  [1] RG1 (GAGE)
  [2] P1 (PUMP)
  [3] OR1 (ORIFICE)
  [4] W1 (WEIR)
  [5] J2 (NODE)

Discovered 5 output(s):
  [0] POND (STORAGE - VOLUME)
  [1] OUT1 (OUTFALL - FLOW)
  [2] OR1 (ORIFICE - FLOW)
  [3] W1 (WEIR - FLOW)
  [4] S1 (SUBCATCH - RUNOFF)

Input count: 6
Output count: 5
```

✅ **Validation Passed** - All 5 input types discovered correctly!

## Step 2: Review the Mapping File

Open `SwmmGoldSimBridge.json` to see the interface definition:

```json
{
  "version": "1.0",
  "inp_file_hash": "...",
  "input_count": 6,
  "output_count": 5,
  "inputs": [
    {"index": 0, "name": "ElapsedTime", "object_type": "SYSTEM", "property": "ELAPSEDTIME"},
    {"index": 1, "name": "RG1", "object_type": "GAGE", "property": "RAINFALL"},
    {"index": 2, "name": "P1", "object_type": "PUMP", "property": "SETTING"},
    {"index": 3, "name": "OR1", "object_type": "ORIFICE", "property": "SETTING"},
    {"index": 4, "name": "W1", "object_type": "WEIR", "property": "SETTING"},
    {"index": 5, "name": "J2", "object_type": "NODE", "property": "LATFLOW"}
  ],
  "outputs": [
    {"index": 0, "name": "POND", "object_type": "STORAGE", "property": "VOLUME"},
    {"index": 1, "name": "OUT1", "object_type": "OUTFALL", "property": "FLOW"},
    {"index": 2, "name": "OR1", "object_type": "ORIFICE", "property": "FLOW"},
    {"index": 3, "name": "W1", "object_type": "WEIR", "property": "FLOW"},
    {"index": 4, "name": "S1", "object_type": "SUBCATCH", "property": "RUNOFF"}
  ]
}
```

## Step 3: Visualize the Model (Optional)

Generate a diagram to see the model structure:

```batch
python generate_diagram.py examples\complete_control_example.inp
```

This creates `examples\complete_control_example.mmd` which you can view in VS Code or at https://mermaid.live

## Step 4: Set Up GoldSim Model

### Create External Element

1. In GoldSim, insert an **External** element
2. Configure the element:
   - **DLL File**: Browse to `GSswmm.dll` (or use full path)
   - **Function Name**: `SwmmGoldSimBridge` (case-sensitive!)
   - **Number of Inputs**: 6
   - **Number of Outputs**: 5

### Configure Inputs

Create these input connections in GoldSim:

| Index | Name | Type | Units | Example Value | Description |
|-------|------|------|-------|---------------|-------------|
| 0 | ElapsedTime | System | seconds | `ETime` | GoldSim's built-in elapsed time |
| 1 | RG1_Rainfall | Rain Gage | in/hr | `2.5` | Constant rainfall or time-varying |
| 2 | P1_Pump | Pump | 0-1 | `If(POND_Volume > 5000, 1.0, 0.0)` | Pump on/off based on pond level |
| 3 | OR1_Orifice | Orifice | 0-1 | `0.5` | 50% open, or proportional control |
| 4 | W1_Weir | Weir | 0-1 | `If(POND_Volume > 8000, 1.0, 0.0)` | Emergency spillway |
| 5 | J2_Inflow | Node | CFS | `10.0` | Constant or time-varying inflow |

### Configure Outputs

Create these output elements in GoldSim:

| Index | Name | Type | Units | Description |
|-------|------|------|-------|-------------|
| 0 | POND_Volume | Storage | ft³ | Pond storage volume |
| 1 | OUT1_Flow | Outfall | CFS | Total outflow from system |
| 2 | OR1_Flow | Orifice | CFS | Flow through orifice |
| 3 | W1_Flow | Weir | CFS | Flow over weir |
| 4 | S1_Runoff | Subcatchment | CFS | Runoff from subcatchment |

### Set Simulation Parameters

- **Time Step**: 300 seconds (5 minutes) - MUST match SWMM ROUTING_STEP
- **Duration**: 21600 seconds (6 hours) - matches SWMM END_TIME
- **Realizations**: 1 (for initial testing)

### Set External Element Options

☑ **Run Cleanup after each realization** (REQUIRED)
☑ **Run in separate process space** (REQUIRED)
☐ Unload DLL after each use (optional)

## Step 5: Test Scenarios

### Scenario A: Rainfall Only (Baseline)

Test basic rainfall-runoff response:

```
Input Values:
- ElapsedTime: ETime
- RG1_Rainfall: 2.5 in/hr
- P1_Pump: 0.0 (off)
- OR1_Orifice: 0.5 (50% open)
- W1_Weir: 0.0 (closed)
- J2_Inflow: 0.0 CFS

Expected Results:
- S1_Runoff increases during rainfall
- POND_Volume increases as runoff enters
- OR1_Flow increases as pond fills
- OUT1_Flow equals OR1_Flow (only outlet active)
```

### Scenario B: Pump Control

Test pump operation:

```
Input Values:
- ElapsedTime: ETime
- RG1_Rainfall: 5.0 in/hr (heavy rain)
- P1_Pump: If(POND_Volume > 5000, 1.0, 0.0)
- OR1_Orifice: 0.0 (closed)
- W1_Weir: 0.0 (closed)
- J2_Inflow: 0.0 CFS

Expected Results:
- POND_Volume increases rapidly
- When POND_Volume > 5000, pump activates
- OUT1_Flow jumps when pump turns on
- POND_Volume stabilizes or decreases
```

### Scenario C: Proportional Control

Test proportional orifice control:

```
Input Values:
- ElapsedTime: ETime
- RG1_Rainfall: 3.0 in/hr
- P1_Pump: 0.0 (off)
- OR1_Orifice: Min(POND_Volume / 10000, 1.0)
- W1_Weir: 0.0 (closed)
- J2_Inflow: 0.0 CFS

Expected Results:
- OR1_Flow increases proportionally with POND_Volume
- Smooth control response (no on/off cycling)
- POND_Volume reaches equilibrium
```

### Scenario D: Emergency Conditions

Test all controls active:

```
Input Values:
- ElapsedTime: ETime
- RG1_Rainfall: 10.0 in/hr (extreme)
- P1_Pump: 1.0 (full speed)
- OR1_Orifice: 1.0 (fully open)
- W1_Weir: If(POND_Volume > 8000, 1.0, 0.0)
- J2_Inflow: 20.0 CFS

Expected Results:
- POND_Volume increases rapidly
- All outlets active (pump + orifice + weir)
- OUT1_Flow is sum of all flows
- System at maximum capacity
```

### Scenario E: Time-Varying Storm

Test dynamic rainfall pattern:

```
Input Values:
- ElapsedTime: ETime
- RG1_Rainfall: If(ETime < 3600, 5.0, If(ETime < 7200, 2.0, 0.0))
- P1_Pump: If(POND_Volume > 5000, 1.0, 0.0)
- OR1_Orifice: Min(Max((POND_Volume - 3000) / 2000, 0.0), 1.0)
- W1_Weir: If(POND_Volume > 8000, 1.0, 0.0)
- J2_Inflow: 5.0 * Sin(ETime / 3600)

Expected Results:
- Complex hydrograph response
- Multiple control activations
- Realistic storm simulation
```

## Step 6: Validation Checks

After running each scenario, verify:

1. **Mass Balance**: 
   - Total inflow ≈ Total outflow + Change in storage
   - Check: S1_Runoff + J2_Inflow ≈ OUT1_Flow + (POND_Volume change)

2. **Control Response**:
   - Pump flow appears when P1_Pump = 1.0
   - OR1_Flow responds to OR1_Orifice setting
   - W1_Flow appears when W1_Weir = 1.0

3. **Physical Limits**:
   - POND_Volume stays between 0 and 15000 ft³ (max depth = 15 ft)
   - All flows are non-negative
   - Control settings stay between 0.0 and 1.0

4. **Timing**:
   - Runoff lags rainfall (infiltration + routing time)
   - Pond fills during rainfall, drains after
   - Controls activate at expected thresholds

## Step 7: Troubleshooting

### Problem: "Cannot load DLL"
**Solution**: Copy `GSswmm.dll` and `swmm5.dll` to your GoldSim model directory

### Problem: "Argument mismatch"
**Solution**: Verify input count = 6 and output count = 5 in GoldSim External element

### Problem: "Mapping file not found"
**Solution**: Copy `SwmmGoldSimBridge.json` to your GoldSim model directory

### Problem: All outputs are zero
**Solution**: 
- Check that GoldSim time step = 300 seconds (5 minutes)
- Verify input values are in correct units
- Check that "Run Cleanup after each realization" is enabled

### Problem: Simulation crashes
**Solution**:
- Enable "Run in separate process space"
- Check SWMM model runs successfully in SWMM GUI
- Verify all input values are reasonable (no negative flows, settings in 0-1 range)

### Problem: Unexpected results
**Solution**:
- Enable logging in `SwmmGoldSimBridge.cpp` (set ENABLE_LOGGING = true)
- Check `bridge_debug.log` for detailed execution trace
- Verify control logic in GoldSim matches intended behavior
- Test each control independently before combining

## Step 8: Advanced Usage

### Monte Carlo Analysis

Run multiple realizations with varying parameters:
- Rainfall intensity: Uniform(1.0, 10.0) in/hr
- Pump threshold: Uniform(4000, 6000) ft³
- Orifice setting: Uniform(0.3, 0.7)

### Optimization

Use GoldSim's optimization features to find optimal control strategies:
- Minimize peak outflow
- Minimize pond overflow risk
- Maximize pump efficiency

### Sensitivity Analysis

Test sensitivity to:
- Rainfall intensity
- Control thresholds
- Lateral inflow rates
- Initial pond level

## Files Checklist

Before running in GoldSim, ensure these files are in your model directory:

- [ ] `model.inp` - SWMM model (**must be named exactly "model.inp"**)
- [ ] `SwmmGoldSimBridge.json` - Mapping file (generated)
- [ ] `GSswmm.dll` - Bridge DLL
- [ ] `swmm5.dll` - SWMM engine
- [ ] Your GoldSim model file (`.gsm`)

**Note:** If your SWMM file has a different name (e.g., `complete_control.inp`), rename it to `model.inp` before running in GoldSim.

## Success Criteria

You've successfully set up the example when:

✅ Mapping file generates without errors
✅ All 6 inputs and 5 outputs are discovered
✅ GoldSim External element loads without errors
✅ Simulation runs to completion
✅ Outputs respond logically to inputs
✅ Mass balance is maintained
✅ Controls activate at expected thresholds

## Next Steps

Once the example works:

1. Modify control logic to test different strategies
2. Add more subcatchments or storage nodes
3. Implement complex control rules
4. Run Monte Carlo simulations
5. Optimize control parameters
6. Apply to your own SWMM models

## Support

If you encounter issues:

1. Check the main README.md for detailed documentation
2. Review the example README.md for model details
3. Enable logging and check bridge_debug.log
4. Verify SWMM model runs in SWMM GUI
5. Test with simpler scenarios first
6. Check that all files are in correct locations
