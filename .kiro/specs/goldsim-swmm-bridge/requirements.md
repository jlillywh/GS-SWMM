# Requirements Document

## Introduction

This document specifies the requirements for a GoldSim-SWMM Bridge DLL that enables bidirectional communication between GoldSim simulation software and the EPA SWMM (Storm Water Management Model) hydraulic engine. The bridge allows GoldSim to control the simulation clock, provide rainfall inputs, and receive hydraulic calculations from SWMM for treatment train modeling. This enables integrated stochastic hydrologic-hydraulic modeling workflows, particularly for Monte Carlo simulation of contaminant transport through treatment systems.

## Glossary

- **GoldSim**: A probabilistic simulation software platform that supports external DLL integration through External elements
- **SWMM**: EPA Storm Water Management Model - a dynamic rainfall-runoff simulation model used for single event or long-term simulation of runoff quantity and quality from primarily urban areas
- **Bridge_DLL**: The dynamic link library that implements the interface between GoldSim and SWMM
- **External_Element**: GoldSim's mechanism for calling external DLL functions with specific method IDs
- **SWMM_Engine**: The SWMM computational engine accessed through the SWMM5 C API
- **Subcatchment**: A SWMM model component representing a drainage area that receives rainfall and generates runoff
- **Storage_Node**: A SWMM node with storage volume capacity, used to model detention/retention facilities
- **Treatment_Train**: A series of stormwater management facilities (bioswale, detention, retention) that progressively treat runoff
- **Method_ID**: Integer value passed by GoldSim to indicate which operation the DLL should perform (initialize, calculate, report version, report arguments, cleanup)
- **Simulation_State**: The current lifecycle phase of the SWMM engine (uninitialized, opened, started, running, ended, closed)
- **Time_Step**: The simulation time increment at which calculations are performed
- **Rainfall_Intensity**: Precipitation rate provided by GoldSim to SWMM subcatchments (typically in inches/hour or mm/hour)
- **Runoff_Flow**: The calculated surface runoff from a subcatchment (typically in CFS or CMS)
- **Storage_Volume**: The volume of water stored in a storage node (typically in cubic feet or cubic meters)
- **Conduit**: A SWMM link element representing a pipe or channel that conveys flow between nodes
- **Outfall**: A SWMM node representing the final discharge point from the system

## Requirements

### Requirement 1: GoldSim External Element Interface Compliance

**User Story:** As a GoldSim user, I want the bridge DLL to comply with the GoldSim External Element API, so that it can be loaded and called correctly by GoldSim.

#### Acceptance Criteria

1. THE Bridge_DLL SHALL export a function with C linkage using the signature: `void __declspec(dllexport) FunctionName(int methodID, int* status, double* inargs, double* outargs)`
2. WHEN GoldSim calls the function with XF_REP_VERSION (methodID = 2), THE Bridge_DLL SHALL return the version number in outargs[0]
3. WHEN GoldSim calls the function with XF_REP_ARGUMENTS (methodID = 3), THE Bridge_DLL SHALL return the number of input arguments in outargs[0] and the number of output arguments in outargs[1]
4. WHEN any method call completes successfully, THE Bridge_DLL SHALL set status to 0 (XF_SUCCESS)
5. WHEN any method call encounters a fatal error, THE Bridge_DLL SHALL set status to a value greater than 0 and less than 99

### Requirement 2: SWMM Lifecycle Management

**User Story:** As a simulation engineer, I want the bridge to properly manage the SWMM engine lifecycle, so that SWMM simulations execute correctly and resources are properly released.

#### Acceptance Criteria

1. WHEN GoldSim calls XF_INITIALIZE (methodID = 0), THE Bridge_DLL SHALL call swmm_open() with the input file path, report file path, and output file path
2. WHEN swmm_open() succeeds, THE Bridge_DLL SHALL call swmm_start() to initialize the simulation
3. WHEN swmm_start() succeeds, THE Bridge_DLL SHALL transition to a running state where SWMM is ready to accept time step calculations
4. WHEN GoldSim calls XF_CLEANUP (methodID = 99), THE Bridge_DLL SHALL call swmm_end() followed by swmm_close() to properly terminate the SWMM simulation
5. WHEN swmm_end() or swmm_close() is called, THE Bridge_DLL SHALL transition to an uninitialized state
6. IF swmm_open(), swmm_start(), swmm_end(), or swmm_close() returns a non-zero error code, THEN THE Bridge_DLL SHALL set status to XF_FAILURE and return an error

