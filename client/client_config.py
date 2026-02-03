"""
Client Configuration Manager
Loads and manages client settings from configuration file
"""

import json
import os
from typing import Dict, Any, Optional


class ClientConfig:
    """
    Client configuration management
    Loads settings from JSON file and provides access to configuration values
    """
    
    DEFAULT_CONFIG = {
        "connection": {
            "host": "localhost",
            "port": 8765,
            "auto_reconnect": True,
            "reconnect_delay": 5,
            "connection_timeout": 10
        },
        "client": {
            "auto_generate_name": False,
            "name_style": "random",
            "log_level": "INFO",
            "save_credentials": False
        },
        "display": {
            "width": 1280,
            "height": 720,
            "fullscreen": False,
            "vsync": True,
            "fps_limit": 60,
            "show_fps": True
        },
        "steam": {
            "enabled": False,
            "auto_login": False,
            "overlay": True
        },
        "audio": {
            "master_volume": 1.0,
            "music_volume": 0.7,
            "sfx_volume": 0.8
        },
        "network": {
            "compression": False,
            "encryption": False,
            "buffer_size": 4096
        }
    }
    
    def __init__(self, config_path: Optional[str] = None):
        """
        Initialize configuration
        
        Args:
            config_path: Path to configuration file (default: client/client_config.json)
        """
        if config_path is None:
            # Default to client_config.json in the client directory
            script_dir = os.path.dirname(os.path.abspath(__file__))
            config_path = os.path.join(script_dir, "client_config.json")
        
        self.config_path = config_path
        self.config: Dict[str, Any] = {}
        
        # Load configuration
        self.load()
    
    def load(self) -> bool:
        """
        Load configuration from file
        
        Returns:
            True if successful, False otherwise
        """
        try:
            if os.path.exists(self.config_path):
                with open(self.config_path, 'r') as f:
                    self.config = json.load(f)
                    # Merge with defaults for missing keys
                    self._merge_defaults()
                    return True
            else:
                # Use default configuration
                self.config = self.DEFAULT_CONFIG.copy()
                # Create default config file
                self.save()
                return True
        except Exception as e:
            print(f"Error loading config from {self.config_path}: {e}")
            self.config = self.DEFAULT_CONFIG.copy()
            return False
    
    def save(self) -> bool:
        """
        Save configuration to file
        
        Returns:
            True if successful, False otherwise
        """
        try:
            # Create directory if it doesn't exist
            os.makedirs(os.path.dirname(self.config_path), exist_ok=True)
            
            with open(self.config_path, 'w') as f:
                json.dump(self.config, f, indent=2)
            return True
        except Exception as e:
            print(f"Error saving config to {self.config_path}: {e}")
            return False
    
    def _merge_defaults(self):
        """Merge default values for any missing configuration keys"""
        def merge_dict(target: Dict, source: Dict):
            for key, value in source.items():
                if key not in target:
                    target[key] = value
                elif isinstance(value, dict) and isinstance(target[key], dict):
                    merge_dict(target[key], value)
        
        merge_dict(self.config, self.DEFAULT_CONFIG)
    
    def get(self, *keys, default=None) -> Any:
        """
        Get configuration value by key path
        
        Args:
            *keys: Key path (e.g., 'connection', 'host')
            default: Default value if key not found
            
        Returns:
            Configuration value or default
        """
        value = self.config
        for key in keys:
            if isinstance(value, dict) and key in value:
                value = value[key]
            else:
                return default
        return value
    
    def set(self, *keys, value: Any) -> bool:
        """
        Set configuration value by key path
        
        Args:
            *keys: Key path (e.g., 'connection', 'host')
            value: Value to set
            
        Returns:
            True if successful, False otherwise
        """
        if len(keys) == 0:
            return False
        
        # Navigate to parent
        target = self.config
        for key in keys[:-1]:
            if key not in target:
                target[key] = {}
            target = target[key]
        
        # Set value
        target[keys[-1]] = value
        return True
    
    # Convenience methods for common settings
    
    def get_host(self) -> str:
        """Get server host"""
        return self.get('connection', 'host', default='localhost')
    
    def get_port(self) -> int:
        """Get server port"""
        return self.get('connection', 'port', default=8765)
    
    def get_auto_reconnect(self) -> bool:
        """Get auto-reconnect setting"""
        return self.get('connection', 'auto_reconnect', default=True)
    
    def get_reconnect_delay(self) -> int:
        """Get reconnect delay in seconds"""
        return self.get('connection', 'reconnect_delay', default=5)
    
    def get_connection_timeout(self) -> int:
        """Get connection timeout in seconds"""
        return self.get('connection', 'connection_timeout', default=10)
    
    def get_display_size(self) -> tuple:
        """Get display width and height"""
        return (
            self.get('display', 'width', default=1280),
            self.get('display', 'height', default=720)
        )
    
    def get_fullscreen(self) -> bool:
        """Get fullscreen setting"""
        return self.get('display', 'fullscreen', default=False)
    
    def get_fps_limit(self) -> int:
        """Get FPS limit"""
        return self.get('display', 'fps_limit', default=60)
    
    def get_show_fps(self) -> bool:
        """Get show FPS setting"""
        return self.get('display', 'show_fps', default=True)
    
    def is_steam_enabled(self) -> bool:
        """Check if Steam integration is enabled"""
        return self.get('steam', 'enabled', default=False)
    
    def get_steam_auto_login(self) -> bool:
        """Get Steam auto-login setting"""
        return self.get('steam', 'auto_login', default=False)
    
    def get_auto_generate_name(self) -> bool:
        """Check if auto name generation is enabled"""
        return self.get('client', 'auto_generate_name', default=False)
    
    def get_name_style(self) -> str:
        """Get name generation style"""
        return self.get('client', 'name_style', default='random')
    
    def get_buffer_size(self) -> int:
        """Get network buffer size"""
        return self.get('network', 'buffer_size', default=4096)
    
    def __str__(self) -> str:
        """String representation"""
        return json.dumps(self.config, indent=2)


# Global configuration instance
_config: Optional[ClientConfig] = None


def get_config(config_path: Optional[str] = None) -> ClientConfig:
    """
    Get global configuration instance
    
    Args:
        config_path: Path to configuration file (optional)
        
    Returns:
        ClientConfig instance
    """
    global _config
    if _config is None:
        _config = ClientConfig(config_path)
    return _config


def reload_config():
    """Reload configuration from file"""
    global _config
    if _config is not None:
        _config.load()


if __name__ == "__main__":
    # Test configuration
    config = ClientConfig()
    print("Configuration loaded:")
    print(config)
    print()
    print(f"Host: {config.get_host()}")
    print(f"Port: {config.get_port()}")
    print(f"Display: {config.get_display_size()}")
    print(f"Steam enabled: {config.is_steam_enabled()}")
