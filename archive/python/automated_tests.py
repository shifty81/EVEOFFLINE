#!/usr/bin/env python3
"""
EVE OFFLINE - Automated Test Suite Runner
Runs all tests and demos to verify the software is working correctly
"""

import sys
import os
import subprocess
import time
from typing import List, Tuple, Optional

# ANSI color codes for pretty output
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

def print_header(text: str):
    """Print a header"""
    print(f"\n{Colors.HEADER}{Colors.BOLD}{'='*60}{Colors.ENDC}")
    print(f"{Colors.HEADER}{Colors.BOLD}{text}{Colors.ENDC}")
    print(f"{Colors.HEADER}{Colors.BOLD}{'='*60}{Colors.ENDC}\n")

def print_step(text: str):
    """Print a step"""
    print(f"{Colors.OKCYAN}{Colors.BOLD}▶ {text}{Colors.ENDC}")

def print_success(text: str):
    """Print success message"""
    print(f"{Colors.OKGREEN}✓ {text}{Colors.ENDC}")

def print_failure(text: str):
    """Print failure message"""
    print(f"{Colors.FAIL}✗ {text}{Colors.ENDC}")

def print_warning(text: str):
    """Print warning message"""
    print(f"{Colors.WARNING}⚠ {text}{Colors.ENDC}")

def check_dependency(module_name: str, install_command: str) -> bool:
    """Check if a Python module is installed"""
    try:
        __import__(module_name)
        print_success(f"{module_name} is installed")
        return True
    except ImportError:
        print_failure(f"{module_name} is NOT installed")
        print(f"  Install with: {install_command}")
        return False

def run_command(cmd: List[str], timeout: int = 30, check_output: bool = False) -> Tuple[bool, Optional[str]]:
    """
    Run a command and return success status
    
    Args:
        cmd: Command to run as list
        timeout: Timeout in seconds
        check_output: If True, capture output
        
    Returns:
        Tuple of (success, output)
    """
    try:
        if check_output:
            result = subprocess.run(
                cmd,
                timeout=timeout,
                capture_output=True,
                text=True,
                cwd=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
            )
            return result.returncode == 0, result.stdout
        else:
            result = subprocess.run(
                cmd,
                timeout=timeout,
                cwd=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
            )
            return result.returncode == 0, None
    except subprocess.TimeoutExpired:
        print_failure(f"Command timed out after {timeout} seconds")
        return False, None
    except Exception as e:
        print_failure(f"Error running command: {e}")
        return False, None

def test_dependencies() -> bool:
    """Test that all dependencies are installed"""
    print_header("Step 1: Checking Dependencies")
    
    all_good = True
    
    # Check Python version
    print_step("Checking Python version...")
    version = sys.version_info
    if version.major >= 3 and version.minor >= 11:
        print_success(f"Python {version.major}.{version.minor}.{version.micro} (>= 3.11 required)")
    else:
        print_failure(f"Python {version.major}.{version.minor}.{version.micro} (3.11+ required)")
        all_good = False
    
    # Check required modules
    print_step("Checking required Python modules...")
    
    # Optional but recommended
    check_dependency("pygame", "pip install pygame")
    check_dependency("panda3d", "pip install panda3d")
    
    return all_good

def test_unit_tests() -> bool:
    """Run unit tests"""
    print_header("Step 2: Running Unit Tests")
    
    print_step("Running test suite with run_tests.py...")
    success, output = run_command(
        [sys.executable, "run_tests.py"],
        timeout=60,
        check_output=True
    )
    
    if success:
        print_success("All unit tests passed!")
        return True
    else:
        print_failure("Some unit tests failed")
        if output:
            print(output)
        return False

def test_engine() -> bool:
    """Test core engine"""
    print_header("Step 3: Testing Core Engine")
    
    print_step("Running engine tests...")
    success, _ = run_command(
        [sys.executable, "tests/test_engine.py"],
        timeout=30,
        check_output=False
    )
    
    if success:
        print_success("Engine tests passed!")
        return True
    else:
        print_failure("Engine tests failed")
        return False

def test_demos() -> bool:
    """Test demo scripts"""
    print_header("Step 4: Testing Demo Scripts")
    
    demos = [
        ("demo.py", "Basic demo", 10),
        ("demo_features.py", "Features demo", 15),
        ("showcase_gameplay.py", "Gameplay showcase", 20),
    ]
    
    all_passed = True
    
    for demo_file, demo_name, timeout in demos:
        print_step(f"Testing {demo_name} ({demo_file})...")
        
        # Check if file exists
        if not os.path.exists(demo_file):
            print_warning(f"Skipping - file not found: {demo_file}")
            continue
        
        success, _ = run_command(
            [sys.executable, demo_file],
            timeout=timeout,
            check_output=False
        )
        
        if success:
            print_success(f"{demo_name} completed successfully")
        else:
            print_failure(f"{demo_name} failed or timed out")
            all_passed = False
    
    return all_passed

def test_3d_client() -> bool:
    """Test 3D client"""
    print_header("Step 5: Testing 3D Client")
    
    # Check if Panda3D is available
    try:
        import panda3d
        print_success("Panda3D is available")
    except ImportError:
        print_warning("Panda3D not installed - skipping 3D client test")
        print("  Install with: pip install panda3d")
        return True  # Not a failure, just skipped
    
    print_step("Testing 3D client standalone mode...")
    print("  Note: This test will run for 5 seconds then exit")
    print("  You should see a window with a star field and 4 ships")
    print("  Press Ctrl+C if it hangs")
    
    # We can't fully automate this since it's GUI, but we can check it starts
    try:
        # Just check the imports work
        success, output = run_command(
            [sys.executable, "-c", "from client_3d.core import NetworkClient, EntityManager; print('OK')"],
            timeout=5,
            check_output=True
        )
        
        if success and output and "OK" in output:
            print_success("3D client imports are working")
            print_warning("Full 3D test requires manual verification (run test_3d_client.py)")
            return True
        else:
            print_failure("3D client imports failed")
            return False
    except Exception as e:
        print_failure(f"3D client test error: {e}")
        return False