### Requirement 3: Time Step Synchronization

**User Story:** As a modeler, I want the bridge to synchronize time steps between GoldSim and SWMM, so that both models advance together through the simulation period with one-to-one correspondence.

#### Acceptance Criteria

1. WHEN GoldSim calls XF_CALCULATE (methodID = 1), THE Bridge_DLL SHALL call swmm_step() exactly once to advance SWMM by one routing time step
2. WHEN swmm_step() returns, THE Bridge_DLL SHALL receive the elapsed simulation time from SWMM
3. IF swmm_step() returns a non-zero error code indicating the simulation has ended, THEN THE Bridge_DLL SHALL call swmm_end() and swmm_close() to properly terminate and return XF_SUCCESS
4. WHEN swmm_step() completes successfully, THE Bridge_DLL SHALL continue accepting subsequent XF_CALCULATE calls
5. THE Bridge_DLL SHALL assume that the GoldSim time step matches the SWMM ROUTING_STEP exactly for one-to-one time step correspondence

### Requirement 4: Rainfall Data Transfer from GoldSim to SWMM

**User Story:** As a hydrologist, I want to pass rainfall intensity values from GoldSim to SWMM rain gages, so that SWMM can calculate runoff based on GoldSim-controlled precipitation for stochastic analysis.

#### Acceptance Criteria

1. WHEN GoldSim calls XF_CALCULATE (methodID = 1), THE Bridge_DLL SHALL read the rainfall intensity value from inargs[0]
2. WHEN a rainfall intensity value is received, THE Bridge_DLL SHALL call swmm_setValue() with property swmm_GAGE_RAINFALL to set the rainfall for the target rain gage
3. WHEN setting rainfall values, THE Bridge_DLL SHALL use rain gage index 0 by default
4. IF swmm_setValue() returns an error, THEN THE Bridge_DLL SHALL set status to XF_FAILURE

### Requirement 5: Treatment Train Hydraulic Data Transfer from SWMM to GoldSim

**User Story:** As a contaminant transport modeler, I want to retrieve hydraulic data from multiple points in the SWMM treatment train and return them to GoldSim, so that GoldSim can calculate contaminant mass balance and removal at each treatment stage.

#### Acceptance Criteria

1. WHEN swmm_step() completes successfully, THE Bridge_DLL SHALL retrieve catchment discharge by calling swmm_getValue() with property swmm_SUBCATCH_RUNOFF for the target subcatchment
2. WHEN swmm_step() completes successfully, THE Bridge_DLL SHALL retrieve bioswale storage volume by calling swmm_getValue() with property swmm_NODE_VOLUME for the bioswale storage node
3. WHEN swmm_step() completes successfully, THE Bridge_DLL SHALL retrieve detention storage volume by calling swmm_getValue() with property swmm_NODE_VOLUME for the detention storage node
4. WHEN swmm_step() completes successfully, THE Bridge_DLL SHALL retrieve retention storage volume by calling swmm_getValue() with property swmm_NODE_VOLUME for the retention storage node
5. WHEN swmm_step() completes successfully, THE Bridge_DLL SHALL retrieve final discharge by calling swmm_getValue() with property swmm_LINK_FLOW for the final conduit to the outfall
6. WHEN all hydraulic values are retrieved, THE Bridge_DLL SHALL write them to the output array in order: outargs[0]=catchment discharge, outargs[1]=bioswale volume, outargs[2]=detention volume, outargs[3]=retention volume, outargs[4]=final discharge
7. IF any swmm_getValue() call fails to retrieve a valid value, THEN THE Bridge_DLL SHALL set status to XF_FAILURE

### Requirement 6: Treatment Train Node and Link Identification

**User Story:** As a system designer, I want the bridge to identify and track the correct SWMM nodes and links for the treatment train, so that hydraulic data is retrieved from the appropriate model elements.

#### Acceptance Criteria

