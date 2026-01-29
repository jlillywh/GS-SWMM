"""
SWMM Model Diagnostic Script

This script analyzes a SWMM input file to identify potential issues
that could cause zero runoff output.

Usage:
    python diagnose_swmm_model.py [model.inp]

If no file is specified, it looks for model.inp in the current directory.
"""

import sys
import os
import re

class SWMMModelDiagnostics:
    def __init__(self, inp_file="model.inp"):
        """Initialize with SWMM input file."""
        self.inp_file = inp_file
        self.content = ""
        self.sections = {}
        
    def load_file(self):
        """Load and parse the SWMM input file."""
        print("=" * 70)
        print("SWMM Model Diagnostics")
        print("=" * 70)
        print(f"File: {self.inp_file}")
        
        if not os.path.exists(self.inp_file):
            print(f"❌ ERROR: File not found: {self.inp_file}")
            return False
            
        try:
            with open(self.inp_file, 'r') as f:
                self.content = f.read()
            print(f"✓ File loaded ({len(self.content)} bytes)")
            return True
        except Exception as e:
            print(f"❌ ERROR reading file: {e}")
            return False
            
    def parse_sections(self):
        """Parse the file into sections."""
        print("\n" + "=" * 70)
        print("Parsing Sections")
        print("=" * 70)
        
        current_section = None
        section_content = []
        
        for line in self.content.split('\n'):
            # Check if this is a section header
            if line.strip().startswith('[') and line.strip().endswith(']'):
                # Save previous section
                if current_section:
                    self.sections[current_section] = '\n'.join(section_content)
                    
                # Start new section
                current_section = line.strip()[1:-1]
                section_content = []
            elif current_section:
                section_content.append(line)
                
        # Save last section
        if current_section:
            self.sections[current_section] = '\n'.join(section_content)
            
        print(f"Found {len(self.sections)} sections:")
        for section in sorted(self.sections.keys()):
            print(f"  • {section}")
            
        return True
        
    def check_options(self):
        """Check OPTIONS section."""
        print("\n" + "=" * 70)
        print("Checking [OPTIONS] Section")
        print("=" * 70)
        
        if 'OPTIONS' not in self.sections:
            print("❌ WARNING: No [OPTIONS] section found")
            return
            
        options = {}
        for line in self.sections['OPTIONS'].split('\n'):
            line = line.strip()
            if line and not line.startswith(';'):
                parts = line.split()
                if len(parts) >= 2:
                    options[parts[0]] = ' '.join(parts[1:])
                    
        # Check critical options
        critical_options = [
            'FLOW_UNITS', 'INFILTRATION', 'FLOW_ROUTING',
            'ROUTING_STEP', 'START_DATE', 'END_DATE'
        ]
        
        for opt in critical_options:
            if opt in options:
                print(f"✓ {opt:20s} = {options[opt]}")
            else:
                print(f"❌ {opt:20s} = NOT SET")
                
        # Check routing step
        if 'ROUTING_STEP' in options:
            routing_step = options['ROUTING_STEP']
            print(f"\n💡 IMPORTANT: GoldSim time step must match: {routing_step}")
            
        return options
        
    def check_subcatchments(self):
        """Check SUBCATCHMENTS section."""
        print("\n" + "=" * 70)
        print("Checking [SUBCATCHMENTS] Section")
        print("=" * 70)
        
        if 'SUBCATCHMENTS' not in self.sections:
            print("❌ CRITICAL: No [SUBCATCHMENTS] section found!")
            print("   This will cause zero runoff - SWMM has nothing to simulate")
            return []
            
        subcatchments = []
        lines = self.sections['SUBCATCHMENTS'].split('\n')
        
        for line in lines:
            line = line.strip()
            if line and not line.startswith(';'):
                parts = line.split()
                if len(parts) >= 7:
                    subcatch = {
                        'name': parts[0],
                        'rain_gage': parts[1],
                        'outlet': parts[2],
                        'area': float(parts[3]),
                        'imperv': float(parts[4]),
                        'width': float(parts[5]),
                        'slope': float(parts[6])
                    }
                    subcatchments.append(subcatch)
                    
        print(f"Found {len(subcatchments)} subcatchment(s)")
        
        if len(subcatchments) == 0:
            print("❌ CRITICAL: No subcatchments defined!")
            print("   This will cause zero runoff")
            return []
            
        # Analyze each subcatchment
        for i, sub in enumerate(subcatchments):
            print(f"\nSubcatchment {i} (Index {i}):")
            print(f"  Name:        {sub['name']}")
            print(f"  Rain Gage:   {sub['rain_gage']}")
            print(f"  Outlet:      {sub['outlet']}")
            print(f"  Area:        {sub['area']} acres", end="")
            if sub['area'] <= 0:
                print(" ❌ ZERO OR NEGATIVE!")
            else:
                print(" ✓")
            print(f"  % Imperv:    {sub['imperv']}%", end="")
            if sub['imperv'] < 0 or sub['imperv'] > 100:
                print(" ❌ OUT OF RANGE!")
            else:
                print(" ✓")
            print(f"  Width:       {sub['width']} ft", end="")
            if sub['width'] <= 0:
                print(" ❌ ZERO OR NEGATIVE!")
            else:
                print(" ✓")
            print(f"  Slope:       {sub['slope']} %", end="")
            if sub['slope'] <= 0:
                print(" ❌ ZERO OR NEGATIVE!")
            else:
                print(" ✓")
                
        return subcatchments
        
    def check_subareas(self):
        """Check SUBAREAS section."""
        print("\n" + "=" * 70)
        print("Checking [SUBAREAS] Section")
        print("=" * 70)
        
        if 'SUBAREAS' not in self.sections:
            print("❌ WARNING: No [SUBAREAS] section found")
            print("   SWMM will use default values")
            return
            
        subareas = []
        lines = self.sections['SUBAREAS'].split('\n')
        
        for line in lines:
            line = line.strip()
            if line and not line.startswith(';'):
                parts = line.split()
                if len(parts) >= 7:
                    subarea = {
                        'subcatch': parts[0],
                        'n_imperv': float(parts[1]),
                        'n_perv': float(parts[2]),
                        's_imperv': float(parts[3]),
                        's_perv': float(parts[4]),
                        'pct_zero': float(parts[5]),
                        'route_to': parts[6]
                    }
                    subareas.append(subarea)
                    
        print(f"Found {len(subareas)} subarea definition(s)")
        
        for subarea in subareas:
            print(f"\n  Subcatchment: {subarea['subcatch']}")
            print(f"    N-Imperv:  {subarea['n_imperv']}")
            print(f"    N-Perv:    {subarea['n_perv']}")
            print(f"    S-Imperv:  {subarea['s_imperv']}")
            print(f"    S-Perv:    {subarea['s_perv']}")
            
    def check_infiltration(self):
        """Check INFILTRATION section."""
        print("\n" + "=" * 70)
        print("Checking [INFILTRATION] Section")
        print("=" * 70)
        
        if 'INFILTRATION' not in self.sections:
            print("❌ WARNING: No [INFILTRATION] section found")
            print("   SWMM will use default values")
            return
            
        infiltrations = []
        lines = self.sections['INFILTRATION'].split('\n')
        
        for line in lines:
            line = line.strip()
            if line and not line.startswith(';'):
                parts = line.split()
                if len(parts) >= 4:
                    infil = {
                        'subcatch': parts[0],
                        'param1': float(parts[1]),
                        'param2': float(parts[2]),
                        'param3': float(parts[3])
                    }
                    infiltrations.append(infil)
                    
        print(f"Found {len(infiltrations)} infiltration definition(s)")
        
        for infil in infiltrations:
            print(f"\n  Subcatchment: {infil['subcatch']}")
            print(f"    Parameters: {infil['param1']}, {infil['param2']}, {infil['param3']}")
            
            # Check if infiltration is too high
            if infil['param1'] > 10:  # Max infiltration rate > 10 in/hr
                print(f"    ⚠️  WARNING: High infiltration rate ({infil['param1']} in/hr)")
                print(f"       This could cause all rainfall to infiltrate (zero runoff)")
                
    def check_raingages(self):
        """Check RAINGAGES section."""
        print("\n" + "=" * 70)
        print("Checking [RAINGAGES] Section")
        print("=" * 70)
        
        if 'RAINGAGES' not in self.sections:
            print("❌ WARNING: No [RAINGAGES] section found")
            return
            
        raingages = []
        lines = self.sections['RAINGAGES'].split('\n')
        
        for line in lines:
            line = line.strip()
            if line and not line.startswith(';'):
                parts = line.split()
                if len(parts) >= 4:
                    gage = {
                        'name': parts[0],
                        'format': parts[1],
                        'interval': parts[2],
                        'scf': parts[3]
                    }
                    raingages.append(gage)
                    
        print(f"Found {len(raingages)} rain gage(s)")
        
        for gage in raingages:
            print(f"\n  Rain Gage: {gage['name']}")
            print(f"    Format:   {gage['format']}")
            print(f"    Interval: {gage['interval']}")
            print(f"    SCF:      {gage['scf']}")
            
    def check_timeseries(self):
        """Check TIMESERIES section."""
        print("\n" + "=" * 70)
        print("Checking [TIMESERIES] Section")
        print("=" * 70)
        
        if 'TIMESERIES' not in self.sections:
            print("⚠️  No [TIMESERIES] section found")
            print("   Rainfall will be set dynamically by bridge (this is OK)")
            return
            
        print("✓ [TIMESERIES] section found")
        print("  Note: Bridge overrides rainfall with GoldSim values")
        
    def generate_recommendations(self, subcatchments):
        """Generate recommendations based on analysis."""
        print("\n" + "=" * 70)
        print("RECOMMENDATIONS")
        print("=" * 70)
        
        if len(subcatchments) == 0:
            print("\n❌ CRITICAL ISSUE: No subcatchments defined")
            print("\nTo fix:")
            print("  1. Add at least one subcatchment to [SUBCATCHMENTS] section")
            print("  2. Define subarea parameters in [SUBAREAS] section")
            print("  3. Define infiltration parameters in [INFILTRATION] section")
            return
            
        # Check for zero area
        zero_area = [s for s in subcatchments if s['area'] <= 0]
        if zero_area:
            print("\n❌ CRITICAL: Subcatchments with zero or negative area:")
            for s in zero_area:
                print(f"     {s['name']}: {s['area']} acres")
            print("   Fix: Set area to positive value (e.g., 10 acres)")
            
        # Check for 100% impervious
        all_imperv = [s for s in subcatchments if s['imperv'] >= 100]
        if all_imperv:
            print("\n⚠️  WARNING: Subcatchments with 100% impervious:")
            for s in all_imperv:
                print(f"     {s['name']}: {s['imperv']}%")
            print("   Note: This is OK, but check if intentional")
            
        # Check for zero width
        zero_width = [s for s in subcatchments if s['width'] <= 0]
        if zero_width:
            print("\n❌ CRITICAL: Subcatchments with zero or negative width:")
            for s in zero_width:
                print(f"     {s['name']}: {s['width']} ft")
            print("   Fix: Set width to positive value (e.g., 500 ft)")
            
        # Check for zero slope
        zero_slope = [s for s in subcatchments if s['slope'] <= 0]
        if zero_slope:
            print("\n❌ CRITICAL: Subcatchments with zero or negative slope:")
            for s in zero_slope:
                print(f"     {s['name']}: {s['slope']}%")
            print("   Fix: Set slope to positive value (e.g., 0.5%)")
            
        print("\n" + "=" * 70)
        print("TESTING SUGGESTIONS")
        print("=" * 70)
        print("\n1. Test SWMM model independently:")
        print("   • Open model.inp in SWMM GUI")
        print("   • Run simulation")
        print("   • Check for errors in Status Report")
        print("   • View Subcatchment Runoff Summary")
        
        print("\n2. Simplify for testing:")
        print("   • Use single subcatchment")
        print("   • Set area = 10 acres")
        print("   • Set % imperv = 50%")
        print("   • Set width = 500 ft")
        print("   • Set slope = 0.5%")
        
        print("\n3. Check time synchronization:")
        print("   • GoldSim time step must equal SWMM ROUTING_STEP")
        print("   • Both must use same time units")
        
        print("\n4. Verify rainfall is being set:")
        print("   • Add debug output to bridge DLL")
        print("   • Check that swmm_setValue is called")
        print("   • Verify rainfall values are non-zero")
        
    def run_diagnostics(self):
        """Run complete diagnostics."""
        if not self.load_file():
            return False
            
        if not self.parse_sections():
            return False
            
        options = self.check_options()
        subcatchments = self.check_subcatchments()
        self.check_subareas()
        self.check_infiltration()
        self.check_raingages()
        self.check_timeseries()
        
        self.generate_recommendations(subcatchments)
        
        return True


def main():
    """Main entry point."""
    inp_file = sys.argv[1] if len(sys.argv) > 1 else "model.inp"
    
    diag = SWMMModelDiagnostics(inp_file)
    
    try:
        diag.run_diagnostics()
    except Exception as e:
        print(f"\n❌ ERROR: {e}")
        import traceback
        traceback.print_exc()
        
    print("\n" + "=" * 70)
    print("Diagnostic complete")
    print("=" * 70)


if __name__ == "__main__":
    main()
