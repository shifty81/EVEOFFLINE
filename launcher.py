#!/usr/bin/env python3
"""
EVE OFFLINE - Easy Launcher
Simple launcher to help users run and test the software
"""

import sys
import os
import subprocess
import time

# ANSI color codes
class Colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'

def print_header(text: str):
    """Print a header"""
    print(f"\n{Colors.HEADER}{Colors.BOLD}{'='*60}{Colors.ENDC}")
    print(f"{Colors.HEADER}{Colors.BOLD}{text:^60}{Colors.ENDC}")
    print(f"{Colors.HEADER}{Colors.BOLD}{'='*60}{Colors.ENDC}\n")

def print_menu():
    """Print main menu"""
    print_header("EVE OFFLINE - Easy Launcher")
    
    print(f"{Colors.OKBLUE}What would you like to do?{Colors.ENDC}\n")
    
    print(f"{Colors.BOLD}Testing & Demos:{Colors.ENDC}")
    print(f"  {Colors.OKCYAN}1.{Colors.ENDC} Run automated tests (verify everything works)")
    print(f"  {Colors.OKCYAN}2.{Colors.ENDC} Run 3D client standalone test (no server needed)")
    print(f"  {Colors.OKCYAN}3.{Colors.ENDC} Run interactive gameplay demo")
    print(f"  {Colors.OKCYAN}4.{Colors.ENDC} Run GUI demo (2D graphics)")
    
    print(f"\n{Colors.BOLD}Multiplayer:{Colors.ENDC}")
    print(f"  {Colors.OKCYAN}5.{Colors.ENDC} Start game server")
    print(f"  {Colors.OKCYAN}6.{Colors.ENDC} Connect text client to server")
    print(f"  {Colors.OKCYAN}7.{Colors.ENDC} Connect 2D GUI client to server")
    print(f"  {Colors.OKCYAN}8.{Colors.ENDC} Connect 3D client to server (NEW!)")
    
    print(f"\n{Colors.BOLD}Development:{Colors.ENDC}")
    print(f"  {Colors.OKCYAN}9.{Colors.ENDC} Check dependencies")
    print(f"  {Colors.OKCYAN}10.{Colors.ENDC} View logs")
    
    print(f"\n  {Colors.OKCYAN}0.{Colors.ENDC} Exit")
    print()

def run_command(cmd: list, description: str = None):
    """Run a command"""
    if description:
        print(f"\n{Colors.OKGREEN}▶ {description}{Colors.ENDC}")
    
    print(f"{Colors.WARNING}Command: {' '.join(cmd)}{Colors.ENDC}")
    print(f"{Colors.WARNING}Press Ctrl+C to stop{Colors.ENDC}\n")
    
    try:
        subprocess.run(cmd, cwd=os.path.dirname(os.path.abspath(__file__)))
    except KeyboardInterrupt:
        print(f"\n{Colors.WARNING}Stopped by user{Colors.ENDC}")
    except Exception as e:
        print(f"\n{Colors.FAIL}Error: {e}{Colors.ENDC}")
    
    input(f"\n{Colors.OKCYAN}Press Enter to continue...{Colors.ENDC}")

def check_dependencies():
    """Check if dependencies are installed"""
    print_header("Checking Dependencies")
    
    # Check Python version
    version = sys.version_info
    print(f"Python version: {version.major}.{version.minor}.{version.micro}")
    if version.major >= 3 and version.minor >= 11:
        print(f"{Colors.OKGREEN}✓ Python version OK{Colors.ENDC}")
    else:
        print(f"{Colors.FAIL}✗ Python 3.11+ required{Colors.ENDC}")
    
    # Check modules
    modules = {
        'pygame': 'pip install pygame',
        'panda3d': 'pip install panda3d',
    }
    
    print(f"\n{Colors.BOLD}Optional dependencies:{Colors.ENDC}")
    for module, install_cmd in modules.items():
        try:
            __import__(module)
            print(f"{Colors.OKGREEN}✓ {module} installed{Colors.ENDC}")
        except ImportError:
            print(f"{Colors.FAIL}✗ {module} NOT installed{Colors.ENDC}")
            print(f"  Install with: {install_cmd}")
    
    input(f"\n{Colors.OKCYAN}Press Enter to continue...{Colors.ENDC}")

