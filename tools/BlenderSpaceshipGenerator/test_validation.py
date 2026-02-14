"""
Syntax and structure validation tests for the Blender Spaceship Generator addon
These tests run without requiring Blender to be installed
"""

import ast
import os
import sys


def test_python_syntax(filepath):
    """Test if a Python file has valid syntax"""
    try:
        with open(filepath, 'r') as f:
            code = f.read()
        ast.parse(code)
        return True, None
    except SyntaxError as e:
        return False, str(e)


def test_addon_structure():
    """Test that all required files exist"""
    print("Testing addon structure...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    required_files = [
        '__init__.py',
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
        'atlas_exporter.py',
        'station_generator.py',
        'asteroid_generator.py',
        'texture_generator.py',
        'brick_system.py',
    ]
    
    all_exist = True
    for filename in required_files:
        filepath = os.path.join(addon_path, filename)
        if os.path.exists(filepath):
            print(f"✓ {filename} exists")
        else:
            print(f"✗ {filename} is missing")
            all_exist = False
    
    return all_exist


def test_file_syntax():
    """Test Python syntax of all module files"""
    print("\nTesting Python syntax...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    python_files = [
        '__init__.py',
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
        'atlas_exporter.py',
        'station_generator.py',
        'asteroid_generator.py',
        'texture_generator.py',
        'brick_system.py',
    ]
    
    all_valid = True
    for filename in python_files:
        filepath = os.path.join(addon_path, filename)
        if os.path.exists(filepath):
            valid, error = test_python_syntax(filepath)
            if valid:
                print(f"✓ {filename} has valid syntax")
            else:
                print(f"✗ {filename} has syntax error: {error}")
                all_valid = False
    
    return all_valid


