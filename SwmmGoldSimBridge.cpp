//-----------------------------------------------------------------------------
//   SwmmGoldSimBridge.cpp
//
//   Project: GoldSim-SWMM Bridge DLL
//   Version: 4.0
//   Description: Bridge DLL enabling bidirectional communication between
//                GoldSim simulation software and EPA SWMM hydraulic engine
//
//   This DLL implements the GoldSim External Element API to allow GoldSim
//   to control SWMM simulation timing, provide rainfall inputs, and receive
//   runoff calculations.
//-----------------------------------------------------------------------------

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "include/swmm5.h"

//-----------------------------------------------------------------------------
// Version Information
//-----------------------------------------------------------------------------
#define DLL_VERSION 4.1
#define DLL_VERSION_STRING "4.1"

//-----------------------------------------------------------------------------
// Debug Logging Configuration
//-----------------------------------------------------------------------------
// Log levels: 0=OFF, 1=ERROR, 2=INFO, 3=DEBUG
// Set LOG_LEVEL to control verbosity:
//   0 = No logging
//   1 = Errors only
//   2 = Errors + Important info (initialization, cleanup, major events)
//   3 = Full debug logging (all operations, values, calculations)
#define LOG_LEVEL 2  // Default: INFO level

#define LOG_ERROR 1
#define LOG_INFO  2
#define LOG_DEBUG 3

//-----------------------------------------------------------------------------
// Debug Logging Helper
//-----------------------------------------------------------------------------
// Logs messages to a file based on log level
static void Log(int level, const char* format, ...)
{
    if (level > LOG_LEVEL) return;  // Skip if message level exceeds configured level
    
    static bool first_log_call = true;
    FILE* log_file;
    
    // On first call, overwrite the log file; afterwards append
    const char* mode = first_log_call ? "w" : "a";
    errno_t err = fopen_s(&log_file, "bridge_debug.log", mode);
    if (err == 0 && log_file != NULL)
    {
        // Write version header on first call
        if (first_log_call)
        {
            fprintf(log_file, "=======================================================\n");
            fprintf(log_file, "GoldSim-SWMM Bridge DLL v%s\n", DLL_VERSION_STRING);
            fprintf(log_file, "Log Level: %d (0=OFF, 1=ERROR, 2=INFO, 3=DEBUG)\n", LOG_LEVEL);
            fprintf(log_file, "=======================================================\n\n");
            first_log_call = false;
        }
        
        // Get current time
        SYSTEMTIME st;
        GetLocalTime(&st);
        
        // Write level prefix
        const char* level_str = "";
        switch(level) {
            case LOG_ERROR: level_str = "ERROR"; break;
            case LOG_INFO:  level_str = "INFO "; break;
            case LOG_DEBUG: level_str = "DEBUG"; break;
        }
        
        fprintf(log_file, "[%02d:%02d:%02d.%03d] [%s] ", 
                st.wHour, st.wMinute, st.wSecond, st.wMilliseconds, level_str);
        
        // Write the formatted message
        va_list args;
        va_start(args, format);
        vfprintf(log_file, format, args);
        va_end(args);
        
        fprintf(log_file, "\n");
        fclose(log_file);
    }
}

// Convenience macros for different log levels
#define LogError(fmt, ...) Log(LOG_ERROR, fmt, ##__VA_ARGS__)
#define LogInfo(fmt, ...)  Log(LOG_INFO, fmt, ##__VA_ARGS__)
#define LogDebug(fmt, ...) Log(LOG_DEBUG, fmt, ##__VA_ARGS__)

//-----------------------------------------------------------------------------
// GoldSim Method ID Enumerations
//-----------------------------------------------------------------------------
// These method IDs are passed by GoldSim to indicate which operation
// the DLL should perform

#define XF_INITIALIZE       0   // Initialize SWMM engine at start of realization
#define XF_CALCULATE        1   // Advance simulation one time step and exchange data
#define XF_REP_VERSION      2   // Report DLL version number
#define XF_REP_ARGUMENTS    3   // Report number of input/output arguments
#define XF_CLEANUP          99  // Terminate SWMM simulation and release resources

//-----------------------------------------------------------------------------
// GoldSim Status Code Enumerations
//-----------------------------------------------------------------------------
// These status codes are returned to GoldSim to indicate operation results

