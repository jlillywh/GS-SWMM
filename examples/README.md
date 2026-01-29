# GoldSim-SWMM Bridge Examples

This directory contains example SWMM models demonstrating the bridge's capabilities.

## Example: Complete Control System

**File:** `complete_control_example.inp`

**Description:** A comprehensive example demonstrating all 3 types of dynamic inputs that can be controlled from GoldSim:

1. **Rain Gage** - Control rainfall intensity
2. **Pump** - Control pump operation (primary method for flow control)
3. **Node Inflow** - Control lateral inflow at a node

**Note:** The model also includes an orifice (OR1) and weir (W1) as passive hydraulic structures. Their flows can be monitored as outputs, but they cannot be controlled from GoldSim. They respond automatically to water levels according to their design geometry. **Use pumps for active flow control.**

### Model Structure

```
Rainfall (RG1) → Subcatchment (S1) → Junction (J1) → Storage (POND)
                                                           ↓
                                                      Pump (P1) → Junction (J2)
                                                           ↓
                                                    Orifice (OR1) → Junction (J2)
                                                           ↓
                                                      Weir (W1) → Junction (J2)
                                                                      ↓
                                              Lateral Inflow (DWF) → J2 → Outfall (OUT1)
```

### Storage Pond Configuration

**POND Storage Node:**
- Elevation: 100 ft
- Max Depth: 15 ft
- Max Water Surface: 115 ft
- Functional storage curve: 5,000 sq ft surface area
- Max Volume: 75,000 cubic feet

**Outlet Configuration:**
- **Orifice (OR1)**: Bottom outlet at elevation 100 ft (0 ft offset)
  - 1.5 ft diameter circular orifice
  - Primary outlet for normal operation
  - Passive hydraulic structure (responds to water level automatically)
  
- **Weir (W1)**: Emergency spillway at elevation 108 ft (8 ft crest height)
  - 2 ft high × 10 ft wide rectangular weir
  - Activates when pond depth > 8 ft (volume > 40,000 cu ft)
  - Passive hydraulic structure (responds to water level automatically)
  - Prevents pond from overtopping (max depth = 15 ft)
  
- **Pump (P1)**: Controlled outlet
  - Pumps from pond to J2
  - Capacity: 5 CFS at full speed
  - GoldSim controls speed (0.0 = off, 1.0 = full speed)
  - **This is the primary method for active flow control**

**Design Intent:**
- Normal operation: Orifice provides base flow, pump provides controlled discharge
- High water: Weir activates at 8 ft depth as emergency spillway
- Pump: Primary control mechanism for managing pond levels
- Large pond volume (75,000 cu ft) provides hydraulic stability and dampens oscillations

### SWMM Elements

**Inputs (4 total):**
- ElapsedTime (always index 0)
- RG1 - Rain gage (TIMESERIES DUMMY)
- P1 - Pump (Pcurve DUMMY)
- J2 - Node lateral inflow (DWF pattern DUMMY)

**Outputs (6 total):**
- POND - Storage volume (cubic feet)
- OUT1 - Outfall flow (CFS)
- OR1 - Orifice flow (CFS) - monitored only, not controlled
- W1 - Weir flow (CFS) - monitored only, not controlled
- P1 - Pump flow (CFS) - actual pumped flow rate
- S1 - Subcatchment runoff (CFS)

### How to Use This Example

1. **Generate the mapping file:**
   ```batch
   python generate_mapping.py examples\complete_control_example.inp
   ```

2. **Review the generated mapping:**
   ```batch
   type SwmmGoldSimBridge.json
   ```
   
   You should see:
   - Input count: 4 (elapsed time + 3 control inputs)
   - Output count: 6 (storage, outfall, orifice, weir, pump flow, subcatchment)

3. **Generate a diagram (optional):**
   ```batch
   python generate_diagram.py examples\complete_control_example.inp
   ```

4. **In GoldSim:**
   - Create External element
   - Set DLL: `GSswmm.dll`
   - Set function: `SwmmGoldSimBridge`
   - Configure 4 inputs and 6 outputs
   - Set time step to 5 minutes (matches ROUTING_STEP)

### GoldSim Input Connections

**Input 0 - ElapsedTime:**
- Connect to GoldSim's `ETime` element

**Input 1 - RG1 (Rainfall):**
- Units: inches/hour (model uses CFS)
- Example: `0.5` for moderate rainfall (recommended for testing)
- Example: `1.0` for steady rainfall
- Example: `If(ETime < 3600, 2.0, 0.0)` for 1-hour storm
- **Note:** High rainfall rates (>2 in/hr) may cause pond to fill rapidly

**Input 2 - P1 (Pump):**
- Units: dimensionless (0.0 = off, 1.0 = full speed)
- Pump capacity: 5 CFS at full speed
- Example: `If(POND_Volume > 40000, 1.0, 0.0)` for on/off control
- Example: `Min((POND_Volume - 30000) / 20000, 1.0)` for proportional control
- **This is the primary method for controlling outflow from the pond**

**Input 3 - J2 (Node Inflow):**
- Units: CFS (matches model FLOW_UNITS)
- Example: `0.0` for no lateral inflow (recommended for initial testing)
- Example: `5.0` for constant 5 CFS inflow
- Example: `If(ETime > 3600, 5.0, 0.0)` for delayed inflow

### GoldSim Output Usage

**Output 0 - POND (Volume):**
- Units: cubic feet
- Range: 0 to 75,000 cu ft (0 to 15 ft depth)
- Use for control logic (pump on/off based on storage)
- Typical operating range: 20,000-50,000 cu ft

**Output 1 - OUT1 (Outfall Flow):**
- Units: CFS
- Total system discharge

**Output 2 - OR1 (Orifice Flow):**
- Units: CFS
- Monitor passive orifice discharge (responds to water level automatically)

