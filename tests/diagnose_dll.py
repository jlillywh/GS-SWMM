"""
GoldSim-SWMM Bridge DLL Diagnostic Script

This script mimics what GoldSim does when calling the bridge DLL,
allowing you to diagnose issues with zero runoff output.

Usage:
    python diagnose_dll.py

Requirements:
    - Python 3.6+
    - ctypes (standard library)
    - GSswmm.dll in same directory
    - swmm5.dll in same directory
    - model.inp in same directory
"""

import ctypes
import os
import sys
from ctypes import c_int, c_double, POINTER, byref

# Method IDs (from GoldSim External API)
XF_INITIALIZE = 0
XF_CALCULATE = 1
XF_REP_VERSION = 2
XF_REP_ARGUMENTS = 3
XF_CLEANUP = 99

# Status codes
XF_SUCCESS = 0
XF_FAILURE = 1
XF_FAILURE_WITH_MSG = -1

class DLLDiagnostics:
    def __init__(self, dll_path="GSswmm.dll"):
        """Initialize the diagnostic tool."""
        self.dll_path = dll_path
        self.dll = None
        self.bridge_func = None
        
    def load_dll(self):
        """Load the bridge DLL."""
        print("=" * 70)
        print("STEP 1: Loading DLL")
        print("=" * 70)
        
        if not os.path.exists(self.dll_path):
            print(f"❌ ERROR: {self.dll_path} not found in current directory")
            print(f"   Current directory: {os.getcwd()}")
            return False
            
        try:
            self.dll = ctypes.CDLL(self.dll_path)
            print(f"✓ DLL loaded: {self.dll_path}")
        except Exception as e:
            print(f"❌ ERROR loading DLL: {e}")
            return False
            
        # Get the bridge function
        try:
            self.bridge_func = self.dll.SwmmGoldSimBridge
            self.bridge_func.argtypes = [
                c_int,                    # methodID
                POINTER(c_int),           # status
                POINTER(c_double),        # inargs
                POINTER(c_double)         # outargs
            ]
            self.bridge_func.restype = None
            print("✓ Function 'SwmmGoldSimBridge' found")
        except Exception as e:
            print(f"❌ ERROR getting function: {e}")
            return False
            
        return True
        
    def test_version(self):
        """Test XF_REP_VERSION."""
        print("\n" + "=" * 70)
        print("STEP 2: Testing Version Reporting")
        print("=" * 70)
        
        status = c_int(0)
        inargs = (c_double * 10)()
        outargs = (c_double * 10)()
        
        self.bridge_func(XF_REP_VERSION, byref(status), inargs, outargs)
        
        print(f"Method ID: {XF_REP_VERSION} (XF_REP_VERSION)")
        print(f"Status: {status.value} {'✓ SUCCESS' if status.value == 0 else '❌ FAILURE'}")
        print(f"Version: {outargs[0]}")
        
        return status.value == XF_SUCCESS
        
    def test_arguments(self):
        """Test XF_REP_ARGUMENTS."""
        print("\n" + "=" * 70)
        print("STEP 3: Testing Argument Reporting")
        print("=" * 70)
        
        status = c_int(0)
        inargs = (c_double * 10)()
        outargs = (c_double * 10)()
        
        self.bridge_func(XF_REP_ARGUMENTS, byref(status), inargs, outargs)
        
        print(f"Method ID: {XF_REP_ARGUMENTS} (XF_REP_ARGUMENTS)")
        print(f"Status: {status.value} {'✓ SUCCESS' if status.value == 0 else '❌ FAILURE'}")
        print(f"Number of inputs: {outargs[0]}")
        print(f"Number of outputs: {outargs[1]}")
        
        return status.value == XF_SUCCESS
        
    def test_initialize(self):
        """Test XF_INITIALIZE."""
        print("\n" + "=" * 70)
        print("STEP 4: Testing Initialization")
        print("=" * 70)
        
        # Check for required files
        required_files = ["model.inp", "swmm5.dll"]
        for filename in required_files:
            if os.path.exists(filename):
                print(f"✓ Found: {filename}")
            else:
                print(f"❌ Missing: {filename}")
                
        status = c_int(0)
        inargs = (c_double * 10)()
        outargs = (c_double * 10)()
        
        print("\nCalling XF_INITIALIZE...")
        self.bridge_func(XF_INITIALIZE, byref(status), inargs, outargs)
        
        print(f"Method ID: {XF_INITIALIZE} (XF_INITIALIZE)")
        print(f"Status: {status.value}", end=" ")
        
        if status.value == XF_SUCCESS:
            print("✓ SUCCESS - SWMM initialized")
            return True
        elif status.value == XF_FAILURE_WITH_MSG:
            print("❌ FAILURE WITH MESSAGE")
            # Try to get error message
            try:
                addr = int(outargs[0])
                if addr != 0:
                    error_msg = ctypes.string_at(addr).decode('utf-8')
                    print(f"   Error message: {error_msg}")
            except:
                print("   (Could not retrieve error message)")
            return False
        else:
            print(f"❌ FAILURE (code: {status.value})")
            return False
            
    def test_calculate(self, rainfall_values):
        """Test XF_CALCULATE with various rainfall values."""
        print("\n" + "=" * 70)
        print("STEP 5: Testing Calculate with Various Rainfall Values")
        print("=" * 70)
        
        results = []
        
        for i, rainfall in enumerate(rainfall_values):
            status = c_int(0)
            inargs = (c_double * 10)()
            outargs = (c_double * 10)()
            
            # Set rainfall input
            inargs[0] = rainfall
            
            # Call calculate
            self.bridge_func(XF_CALCULATE, byref(status), inargs, outargs)
            
            runoff = outargs[0]
            results.append((rainfall, runoff, status.value))
            
            print(f"\nTime Step {i+1}:")
            print(f"  Rainfall: {rainfall:8.2f} in/hr")
            print(f"  Runoff:   {runoff:8.4f} CFS")
            print(f"  Status:   {status.value} {'✓' if status.value == 0 else '❌'}")
            
            if status.value != XF_SUCCESS:
                print(f"  ⚠️  WARNING: Calculate returned non-success status")
                if status.value == XF_FAILURE_WITH_MSG:
                    try:
                        addr = int(outargs[0])
                        if addr != 0:
                            error_msg = ctypes.string_at(addr).decode('utf-8')
                            print(f"  Error: {error_msg}")
                    except:
                        pass
                        
        return results
        
    def test_cleanup(self):
        """Test XF_CLEANUP."""
        print("\n" + "=" * 70)
        print("STEP 6: Testing Cleanup")
        print("=" * 70)
        
        status = c_int(0)
        inargs = (c_double * 10)()
        outargs = (c_double * 10)()
        
        self.bridge_func(XF_CLEANUP, byref(status), inargs, outargs)
        
        print(f"Method ID: {XF_CLEANUP} (XF_CLEANUP)")
        print(f"Status: {status.value} {'✓ SUCCESS' if status.value == 0 else '❌ FAILURE'}")
        
        return status.value == XF_SUCCESS
        
    def check_swmm_report(self):
        """Check the SWMM report file for clues."""
        print("\n" + "=" * 70)
        print("STEP 7: Checking SWMM Report File")
        print("=" * 70)
        
        if not os.path.exists("model.rpt"):
            print("❌ model.rpt not found - SWMM may not have run")
            return
            
        print("✓ model.rpt found")
        print("\nSearching for key information...")
        
        try:
            with open("model.rpt", "r") as f:
                content = f.read()
                
            # Look for errors
            if "ERROR" in content.upper():
                print("\n⚠️  ERRORS FOUND IN REPORT:")
                lines = content.split('\n')
                for i, line in enumerate(lines):
                    if "ERROR" in line.upper():
                        print(f"  Line {i+1}: {line.strip()}")
                        
            # Look for warnings
            if "WARNING" in content.upper():
                print("\n⚠️  WARNINGS FOUND IN REPORT:")
                lines = content.split('\n')
                for i, line in enumerate(lines):
                    if "WARNING" in line.upper():
                        print(f"  Line {i+1}: {line.strip()}")
                        
            # Look for subcatchment summary
            if "Subcatchment Runoff Summary" in content:
                print("\n✓ Found Subcatchment Runoff Summary")
                # Extract summary section
                start = content.find("Subcatchment Runoff Summary")
                end = content.find("\n\n", start + 100)
                if end > start:
                    summary = content[start:end]
                    print(summary[:500])  # Print first 500 chars
            else:
                print("\n❌ No Subcatchment Runoff Summary found")
                
            # Look for simulation statistics
            if "Total Runoff" in content:
                lines = content.split('\n')
                for line in lines:
                    if "Total Runoff" in line or "Total Rainfall" in line:
                        print(f"  {line.strip()}")
                        
        except Exception as e:
            print(f"❌ Error reading report: {e}")
            
    def check_swmm_input(self):
        """Check the SWMM input file for potential issues."""
        print("\n" + "=" * 70)
        print("STEP 8: Checking SWMM Input File")
        print("=" * 70)
        
        if not os.path.exists("model.inp"):
            print("❌ model.inp not found")
            return
            
        print("✓ model.inp found")
        
        try:
            with open("model.inp", "r") as f:
                content = f.read()
                
            # Check for subcatchments
            if "[SUBCATCHMENTS]" in content:
                print("\n✓ [SUBCATCHMENTS] section found")
                start = content.find("[SUBCATCHMENTS]")
                end = content.find("\n[", start + 1)
                section = content[start:end] if end > start else content[start:start+500]
                
                # Count subcatchments
                lines = section.split('\n')
                subcatch_count = 0
                for line in lines:
                    if line.strip() and not line.strip().startswith(';') and not line.strip().startswith('['):
                        if len(line.split()) >= 7:  # Valid subcatchment line
                            subcatch_count += 1
                            if subcatch_count == 1:
                                print(f"\n  First subcatchment: {line.strip()}")
                                
                print(f"  Total subcatchments: {subcatch_count}")
                
                if subcatch_count == 0:
                    print("  ❌ WARNING: No subcatchments defined!")
            else:
                print("❌ No [SUBCATCHMENTS] section found")
                
            # Check routing step
            if "ROUTING_STEP" in content:
                lines = content.split('\n')
                for line in lines:
                    if "ROUTING_STEP" in line and not line.strip().startswith(';'):
                        print(f"\n✓ Routing step: {line.strip()}")
                        
            # Check flow units
            if "FLOW_UNITS" in content:
                lines = content.split('\n')
                for line in lines:
                    if "FLOW_UNITS" in line and not line.strip().startswith(';'):
                        print(f"✓ Flow units: {line.strip()}")
                        
        except Exception as e:
            print(f"❌ Error reading input file: {e}")
            
    def run_full_diagnostic(self):
        """Run complete diagnostic sequence."""
        print("\n" + "=" * 70)
        print("GoldSim-SWMM Bridge DLL Diagnostic Tool")
        print("=" * 70)
        print(f"Working directory: {os.getcwd()}")
        print()
        
        # Load DLL
        if not self.load_dll():
            return False
            
        # Test version
        if not self.test_version():
            print("\n❌ Version test failed - stopping")
            return False
            
        # Test arguments
        if not self.test_arguments():
            print("\n❌ Arguments test failed - stopping")
            return False
            
        # Check input file first
        self.check_swmm_input()
        
        # Test initialize
        if not self.test_initialize():
            print("\n❌ Initialize failed - stopping")
            print("\n💡 SUGGESTION: Check that model.inp is valid and swmm5.dll is present")
            return False
            
        # Test calculate with various rainfall values
        rainfall_values = [0.0, 1.0, 5.0, 10.0, 25.0, 50.0, 100.0]
        results = self.test_calculate(rainfall_values)
        
        # Analyze results
        print("\n" + "=" * 70)
        print("RESULTS ANALYSIS")
        print("=" * 70)
        
        all_zero = all(runoff == 0.0 for _, runoff, _ in results)
        any_nonzero = any(runoff != 0.0 for _, runoff, _ in results)
        
        if all_zero:
            print("❌ PROBLEM IDENTIFIED: All runoff values are ZERO")
            print("\nPossible causes:")
            print("  1. SWMM model has no subcatchments")
            print("  2. Subcatchment parameters are invalid (zero area, etc.)")
            print("  3. Time step mismatch (SWMM not advancing)")
            print("  4. Infiltration rate too high (all rainfall infiltrates)")
            print("  5. SWMM simulation ended prematurely")
            print("\nCheck the SWMM report file below for details...")
        elif any_nonzero:
            print("✓ SUCCESS: Some non-zero runoff values detected")
            max_runoff = max(runoff for _, runoff, _ in results)
            print(f"  Maximum runoff: {max_runoff:.4f} CFS")
        else:
            print("⚠️  UNCLEAR: Mixed or unexpected results")
            
        # Test cleanup
        self.test_cleanup()
        
        # Check SWMM report
        self.check_swmm_report()
        
        print("\n" + "=" * 70)
        print("DIAGNOSTIC COMPLETE")
        print("=" * 70)
        
        return True


def main():
    """Main entry point."""
    diag = DLLDiagnostics()
    
    try:
        diag.run_full_diagnostic()
    except KeyboardInterrupt:
        print("\n\nDiagnostic interrupted by user")
    except Exception as e:
        print(f"\n\n❌ UNEXPECTED ERROR: {e}")
        import traceback
        traceback.print_exc()
    
    print("\n" + "=" * 70)
    print("Next Steps:")
    print("=" * 70)
    print("1. Review the output above for errors and warnings")
    print("2. Check model.rpt for SWMM-specific errors")
    print("3. Verify model.inp has valid subcatchments")
    print("4. Try running model.inp directly in SWMM GUI")
    print("5. Check that time steps are synchronized")
    print("=" * 70)


if __name__ == "__main__":
    main()
