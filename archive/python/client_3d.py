#!/usr/bin/env python3
"""
EVE OFFLINE - 3D Client Entry Point
Panda3D-based 3D graphical client

Usage:
    python client_3d.py "CharacterName" [server_host] [server_port] [--debug]
    
Examples:
    python client_3d.py "TestPilot"
    python client_3d.py "TestPilot" localhost 8765
    python client_3d.py "TestPilot" localhost 8765 --debug
"""

import sys
import os
import asyncio
import uuid
import argparse

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

# Setup logging first
from client_3d.utils import setup_logging, get_logger

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
    parser = argparse.ArgumentParser(
        description='EVE OFFLINE 3D Client',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  python client_3d.py "TestPilot"
  python client_3d.py "TestPilot" localhost 8765
  python client_3d.py "TestPilot" localhost 8765 --debug
        """
    )
    
    parser.add_argument('character_name', help='Character name')
    parser.add_argument('server_host', nargs='?', default='localhost', help='Server host (default: localhost)')
    parser.add_argument('server_port', nargs='?', type=int, default=8765, help='Server port (default: 8765)')
    parser.add_argument('--debug', action='store_true', help='Enable debug logging')
    parser.add_argument('--log-dir', default='logs', help='Log directory (default: logs)')
    
    args = parser.parse_args()
    
    # Generate unique player ID
    player_id = f"player_{uuid.uuid4().hex[:8]}"
    
    return player_id, args.character_name, args.server_host, args.server_port, args
    character_name = sys.argv[1]
    server_host = sys.argv[2] if len(sys.argv) > 2 else "localhost"
    server_port = int(sys.argv[3]) if len(sys.argv) > 3 else 8765
    
    # Generate unique player ID
    player_id = f"player_{uuid.uuid4().hex[:8]}"
    
    return player_id, character_name, server_host, server_port


def main():
    """Main entry point"""
    # Parse arguments first
    player_id, character_name, server_host, server_port, args = parse_args()
    
    # Setup logging
    log_level = "DEBUG" if args.debug else "INFO"
    logger = setup_logging(
        name="eve_offline_3d",
        level=log_level,
        log_to_file=True,
        log_dir=args.log_dir
    )
    
    logger.info("="*60)
    logger.info("EVE OFFLINE - 3D Client")
    logger.info("="*60)
    logger.info("Version: 0.1.0 (Phase 5 Development)")
    logger.info("Engine: Panda3D")
    logger.info(f"Log Level: {log_level}")
    logger.info("="*60)
    
    logger.info(f"Character: {character_name}")
    logger.info(f"Player ID: {player_id}")
    logger.info(f"Server: {server_host}:{server_port}")
    
    # Create game client
    try:
        logger.info("Initializing 3D client...")
        client = GameClient3D(player_id, character_name, server_host, server_port)
        
        # Connect to server asynchronously
        # We need to integrate asyncio with Panda3D's main loop
        # For now, we'll run the connection in a separate thread/task
        
        # Create event loop for network operations
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        
        # Run connection
        logger.info("Connecting to server...")
        loop.run_until_complete(client.connect_to_server())
        
        # Run Panda3D main loop
        # The network receive loop will run as an asyncio task
        logger.info("Starting main loop...")
        client.run()
        
        # Cleanup
        logger.info("Shutting down...")
        client.cleanup()
        logger.info("Goodbye!")
        
    except KeyboardInterrupt:
        logger.info("Interrupted by user")
    except Exception as e:
        logger.error(f"Fatal error: {e}", exc_info=True)
        import traceback
        traceback.print_exc()
    finally:
        logger.info("Exiting...")
        sys.exit(0)


if __name__ == "__main__":
    main()
