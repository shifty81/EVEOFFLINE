#!/usr/bin/env python3
"""
EVE OFFLINE Demo
Demonstrates the server and client working together
"""

import subprocess
import time
import sys
import os

def print_banner():
    banner = """
    ╔══════════════════════════════════════════════════════════╗
    ║                    EVE OFFLINE                           ║
    ║         Custom Game Engine Demo                          ║
    ║                                                          ║
    ║  A PVE-focused space MMO inspired by EVE ONLINE         ║
    ║  Built with custom Python engine + ECS architecture      ║
    ╚══════════════════════════════════════════════════════════╝
    """
    print(banner)

def main():
    print_banner()
    
    print("\n📋 Demo Options:")
    print("1. Run engine tests")
    print("2. Start server only")
    print("3. Start server + 1 client")
    print("4. Start server + 3 clients (multiplayer demo)")
    print("5. Exit")
    
    choice = input("\nSelect option (1-5): ").strip()
    
    if choice == "1":
        print("\n🧪 Running engine tests...\n")
        result = subprocess.run([sys.executable, "tests/test_engine.py"])
        sys.exit(result.returncode)
        
    elif choice == "2":
        print("\n🖥️  Starting server...")
        print("Press Ctrl+C to stop\n")
        subprocess.run([sys.executable, "server/server.py"])
        
    elif choice == "3":
        print("\n🖥️  Starting server and 1 client...")
        print("Server will start in background")
        print("Press Ctrl+C to stop\n")
        
        server_proc = subprocess.Popen([sys.executable, "server/server.py"])
        time.sleep(2)  # Give server time to start
        
        try:
            subprocess.run([sys.executable, "client/client.py", "TestPilot1"])
        finally:
            server_proc.terminate()
            server_proc.wait()
            
    elif choice == "4":
        print("\n🖥️  Starting server and 3 clients for multiplayer demo...")
        print("This will open the server in the background")
        print("Press Ctrl+C in any window to stop\n")
        
        server_proc = subprocess.Popen([sys.executable, "server/server.py"])
        time.sleep(2)
        
        print("\n📝 To run the clients, open 3 new terminal windows and run:")
        print("   python client/client.py Pilot1")
        print("   python client/client.py Pilot2")
        print("   python client/client.py Pilot3")
        print("\nPress Enter to stop the server...")
        
        try:
            input()
        finally:
            server_proc.terminate()
            server_proc.wait()
            
    elif choice == "5":
        print("\n👋 Goodbye!")
        sys.exit(0)
        
    else:
        print("\n❌ Invalid option")
        sys.exit(1)

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n👋 Demo stopped. Goodbye!")
        sys.exit(0)
