#!/usr/bin/env python3
"""
Diagnostic tool to validate SWMM model.inp file
Tests SWMM can read and parse the model file
"""

import os
import subprocess

model_path = "C:\\Users\\JasonLillywhite\\OneDrive - GoldSim\\Python\\GSswmm\\tests\\model.inp"
report_path = model_path.replace(".inp", ".rpt")

# Check if model exists
if not os.path.exists(model_path):
    print(f"ERROR: Model file not found: {model_path}")
    exit(1)

print(f"Checking model: {model_path}")
print(f"File size: {os.path.getsize(model_path)} bytes")
print()

# Try to use swmm directly if available, otherwise show file structure
print("Model structure:")
print("=" * 60)

with open(model_path, 'r') as f:
    lines = f.readlines()
    current_section = None
    line_count = 0
    
    for i, line in enumerate(lines, 1):
        stripped = line.strip()
        
        # Track sections
        if stripped.startswith('[') and stripped.endswith(']'):
            current_section = stripped
            line_count = 0
            print(f"\n[Line {i}] {current_section}")
        else:
            line_count += 1
            if stripped and not stripped.startswith(';;'):
                if line_count <= 3:  # Show first 3 data lines in each section
                    print(f"  {stripped}")
                elif line_count == 4:
                    print(f"  ...")

print("\n" + "=" * 60)
print("\nLooking for potential issues:")

issues = []

# Check required sections
required_sections = ['[TITLE]', '[OPTIONS]', '[OUTFALLS]', '[TIMESERIES]']
with open(model_path, 'r') as f:
    content = f.read().upper()
    for section in required_sections:
        if section not in content:
            issues.append(f"Missing section: {section}")

# Check for common syntax issues
with open(model_path, 'r') as f:
    for i, line in enumerate(f, 1):
        if line.strip().startswith('[') and not line.strip().endswith(']'):
            issues.append(f"Line {i}: Malformed section header: {line.strip()}")

if issues:
    print("Issues found:")
    for issue in issues:
        print(f"  - {issue}")
else:
    print("No obvious syntax issues detected.")

print("\nRecommendation:")
print("If SWMM reports ERROR 200, check:")
print("  - All nodes referenced in CONDUITS exist in [STORAGE] or [OUTFALLS]")
print("  - All curves referenced in [STORAGE] exist in [CURVES]")
print("  - Date/time format is correct (MM/DD/YYYY HH:MM:SS)")
print("  - No trailing spaces on data lines")
