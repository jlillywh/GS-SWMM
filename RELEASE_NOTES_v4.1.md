# Release Notes - GSswmm Bridge v4.1

**Release Date**: January 29, 2026  
**Type**: Documentation Update  
**Status**: Stable

## Overview

Version 4.1 is a documentation-focused release that clarifies an important limitation of the SWMM5 API regarding water quality data access. No code changes were made to the DLL itself.

## What's New

### Documentation Enhancements

#### Water Quality Limitation Documentation
- Added comprehensive section explaining that pollutant concentrations (TSS, BOD, etc.) are **not accessible** during live simulation
- Documented which properties ARE available (hydraulic: flow, depth, volume)
- Explained the technical reason for this limitation (SWMM API design)
- Provided workaround guidance for users who need water quality data

#### Updated Changelog
- Expanded changelog to include all versions from 1.0 to 4.1
- Documented the evolution of features across versions
- Clarified what each version added to the bridge

## Technical Details

### API Investigation Results

We investigated the SWMM5 API (version 5.2) to determine if water quality data could be accessed during runtime. Our findings:

**Available via `swmm_getValue()`**:
- ✅ `swmm_NODE_DEPTH` - Water depth at nodes
- ✅ `swmm_NODE_VOLUME` - Storage volume
- ✅ `swmm_NODE_INFLOW` - Total inflow
- ✅ `swmm_LINK_FLOW` - Flow rate in links
- ✅ `swmm_LINK_DEPTH` - Water depth in links
- ✅ `swmm_LINK_VELOCITY` - Flow velocity

**NOT Available**:
- ❌ `swmm_NODE_QUAL` or `swmm_NODE_QUALITY` (does not exist)
- ❌ `swmm_LINK_QUAL` or `swmm_LINK_QUALITY` (does not exist)
- ❌ Any pollutant-specific properties

### Why This Matters

Users working on stormwater quality projects (like Example 6 with TSS tracking) need to understand that:
1. The bridge can only pass hydraulic data to GoldSim in real-time
2. Water quality results must be extracted from SWMM's output file after simulation
3. This is a limitation of the SWMM engine API, not the bridge implementation

## Breaking Changes

**None** - This is a documentation-only release. All existing models will continue to work exactly as before.

## Upgrade Instructions

### For Existing Users

No action required. Your existing GoldSim models will continue to work with v4.1 exactly as they did with v4.0.

### For New Users

Follow the Quick Start guide in the README.md. Be aware of the water quality limitation if your project requires pollutant tracking.

## Files Changed

| File | Change Type | Description |
|------|-------------|-------------|
| `SwmmGoldSimBridge.cpp` | Version bump | Updated version from 4.0 to 4.1 |
| `README.md` | Documentation | Added "Known Limitations" section |
| `README.md` | Documentation | Expanded changelog with all versions |
| `RELEASE_NOTES_v4.1.md` | New file | This document |

## Compatibility

- **GoldSim**: Version 14+ (unchanged)
- **SWMM**: Version 5.2 (unchanged)
- **Windows**: x64 (unchanged)
- **Visual Studio**: 2022 or later (for building from source)

## Known Issues

None. All issues from v4.0 remain resolved.

## Future Considerations

### Potential Future Enhancements

If EPA releases an updated SWMM API that exposes water quality data during runtime, we could add:
- Real-time pollutant concentration outputs
- Dynamic water quality parameter inputs
- Treatment efficiency calculations

However, this would require changes to the SWMM engine itself, which is outside the scope of this bridge project.

## Testing

All existing tests pass without modification:
- ✅ Unit tests (lifecycle, error handling, file validation)
- ✅ Integration tests (treatment train model)
- ✅ Property-based tests
- ✅ Subcatchment validation tests

No new tests were added since no code functionality changed.

## Support

For questions or issues:
1. Review the README.md "Known Limitations" section
2. Check the "Troubleshooting" section for common problems
3. Review test files in the `tests/` directory for examples
4. Open an issue on GitHub with detailed information

## Credits

- **Investigation**: Water quality API capability research
- **Documentation**: Comprehensive limitation documentation
- **Testing**: Verification that existing functionality remains intact

## Next Steps

For users who need water quality tracking:
1. Run SWMM simulation to completion
2. Use SWMM's built-in reporting tools or output file readers
3. Extract pollutant data from the `.out` binary file
4. Import extracted data into GoldSim as time series or lookup tables

This workflow allows you to use SWMM's water quality capabilities, just not in real-time coupling mode.

---

**Full Changelog**: v4.0...v4.1  
**Download**: See GitHub releases page
