# Changelog

All notable changes to the GSswmm Bridge project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [4.1] - 2026-01-29

### Added
- Comprehensive "Known Limitations" section in README documenting water quality API constraints
- Detailed explanation of SWMM5 API capabilities and limitations
- Workaround guidance for users requiring water quality data
- Complete version history in README changelog
- Release notes document (RELEASE_NOTES_v4.1.md)
- GitHub release preparation checklist

### Changed
- Updated DLL version from 4.0 to 4.1
- Expanded README changelog with all versions from 1.0 to 4.1
- Improved documentation clarity around API capabilities

### Technical Notes
- No code functionality changes
- All existing tests pass without modification
- Confirmed SWMM5 API does not expose pollutant concentrations during runtime
- Only hydraulic properties (flow, depth, volume) accessible via `swmm_getValue()`

## [4.0] - 2026-01-XX

### Added
- Support for multi-stage treatment train systems
- Seven output variables: catchment discharge, bioswale volume, detention volume, retention volume, C1 flow, C2 flow, C3 flow
- Comprehensive element validation during initialization
- Automatic element lookup by name (ST1, ST2, ST3, C1, C2, C3, J2)
- Enhanced debug logging with configurable log levels (ERROR, INFO, DEBUG)
- Detailed time synchronization logging

### Changed
- Expanded from 1 output to 7 outputs for treatment train monitoring
- Improved error handling with specific validation for each element
- Enhanced cleanup procedures with better error reporting

### Fixed
- Element validation now checks all required nodes and links before starting simulation
- Better error messages when elements are not found in SWMM model

## [3.0] - 2026-01-XX

### Added
- Support for multiple storage nodes (bioswale, detention, retention)
- Link flow tracking between treatment stages
- Element discovery using `swmm_getIndex()` API function

### Changed
- Expanded output from single runoff value to include storage volumes
- Updated GoldSim interface to support multiple outputs

## [2.0] - 2026-01-XX

### Added
- Storage volume output capability
- Comprehensive test suite with unit and integration tests
- Test framework using minimal gtest and rapidcheck headers
- Integration test for treatment train model

### Changed
- Enhanced API to retrieve node volumes using `swmm_getValue(swmm_NODE_VOLUME, index)`

### Testing
- Added unit tests for lifecycle management
- Added integration tests for multi-node systems
- Added property-based tests for validation

## [1.0] - 2026-01-XX

### Added
- Initial release of GSswmm Bridge DLL
- Basic SWMM-GoldSim integration
- Support for single subcatchment models
- Time-synchronized rainfall input
- Runoff output with automatic unit conversion
- GoldSim External Element API implementation
- Basic error handling and logging
- File validation (input file existence checks)

### Features
- XF_INITIALIZE: Load and start SWMM simulation
- XF_CALCULATE: Advance simulation one time step
- XF_REP_VERSION: Report DLL version
- XF_REP_ARGUMENTS: Report input/output argument counts
- XF_CLEANUP: Terminate simulation and release resources

### Inputs
- ETime: Elapsed simulation time (seconds)
- Rainfall: Rainfall intensity (in/hr or mm/hr)

### Outputs
- Runoff: Subcatchment runoff discharge (CFS, CMS, etc.)

### Documentation
- README with Quick Start guide
- API reference documentation
- Troubleshooting guide
- Build instructions

---

## Version Numbering

This project uses semantic versioning (MAJOR.MINOR.PATCH):

- **MAJOR**: Incompatible API changes or major feature additions
- **MINOR**: Backwards-compatible functionality additions
- **PATCH**: Backwards-compatible bug fixes

### Version History Summary

| Version | Type | Focus |
|---------|------|-------|
| 4.1 | Documentation | Water quality limitation clarification |
| 4.0 | Feature | Treatment train with 7 outputs |
| 3.0 | Feature | Multiple storage nodes and link flows |
| 2.0 | Feature | Storage volume output and testing |
| 1.0 | Initial | Basic SWMM-GoldSim integration |

## Links

- [GitHub Repository](https://github.com/yourusername/GSswmm)
- [Issue Tracker](https://github.com/yourusername/GSswmm/issues)
- [EPA SWMM](https://www.epa.gov/water-research/storm-water-management-model-swmm)
- [GoldSim](https://www.goldsim.com/)