#define XF_SUCCESS              0   // Operation completed successfully
#define XF_FAILURE              1   // Operation failed (generic error)
#define XF_FAILURE_WITH_MSG    -1   // Operation failed with error message

//-----------------------------------------------------------------------------
// Global State Variables
//-----------------------------------------------------------------------------
// These variables maintain the bridge state across method calls

static bool is_swmm_running = false;           // Tracks whether SWMM is initialized
static int subcatchment_index = 0;             // Subcatchment S1 index
static int bioswale_index = 0;                 // ST1 storage node index
static int detention_index = 0;                // ST2 storage node index
static int retention_index = 0;                // ST3 storage node index
static int outfall_index = 0;                  // J2 outfall node index
static int conduit_c1_index = 0;               // Conduit C1 (Bioswale to Detention)
static int conduit_c2_index = 0;               // Conduit C2 (Detention to Retention)
static int conduit_c3_index = 0;               // Conduit C3 (Retention to Outfall)
static char input_file_path[260] = "model.inp";   // SWMM input file path
static char report_file_path[260] = "model.rpt";  // SWMM report file path
static char output_file_path[260] = "model.out";  // SWMM output file path
static char error_message_buffer[200];         // Buffer for error messages
static double last_swmm_time = -1.0;           // Last SWMM elapsed time (days)
static double accumulated_rainfall = 0.0;      // Accumulated rainfall since last step

//-----------------------------------------------------------------------------
// Helper Function: HandleSwmmError
//-----------------------------------------------------------------------------
// Retrieves SWMM error messages and formats them for GoldSim
//
// Parameters:
//   error_code - The error code returned by SWMM API function
//   outargs    - Output arguments array to store error message pointer
//   status     - Status pointer to set to XF_FAILURE_WITH_MSG
//-----------------------------------------------------------------------------
void HandleSwmmError(int error_code, double* outargs, int* status)
{
    // Retrieve error message from SWMM
    swmm_getError(error_message_buffer, sizeof(error_message_buffer));
    
    // Ensure null termination
    error_message_buffer[sizeof(error_message_buffer) - 1] = '\0';
    
    // Cast outargs to pointer type and store buffer address
    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
    *pAddr = (ULONG_PTR)error_message_buffer;
    
    // Set status to indicate error with message
    *status = XF_FAILURE_WITH_MSG;
}

//-----------------------------------------------------------------------------
// Helper Function: SetSubcatchmentIndex (for testing purposes)
//-----------------------------------------------------------------------------
// Allows test code to set the subcatchment index to test validation logic
//
// Parameters:
//   index - The subcatchment index to set
//-----------------------------------------------------------------------------
extern "C" void __declspec(dllexport) SetSubcatchmentIndex(int index)
{
    subcatchment_index = index;
}

//-----------------------------------------------------------------------------
// Bridge Entry Point Function
//-----------------------------------------------------------------------------
// This is the main function exported to GoldSim. It dispatches method calls
// based on the methodID parameter.
//
// Parameters:
//   methodID - Integer indicating requested operation (0, 1, 2, 3, or 99)
//   status   - Pointer to integer for returning operation status
//   inargs   - Array of input arguments from GoldSim
//   outargs  - Array of output arguments to GoldSim
//-----------------------------------------------------------------------------

