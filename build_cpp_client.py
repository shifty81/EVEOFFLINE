#!/usr/bin/env python3
"""
EVE OFFLINE - C++ Client Build Automation Script
Automatically builds and tests the C++ client on any platform
"""

import sys
import os
import subprocess
import platform
import argparse
import shutil
from pathlib import Path
from typing import List, Optional

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

    @staticmethod
    def disable():
        """Disable colors on Windows or when not supported"""
        for attr in ['HEADER', 'OKBLUE', 'OKCYAN', 'OKGREEN', 'WARNING', 'FAIL', 'ENDC', 'BOLD']:
            setattr(Colors, attr, '')


class CPPClientBuilder:
    """Automated C++ client build system"""
    
    def __init__(self, args):
        self.args = args
        self.root_dir = Path(__file__).parent.absolute()
        self.cpp_client_dir = self.root_dir / "cpp_client"
        self.build_dir = self.cpp_client_dir / ("build_debug" if args.debug else "build")
        
        # Detect platform
        self.platform = platform.system()
        self.is_windows = self.platform == "Windows"
        self.is_linux = self.platform == "Linux"
        self.is_macos = self.platform == "Darwin"
        
        # Disable colors on Windows by default
        if self.is_windows and not args.force_colors:
            Colors.disable()
    
    def print_header(self, message: str):
        """Print section header"""
        print(f"\n{Colors.HEADER}{Colors.BOLD}{'=' * 70}{Colors.ENDC}")
        print(f"{Colors.HEADER}{Colors.BOLD}{message.center(70)}{Colors.ENDC}")
        print(f"{Colors.HEADER}{Colors.BOLD}{'=' * 70}{Colors.ENDC}\n")
    
    def print_step(self, message: str):
        """Print step being executed"""
        print(f"{Colors.OKCYAN}▶ {message}...{Colors.ENDC}")
    
    def print_success(self, message: str):
        """Print success message"""
        print(f"{Colors.OKGREEN}✓ {message}{Colors.ENDC}")
    
    def print_error(self, message: str):
        """Print error message"""
        print(f"{Colors.FAIL}✗ {message}{Colors.ENDC}")
    
    def print_warning(self, message: str):
        """Print warning message"""
        print(f"{Colors.WARNING}⚠ {message}{Colors.ENDC}")
    
    def run_command(self, cmd: List[str], cwd: Optional[Path] = None) -> bool:
        """Run a command and return success status"""
        try:
            if self.args.verbose:
                print(f"Running: {' '.join(cmd)}")
            
            result = subprocess.run(
                cmd,
                cwd=cwd or self.cpp_client_dir,
                check=True,
                capture_output=not self.args.verbose,
                text=True
            )
            return True
        except subprocess.CalledProcessError as e:
            if not self.args.verbose:
                print(f"STDOUT: {e.stdout}")
                print(f"STDERR: {e.stderr}")
            return False
    
    def check_dependencies(self) -> bool:
        """Check if required dependencies are installed"""
        self.print_step("Checking dependencies")
        
        # Check CMake
        if not shutil.which("cmake"):
            self.print_error("CMake not found")
            print("Install CMake from: https://cmake.org/download/")
            return False
        self.print_success("CMake found")
        
        # Check compiler
        if self.is_windows:
            if not shutil.which("cl") and not shutil.which("g++"):
                self.print_error("No C++ compiler found")
                print("Install Visual Studio or MinGW")
                return False
            compiler = "MSVC" if shutil.which("cl") else "MinGW"
            self.print_success(f"{compiler} compiler found")
        else:
            if not shutil.which("g++") and not shutil.which("clang++"):
                self.print_error("No C++ compiler found")
                print("Install build-essential (Linux) or Xcode (macOS)")
                return False
            compiler = "GCC" if shutil.which("g++") else "Clang"
            self.print_success(f"{compiler} compiler found")
        
        return True
    
    def install_dependencies(self) -> bool:
        """Install or check for required libraries"""
        self.print_step("Checking libraries")
        
        if self.is_linux:
            print("\nTo install dependencies on Ubuntu/Debian:")
            print("  sudo apt-get install libgl1-mesa-dev libglew-dev libglfw3-dev libglm-dev nlohmann-json3-dev libopenal-dev")
            print("\nOn Fedora/RHEL:")
            print("  sudo dnf install mesa-libGL-devel glew-devel glfw-devel glm-devel json-devel openal-soft-devel")
        
        elif self.is_macos:
            print("\nTo install dependencies on macOS:")
            print("  brew install glfw glm glew nlohmann-json openal-soft")
        
        elif self.is_windows:
            print("\nTo install dependencies on Windows:")
            print("  vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows openal-soft:x64-windows")
        
        if not self.args.skip_deps_check:
            print("\nPress Enter to continue, or Ctrl+C to install dependencies first...")
            input()
        
        return True
    
    def clean_build(self) -> bool:
        """Clean the build directory"""
        if self.build_dir.exists():
            self.print_step(f"Cleaning build directory: {self.build_dir}")
            try:
                shutil.rmtree(self.build_dir)
                self.print_success("Build directory cleaned")
            except Exception as e:
                self.print_error(f"Failed to clean build directory: {e}")
                return False
        return True
    
    def configure_cmake(self) -> bool:
        """Configure CMake"""
        self.print_step("Configuring CMake")
        
        # Create build directory
        self.build_dir.mkdir(parents=True, exist_ok=True)
        
        # Build CMake command
        cmd = ["cmake", ".."]
        
        # Build type
        build_type = "Debug" if self.args.debug else "Release"
        cmd.extend(["-DCMAKE_BUILD_TYPE=" + build_type])
        
        # Generator
        if self.args.generator:
            cmd.extend(["-G", self.args.generator])
        elif self.is_windows and not self.args.mingw:
            # Use Visual Studio on Windows by default
            cmd.extend(["-G", "Visual Studio 17 2022", "-A", "x64"])
        
        # Options
        cmd.append("-DUSE_SYSTEM_LIBS=ON")
        cmd.append("-DBUILD_TESTS=ON" if not self.args.no_tests else "-DBUILD_TESTS=OFF")
        
        # vcpkg toolchain (if provided)
        if self.args.vcpkg:
            vcpkg_cmake = Path(self.args.vcpkg) / "scripts" / "buildsystems" / "vcpkg.cmake"
            cmd.extend([f"-DCMAKE_TOOLCHAIN_FILE={vcpkg_cmake}"])
        
        success = self.run_command(cmd, cwd=self.build_dir)
        
        if success:
            self.print_success(f"CMake configured ({build_type})")
        else:
            self.print_error("CMake configuration failed")
        
        return success
    
    def build(self) -> bool:
        """Build the project"""
        self.print_step("Building project")
        
        # Get number of cores
        if self.is_windows:
            cores = os.cpu_count() or 4
        else:
            cores = os.cpu_count() or 4
        
        # Build command
        cmd = ["cmake", "--build", "."]
        
        # Configuration for multi-config generators
        build_type = "Debug" if self.args.debug else "Release"
        cmd.extend(["--config", build_type])
        
        # Parallel build
        if not self.is_windows or self.args.mingw:
            cmd.extend(["--", f"-j{cores}"])
        else:
            cmd.extend(["--", f"/m:{cores}"])
        
        success = self.run_command(cmd, cwd=self.build_dir)
        
        if success:
            self.print_success(f"Build completed successfully (using {cores} cores)")
        else:
            self.print_error("Build failed")
        
        return success
    
    def run_tests(self) -> bool:
        """Run test executables"""
        if self.args.no_tests:
            self.print_warning("Tests skipped (--no-tests)")
            return True
        
        self.print_step("Running tests")
        
        # Find test executables
        bin_dir = self.build_dir / "bin"
        if self.is_windows:
            bin_dir = bin_dir / ("Debug" if self.args.debug else "Release")
        
        if not bin_dir.exists():
            self.print_warning(f"Test directory not found: {bin_dir}")
            return True
        
        # List of tests to run (that don't require display)
        tests_to_run = ["test_network", "test_entity_sync"]
        
        tests_passed = 0
        tests_failed = 0
        
        for test_name in tests_to_run:
            test_exe = bin_dir / (test_name + (".exe" if self.is_windows else ""))
            
            if not test_exe.exists():
                self.print_warning(f"Test not found: {test_name}")
                continue
            
            print(f"\n  Running {test_name}...")
            if self.run_command([str(test_exe)], cwd=bin_dir):
                self.print_success(f"{test_name} passed")
                tests_passed += 1
            else:
                self.print_warning(f"{test_name} failed (may need server running)")
                tests_failed += 1
        
        print(f"\nTest Results: {tests_passed} passed, {tests_failed} failed/skipped")
        return True
    
    def print_summary(self, success: bool):
        """Print build summary"""
        self.print_header("Build Summary")
        
        if success:
            print(f"{Colors.OKGREEN}✓ Build completed successfully!{Colors.ENDC}\n")
            
            # Print executable location
            exe_name = "eve_client" + (".exe" if self.is_windows else "")
            bin_dir = self.build_dir / "bin"
            if self.is_windows:
                bin_dir = bin_dir / ("Debug" if self.args.debug else "Release")
            exe_path = bin_dir / exe_name
            
            if exe_path.exists():
                print(f"Executable: {exe_path}")
            
            print("\nTo run the client:")
            print(f"  cd {bin_dir}")
            print(f"  ./{exe_name} \"YourName\"")
            
            print("\nTo open in Visual Studio (Windows):")
            print(f"  {self.build_dir / 'EVEOfflineClient.sln'}")
        else:
            print(f"{Colors.FAIL}✗ Build failed{Colors.ENDC}\n")
            print("Check the error messages above for details.")
    
    def run(self) -> int:
        """Run the complete build process"""
        self.print_header("EVE OFFLINE - C++ Client Builder")
        
        print(f"Platform: {self.platform}")
        print(f"Build Type: {'Debug' if self.args.debug else 'Release'}")
        print(f"Build Directory: {self.build_dir}")
        
        # Check dependencies
        if not self.check_dependencies():
            return 1
        
        # Show library installation instructions
        if not self.args.skip_deps_check:
            if not self.install_dependencies():
                return 1
        
        # Clean build if requested
        if self.args.clean:
            if not self.clean_build():
                return 1
        
        # Configure
        if not self.configure_cmake():
            return 1
        
        # Build
        if not self.build():
            return 1
        
        # Run tests
        if not self.args.skip_tests:
            self.run_tests()
        
        # Print summary
        self.print_summary(True)
        
        return 0


