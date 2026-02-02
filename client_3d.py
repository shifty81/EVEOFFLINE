#!/usr/bin/env python3
"""
EVE OFFLINE - 3D Client Entry Point
Panda3D-based 3D graphical client

Usage:
    python client_3d.py "CharacterName" [server_host] [server_port]
    
Examples:
    python client_3d.py "TestPilot"
    python client_3d.py "TestPilot" localhost 8765
"""

import sys
import os
import asyncio
import uuid

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Check Panda3D availability
try:
    from direct.showbase.ShowBase import ShowBase
    PANDA3D_AVAILABLE = True
except ImportError:
    PANDA3D_AVAILABLE = False
    print("="*60)
    print("ERROR: Panda3D not installed!")
    print("="*60)
    print("\nPanda3D is required for the 3D client.")
    print("\nInstall it with:")
    print("  pip install panda3d")
    print("\nOr install all requirements:")
    print("  pip install -r requirements.txt")
    print("\n" + "="*60)
    sys.exit(1)

from client_3d.core.game_client import GameClient3D


def parse_args():
    """Parse command line arguments"""
    if len(sys.argv) < 2:
        print("Usage: python client_3d.py \"CharacterName\" [server_host] [server_port]")
        print("\nExamples:")
        print("  python client_3d.py \"TestPilot\"")
        print("  python client_3d.py \"TestPilot\" localhost 8765")
        sys.exit(1)
    
    character_name = sys.argv[1]
    server_host = sys.argv[2] if len(sys.argv) > 2 else "localhost"
    server_port = int(sys.argv[3]) if len(sys.argv) > 3 else 8765
    
    # Generate unique player ID
    player_id = f"player_{uuid.uuid4().hex[:8]}"
    
    return player_id, character_name, server_host, server_port


def main():
    """Main entry point"""
    print("="*60)
    print("EVE OFFLINE - 3D Client")
    print("="*60)
    print("Version: 0.1.0 (Phase 5 Development)")
    print("Engine: Panda3D")
    print("="*60 + "\n")
    
    # Parse arguments
    player_id, character_name, server_host, server_port = parse_args()
    
    print(f"Character: {character_name}")
    print(f"Player ID: {player_id}")
    print(f"Server: {server_host}:{server_port}")
    print("")
    
    # Create game client
    try:
        client = GameClient3D(player_id, character_name, server_host, server_port)
        
        # Connect to server asynchronously
        # We need to integrate asyncio with Panda3D's main loop
        # For now, we'll run the connection in a separate thread/task
        
        # Create event loop for network operations
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        
        # Run connection
        print("[Main] Connecting to server...")
        loop.run_until_complete(client.connect_to_server())
        
        # Run Panda3D main loop
        # The network receive loop will run as an asyncio task
        print("[Main] Starting main loop...")
        client.run()
        
        # Cleanup
        client.cleanup()
        
    except KeyboardInterrupt:
        print("\n[Main] Interrupted by user")
    except Exception as e:
        print(f"[Main] Error: {e}")
        import traceback
        traceback.print_exc()
    finally:
        print("[Main] Exiting...")
        sys.exit(0)


if __name__ == "__main__":
    main()
