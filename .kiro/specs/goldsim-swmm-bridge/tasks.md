# Implementation Plan: GoldSim-SWMM Bridge DLL

## Overview

This implementation plan breaks down the GoldSim-SWMM Bridge DLL into discrete coding tasks for treatment train modeling. The approach follows an incremental development pattern: establish the core interface, implement lifecycle management with treatment train element identification, add multi-output data exchange, implement error handling, and add comprehensive testing. Each task builds on previous work and includes validation through tests.

## Tasks

- [x] 1. Set up project structure and core interface
  - Create main source file (SwmmGoldSimBridge.cpp) with entry point function
  - Define GoldSim method ID enumerations (XF_INITIALIZE, XF_CALCULATE, etc.)
  - Define status code enumerations (XF_SUCCESS, XF_FAILURE, etc.)
  - Implement basic switch-case dispatcher for method IDs
  - Add SWMM header includes and library linkage
  - _Requirements: 1.1, 1.2, 1.3_

- [x] 2. Implement version and argument reporting for treatment train
  - [x] 2.1 Update XF_REP_VERSION handler
    - Return version number 3.0 in outargs[0] (treatment train version)
    - Set status to XF_SUCCESS
    - _Requirements: 1.2_
  
  - [x] 2.2 Update XF_REP_ARGUMENTS handler
    - Return 1 input argument in outargs[0] (rainfall)
    - Return 5 output arguments in outargs[1] (catchment discharge, 3 volumes, final discharge)
    - Set status to XF_SUCCESS
    - _Requirements: 1.3_
  
  - [ ]* 2.3 Write unit tests for version and argument reporting
    - Test XF_REP_VERSION returns 3.0
    - Test XF_REP_ARGUMENTS returns (1, 5)
    - Test both set status to XF_SUCCESS
    - _Requirements: 1.2, 1.3_

- [x] 3. Implement global state management for treatment train
  - [x] 3.1 Define global state variables for treatment train elements
    - Add is_swmm_running boolean flag
    - Add subcatchment_index integer (for S1)
    - Add bioswale_index integer (for ST1)
    - Add detention_index integer (for ST2)
    - Add retention_index integer (for ST3)
    - Add outfall_index integer (for J2)
    - Add conduit_c3_index integer (for C3)
    - Add file path character arrays (input, report, output)
    - Add error_message_buffer static array
    - _Requirements: 9.1, 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 7.1, 7.2, 7.3_
  
  - [ ]* 3.2 Write property test for state invariant maintenance
    - **Property 8: State invariant maintenance**
    - **Validates: Requirements 9.1**
    - Generate random sequences of method calls
    - Verify is_swmm_running accurately reflects SWMM state
    - _Requirements: 9.1_

- [x] 4. Implement SWMM lifecycle management with treatment train element identification
  - [x] 4.1 Update XF_INITIALIZE handler for treatment train
    - Check if SWMM is already running; if so, call cleanup
    - Call swmm_open() with file paths
    - If successful, call swmm_start(1)
    - Retrieve element indices using swmm_getIndex():
      - subcatchment_index for "S1"
      - bioswale_index for "ST1"
      - detention_index for "ST2"
      - retention_index for "ST3"
      - outfall_index for "J2"
      - conduit_c3_index for "C3"
    - Validate all indices are non-negative
    - Set is_swmm_running = true on success
    - Handle errors and return appropriate status
    - _Requirements: 2.1, 2.2, 2.3, 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 9.2_
  
  - [x] 4.2 Update XF_CLEANUP handler
    - Check if is_swmm_running is true
    - If true, call swmm_end() then swmm_close()
    - Set is_swmm_running = false
    - Reset all element indices to 0
    - Return XF_SUCCESS
    - _Requirements: 2.4, 2.5, 9.4_
  
  - [ ]* 4.3 Write unit tests for lifecycle management with treatment train
    - Test initialize opens, starts SWMM, and retrieves all element indices
    - Test cleanup ends, closes SWMM, and resets indices
    - Test re-initialization while running
    - Test cleanup when not running
    - Test missing treatment train element causes failure
    - _Requirements: 2.1, 2.2, 2.3, 2.4, 2.5, 6.1, 6.2, 6.3, 6.4, 6.5, 6.6, 6.7, 9.2, 9.4_
  
  - [ ]* 4.4 Write property test for SWMM API error propagation
    - **Property 3: SWMM API errors propagate to status**
    - **Validates: Requirements 2.6, 8.1**
    - Mock various SWMM functions to return errors
    - Verify all errors result in XF_FAILURE or XF_FAILURE_WITH_MSG status
    - _Requirements: 2.6, 8.1_

