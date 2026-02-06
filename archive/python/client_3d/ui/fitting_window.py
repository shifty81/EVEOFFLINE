"""
Ship Fitting Window UI
Displays ship fitting interface with slots and modules
"""

from direct.gui.DirectGui import DirectButton, DirectLabel, DirectFrame
from panda3d.core import TextNode, TransparencyAttrib
from typing import Dict, List, Optional, Callable
from .base_panel import EVEPanel


class FittingWindow(EVEPanel):
    """
    Ship fitting window
    Shows high/mid/low/rig slots and allows module management
    """
    
    def __init__(self, parent, pos=(0.2, 0.2), size=(0.8, 0.8)):
        """Initialize fitting window"""
        super().__init__(
            parent=parent,
            title="Ship Fitting",
            pos=pos,
            size=size,
            closeable=True,
            draggable=True
        )
        
        # Current ship data
        self.ship_name = "Unknown Ship"
        self.ship_type = "Frigate"
        self.cpu_used = 0.0
        self.cpu_max = 100.0
        self.powergrid_used = 0.0
        self.powergrid_max = 100.0
        
        # Slots data
        self.high_slots = [None, None, None]
        self.mid_slots = [None, None, None]
        self.low_slots = [None, None, None]
        self.rig_slots = [None, None, None]
        
        # Callbacks
        self.on_fit_module: Optional[Callable] = None
        self.on_unfit_module: Optional[Callable] = None
        self.on_online_module: Optional[Callable] = None
        self.on_offline_module: Optional[Callable] = None
        
        # Create fitting UI
        self._create_fitting_ui()
        
        print("[FittingWindow] Fitting window initialized")
    
    def _create_fitting_ui(self):
        """Create fitting window UI elements"""
        # Ship info panel
        self._create_ship_info()
        
        # Resource bars (CPU/Powergrid)
        self._create_resource_bars()
        
        # Slot displays
        self._create_slot_displays()
        
        # Module list
        self._create_module_list()
    
    def _create_ship_info(self):
        """Create ship information display"""
        # Ship name
        self.ship_name_label = DirectLabel(
            text="Ship: Unknown",
            text_scale=0.05,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.02, 0, -0.02),
            parent=self.content_frame
        )
        
        # Ship type
        self.ship_type_label = DirectLabel(
            text="Type: Frigate",
            text_scale=0.04,
            text_fg=(0.7, 0.75, 0.8, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.02, 0, -0.07),
            parent=self.content_frame
        )
    
    def _create_resource_bars(self):
        """Create CPU and Powergrid bars"""
        bar_y = -0.12
        bar_width = 0.35
        bar_height = 0.04
        
        # CPU label and bar
        DirectLabel(
            text="CPU:",
            text_scale=0.035,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.02, 0, bar_y),
            parent=self.content_frame
        )
        
        self.cpu_bar_frame = DirectFrame(
            frameColor=(0.1, 0.1, 0.15, 0.8),
            frameSize=(0, bar_width, -bar_height/2, bar_height/2),
            pos=(0.1, 0, bar_y),
            parent=self.content_frame
        )
        self.cpu_bar_frame.setTransparency(TransparencyAttrib.MAlpha)
        
        self.cpu_bar = DirectFrame(
            frameColor=(0.2, 0.6, 0.8, 0.9),
            frameSize=(0, 0.01, -bar_height/2, bar_height/2),
            pos=(0, 0, 0),
            parent=self.cpu_bar_frame
        )
        self.cpu_bar.setTransparency(TransparencyAttrib.MAlpha)
        
        self.cpu_text = DirectLabel(
            text="0 / 100",
            text_scale=0.03,
            text_fg=(1, 1, 1, 1),
            frameColor=(0, 0, 0, 0),
            pos=(bar_width/2, 0, 0),
            parent=self.cpu_bar_frame
        )
        
        # Powergrid label and bar
        pg_y = bar_y - 0.06
        DirectLabel(
            text="PG:",
            text_scale=0.035,
            text_fg=(0.9, 0.95, 1.0, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.02, 0, pg_y),
            parent=self.content_frame
        )
        
        self.pg_bar_frame = DirectFrame(
            frameColor=(0.1, 0.1, 0.15, 0.8),
            frameSize=(0, bar_width, -bar_height/2, bar_height/2),
            pos=(0.1, 0, pg_y),
            parent=self.content_frame
        )
        self.pg_bar_frame.setTransparency(TransparencyAttrib.MAlpha)
        
        self.pg_bar = DirectFrame(
            frameColor=(1.0, 0.8, 0.2, 0.9),
            frameSize=(0, 0.01, -bar_height/2, bar_height/2),
            pos=(0, 0, 0),
            parent=self.pg_bar_frame
        )
        self.pg_bar.setTransparency(TransparencyAttrib.MAlpha)
        
        self.pg_text = DirectLabel(
            text="0 / 100",
            text_scale=0.03,
            text_fg=(1, 1, 1, 1),
            frameColor=(0, 0, 0, 0),
            pos=(bar_width/2, 0, 0),
            parent=self.pg_bar_frame
        )
    
    def _create_slot_displays(self):
        """Create slot displays for high/mid/low/rig"""
        start_y = -0.22
        slot_spacing = 0.08
        
        # High slots
        self._create_slot_group("High Slots", start_y, self.high_slots)
        
        # Mid slots
        self._create_slot_group("Mid Slots", start_y - slot_spacing * 4, self.mid_slots)
        
        # Low slots
        self._create_slot_group("Low Slots", start_y - slot_spacing * 8, self.low_slots)
        
        # Rig slots
        self._create_slot_group("Rig Slots", start_y - slot_spacing * 12, self.rig_slots)
    
    def _create_slot_group(self, title: str, y_pos: float, slots: List):
        """Create a group of slots"""
        # Title
        DirectLabel(
            text=title,
            text_scale=0.04,
            text_fg=(0.7, 0.75, 0.8, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.02, 0, y_pos),
            parent=self.content_frame
        )
        
        # Slot buttons
        slot_width = 0.35
        slot_height = 0.06
        for i in range(len(slots)):
            slot_y = y_pos - (i + 1) * 0.07
            slot_button = DirectButton(
                text=f"[Empty Slot {i+1}]" if slots[i] is None else slots[i],
                text_scale=0.035,
                text_fg=(0.9, 0.95, 1.0, 1.0),
                text_align=TextNode.ALeft,
                frameColor=(0.1, 0.15, 0.2, 0.8),
                frameSize=(0, slot_width, -slot_height/2, slot_height/2),
                pos=(0.05, 0, slot_y),
                command=self._on_slot_clicked,
                extraArgs=[title, i],
                parent=self.content_frame
            )
            slot_button.setTransparency(TransparencyAttrib.MAlpha)
    
    def _create_module_list(self):
        """Create available modules list"""
        # This would show modules available in cargo/hangar
        # For now, just a placeholder label
        DirectLabel(
            text="Available Modules:",
            text_scale=0.04,
            text_fg=(0.7, 0.75, 0.8, 1.0),
            text_align=TextNode.ALeft,
            frameColor=(0, 0, 0, 0),
            pos=(0.45, 0, -0.22),
            parent=self.content_frame
        )
    
    def _on_slot_clicked(self, slot_type: str, slot_index: int):
        """Handle slot click"""
        print(f"[FittingWindow] Slot clicked: {slot_type} slot {slot_index}")
        # This would open a module selection dialog
    
    def update_ship_info(self, ship_name: str, ship_type: str):
        """Update ship information"""
        self.ship_name = ship_name
        self.ship_type = ship_type
        self.ship_name_label['text'] = f"Ship: {ship_name}"
        self.ship_type_label['text'] = f"Type: {ship_type}"
    
    def update_resources(self, cpu_used: float, cpu_max: float, pg_used: float, pg_max: float):
        """Update CPU and Powergrid displays"""
        self.cpu_used = cpu_used
        self.cpu_max = cpu_max
        self.powergrid_used = pg_used
        self.powergrid_max = pg_max
        
        # Update CPU bar
        cpu_percent = (cpu_used / cpu_max) if cpu_max > 0 else 0
        bar_width = 0.35
        self.cpu_bar['frameSize'] = (0, bar_width * cpu_percent, -0.02, 0.02)
        self.cpu_text['text'] = f"{cpu_used:.1f} / {cpu_max:.1f}"
        
        # Update color based on usage
        if cpu_percent > 1.0:
            self.cpu_bar['frameColor'] = (1.0, 0.2, 0.2, 0.9)  # Red (overload)
        elif cpu_percent > 0.9:
            self.cpu_bar['frameColor'] = (1.0, 0.7, 0.2, 0.9)  # Orange
        else:
            self.cpu_bar['frameColor'] = (0.2, 0.6, 0.8, 0.9)  # Blue
        
        # Update Powergrid bar
        pg_percent = (pg_used / pg_max) if pg_max > 0 else 0
        self.pg_bar['frameSize'] = (0, bar_width * pg_percent, -0.02, 0.02)
        self.pg_text['text'] = f"{pg_used:.1f} / {pg_max:.1f}"
        
        # Update color based on usage
        if pg_percent > 1.0:
            self.pg_bar['frameColor'] = (1.0, 0.2, 0.2, 0.9)  # Red (overload)
        elif pg_percent > 0.9:
            self.pg_bar['frameColor'] = (1.0, 0.5, 0.2, 0.9)  # Orange
        else:
            self.pg_bar['frameColor'] = (1.0, 0.8, 0.2, 0.9)  # Yellow
    
    def update_slots(self, high: List[Optional[str]], mid: List[Optional[str]], 
                     low: List[Optional[str]], rig: List[Optional[str]]):
        """Update slot displays with fitted modules"""
        self.high_slots = high
        self.mid_slots = mid
        self.low_slots = low
        self.rig_slots = rig
        # Would need to refresh slot displays here
