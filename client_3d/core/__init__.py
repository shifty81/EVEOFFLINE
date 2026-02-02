"""
Core module initialization
"""

from .network_client import NetworkClient
from .entity_manager import EntityManager, Entity

__all__ = ['NetworkClient', 'EntityManager', 'Entity']