- [x] 5. Checkpoint - Ensure lifecycle tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [x] 6. Implement time step synchronization and multi-output data exchange
  - [x] 6.1 Update XF_CALCULATE handler for treatment train outputs
    - Verify is_swmm_running is true; return XF_FAILURE if false
    - Read rainfall intensity from inargs[0]
    - Call swmm_setValue(swmm_GAGE_RAINFALL, 0, rainfall)
    - Call swmm_step(&elapsed_time) exactly once
    - Handle swmm_step return codes (0=continue, >0=end, <0=error)
    - Retrieve 5 hydraulic values:
      - catchment_discharge from swmm_SUBCATCH_RUNOFF
      - bioswale_volume from swmm_NODE_VOLUME (ST1)
      - detention_volume from swmm_NODE_VOLUME (ST2)
      - retention_volume from swmm_NODE_VOLUME (ST3)
      - final_discharge from swmm_LINK_FLOW (C3)
    - Write all 5 values to outargs[0-4] in correct order
    - _Requirements: 3.1, 3.2, 3.3, 3.4, 4.1, 4.2, 4.3, 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 9.3_
  
  - [ ]* 6.2 Write unit tests for calculate handler with treatment train
    - Test calculate before initialize returns failure
    - Test rainfall value passed to swmm_setValue
    - Test swmm_step called exactly once
    - Test all 5 hydraulic values retrieved and written to outargs
    - Test output array ordering is correct
    - Test simulation end handling
    - _Requirements: 3.1, 3.3, 3.4, 4.1, 4.2, 4.3, 5.1, 5.2, 5.3, 5.4, 5.5, 5.6, 9.3_
  
  - [ ]* 6.3 Write property test for one swmm_step per calculate call
    - **Property 4: One swmm_step per calculate call**
    - **Validates: Requirements 3.1**
    - Generate random calculate calls
    - Verify swmm_step called exactly once per call
    - _Requirements: 3.1_
  
  - [ ]* 6.4 Write property test for rainfall propagation
    - **Property 5: Rainfall values propagate to SWMM**
    - **Validates: Requirements 4.2**
    - Generate random rainfall values
    - Verify swmm_setValue called with correct property and value
    - _Requirements: 4.2_
  
  - [ ]* 6.5 Write property test for all treatment train hydraulic values retrieved
    - **Property 6: All treatment train hydraulic values retrieved**
    - **Validates: Requirements 5.1, 5.2, 5.3, 5.4, 5.5, 5.6**
    - Generate random SWMM states
    - Verify all 5 values retrieved and written in correct order
    - _Requirements: 5.1, 5.2, 5.3, 5.4, 5.5, 5.6_

- [x] 7. Implement error handling and reporting
  - [x] 7.1 Create HandleSwmmError helper function
    - Call swmm_getError() to retrieve error message
    - Store message in error_message_buffer
    - Ensure null termination
    - Cast outargs to ULONG_PTR* and store buffer address
    - Set status to XF_FAILURE_WITH_MSG (-1)
    - _Requirements: 8.1, 8.2, 8.4, 8.5_
  
  - [x] 7.2 Integrate error handling into all method handlers
    - Add HandleSwmmError calls for all SWMM API failures
    - Ensure cleanup on initialization errors
    - Add error handling for invalid state transitions
    - Add error handling for missing treatment train elements
    - _Requirements: 2.6, 5.7, 6.7, 8.1, 8.3_
  
  - [ ]* 7.3 Write unit tests for error handling
    - Test error message retrieval and formatting
    - Test cleanup on initialization failure
    - Test error message buffer address in outargs[0]
    - Test status set to XF_FAILURE_WITH_MSG
    - Test missing element error handling
    - _Requirements: 5.7, 6.7, 8.1, 8.2, 8.3, 8.4, 8.5_
  
  - [ ]* 7.4 Write property test for successful calls return success
    - **Property 1: Successful method calls return XF_SUCCESS**
    - **Validates: Requirements 1.4**
    - Generate random valid method calls
    - Mock SWMM to succeed
    - Verify status is always 0
    - _Requirements: 1.4_
  
  - [ ]* 7.5 Write property test for failed calls return error codes
    - **Property 2: Failed method calls return appropriate error codes**
    - **Validates: Requirements 1.5**
    - Generate error conditions
    - Verify status is >0 and <99, or -1 for messages
    - _Requirements: 1.5_
  
  - [ ]* 7.6 Write property test for error messages retrieved and formatted
    - **Property 10: Error messages retrieved and formatted**
    - **Validates: Requirements 8.2, 8.4**
    - Generate SWMM errors
    - Verify swmm_getError called and message returned correctly
    - _Requirements: 8.2, 8.4_