def view_logs():
    """View recent logs"""
    print_header("Recent Logs")
    
    log_dir = "logs"
    if not os.path.exists(log_dir):
        print(f"{Colors.WARNING}No logs directory found{Colors.ENDC}")
        input(f"\n{Colors.OKCYAN}Press Enter to continue...{Colors.ENDC}")
        return
    
    # List log files
    logs = sorted([f for f in os.listdir(log_dir) if f.endswith('.log')], reverse=True)
    
    if not logs:
        print(f"{Colors.WARNING}No log files found{Colors.ENDC}")
        input(f"\n{Colors.OKCYAN}Press Enter to continue...{Colors.ENDC}")
        return
    
    print(f"Recent log files (newest first):\n")
    for i, log_file in enumerate(logs[:10], 1):
        print(f"  {i}. {log_file}")
    
    print(f"\n  0. Back to menu")
    
    choice = input(f"\n{Colors.OKCYAN}Select log file to view (0-{min(10, len(logs))}): {Colors.ENDC}").strip()
    
    if choice == "0":
        return
    
    try:
        idx = int(choice) - 1
        if 0 <= idx < len(logs):
            log_file = os.path.join(log_dir, logs[idx])
            print(f"\n{Colors.BOLD}Contents of {logs[idx]}:{Colors.ENDC}\n")
            with open(log_file, 'r') as f:
                # Show last 50 lines
                lines = f.readlines()
                for line in lines[-50:]:
                    print(line.rstrip())
        else:
            print(f"{Colors.FAIL}Invalid selection{Colors.ENDC}")
    except ValueError:
        print(f"{Colors.FAIL}Invalid input{Colors.ENDC}")
    except Exception as e:
        print(f"{Colors.FAIL}Error reading log: {e}{Colors.ENDC}")
    
    input(f"\n{Colors.OKCYAN}Press Enter to continue...{Colors.ENDC}")

def get_character_name():
    """Get character name from user"""
    name = input(f"{Colors.OKCYAN}Enter your character name: {Colors.ENDC}").strip()
    if not name:
        name = "TestPilot"
        print(f"{Colors.WARNING}Using default name: {name}{Colors.ENDC}")
    return name

def main():
    """Main launcher"""
    while True:
        print_menu()
        
        choice = input(f"{Colors.OKCYAN}Select option (0-10): {Colors.ENDC}").strip()
        
        if choice == "0":
            print(f"\n{Colors.OKGREEN}Goodbye!{Colors.ENDC}\n")
            sys.exit(0)
        
        elif choice == "1":
            run_command([sys.executable, "automated_tests.py"], "Running automated tests...")
        
        elif choice == "2":
            run_command([sys.executable, "test_3d_client.py"], "Running 3D client standalone test...")
        
        elif choice == "3":
            run_command([sys.executable, "interactive_demo.py"], "Running interactive gameplay demo...")
        
        elif choice == "4":
            run_command([sys.executable, "gui_demo.py"], "Running GUI demo (2D)...")
        
        elif choice == "5":
            run_command([sys.executable, "server/server.py"], "Starting game server...")
        
        elif choice == "6":
            name = get_character_name()
            run_command([sys.executable, "client/client.py", name], "Connecting text client...")
        
        elif choice == "7":
            name = get_character_name()
            run_command([sys.executable, "client/gui_client.py", name], "Connecting 2D GUI client...")
        
        elif choice == "8":
            name = get_character_name()
            run_command([sys.executable, "client_3d.py", name], "Connecting 3D client...")
        
        elif choice == "9":
            check_dependencies()
        
        elif choice == "10":
            view_logs()
        
        else:
            print(f"{Colors.FAIL}Invalid choice{Colors.ENDC}")
            time.sleep(1)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print(f"\n\n{Colors.OKGREEN}Goodbye!{Colors.ENDC}\n")
        sys.exit(0)
