#!/usr/bin/env python3
"""
Demo script to showcase the new client features
"""

import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from engine.utils.name_generator import NameGenerator
from client.client_config import ClientConfig


def demo_client_config():
    """Demo the client configuration system"""
    print("=" * 70)
    print("Client Configuration System Demo")
    print("=" * 70)
    print()
    
    # Load configuration
    config = ClientConfig()
    
    print("Configuration loaded from: client/client_config.json")
    print()
    
    print("Connection Settings:")
    print(f"  Host: {config.get_host()}")
    print(f"  Port: {config.get_port()}")
    print(f"  Auto-reconnect: {config.get_auto_reconnect()}")
    print(f"  Reconnect delay: {config.get_reconnect_delay()}s")
    print(f"  Connection timeout: {config.get_connection_timeout()}s")
    print()
    
    print("Display Settings:")
    width, height = config.get_display_size()
    print(f"  Resolution: {width}x{height}")
    print(f"  Fullscreen: {config.get_fullscreen()}")
    print(f"  FPS limit: {config.get_fps_limit()}")
    print(f"  Show FPS: {config.get_show_fps()}")
    print()
    
    print("Network Settings:")
    print(f"  Buffer size: {config.get_buffer_size()} bytes")
    print()
    
    print("Client Features:")
    print(f"  Auto-generate names: {config.get_auto_generate_name()}")
    print(f"  Name style: {config.get_name_style()}")
    print()
    
    print("Steam Integration:")
    print(f"  Steam enabled: {config.is_steam_enabled()}")
    print(f"  Auto-login: {config.get_steam_auto_login()}")
    print()


def demo_name_integration():
    """Demo name generator integration with client"""
    print("=" * 70)
    print("Name Generator Integration Demo")
    print("=" * 70)
    print()
    
    print("Generating character names for different styles:")
    print()
    
    # Male names
    print("Male Names:")
    for i in range(3):
        name = NameGenerator.generate_character_name('male')
        print(f"  {i+1}. {name}")
    print()
    
    # Female names
    print("Female Names:")
    for i in range(3):
        name = NameGenerator.generate_character_name('female')
        print(f"  {i+1}. {name}")
    print()
    
    # Random names
    print("Random Names:")
    for i in range(3):
        name = NameGenerator.generate_character_name()
        print(f"  {i+1}. {name}")
    print()


def demo_client_usage():
    """Demo client usage examples"""
    print("=" * 70)
    print("Client Usage Examples")
    print("=" * 70)
    print()
    
    print("Text Client:")
    print("  # Basic usage")
    print("  python client/client.py \"MyCharacter\"")
    print()
    print("  # With auto-generated name")
    print("  python client/client.py --generate-name")
    print()
    print("  # With specific name style")
    print("  python client/client.py --generate-name --name-style male")
    print()
    print("  # Connect to specific server")
    print("  python client/client.py \"MyCharacter\" --host 192.168.1.100 --port 8765")
    print()
    print("  # Use custom config file")
    print("  python client/client.py \"MyCharacter\" --config my_config.json")
    print()
    
    print("GUI Client:")
    print("  # Basic usage")
    print("  python client/gui_client.py \"MyCharacter\"")
    print()
    print("  # With auto-generated name")
    print("  python client/gui_client.py --generate-name --name-style female")
    print()
    print("  # Fullscreen mode")
    print("  python client/gui_client.py \"MyCharacter\" --fullscreen")
    print()
    print("  # Connect to remote server")
    print("  python client/gui_client.py \"MyCharacter\" --host game.server.com --port 8765")
    print()


def demo_client_features():
    """Demo client feature list"""
    print("=" * 70)
    print("New Client Features")
    print("=" * 70)
    print()
    
    features = [
        ("Configuration System", [
            "JSON-based configuration file",
            "Connection settings (host, port, timeouts)",
            "Display settings (resolution, FPS, fullscreen)",
            "Network settings (buffer size)",
            "Auto-save and hot-reload support"
        ]),
        ("Name Generator Integration", [
            "Generate character names on the fly",
            "Support for male, female, and random styles",
            "Command-line options for name generation",
            "Configuration-based auto-generation"
        ]),
        ("Enhanced Connection", [
            "Connection timeout with configurable duration",
            "Auto-reconnect on disconnect",
            "Reconnect delay and attempt limits",
            "Better error handling and reporting"
        ]),
        ("Improved UI (GUI Client)", [
            "Connection status indicator with colors",
            "FPS counter (configurable)",
            "Real-time connection quality display",
            "Error messages in UI"
        ]),
        ("Better Error Handling", [
            "Detailed error messages from server",
            "Graceful handling of protocol errors",
            "Connection failure diagnostics",
            "Timeout detection"
        ])
    ]
    
    for feature_name, feature_list in features:
        print(f"✓ {feature_name}:")
        for item in feature_list:
            print(f"  • {item}")
        print()


def main():
    """Main demo function"""
    print()
    print("╔" + "═" * 68 + "╗")
    print("║" + " " * 10 + "EVE OFFLINE - Client Features Demo" + " " * 23 + "║")
    print("╚" + "═" * 68 + "╝")
    print()
    
    demo_client_config()
    demo_name_integration()
    demo_client_usage()
    demo_client_features()
    
    print("=" * 70)
    print("Demo Complete!")
    print("=" * 70)
    print()
    print("The clients now support:")
    print("  • Configuration files for all settings")
    print("  • Integrated name generator")
    print("  • Auto-reconnect and error handling")
    print("  • Enhanced UI with status indicators")
    print("  • Better connection management")
    print()


if __name__ == "__main__":
    main()