**Output 3 - W1 (Weir Flow):**
- Units: CFS
- Monitor emergency spillway activation (passive, responds to water level)
- Activates when pond depth > 8 ft (volume > 40,000 cu ft)
- Flow increases with water level above weir crest

**Output 4 - P1 (Pump Flow):**
- Units: CFS
- Monitor actual pumped flow rate
- Maximum: 5 CFS at full speed (setting = 1.0)
- Useful for verifying pump operation and calculating energy usage

**Output 5 - S1 (Subcatchment Runoff):**
- Units: CFS
- Monitor rainfall-runoff response

### Design Philosophy

**Active Control (Pumps):**
- Use pumps for any flow control you want to manage from GoldSim
- Pumps can be turned on/off or modulated based on storage levels, time, or other conditions
- Multiple pumps can provide staged control

**Passive Hydraulics (Orifices/Weirs):**
- Orifices and weirs respond automatically to water levels
- Design their geometry (size, crest height) for desired hydraulic performance
- Monitor their flows as outputs to understand system behavior
- Cannot be controlled externally - this is by design in SWMM

**Input 5 - J2 (Node Inflow):**
- Units: CFS (matches FLOW_UNITS)
- Example: `10.0` for constant 10 CFS inflow
- Example: `5.0 * Sin(ETime / 3600)` for time-varying inflow

### Expected Behavior

With recommended control settings:
1. Rainfall generates runoff from subcatchment S1 (~0.5 CFS per inch/hour of rain)
2. Runoff flows to storage pond POND
3. Pond volume increases gradually (large volume provides stability)
4. Orifice OR1 provides continuous base flow (increases with pond depth)
5. Pump P1 activates when pond reaches control threshold (e.g., 40,000 cu ft)
6. Weir W1 activates as emergency spillway when depth > 8 ft
7. All flows combine at junction J2 with lateral inflow
8. Combined flow exits through outfall OUT1

**Typical Flow Ranges:**
- Orifice flow: 0-15 CFS (depends on pond depth)
- Pump flow: 0-5 CFS (controlled by GoldSim)
- Weir flow: 0-100 CFS (emergency overflow, depth > 8 ft)
- Total outfall: 5-30 CFS under normal conditions

### Testing Scenarios

**Scenario 1: Steady Rainfall (Recommended for Initial Testing)**
- RG1 = 0.5 in/hr (moderate rainfall)
- P1 = `If(POND > 40000, 1.0, 0.0)` (pump on when half full)
- J2 = 0.0 (no lateral inflow)
- Expected: Pond fills to ~40,000-50,000 cu ft, pump cycles, stable operation

**Scenario 2: Pump Control Test**
- RG1 = 0.0 (no rain)
- P1 = 1.0 (pump on continuously)
- J2 = 0.0 (no lateral inflow)
- Expected: Pond drains via pump (5 CFS) and orifice, reaches equilibrium

**Scenario 3: High Water Event**
- RG1 = 2.0 in/hr (heavy rainfall)
- P1 = 1.0 (pump at full speed)
- J2 = 0.0 (no lateral inflow)
- Expected: Pond fills, weir activates at 8 ft depth, combined discharge 20-50 CFS

**Scenario 4: Proportional Pump Control**
- RG1 = 1.0 in/hr (steady rainfall)
- P1 = `Min(Max((POND - 30000) / 20000, 0), 1)` (proportional control)
- J2 = 0.0 (no lateral inflow)
- Expected: Smooth pump modulation, pond stabilizes at 30,000-50,000 cu ft

### Validation

After running in GoldSim, verify:
- POND volume increases during rainfall and decreases during pumping
- POND volume stays within 0-75,000 cu ft range
- Orifice flow (OR1) increases with pond depth (passive response)
- Weir flow (W1) activates when POND > 40,000 cu ft (8 ft depth)
- Pump flow (P1) matches your control setting (0-5 CFS)
- OUT1 flow equals sum of pump + orifice + weir + lateral inflow
- No wild oscillations (pond should change gradually, not jump between extremes)
- S1 runoff responds to rainfall (~0.5 CFS per inch/hour)

**Stability Check:**
If you see oscillations or unstable behavior:
- Reduce rainfall intensity (try 0.5 in/hr instead of 1.0+)
- Use proportional pump control instead of on/off
- Check that time step is 5 minutes (300 seconds)
- Verify pond volume is changing gradually (not jumping)

### Notes

- Model uses 5-minute routing step - GoldSim time step must match
- All flows are in CFS (cubic feet per second)
- Rainfall is in inches/hour (US customary units)
- Storage volume is in cubic feet
- Control settings are dimensionless (0.0 to 1.0)

### Troubleshooting

If the model doesn't run:
1. Validate the .inp file: `python generate_mapping.py examples\complete_control_example.inp`
2. Check for validation errors in the output
3. Verify all DUMMY references are correct
4. Ensure GoldSim time step matches ROUTING_STEP (5 minutes = 300 seconds)
5. Check that all input values are in correct units

If you see oscillations or instability:
1. **Reduce rainfall intensity** - Start with 0.5 in/hr, not 1.0+
2. **Use proportional pump control** - Avoid on/off switching
3. **Check pond volume range** - Should be 0-75,000 cu ft, changing gradually
4. **Verify weir activation** - Should only activate when POND > 40,000 cu ft
5. **Monitor time step** - Must be exactly 300 seconds (5 minutes)

**Common Issues:**
- **Pond fills too fast**: Reduce rainfall or increase pond size
- **Pump doesn't work**: Check that P1 setting is between 0.0 and 1.0
- **Weir flow too high**: This is normal when pond is very full (>8 ft depth)
- **Negative flows**: Check node elevations and link offsets
