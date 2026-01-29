# GSswmm Bridge v4.1 - Release Summary

**Release Date**: January 29, 2026  
**Type**: Documentation Update  
**Stability**: Stable  

## Quick Facts

| Aspect | Details |
|--------|---------|
| **Version** | 4.1 |
| **Previous Version** | 4.0 |
| **Code Changes** | None (documentation only) |
| **Breaking Changes** | None |
| **Upgrade Required** | No |
| **Test Status** | All passing ✅ |

## What Changed

### Documentation Only Release

This release **does not change any code functionality**. It adds important documentation about SWMM API limitations.

### Key Addition: Water Quality Limitation

**The SWMM5 API cannot access pollutant concentrations during live simulation.**

This means:
- ❌ Cannot get TSS, BOD, or other pollutant values in real-time
- ✅ Can still get flow, depth, volume (hydraulic properties)
- ℹ️ This is an EPA SWMM engine limitation, not a bridge bug

## Who Should Upgrade

### You SHOULD upgrade if:
- You're starting a new project
- You want the latest documentation
- You need clarity on water quality capabilities
- You want the complete changelog

### You DON'T need to upgrade if:
- Your current v4.0 setup works fine
- You're not interested in water quality features
- You don't need updated documentation

## Files Changed

```
SwmmGoldSimBridge.cpp    - Version number only (4.0 → 4.1)
README.md                - Added "Known Limitations" section
README.md                - Expanded changelog
RELEASE_NOTES_v4.1.md    - New file (this release's details)
CHANGELOG.md             - New file (all versions)
```

## Installation

### New Installation
1. Download `GSswmm.dll` from GitHub releases
2. Download `swmm5.dll` dependency
3. Follow README Quick Start guide

### Upgrade from v4.0
1. Optional: Replace `GSswmm.dll` with v4.1
2. Read the new "Known Limitations" section in README
3. That's it! Your models will work exactly the same

## Technical Details

### API Investigation Results

We investigated whether SWMM5 API supports water quality data access:

**Checked**: `swmm5.h` header file (EPA SWMM 5.2)  
**Result**: No quality properties available

**Available Properties**:
```c
// Nodes
swmm_NODE_DEPTH
swmm_NODE_VOLUME
swmm_NODE_INFLOW
swmm_NODE_OVERFLOW

// Links
swmm_LINK_FLOW
swmm_LINK_DEPTH
swmm_LINK_VELOCITY
```

**NOT Available**:
```c
// These do not exist in the API
swmm_NODE_QUAL      ❌
swmm_NODE_QUALITY   ❌
swmm_LINK_QUAL      ❌
swmm_LINK_QUALITY   ❌
```

### Why This Matters

Users working on water quality projects (Example 6, TSS tracking, etc.) need to know:
1. Real-time pollutant data is not available through the bridge
2. Water quality results must be extracted from SWMM output files post-simulation
3. This is a fundamental SWMM API limitation

## Compatibility

| Component | Version | Status |
|-----------|---------|--------|
| GoldSim | 14+ | ✅ Compatible |
| SWMM | 5.2 | ✅ Compatible |
| Windows | x64 | ✅ Compatible |
| Visual Studio | 2022+ | ✅ For building |

## Testing Status

All tests pass without modification:

```
✅ Unit Tests
   - Lifecycle management
   - Error handling
   - File validation
   - Subcatchment validation

✅ Integration Tests
   - Treatment train model
   - Multi-node systems

✅ Property-Based Tests
   - Input validation
   - State transitions
```

## Support Resources

- **README.md**: Complete user guide with Quick Start
- **RELEASE_NOTES_v4.1.md**: Detailed release information
- **CHANGELOG.md**: Full version history
- **Known Limitations**: New section in README

## Next Steps for Users

### If You Need Water Quality Data

Since real-time access isn't available, use this workflow:

1. **Run SWMM Simulation**: Complete the full simulation
2. **Extract Data**: Use SWMM's output tools to read the `.out` file
3. **Import to GoldSim**: Load extracted data as time series or tables
4. **Analyze**: Use GoldSim's tools to analyze the imported quality data

### If You Only Need Hydraulic Data

Continue using the bridge as normal. All hydraulic properties work perfectly:
- Flow rates
- Water depths
- Storage volumes
- Inflows and overflows

## Questions?

- **Documentation**: See README.md
- **Issues**: Check GitHub Issues
- **API Details**: See `include/swmm5.h`
- **Examples**: See `tests/` directory

## Bottom Line

**v4.1 = v4.0 + Better Documentation**

No functional changes. Upgrade is optional but recommended for clarity.

---

**Download**: [GitHub Releases](https://github.com/yourusername/GSswmm/releases/tag/v4.1)  
**Full Notes**: See RELEASE_NOTES_v4.1.md  
**Changelog**: See CHANGELOG.md
