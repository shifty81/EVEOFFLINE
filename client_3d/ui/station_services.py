"""
Station Services UI
Displays available station services (repair, refit, etc.)
"""

from direct.gui.DirectGui import DirectButton, DirectLabel
from panda3d.core import TextNode, TransparencyAttrib
from typing import Optional, Callable
from .base_panel import EVEPanel


class StationServicesWindow(EVEPanel):
    """
    Station services window
    Shows available services at current station
    """
    
    def __init__(self, parent, pos=(0.3, 0.3), size=(0.5, 0.6)):
        """Initialize station services window"""
        super().__init__(
            parent=parent,
            title="Station Services",
            pos=pos,
            size=size,
            closeable=True,
            draggable=True
        )
        
        # Current station
        self.station_name = "Unknown Station"
        self.station_type = "NPC Station"
        
        # Callbacks
        self.on_repair_ship: Optional[Callable] = None
        self.on_refit_ship: Optional[Callable] = None
        self.on_reprocess_items: Optional[Callable] = None
        self.on_manufacturing: Optional[Callable] = None
        self.on_research: Optional[Callable] = None
        
        # Create services UI
        self._create_services_ui()
        
        print("[StationServicesWindow] Station services window initialized")
    
    def _create_services_ui(self):
        """Create station services UI elements"""
        # Station info
        self._create_station_info()
        
        # Service buttons
        self._create_service_buttons()
    
    def _create_station_info(self):
        """Create station information display"""
        # Station name
        self.station_name_label = DirectLabel(
            text=f"Station: {self.station_name}",
            text_scale=0.05,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.02, 0, -0.02),
            parent=self.content_frame
        )
        
        # Station type
        self.station_type_label = DirectLabel(
            text=f"Type: {self.station_type}",
            text_scale=0.04,
            text_fg=(0.7, 0.75, 0.8, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.02, 0, -0.08),
            parent=self.content_frame
        )
    
    def _create_service_buttons(self):
        """Create service buttons"""
        button_width = 0.4
        button_height = 0.06
        start_y = -0.15
        spacing = 0.08
        
        # Repair Service
        self.repair_button = self._create_service_button(
            text="Repair Ship",
            description="Repair ship damage (hull, armor, shield)",
            y_pos=start_y,
            command=self._on_repair_clicked,
            icon_color=(0.2, 0.8, 0.4, 0.8)
        )
        
        # Fitting Service
        self.refit_button = self._create_service_button(
            text="Fitting Service",
            description="Manage ship fitting and modules",
            y_pos=start_y - spacing,
            command=self._on_refit_clicked,
            icon_color=(0.2, 0.6, 0.8, 0.8)
        )
        
        # Reprocessing Service
        self.reprocess_button = self._create_service_button(
            text="Reprocessing",
            description="Reprocess items into minerals",
            y_pos=start_y - spacing * 2,
            command=self._on_reprocess_clicked,
            icon_color=(0.8, 0.6, 0.2, 0.8)
        )
        
        # Manufacturing Service
        self.manufacturing_button = self._create_service_button(
            text="Manufacturing",
            description="Build items from blueprints",
            y_pos=start_y - spacing * 3,
            command=self._on_manufacturing_clicked,
            icon_color=(0.6, 0.4, 0.8, 0.8)
        )
        
        # Research Service
        self.research_button = self._create_service_button(
            text="Research",
            description="Research blueprints (ME/TE)",
            y_pos=start_y - spacing * 4,
            command=self._on_research_clicked,
            icon_color=(0.4, 0.8, 0.8, 0.8)
        )
    
    def _create_service_button(
        self,
        text: str,
        description: str,
        y_pos: float,
        command: Callable,
        icon_color: tuple
    ) -> DirectButton:
        """Create a service button with description"""
        button_width = 0.4
        button_height = 0.06
        
        # Main button
        button = DirectButton(
            text=text,
            text_scale=0.04,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            text_align=TextNode.ALeft,
            frameColor=icon_color,
            frameSize=(0, button_width, -button_height/2, button_height/2),
            pos=(0.05, 0, y_pos),
            command=command,
            parent=self.content_frame
        )
        button.setTransparency(TransparencyAttrib.MAlpha)
        
        # Description label
        desc_label = DirectLabel(
            text=description,
            text_scale=0.03,
            text_fg=(0.6, 0.65, 0.7, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.05, 0, y_pos - 0.04),
            parent=self.content_frame
        )
        
        return button
    
    def _on_repair_clicked(self):
        """Handle repair service click"""
        print("[StationServicesWindow] Repair service clicked")
        if self.on_repair_ship:
            self.on_repair_ship()
    
    def _on_refit_clicked(self):
        """Handle refit service click"""
        print("[StationServicesWindow] Fitting service clicked")
        if self.on_refit_ship:
            self.on_refit_ship()
    
    def _on_reprocess_clicked(self):
        """Handle reprocess service click"""
        print("[StationServicesWindow] Reprocessing service clicked")
        if self.on_reprocess_items:
            self.on_reprocess_items()
    
    def _on_manufacturing_clicked(self):
        """Handle manufacturing service click"""
        print("[StationServicesWindow] Manufacturing service clicked")
        if self.on_manufacturing:
            self.on_manufacturing()
    
    def _on_research_clicked(self):
        """Handle research service click"""
        print("[StationServicesWindow] Research service clicked")
        if self.on_research:
            self.on_research()
    
    def update_station_info(self, station_name: str, station_type: str):
        """Update station information"""
        self.station_name = station_name
        self.station_type = station_type
        self.station_name_label['text'] = f"Station: {station_name}"
        self.station_type_label['text'] = f"Type: {station_type}"
    
    def enable_service(self, service_name: str, enabled: bool = True):
        """Enable or disable a specific service"""
        service_buttons = {
            'repair': self.repair_button,
            'refit': self.refit_button,
            'reprocess': self.reprocess_button,
            'manufacturing': self.manufacturing_button,
            'research': self.research_button
        }
        
        button = service_buttons.get(service_name)
        if button:
            button['state'] = DirectButton.NORMAL if enabled else DirectButton.DISABLED
    
    def set_repair_callback(self, callback: Callable):
        """Set callback for repair service"""
        self.on_repair_ship = callback
    
    def set_refit_callback(self, callback: Callable):
        """Set callback for refit service"""
        self.on_refit_ship = callback
    
    def set_reprocess_callback(self, callback: Callable):
        """Set callback for reprocess service"""
        self.on_reprocess_items = callback
    
    def set_manufacturing_callback(self, callback: Callable):
        """Set callback for manufacturing service"""
        self.on_manufacturing = callback
    
    def set_research_callback(self, callback: Callable):
        """Set callback for research service"""
        self.on_research = callback
