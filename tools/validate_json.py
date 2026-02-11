#!/usr/bin/env python3
"""
EVE OFFLINE - JSON Validation Tool

Validates all JSON files in the data/ directory for syntax errors,
missing required fields, and common modding mistakes.

Usage:
    python tools/validate_json.py
    python tools/validate_json.py --verbose
    python tools/validate_json.py --file data/ships/frigates.json
"""

import json
import os
import sys
import argparse
from pathlib import Path
from typing import Dict, List, Tuple, Any


class Colors:
    """ANSI color codes for terminal output"""
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'


def print_success(msg: str):
    print(f"{Colors.OKGREEN}✓{Colors.ENDC} {msg}")


def print_error(msg: str):
    print(f"{Colors.FAIL}✗{Colors.ENDC} {msg}")


def print_warning(msg: str):
    print(f"{Colors.WARNING}⚠{Colors.ENDC} {msg}")


def print_info(msg: str):
    print(f"{Colors.OKBLUE}ℹ{Colors.ENDC} {msg}")


def validate_json_syntax(filepath: Path) -> Tuple[bool, str]:
    """
    Validate that a file contains valid JSON syntax.
    Returns (is_valid, error_message)
    """
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            json.load(f)
        return True, ""
    except json.JSONDecodeError as e:
        return False, f"Line {e.lineno}, Column {e.colno}: {e.msg}"
    except Exception as e:
        return False, str(e)


def validate_ship(ship_id: str, ship_data: Dict, filepath: Path) -> List[str]:
    """Validate a ship definition"""
    errors = []
    warnings = []
    
    # Required fields
    required = ['name', 'class', 'tech_level', 'base_hp', 'base_capacitor', 
                'base_shield', 'base_armor', 'cargo_capacity', 'max_velocity',
                'slot_layout']
    
    for field in required:
        if field not in ship_data:
            errors.append(f"Ship '{ship_id}' missing required field: {field}")
    
    # Validate slot_layout
    if 'slot_layout' in ship_data:
        slot_layout = ship_data['slot_layout']
        if not isinstance(slot_layout, dict):
            errors.append(f"Ship '{ship_id}': slot_layout must be an object")
        else:
            for slot_type in ['high_slots', 'mid_slots', 'low_slots']:
                if slot_type not in slot_layout:
                    warnings.append(f"Ship '{ship_id}': missing {slot_type} in slot_layout")
    
    # Validate numeric values
    numeric_fields = {
        'base_hp': (1, 1000000),
        'base_capacitor': (1, 100000),
        'base_shield': (0, 100000),
        'base_armor': (0, 100000),
        'cargo_capacity': (0, 1000000),
        'max_velocity': (1, 10000),
        'tech_level': (1, 3)
    }
    
    for field, (min_val, max_val) in numeric_fields.items():
        if field in ship_data:
            value = ship_data[field]
            if not isinstance(value, (int, float)):
                errors.append(f"Ship '{ship_id}': {field} must be a number")
            elif value < min_val or value > max_val:
                warnings.append(f"Ship '{ship_id}': {field}={value} outside typical range [{min_val}, {max_val}]")
    
    # Validate class
    valid_classes = ['Frigate', 'Destroyer', 'Cruiser', 'Battlecruiser', 
                     'Battleship', 'Carrier', 'Dreadnought', 'Titan',
                     'Mining Barge', 'Exhumer', 'Industrial',
                     'Assault Frigate', 'Interceptor', 'Covert Ops',
                     'Heavy Assault Cruiser', 'Heavy Interdictor', 'Recon Ship',
                     'Logistics Cruiser', 'Command Ship', 'Interdictor',
                     'Stealth Bomber', 'Marauder']
    
    if 'class' in ship_data and ship_data['class'] not in valid_classes:
        warnings.append(f"Ship '{ship_id}': class '{ship_data['class']}' not in standard classes")
    
    return errors + warnings


def validate_module(module_id: str, module_data: Dict, filepath: Path) -> List[str]:
    """Validate a module definition"""
    errors = []
    warnings = []
    
    # Required fields
    required = ['name', 'type', 'slot_type', 'cpu_usage', 'powergrid_usage']
    
    for field in required:
        if field not in module_data:
            errors.append(f"Module '{module_id}' missing required field: {field}")
    
    # Validate slot_type
    valid_slot_types = ['high', 'mid', 'low', 'rig', 'subsystem']
    if 'slot_type' in module_data:
        if module_data['slot_type'] not in valid_slot_types:
            errors.append(f"Module '{module_id}': invalid slot_type '{module_data['slot_type']}'")
    
    # Validate numeric values
    if 'cpu_usage' in module_data and module_data['cpu_usage'] < 0:
        errors.append(f"Module '{module_id}': cpu_usage cannot be negative")
    
    if 'powergrid_usage' in module_data and module_data['powergrid_usage'] < 0:
        errors.append(f"Module '{module_id}': powergrid_usage cannot be negative")
    
    return errors + warnings


