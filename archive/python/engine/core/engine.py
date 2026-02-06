"""
Core Game Engine
Main game loop and engine management
"""

import time
from typing import Optional
from engine.core.ecs import World


class Engine:
    """
    Main game engine
    Handles initialization, main loop, and shutdown
    """
    
    def __init__(self, target_fps: int = 60):
        self.target_fps = target_fps
        self.target_frame_time = 1.0 / target_fps
        self.running = False
        self.world = World()
        self.current_time = 0.0
        self.frame_count = 0
        
    def initialize(self):
        """Initialize the engine"""
        print(f"[Engine] Initializing EVE OFFLINE Engine v0.1.0")
        print(f"[Engine] Target FPS: {self.target_fps}")
        
    def shutdown(self):
        """Shutdown the engine"""
        print(f"[Engine] Shutting down... Total frames: {self.frame_count}")
        self.running = False
        
    def run(self):
        """Main game loop"""
        self.running = True
        last_time = time.time()
        
        print("[Engine] Starting main loop...")
        
        while self.running:
            # Calculate delta time
            current_time = time.time()
            delta_time = current_time - last_time
            last_time = current_time
            
            # Update world
            self.update(delta_time)
            
            # Frame limiting
            frame_time = time.time() - current_time
            if frame_time < self.target_frame_time:
                time.sleep(self.target_frame_time - frame_time)
                
            self.frame_count += 1
            self.current_time += delta_time
            
    def update(self, delta_time: float):
        """Update game state"""
        # Update all systems in the world
        self.world.update(delta_time)
        
    def stop(self):
        """Stop the engine"""
        self.running = False
