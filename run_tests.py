#!/usr/bin/env python3
"""
Test Runner for EVEOFFLINE
Runs all test suites and provides summary
"""

import sys
import os
import subprocess
import time

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

def run_test(test_file, test_name):
    """Run a single test file and return results"""
    print(f"\n{'='*70}")
    print(f"Running {test_name}...")
    print('='*70)
    
    start_time = time.time()
    try:
        # Get the repository root directory
        repo_root = os.path.dirname(os.path.abspath(__file__))
        test_path = os.path.join(repo_root, test_file)
        
        result = subprocess.run(
            [sys.executable, test_path],
            cwd=repo_root,
            capture_output=True,
            text=True,
            timeout=60
        )
        elapsed = time.time() - start_time
        
        if result.returncode == 0:
            print(f"✅ {test_name} PASSED ({elapsed:.2f}s)")
            return True, elapsed
        else:
            print(f"❌ {test_name} FAILED ({elapsed:.2f}s)")
            print("\nError Output:")
            print(result.stderr)
            print("\nStdout:")
            print(result.stdout)
            return False, elapsed
    except subprocess.TimeoutExpired:
        print(f"❌ {test_name} TIMEOUT (>60s)")
        return False, 60.0
    except Exception as e:
        print(f"❌ {test_name} ERROR: {e}")
        return False, 0.0


def main():
    """Run all tests"""
    print("\n" + "="*70)
    print("EVEOFFLINE Test Suite Runner")
    print("="*70)
    
    tests = [
        ("tests/test_engine.py", "Core Engine Tests"),
        ("tests/test_advanced_systems.py", "Advanced Systems Tests"),
        ("tests/test_exploration_angle.py", "Exploration Angle Tests"),
        ("tests/test_manufacturing.py", "Manufacturing System Tests"),
        ("tests/test_market.py", "Market System Tests"),
    ]
    
    results = []
    total_time = 0.0
    repo_root = os.path.dirname(os.path.abspath(__file__))
    
    for test_file, test_name in tests:
        full_path = os.path.join(repo_root, test_file)
        if os.path.exists(full_path):
            passed, elapsed = run_test(test_file, test_name)
            results.append((test_name, passed, elapsed))
            total_time += elapsed
        else:
            print(f"⚠️  {test_name} - Test file not found: {test_file}")
            results.append((test_name, False, 0.0))
    
    # Print summary
    print("\n" + "="*70)
    print("TEST SUMMARY")
    print("="*70)
    
    passed_count = sum(1 for _, passed, _ in results if passed)
    total_count = len(results)
    
    for test_name, passed, elapsed in results:
        status = "✅ PASS" if passed else "❌ FAIL"
        print(f"{status:10} {test_name:40} ({elapsed:.2f}s)")
    
    print("="*70)
    print(f"Total: {passed_count}/{total_count} tests passed")
    print(f"Time: {total_time:.2f}s")
    
    if passed_count == total_count:
        print("\n🎉 ALL TESTS PASSED! 🎉\n")
        return 0
    else:
        print(f"\n⚠️  {total_count - passed_count} test(s) failed\n")
        return 1


if __name__ == "__main__":
    sys.exit(main())