def test_bl_info():
    """Test that __init__.py has valid bl_info"""
    print("\nTesting bl_info metadata...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    init_file = os.path.join(addon_path, '__init__.py')
    
    try:
        with open(init_file, 'r') as f:
            content = f.read()
        
        # Check for bl_info
        if 'bl_info' not in content:
            print("✗ bl_info not found in __init__.py")
            return False
        
        # Parse and check bl_info
        tree = ast.parse(content)
        bl_info_found = False
        
        for node in ast.walk(tree):
            if isinstance(node, ast.Assign):
                for target in node.targets:
                    if isinstance(target, ast.Name) and target.id == 'bl_info':
                        bl_info_found = True
                        
                        # Check if it's a dictionary
                        if isinstance(node.value, ast.Dict):
                            keys = [k.s if isinstance(k, ast.Str) else (k.value if isinstance(k, ast.Constant) else None) 
                                   for k in node.value.keys]
                            
                            required_keys = ['name', 'author', 'version', 'blender', 'category']
                            for key in required_keys:
                                if key in keys:
                                    print(f"✓ bl_info has '{key}' key")
                                else:
                                    print(f"✗ bl_info missing '{key}' key")
                                    return False
                        break
        
        if not bl_info_found:
            print("✗ bl_info assignment not found")
            return False
        
        return True
        
    except Exception as e:
        print(f"✗ Error checking bl_info: {e}")
        return False


def test_register_functions():
    """Test that register/unregister functions exist"""
    print("\nTesting register/unregister functions...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    files_to_check = [
        '__init__.py',
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
        'atlas_exporter.py',
        'station_generator.py',
        'asteroid_generator.py',
        'texture_generator.py',
        'brick_system.py',
    ]
    
    all_valid = True
    for filename in files_to_check:
        filepath = os.path.join(addon_path, filename)
        
        try:
            with open(filepath, 'r') as f:
                content = f.read()
            
            has_register = 'def register()' in content
            has_unregister = 'def unregister()' in content
            
            if has_register and has_unregister:
                print(f"✓ {filename} has register/unregister functions")
            else:
                missing = []
                if not has_register:
                    missing.append('register')
                if not has_unregister:
                    missing.append('unregister')
                print(f"✗ {filename} missing: {', '.join(missing)}")
                all_valid = False
                
        except Exception as e:
            print(f"✗ Error checking {filename}: {e}")
            all_valid = False
    
    return all_valid


def test_documentation():
    """Test that documentation files exist"""
    print("\nTesting documentation...")
    
    addon_path = os.path.dirname(os.path.abspath(__file__))
    doc_files = ['README.md', 'USAGE.md']
    
    all_exist = True
    for filename in doc_files:
        filepath = os.path.join(addon_path, filename)
        if os.path.exists(filepath):
            size = os.path.getsize(filepath)
            print(f"✓ {filename} exists ({size} bytes)")
        else:
            print(f"✗ {filename} is missing")
            all_exist = False
    
    return all_exist


def test_turret_hardpoint_configs():
    """Test that all ship configs include turret_hardpoints and respect max 10"""
    print("\nTesting turret hardpoint configurations...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sg_path = os.path.join(addon_path, 'ship_generator.py')

    with open(sg_path, 'r') as f:
        content = f.read()

    tree = ast.parse(content)
    ship_configs = None

    for node in ast.walk(tree):
        if isinstance(node, ast.Assign):
            for target in node.targets:
                if isinstance(target, ast.Name) and target.id == 'SHIP_CONFIGS':
                    ship_configs = node.value
                    break

    if ship_configs is None:
        print("✗ SHIP_CONFIGS not found in ship_generator.py")
        return False

    if not isinstance(ship_configs, ast.Dict):
        print("✗ SHIP_CONFIGS is not a dictionary")
        return False

    all_valid = True
    for key_node, value_node in zip(ship_configs.keys, ship_configs.values):
        class_name = key_node.value if isinstance(key_node, ast.Constant) else '?'
        if not isinstance(value_node, ast.Dict):
            continue
        inner_keys = []
        for k in value_node.keys:
            if isinstance(k, ast.Constant):
                inner_keys.append(k.value)
        if 'turret_hardpoints' not in inner_keys:
            print(f"✗ {class_name} missing 'turret_hardpoints' key")
            all_valid = False
        else:
            # Find the value
            idx = inner_keys.index('turret_hardpoints')
            val_node = value_node.values[idx]
            if isinstance(val_node, ast.Constant) and isinstance(val_node.value, int):
                if val_node.value > 10:
                    print(f"✗ {class_name} turret_hardpoints={val_node.value} exceeds max 10")
                    all_valid = False
                else:
                    print(f"✓ {class_name} turret_hardpoints={val_node.value}")
            else:
                print(f"✗ {class_name} turret_hardpoints is not an integer")
                all_valid = False

    return all_valid


def test_naming_prefix_support():
    """Test that key generation modules define _prefixed_name helper"""
    print("\nTesting naming prefix support...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    files_needing_prefix = [
        'ship_generator.py',
        'ship_parts.py',
        'interior_generator.py',
        'module_system.py',
    ]

    all_valid = True
    for filename in files_needing_prefix:
        filepath = os.path.join(addon_path, filename)
        with open(filepath, 'r') as f:
            content = f.read()
        if 'def _prefixed_name(' in content:
            print(f"✓ {filename} has _prefixed_name helper")
        else:
            print(f"✗ {filename} missing _prefixed_name helper")
            all_valid = False

    return all_valid


def test_turret_generation_function():
    """Test that ship_parts.py defines generate_turret_hardpoints"""
    print("\nTesting turret generation function...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'def generate_turret_hardpoints(': 'generate_turret_hardpoints function',
        '"turret_index"': 'turret_index custom property',
        '"turret_type"': 'turret_type custom property',
        '"tracking_speed"': 'tracking_speed custom property',
        '"rotation_limits"': 'rotation_limits custom property',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_brick_system():
    """Test the brick taxonomy, scale bands, grid system and Ship DNA helpers"""
    print("\nTesting brick system...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    bs_path = os.path.join(addon_path, 'brick_system.py')

    # Import brick_system directly (no bpy dependency)
    import importlib.util
    spec = importlib.util.spec_from_file_location("brick_system", bs_path)
    bs = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(bs)

    all_valid = True

    # Check BRICK_TYPES has entries for every category
    for category, brick_names in bs.BRICK_CATEGORIES.items():
        for name in brick_names:
            if name not in bs.BRICK_TYPES:
                print(f"✗ Brick type {name} (category {category}) missing from BRICK_TYPES")
                all_valid = False
            else:
                bt = bs.BRICK_TYPES[name]
                for key in ('category', 'size', 'shape', 'scale_band', 'hardpoints'):
                    if key not in bt:
                        print(f"✗ Brick {name} missing key '{key}'")
                        all_valid = False
    if all_valid:
        print(f"✓ All {sum(len(v) for v in bs.BRICK_CATEGORIES.values())} brick types valid")

    # Scale bands
    for band in ('primary', 'structural', 'detail'):
        val = bs.get_scale_factor(band)
        if not (0 < val <= 1.0):
            print(f"✗ Scale band '{band}' has invalid factor {val}")
            all_valid = False
    if bs.get_scale_factor('primary') > bs.get_scale_factor('structural') > bs.get_scale_factor('detail'):
        print("✓ Scale hierarchy correct (primary > structural > detail)")
    else:
        print("✗ Scale hierarchy incorrect")
        all_valid = False

    # Grid snapping
    snapped = bs.snap_to_grid((1.3, 2.7, 0.1), 1.0)
    if snapped == (1.0, 3.0, 0.0):
        print("✓ snap_to_grid works correctly")
    else:
        print(f"✗ snap_to_grid returned {snapped}, expected (1.0, 3.0, 0.0)")
        all_valid = False

    # Engine archetypes
    for arch in ('MAIN_THRUST', 'MANEUVERING', 'UTILITY_EXHAUST'):
        if bs.get_engine_archetype(arch) is None:
            print(f"✗ Engine archetype {arch} missing")
            all_valid = False
    print("✓ All engine archetypes defined")

    # Ship DNA round-trip
    dna = bs.generate_ship_dna('CRUISER', 42, [{'type': 'REACTOR_CORE', 'pos': [0, 0, 0]}])
    json_str = bs.ship_dna_to_json(dna)
    loaded = bs.ship_dna_from_json(json_str)
    if loaded['seed'] == 42 and loaded['class'] == 'CRUISER' and len(loaded['bricks']) == 1:
        print("✓ Ship DNA round-trip works")
    else:
        print("✗ Ship DNA round-trip failed")
        all_valid = False

    return all_valid


def test_hull_taper_and_cleanup():
    """Test that ship_generator defines taper_hull and apply_cleanup_pass"""
    print("\nTesting hull taper and cleanup pass...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sg_path = os.path.join(addon_path, 'ship_generator.py')

    with open(sg_path, 'r') as f:
        content = f.read()

    checks = {
        'def taper_hull(': 'taper_hull function',
        'def apply_cleanup_pass(': 'apply_cleanup_pass function',
        'hull_taper': 'hull_taper parameter',
        '"ship_dna"': 'Ship DNA custom property',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def test_engine_archetypes():
    """Test that ship_parts uses engine archetypes"""
    print("\nTesting engine archetype integration...")

    addon_path = os.path.dirname(os.path.abspath(__file__))
    sp_path = os.path.join(addon_path, 'ship_parts.py')

    with open(sp_path, 'r') as f:
        content = f.read()

    checks = {
        'engine_archetype': 'engine archetype custom property',
        'select_engine_archetype': 'archetype selection',
        '_add_nozzle_flare': 'nozzle flare helper',
    }

    all_valid = True
    for pattern, description in checks.items():
        if pattern in content:
            print(f"✓ {description} found")
        else:
            print(f"✗ {description} not found")
            all_valid = False

    return all_valid


def run_tests():
    """Run all validation tests"""
    print("=" * 60)
    print("Blender Spaceship Generator - Validation Tests")
    print("=" * 60)
    
    tests = [
        ("Addon Structure", test_addon_structure),
        ("Python Syntax", test_file_syntax),
        ("bl_info Metadata", test_bl_info),
        ("Register Functions", test_register_functions),
        ("Documentation", test_documentation),
        ("Turret Hardpoint Configs", test_turret_hardpoint_configs),
        ("Naming Prefix Support", test_naming_prefix_support),
        ("Turret Generation Function", test_turret_generation_function),
        ("Brick System", test_brick_system),
        ("Hull Taper & Cleanup", test_hull_taper_and_cleanup),
        ("Engine Archetypes", test_engine_archetypes),
    ]
    
    results = []
    for test_name, test_func in tests:
        try:
            result = test_func()
            results.append((test_name, result))
        except Exception as e:
            print(f"\n✗ {test_name} raised exception: {e}")
            import traceback
            traceback.print_exc()
            results.append((test_name, False))
    
    print("\n" + "=" * 60)
    print("Test Results Summary:")
    print("=" * 60)
    
    passed = sum(1 for _, result in results if result)
    total = len(results)
    
    for test_name, result in results:
        status = "✓ PASS" if result else "✗ FAIL"
        print(f"{status}: {test_name}")
    
    print("=" * 60)
    print(f"Total: {passed}/{total} tests passed")
    
    if passed == total:
        print("✓ All validation tests passed!")
    else:
        print("✗ Some tests failed")
    
    print("=" * 60)
    
    return passed == total


if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)