def validate_mission(mission_id: str, mission_data: Dict, filepath: Path) -> List[str]:
    """Validate a mission definition"""
    errors = []
    warnings = []
    
    # Required fields
    required = ['name', 'level', 'type', 'objective']
    
    for field in required:
        if field not in mission_data:
            errors.append(f"Mission '{mission_id}' missing required field: {field}")
    
    # Validate level
    if 'level' in mission_data:
        level = mission_data['level']
        if not isinstance(level, int) or level < 1 or level > 5:
            errors.append(f"Mission '{mission_id}': level must be 1-5")
    
    # Validate type
    valid_types = ['combat', 'mining', 'courier', 'trade', 'scenario', 'exploration', 'storyline']
    if 'type' in mission_data and mission_data['type'] not in valid_types:
        warnings.append(f"Mission '{mission_id}': type '{mission_data['type']}' not in standard types")
    
    return errors + warnings


def validate_file(filepath: Path, verbose: bool = False) -> Tuple[int, int]:
    """
    Validate a single JSON file.
    Returns (error_count, warning_count)
    """
    # First check JSON syntax
    is_valid, error_msg = validate_json_syntax(filepath)
    if not is_valid:
        print_error(f"{filepath}: {error_msg}")
        return 1, 0
    
    # Determine file type and validate content
    errors = []
    warnings = []
    
    try:
        with open(filepath, 'r', encoding='utf-8') as f:
            data = json.load(f)
        
        # Detect file type by path
        try:
            relative_path = filepath.relative_to(Path.cwd())
        except ValueError:
            relative_path = filepath
        path_str = str(relative_path).lower()
        
        if 'ships/' in path_str:
            for ship_id, ship_data in data.items():
                issues = validate_ship(ship_id, ship_data, filepath)
                for issue in issues:
                    if 'missing required' in issue or 'must be' in issue or 'cannot be' in issue:
                        errors.append(issue)
                    else:
                        warnings.append(issue)
        
        elif 'modules/' in path_str:
            for module_id, module_data in data.items():
                issues = validate_module(module_id, module_data, filepath)
                for issue in issues:
                    if 'missing required' in issue or 'invalid' in issue or 'cannot be' in issue:
                        errors.append(issue)
                    else:
                        warnings.append(issue)
        
        elif 'missions/' in path_str:
            for mission_id, mission_data in data.items():
                issues = validate_mission(mission_id, mission_data, filepath)
                for issue in issues:
                    if 'missing required' in issue or 'must be' in issue:
                        errors.append(issue)
                    else:
                        warnings.append(issue)
    
    except Exception as e:
        print_error(f"{filepath}: Unexpected error: {e}")
        return 1, 0
    
    # Print results
    if errors:
        print_error(f"{filepath}: {len(errors)} error(s)")
        if verbose:
            for error in errors:
                print(f"  {error}")
    elif warnings:
        print_warning(f"{filepath}: {len(warnings)} warning(s)")
        if verbose:
            for warning in warnings:
                print(f"  {warning}")
    else:
        if verbose:
            print_success(f"{filepath}: Valid")
    
    return len(errors), len(warnings)


def main():
    parser = argparse.ArgumentParser(description='Validate EVE OFFLINE JSON files')
    parser.add_argument('--file', type=str, help='Validate a single file')
    parser.add_argument('--verbose', '-v', action='store_true', help='Show detailed output')
    parser.add_argument('--data-dir', type=str, default='data', help='Data directory to scan (default: data)')
    args = parser.parse_args()
    
    print(f"{Colors.BOLD}{Colors.HEADER}EVE OFFLINE - JSON Validation Tool{Colors.ENDC}\n")
    
    total_errors = 0
    total_warnings = 0
    total_files = 0
    
    if args.file:
        # Validate single file
        filepath = Path(args.file)
        if not filepath.exists():
            print_error(f"File not found: {filepath}")
            return 1
        
        errors, warnings = validate_file(filepath, args.verbose)
        total_errors += errors
        total_warnings += warnings
        total_files = 1
    else:
        # Validate all JSON files in data directory
        data_dir = Path(args.data_dir)
        if not data_dir.exists():
            print_error(f"Data directory not found: {data_dir}")
            return 1
        
        # Find all JSON files
        json_files = sorted(data_dir.glob('**/*.json'))
        
        if not json_files:
            print_warning(f"No JSON files found in {data_dir}")
            return 0
        
        print_info(f"Scanning {len(json_files)} JSON files in {data_dir}/\n")
        
        for filepath in json_files:
            errors, warnings = validate_file(filepath, args.verbose)
            total_errors += errors
            total_warnings += warnings
            total_files += 1
    
    # Print summary
    print(f"\n{Colors.BOLD}Summary:{Colors.ENDC}")
    print(f"  Files checked: {total_files}")
    
    if total_errors == 0:
        print_success(f"No errors found")
    else:
        print_error(f"{total_errors} error(s) found")
    
    if total_warnings > 0:
        print_warning(f"{total_warnings} warning(s) found")
    
    return 1 if total_errors > 0 else 0


if __name__ == '__main__':
    sys.exit(main())