- [x] 8. Implement validation and configuration
  - [x] 8.1 Add file path validation
    - Validate file paths during XF_INITIALIZE
    - Check if input file exists
    - Return XF_FAILURE for invalid paths
    - _Requirements: 7.4, 7.5_
  
  - [ ]* 8.2 Write property test for file path validation
    - **Property 7: File path validation**
    - **Validates: Requirements 7.5**
    - Generate invalid file paths
    - Verify XF_INITIALIZE returns XF_FAILURE
    - _Requirements: 7.5_
  
  - [ ]* 8.3 Write property test for path resolution consistency
    - **Property 9: Path resolution consistency**
    - **Validates: Requirements 7.4**
    - Generate relative file paths
    - Verify consistent resolution across operations
    - _Requirements: 7.4_

- [x] 9. Checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

- [x] 10. Create test infrastructure and mocks
  - [x] 10.1 Create SWMM API mock implementation
    - Implement mock versions of swmm_open, swmm_start, swmm_step, etc.
    - Add call tracking and parameter verification
    - Add configurable return values for testing
    - Add support for treatment train element indices
    - _Requirements: All testing requirements_
  
  - [x] 10.2 Set up Google Test framework
    - Add gtest library to project
    - Create test runner main function
    - Configure build system for tests
    - _Requirements: All testing requirements_
  
  - [x] 10.3 Set up RapidCheck framework
    - Add RapidCheck library to project
    - Configure for minimum 100 iterations per property
    - Create property test utilities and generators
    - _Requirements: All property testing requirements_

- [x] 11. Integration and final validation
  - [x] 11.1 Create integration test with real SWMM treatment train model
    - Create test SWMM model with S1, ST1, ST2, ST3, J2, C3 elements
    - Test full lifecycle with actual SWMM engine
    - Verify all 5 hydraulic values are correct
    - Verify numerical results match expected treatment train behavior
    - _Requirements: All requirements_
  
  - [x] 11.2 Add build configuration
    - Configure Visual Studio project for x64 release build
    - Ensure proper linking with swmm5.lib
    - Verify DLL exports function correctly
    - _Requirements: 10.1, 10.2, 10.3, 10.4_
  
  - [ ]* 11.3 Write end-to-end integration tests
    - Test complete simulation sequence with treatment train
    - Test multiple realizations for Monte Carlo support
    - Test various SWMM model configurations
    - _Requirements: All requirements_
  
  - [ ]* 11.4 Write property test for multiple realization support
    - **Property 11: Multiple realization support**
    - **Validates: Requirements 11.1, 11.2, 11.3, 11.4**
    - Generate sequences of multiple realizations
    - Verify proper initialization, cleanup, state reset
    - Verify no memory leaks or performance degradation
    - _Requirements: 11.1, 11.2, 11.3, 11.4_

- [ ] 12. Final checkpoint - Ensure all tests pass
  - Ensure all tests pass, ask the user if questions arise.

## Notes

- Tasks marked with `*` are optional and can be skipped for faster MVP
- Each task references specific requirements for traceability
- Checkpoints ensure incremental validation at key milestones
- Property tests validate universal correctness properties with 100+ iterations
- Unit tests validate specific examples, edge cases, and error conditions
- Mock SWMM API allows fast unit testing without file system dependencies
- Integration tests with real SWMM validate end-to-end functionality
- Treatment train architecture requires 5 outputs: catchment discharge, bioswale volume, detention volume, retention volume, final discharge
- Version 3.0 indicates treatment train support (vs 1.0 for single output)
