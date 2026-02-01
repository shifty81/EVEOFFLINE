"""
EVE OFFLINE - Custom Game Engine
Main engine package initialization
"""

__version__ = "0.1.0"
__author__ = "EVE OFFLINE Team"

from engine.core.ecs import Entity, Component, System, World
from engine.core.engine import Engine

__all__ = [
    'Entity',
    'Component', 
    'System',
    'World',
    'Engine',
]