1. WHEN XF_INITIALIZE completes successfully, THE Bridge_DLL SHALL call swmm_getIndex() to retrieve the index for subcatchment "S1"
2. WHEN XF_INITIALIZE completes successfully, THE Bridge_DLL SHALL call swmm_getIndex() to retrieve the index for storage node "ST1" (bioswale)
3. WHEN XF_INITIALIZE completes successfully, THE Bridge_DLL SHALL call swmm_getIndex() to retrieve the index for storage node "ST2" (detention)
4. WHEN XF_INITIALIZE completes successfully, THE Bridge_DLL SHALL call swmm_getIndex() to retrieve the index for storage node "ST3" (retention)
5. WHEN XF_INITIALIZE completes successfully, THE Bridge_DLL SHALL call swmm_getIndex() to retrieve the index for outfall node "J2"
6. WHEN XF_INITIALIZE completes successfully, THE Bridge_DLL SHALL call swmm_getIndex() to retrieve the index for conduit "C3" (final discharge conduit)
7. IF any swmm_getIndex() call returns a negative value indicating the element does not exist, THEN THE Bridge_DLL SHALL set status to XF_FAILURE

### Requirement 7: File Path Configuration

**User Story:** As a user, I want to specify the SWMM input file and output file paths, so that the bridge can locate and use my SWMM model files.

#### Acceptance Criteria

1. THE Bridge_DLL SHALL accept the SWMM input file path (.inp) as a configuration parameter
2. THE Bridge_DLL SHALL accept the SWMM report file path (.rpt) as a configuration parameter
3. THE Bridge_DLL SHALL accept the SWMM output file path (.out) as a configuration parameter
4. WHEN file paths are relative, THE Bridge_DLL SHALL resolve them relative to the directory containing the GoldSim model file
5. IF any required file path is not provided or is invalid, THEN THE Bridge_DLL SHALL return XF_FAILURE during XF_INITIALIZE

### Requirement 8: Error Handling and Reporting

**User Story:** As a developer, I want clear error messages when the bridge encounters problems, so that I can diagnose and fix configuration or runtime issues.

#### Acceptance Criteria

1. WHEN any SWMM API function returns a non-zero error code, THE Bridge_DLL SHALL set status to XF_FAILURE
2. WHEN a fatal error occurs, THE Bridge_DLL SHALL call swmm_getError() to retrieve the SWMM error message
3. IF the Bridge_DLL encounters an error during XF_INITIALIZE, THEN THE Bridge_DLL SHALL ensure SWMM is properly closed before returning
4. WHEN an error occurs, THE Bridge_DLL SHALL return a descriptive error message to GoldSim using the XF_FAILURE_WITH_MSG mechanism (status = -1)
5. WHEN returning an error message, THE Bridge_DLL SHALL store the message in a static buffer and return its address in outargs[0]

### Requirement 9: State Management

**User Story:** As a simulation engineer, I want the bridge to track its internal state, so that it can prevent invalid operations and ensure proper sequencing of SWMM API calls.

#### Acceptance Criteria

1. THE Bridge_DLL SHALL maintain a state variable tracking whether SWMM is currently running
2. WHEN XF_INITIALIZE is called while SWMM is already running, THE Bridge_DLL SHALL first call swmm_end() and swmm_close() before opening a new simulation
3. WHEN XF_CALCULATE is called before SWMM has been initialized, THE Bridge_DLL SHALL return XF_FAILURE
4. WHEN XF_CLEANUP is called while SWMM is not running, THE Bridge_DLL SHALL return XF_SUCCESS without attempting to close SWMM

### Requirement 10: Platform and Architecture Requirements

**User Story:** As a GoldSim user on Windows, I want the bridge DLL to be compatible with my system architecture, so that it loads and runs correctly.

#### Acceptance Criteria

1. THE Bridge_DLL SHALL be compiled as a Windows x64 DLL for compatibility with modern GoldSim versions
2. THE Bridge_DLL SHALL link against the x64 version of swmm5.lib
3. THE Bridge_DLL SHALL be loadable by GoldSim using the standard LoadLibrary mechanism
4. THE Bridge_DLL SHALL export its function name in a case-sensitive manner matching the name specified in the GoldSim External element

### Requirement 11: Monte Carlo Simulation Support

**User Story:** As a risk analyst, I want the bridge to support multiple realizations with different rainfall patterns, so that I can perform Monte Carlo analysis of treatment train performance under uncertainty.

#### Acceptance Criteria

1. WHEN GoldSim runs multiple realizations, THE Bridge_DLL SHALL properly initialize and cleanup SWMM for each realization
2. WHEN a new realization begins, THE Bridge_DLL SHALL reset all internal state variables to initial values
3. WHEN XF_CLEANUP is called at the end of a realization, THE Bridge_DLL SHALL release all SWMM resources to prevent memory leaks
4. THE Bridge_DLL SHALL support thousands of realizations without resource exhaustion or performance degradation