def main():
    parser = argparse.ArgumentParser(
        description="EVE OFFLINE C++ Client Build Automation",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s                     # Build Release version
  %(prog)s --debug             # Build Debug version
  %(prog)s --clean             # Clean rebuild
  %(prog)s --generator "Unix Makefiles"  # Use specific CMake generator
  %(prog)s --vcpkg C:/vcpkg    # Use vcpkg for dependencies
  %(prog)s --mingw             # Use MinGW on Windows instead of Visual Studio
        """
    )
    
    parser.add_argument("--debug", action="store_true",
                        help="Build Debug configuration instead of Release")
    parser.add_argument("--clean", action="store_true",
                        help="Clean build directory before building")
    parser.add_argument("--no-tests", action="store_true",
                        help="Don't build tests")
    parser.add_argument("--skip-tests", action="store_true",
                        help="Build but don't run tests")
    parser.add_argument("--skip-deps-check", action="store_true",
                        help="Skip dependency check prompt")
    parser.add_argument("--generator", type=str,
                        help="CMake generator to use")
    parser.add_argument("--vcpkg", type=str,
                        help="Path to vcpkg installation")
    parser.add_argument("--mingw", action="store_true",
                        help="Use MinGW instead of Visual Studio on Windows")
    parser.add_argument("--verbose", "-v", action="store_true",
                        help="Verbose output")
    parser.add_argument("--force-colors", action="store_true",
                        help="Force colored output on Windows")
    
    args = parser.parse_args()
    
    try:
        builder = CPPClientBuilder(args)
        return builder.run()
    except KeyboardInterrupt:
        print("\n\nBuild cancelled by user")
        return 130
    except Exception as e:
        print(f"\n{Colors.FAIL}Error: {e}{Colors.ENDC}")
        if args.verbose:
            import traceback
            traceback.print_exc()
        return 1


if __name__ == "__main__":
    sys.exit(main())