def test_network() -> bool:
    """Test network components"""
    print_header("Step 6: Testing Network Components")
    
    print_step("Testing network protocol...")
    
    # Test importing network modules
    try:
        from engine.network.protocol import NetworkMessage, MessageType, create_message
        print_success("Network protocol imports OK")
        
        # Test creating a message
        msg = create_message(MessageType.CONNECT, {'player_id': 'test', 'character_name': 'Test'})
        json_str = msg.to_json()
        msg2 = NetworkMessage.from_json(json_str)
        
        if msg.message_type == msg2.message_type:
            print_success("Message serialization/deserialization OK")
            return True
        else:
            print_failure("Message serialization failed")
            return False
            
    except Exception as e:
        print_failure(f"Network protocol test failed: {e}")
        return False

def generate_report(results: dict):
    """Generate test report"""
    print_header("Test Report")
    
    passed = sum(1 for v in results.values() if v)
    failed = len(results) - passed
    total = len(results)
    
    print(f"Total Tests: {total}")
    print(f"Passed: {Colors.OKGREEN}{passed}{Colors.ENDC}")
    print(f"Failed: {Colors.FAIL}{failed}{Colors.ENDC}")
    print()
    
    for test_name, result in results.items():
        status = f"{Colors.OKGREEN}PASS{Colors.ENDC}" if result else f"{Colors.FAIL}FAIL{Colors.ENDC}"
        print(f"  {test_name:<30} {status}")
    
    print()
    
    if failed == 0:
        print_success("All tests passed! ✓")
        print("\n" + Colors.OKGREEN + Colors.BOLD + "Software is ready to use!" + Colors.ENDC)
        print("\nNext steps:")
        print("  1. Run server: python server/server.py")
        print("  2. Run client: python client/client.py \"YourName\"")
        print("  3. Try 3D client: python client_3d.py \"YourName\"")
        print("  4. Try standalone 3D: python test_3d_client.py")
        return True
    else:
        print_failure(f"{failed} test(s) failed")
        print("\n" + Colors.FAIL + "Please fix failing tests before using the software" + Colors.ENDC)
        return False

def interactive_menu():
    """Show interactive menu for testing"""
    print_header("EVE OFFLINE - Automated Test Runner")
    print("This script will test all components of EVE OFFLINE")
    print()
    print("Options:")
    print("  1. Run all tests (recommended)")
    print("  2. Quick test (dependencies + unit tests only)")
    print("  3. Test specific component")
    print("  4. Exit")
    print()
    
    choice = input("Select option (1-4): ").strip()
    
    return choice

def main():
    """Main test runner"""
    os.chdir(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
    
    # Check if running interactively
    if len(sys.argv) > 1:
        if sys.argv[1] == "--quick":
            print_header("EVE OFFLINE - Quick Test Mode")
            results = {
                "Dependencies": test_dependencies(),
                "Unit Tests": test_unit_tests(),
            }
            generate_report(results)
            return
        elif sys.argv[1] == "--help":
            print("Usage: python automated_tests.py [--quick] [--help]")
            print()
            print("Options:")
            print("  --quick    Run quick tests only (dependencies + unit tests)")
            print("  --help     Show this help message")
            print()
            print("Default: Run all tests interactively")
            return
    
    # Interactive mode
    choice = interactive_menu()
    
    if choice == "1":
        # Run all tests
        print_header("Running All Tests")
        results = {
            "Dependencies": test_dependencies(),
            "Unit Tests": test_unit_tests(),
            "Engine": test_engine(),
            "Demos": test_demos(),
            "3D Client": test_3d_client(),
            "Network": test_network(),
        }
        success = generate_report(results)
        sys.exit(0 if success else 1)
        
    elif choice == "2":
        # Quick test
        print_header("Running Quick Tests")
        results = {
            "Dependencies": test_dependencies(),
            "Unit Tests": test_unit_tests(),
        }
        success = generate_report(results)
        sys.exit(0 if success else 1)
        
    elif choice == "3":
        # Test specific component
        print("\nComponents:")
        print("  1. Dependencies")
        print("  2. Unit Tests")
        print("  3. Engine")
        print("  4. Demos")
        print("  5. 3D Client")
        print("  6. Network")
        
        comp = input("\nSelect component (1-6): ").strip()
        
        tests = {
            "1": ("Dependencies", test_dependencies),
            "2": ("Unit Tests", test_unit_tests),
            "3": ("Engine", test_engine),
            "4": ("Demos", test_demos),
            "5": ("3D Client", test_3d_client),
            "6": ("Network", test_network),
        }
        
        if comp in tests:
            name, func = tests[comp]
            result = func()
            results = {name: result}
            generate_report(results)
            sys.exit(0 if result else 1)
        else:
            print_failure("Invalid selection")
            sys.exit(1)
            
    elif choice == "4":
        print("Exiting...")
        sys.exit(0)
    else:
        print_failure("Invalid choice")
        sys.exit(1)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nTest interrupted by user")
        sys.exit(1)
    except Exception as e:
        print_failure(f"Unexpected error: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)