extern "C" void __declspec(dllexport) SwmmGoldSimBridge(
    int methodID,
    int* status,
    double* inargs,
    double* outargs
)
{
    // Initialize status to success by default
    *status = XF_SUCCESS;
    
    // Log the method call
    LogDebug("=== Method Called: %d ===", methodID);

    // Dispatch based on method ID
    switch (methodID)
    {
        case XF_INITIALIZE:
            // Initialize SWMM engine at start of realization
            {
                LogDebug("XF_INITIALIZE: Starting initialization");
                LogDebug("XF_INITIALIZE: Starting initialization");
                // Check if SWMM is already running; if so, cleanup first
                if (is_swmm_running)
                {
                    LogDebug("XF_INITIALIZE: SWMM already running, cleaning up first");
                    int end_error = swmm_end();
                    int close_error = swmm_close();
                    is_swmm_running = false;
                    
                    // If cleanup during re-initialization fails, report it
                    if (end_error != 0)
                    {
                    LogError("XF_INITIALIZE: swmm_end() failed with code %d", end_error);
                        HandleSwmmError(end_error, outargs, status);
                        break;
                    }
                    if (close_error != 0)
                    {
                    LogError("XF_INITIALIZE: swmm_close() failed with code %d", close_error);
                        HandleSwmmError(close_error, outargs, status);
                        break;
                    }
                }
                
                LogInfo("XF_INITIALIZE: Input file: %s", input_file_path);
                LogDebug("XF_INITIALIZE: Input file: %s", input_file_path);
                
                // Validate file paths before attempting to open SWMM
                // Check if input file path is provided (not empty)
                if (input_file_path[0] == '\0')
                {
                    LogDebug("XF_INITIALIZE: ERROR - Input file path is empty");
                    strcpy_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Input file path is not provided");
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                // Check if input file exists using Windows API
                DWORD fileAttrib = GetFileAttributesA(input_file_path);
                if (fileAttrib == INVALID_FILE_ATTRIBUTES)
                {
                    LogDebug("XF_INITIALIZE: ERROR - Input file does not exist");
                    // File does not exist or path is invalid
                    sprintf_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Input file does not exist: %s", input_file_path);
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                // Check if the path points to a directory instead of a file
                if (fileAttrib & FILE_ATTRIBUTE_DIRECTORY)
                {
                    LogDebug("XF_INITIALIZE: ERROR - Input file path is a directory");
                    sprintf_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Input file path is a directory: %s", input_file_path);
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                LogDebug("XF_INITIALIZE: Calling swmm_open()");
                // Call swmm_open() with file paths
                int error_code = swmm_open(input_file_path, report_file_path, output_file_path);
                
                if (error_code != 0)
                {
                    LogDebug("XF_INITIALIZE: swmm_open() failed with code %d", error_code);
                    // swmm_open failed, return error
                    // No cleanup needed since open failed
                    HandleSwmmError(error_code, outargs, status);
                    break;
                }
                
                LogDebug("XF_INITIALIZE: swmm_open() succeeded");
                
                // Validate subcatchment index before starting simulation
                // Get the count of subcatchments in the loaded model
                int subcatch_count = swmm_getCount(swmm_SUBCATCH);
                LogDebug("XF_INITIALIZE: Subcatchment count = %d, using index = %d", 
                         subcatch_count, subcatchment_index);
                LogDebug("XF_INITIALIZE: Subcatchment count = %d, using index = %d", 
                         subcatch_count, subcatchment_index);
                
                // Check if subcatchment index is within valid range [0, subcatch_count-1]
                if (subcatchment_index < 0 || subcatchment_index >= subcatch_count)
                {
                    LogDebug("XF_INITIALIZE: ERROR - Subcatchment index out of range");
                    // Subcatchment index is out of range
                    // Must close since open succeeded
                    swmm_close();
                    
                    sprintf_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Subcatchment index %d is out of range (valid range: 0-%d)", 
                             subcatchment_index, subcatch_count - 1);
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                LogDebug("XF_INITIALIZE: Calling swmm_start()");
                // Call swmm_start() to begin simulation
                // Parameter 1 = save results to output file
                error_code = swmm_start(1);
                
                if (error_code != 0)
                {
                    LogDebug("XF_INITIALIZE: swmm_start() failed with code %d", error_code);
                    // swmm_start failed, cleanup and return error
                    // Must close since open succeeded
                    int close_error = swmm_close();
                    
                    // Report the start error (more relevant than close error)
                    HandleSwmmError(error_code, outargs, status);
                    
                    // If close also failed, append that information
                    if (close_error != 0)
                    {
                        // Note: We prioritize the start error message
                        // The close error is secondary
                    }
                    break;
                }
                
                // Retrieve element indices using swmm_getIndex()
                // Get subcatchment index for S1
                subcatchment_index = swmm_getIndex(swmm_SUBCATCH, "S1");
                
                // Get node indices for treatment train
                bioswale_index = swmm_getIndex(swmm_NODE, "ST1");
                detention_index = swmm_getIndex(swmm_NODE, "ST2");
                retention_index = swmm_getIndex(swmm_NODE, "ST3");
                outfall_index = swmm_getIndex(swmm_NODE, "J2");
                
                // Get link indices for C1, C2, C3 (orifices connecting storage nodes)
                conduit_c1_index = swmm_getIndex(swmm_LINK, "C1");
                conduit_c2_index = swmm_getIndex(swmm_LINK, "C2");
                conduit_c3_index = swmm_getIndex(swmm_LINK, "C3");
                
                LogDebug("XF_INITIALIZE: Subcatchment S1 index = %d", subcatchment_index);
                LogDebug("XF_INITIALIZE: Bioswale ST1 index = %d", bioswale_index);
                LogDebug("XF_INITIALIZE: Detention ST2 index = %d", detention_index);
                LogDebug("XF_INITIALIZE: Retention ST3 index = %d", retention_index);
                LogDebug("XF_INITIALIZE: Outfall J2 index = %d", outfall_index);
                LogDebug("XF_INITIALIZE: Link C1 index = %d", conduit_c1_index);
                LogDebug("XF_INITIALIZE: Link C2 index = %d", conduit_c2_index);
                LogDebug("XF_INITIALIZE: Link C3 index = %d", conduit_c3_index);
                
                // Validate all indices are non-negative (element exists)
                if (subcatchment_index < 0)
                {
                    LogDebug("XF_INITIALIZE: ERROR - Subcatchment S1 not found");
                    swmm_end();
                    swmm_close();
                    sprintf_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Subcatchment S1 not found in SWMM model");
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                if (bioswale_index < 0)
                {
                    LogDebug("XF_INITIALIZE: ERROR - Storage node ST1 not found");
                    swmm_end();
                    swmm_close();
                    sprintf_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Storage node ST1 (bioswale) not found in SWMM model");
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                if (detention_index < 0)
                {
                    LogDebug("XF_INITIALIZE: ERROR - Storage node ST2 not found");
                    swmm_end();
                    swmm_close();
                    sprintf_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Storage node ST2 (detention) not found in SWMM model");
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                if (retention_index < 0)
                {
                    LogDebug("XF_INITIALIZE: ERROR - Storage node ST3 not found");
                    swmm_end();
                    swmm_close();
                    sprintf_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Storage node ST3 (retention) not found in SWMM model");
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                if (outfall_index < 0)
                {
                    LogDebug("XF_INITIALIZE: ERROR - Outfall J2 not found");
                    swmm_end();
                    swmm_close();
                    sprintf_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Outfall J2 not found in SWMM model");
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                if (conduit_c1_index < 0)
                {
                    LogDebug("XF_INITIALIZE: ERROR - Link C1 not found");
                    swmm_end();
                    swmm_close();
                    sprintf_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Link C1 not found in SWMM model");
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                if (conduit_c2_index < 0)
                {
                    LogDebug("XF_INITIALIZE: ERROR - Link C2 not found");
                    swmm_end();
                    swmm_close();
                    sprintf_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Link C2 not found in SWMM model");
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                if (conduit_c3_index < 0)
                {
                    LogDebug("XF_INITIALIZE: ERROR - Link C3 not found");
                    swmm_end();
                    swmm_close();
                    sprintf_s(error_message_buffer, sizeof(error_message_buffer), 
                             "Error: Link C3 not found in SWMM model");
                    ULONG_PTR* pAddr = (ULONG_PTR*)outargs;
                    *pAddr = (ULONG_PTR)error_message_buffer;
                    *status = XF_FAILURE_WITH_MSG;
                    break;
                }
                
                // All elements found successfully, set flag to indicate SWMM is running
                is_swmm_running = true;
                
                // Reset time tracking variables
                last_swmm_time = -1.0;
                accumulated_rainfall = 0.0;
                
                *status = XF_SUCCESS;
                LogInfo("XF_INITIALIZE: Success - SWMM is now running");
            }
            break;

        case XF_CALCULATE:
            // Advance simulation one time step and exchange data
            {
                LogDebug("===== DLL Version 2.0 - Pond/Pump/Spillway Model ====");
                LogDebug("XF_CALCULATE: Called");
                
                // Verify SWMM is running
                if (!is_swmm_running)
                {
                    LogDebug("XF_CALCULATE: ERROR - SWMM not running");
                    // Cannot calculate before initialization - invalid state transition
                    *status = XF_FAILURE;
                    break;
                }
                
                // Read inputs from GoldSim
                // inargs[0] = ETime (seconds) - GoldSim elapsed time
                // inargs[1] = rainfall intensity (in/hr)
                double etime_seconds = inargs[0];
                double rainfall = inargs[1];
                
                // Convert ETime to days for comparison with SWMM time
                double etime_days = etime_seconds / 86400.0;
                
                LogDebug("XF_CALCULATE: GoldSim ETime = %.6f seconds (%.6f days), Rainfall input = %.6f in/hr", 
                         etime_seconds, etime_days, rainfall);
                
                // Set rainfall on the rain gage (gage index 0)
                LogDebug("XF_CALCULATE: Calling swmm_setValue(GAGE_RAINFALL, 0, %.6f)", rainfall);
                swmm_setValue(swmm_GAGE_RAINFALL, 0, rainfall);
                
                // Advance SWMM simulation by one time step
                LogDebug("XF_CALCULATE: Calling swmm_step()");
                double elapsed_time = 0.0;
                int error_code = swmm_step(&elapsed_time);
                
                // Calculate time difference
                double time_diff_days = (last_swmm_time >= 0) ? (elapsed_time - last_swmm_time) : 0.0;
                double time_diff_minutes = time_diff_days * 1440.0;  // Convert days to minutes
                
                LogDebug("XF_CALCULATE: swmm_step() returned %d, SWMM elapsed_time = %.6f days (%.2f minutes since last step)", 
                         error_code, elapsed_time, time_diff_minutes);
                LogDebug("XF_CALCULATE: Time sync check - GoldSim: %.6f days, SWMM: %.6f days, Difference: %.6f days (%.2f minutes)",
                         etime_days, elapsed_time, elapsed_time - etime_days, (elapsed_time - etime_days) * 1440.0);
                
                // Update last SWMM time
                last_swmm_time = elapsed_time;
                
                // Handle swmm_step return codes
                if (error_code < 0)
                {
                    LogDebug("XF_CALCULATE: ERROR - swmm_step() returned error code %d", error_code);
                    // Error occurred during simulation
                    HandleSwmmError(error_code, outargs, status);
                    break;
                }
                else if (error_code > 0)
                {
                    LogDebug("XF_CALCULATE: Simulation ended (code %d), cleaning up", error_code);
                    // Simulation has ended normally
                    // Call cleanup sequence with error handling
                    int end_error = swmm_end();
                    if (end_error != 0)
                    {
                        LogDebug("XF_CALCULATE: swmm_end() failed with code %d", end_error);
                        // swmm_end failed, but still try to close
                        swmm_close();
                        is_swmm_running = false;
                        HandleSwmmError(end_error, outargs, status);
                        break;
                    }
                    
                    int close_error = swmm_close();
                    is_swmm_running = false;
                    
                    if (close_error != 0)
                    {
                        LogDebug("XF_CALCULATE: swmm_close() failed with code %d", close_error);
                        // swmm_close failed
                        HandleSwmmError(close_error, outargs, status);
                        break;
                    }
                    
                    LogInfo("XF_CALCULATE: Cleanup successful, simulation complete");
                    *status = XF_SUCCESS;
                    break;
                }
                
                // Simulation continues (error_code == 0)
                // Get subcatchment runoff discharge (outlet of subcatchment)
                double catchment_discharge = swmm_getValue(swmm_SUBCATCH_RUNOFF, subcatchment_index);
                LogDebug("XF_CALCULATE: Subcatchment S1 runoff discharge = %.6f CFS", catchment_discharge);
                
                // Get storage volumes
                double bioswale_volume = swmm_getValue(swmm_NODE_VOLUME, bioswale_index);
                double detention_volume = swmm_getValue(swmm_NODE_VOLUME, detention_index);
                double retention_volume = swmm_getValue(swmm_NODE_VOLUME, retention_index);
                LogDebug("XF_CALCULATE: Bioswale volume = %.6f cu.ft", bioswale_volume);
                LogDebug("XF_CALCULATE: Detention volume = %.6f cu.ft", detention_volume);
                LogDebug("XF_CALCULATE: Retention volume = %.6f cu.ft", retention_volume);
                
                // Get flows from links C1, C2, C3
                double c1_flow = swmm_getValue(swmm_LINK_FLOW, conduit_c1_index);
                double c2_flow = swmm_getValue(swmm_LINK_FLOW, conduit_c2_index);
                double final_discharge = swmm_getValue(swmm_LINK_FLOW, conduit_c3_index);
                LogDebug("XF_CALCULATE: Link C1 flow (Bioswale to Detention) = %.6f CFS", c1_flow);
                LogDebug("XF_CALCULATE: Link C2 flow (Detention to Retention) = %.6f CFS", c2_flow);
                LogDebug("XF_CALCULATE: Link C3 flow (final discharge) = %.6f CFS", final_discharge);
                
                // Write outputs to outargs array
                outargs[0] = catchment_discharge;    // Catchment discharge (CFS)
                outargs[1] = bioswale_volume;        // Bioswale volume (cu.ft)
                outargs[2] = detention_volume;       // Detention volume (cu.ft)
                outargs[3] = retention_volume;       // Retention volume (cu.ft)
                outargs[4] = c1_flow;                // C1 flow: Bioswale to Detention (CFS)
                outargs[5] = c2_flow;                // C2 flow: Detention to Retention (CFS)
                outargs[6] = final_discharge;        // C3 flow: Final discharge (CFS)
                
                *status = XF_SUCCESS;
                LogDebug("XF_CALCULATE: Success - catchment=%.6f CFS, bioswale=%.6f cu.ft, detention=%.6f cu.ft, retention=%.6f cu.ft, C1=%.6f CFS, C2=%.6f CFS, C3=%.6f CFS", 
                         catchment_discharge, bioswale_volume, detention_volume, retention_volume, c1_flow, c2_flow, final_discharge);
            }
            break;

        case XF_REP_VERSION:
            // Report DLL version number to GoldSim
            // Version 4.1 - Treatment train with 7 outputs + water quality limitation documentation
            LogInfo("XF_REP_VERSION: Returning version %s", DLL_VERSION_STRING);
            outargs[0] = DLL_VERSION;
            *status = XF_SUCCESS;
            break;

        case XF_REP_ARGUMENTS:
            // Report number of input and output arguments to GoldSim
            // 2 inputs: ETime (seconds), rainfall intensity (in/hr)
            // 7 outputs: catchment discharge (CFS), bioswale volume (cu.ft), detention volume (cu.ft), 
            //            retention volume (cu.ft), C1 flow (CFS), C2 flow (CFS), C3 flow (CFS)
            LogInfo("XF_REP_ARGUMENTS: Returning 2 inputs, 7 outputs");
            outargs[0] = 2.0;  // Number of input arguments
            outargs[1] = 7.0;  // Number of output arguments
            *status = XF_SUCCESS;
            break;

        case XF_CLEANUP:
            // Terminate SWMM simulation and release resources
            {
                LogDebug("XF_CLEANUP: Called");
                
                // Check if SWMM is running
                if (is_swmm_running)
                {
                    LogDebug("XF_CLEANUP: SWMM is running, cleaning up");
                    // Call swmm_end() to finalize simulation
                    int end_error = swmm_end();
                    
                    // Call swmm_close() to close files and release memory
                    // Always call close even if end failed
                    int close_error = swmm_close();
                    
                    // Clear the running flag
                    is_swmm_running = false;
                    
                    // Reset all element indices to 0
                    subcatchment_index = 0;
                    bioswale_index = 0;
                    detention_index = 0;
                    retention_index = 0;
                    outfall_index = 0;
                    conduit_c1_index = 0;
                    conduit_c2_index = 0;
                    conduit_c3_index = 0;
                    
                    // Check for errors and report the first one encountered
                    if (end_error != 0)
                    {
                        LogDebug("XF_CLEANUP: swmm_end() failed with code %d", end_error);
                        HandleSwmmError(end_error, outargs, status);
                        break;
                    }
                    
                    if (close_error != 0)
                    {
                        LogDebug("XF_CLEANUP: swmm_close() failed with code %d", close_error);
                        HandleSwmmError(close_error, outargs, status);
                        break;
                    }
                    
                    LogDebug("XF_CLEANUP: Cleanup successful");
                }
                else
                {
                    LogDebug("XF_CLEANUP: SWMM not running, nothing to clean up");
                }
                
                // Return success (cleanup when not running is valid, or cleanup succeeded)
                *status = XF_SUCCESS;
            }
            break;

        default:
            // Unknown method ID
            LogDebug("ERROR: Unknown method ID %d", methodID);
            *status = XF_FAILURE;
            break;
    }
    
    LogDebug("=== Method %d Complete, Status = %d ===\n", methodID, *status);
}
