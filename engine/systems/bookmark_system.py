"""
Bookmark System
Allows players to save locations in space for quick navigation.
Based on EVE Online's bookmark mechanic where players can bookmark
any location, station, or celestial for future reference.
"""

from typing import Optional, Dict, List
from dataclasses import dataclass, field


@dataclass
class Bookmark:
    """A saved location in space"""
    bookmark_id: str
    owner_id: str
    label: str
    memo: str = ""
    # Location data
    system_id: str = ""
    x: float = 0.0
    y: float = 0.0
    z: float = 0.0
    # What type of thing was bookmarked
    bookmark_type: str = "coordinate"  # coordinate, station, stargate, asteroid_belt, wreck, container
    # Reference to a specific entity if applicable
    reference_id: Optional[str] = None
    reference_name: str = ""
    # Folder organization
    folder: str = "Personal Locations"
    # Metadata
    created_time: float = 0.0
    # Sharing
    shared_with_corp: bool = False


@dataclass
class BookmarkFolder:
    """Folder for organizing bookmarks"""
    folder_id: str
    owner_id: str
    name: str
    bookmark_ids: List[str] = field(default_factory=list)


class BookmarkSystem:
    """
    Manages spatial bookmarks for players.
    Players can save locations for quick warp-to navigation.
    """

    def __init__(self):
        # All bookmarks: {bookmark_id: Bookmark}
        self.bookmarks: Dict[str, Bookmark] = {}
        # Player bookmarks index: {owner_id: [bookmark_id]}
        self.player_bookmarks: Dict[str, List[str]] = {}
        # Folders: {owner_id: {folder_name: BookmarkFolder}}
        self.folders: Dict[str, Dict[str, BookmarkFolder]] = {}
        # Counter for ID generation
        self._bookmark_counter = 0
        self._folder_counter = 0
        # Maximum bookmarks per player
        self.max_bookmarks = 500

    def create_bookmark(
        self,
        owner_id: str,
        label: str,
        system_id: str,
        x: float,
        y: float,
        z: float,
        memo: str = "",
        bookmark_type: str = "coordinate",
        reference_id: Optional[str] = None,
        reference_name: str = "",
        folder: str = "Personal Locations",
        current_time: float = 0.0
    ) -> Optional[Bookmark]:
        """
        Create a new bookmark at a location.

        Returns:
            Bookmark if successful, None if limit reached
        """
        # Check bookmark limit
        player_bms = self.player_bookmarks.get(owner_id, [])
        if len(player_bms) >= self.max_bookmarks:
            return None

        self._bookmark_counter += 1
        bookmark_id = f"bm_{self._bookmark_counter}"

        bookmark = Bookmark(
            bookmark_id=bookmark_id,
            owner_id=owner_id,
            label=label,
            memo=memo,
            system_id=system_id,
            x=x,
            y=y,
            z=z,
            bookmark_type=bookmark_type,
            reference_id=reference_id,
            reference_name=reference_name,
            folder=folder,
            created_time=current_time,
        )

        self.bookmarks[bookmark_id] = bookmark

        if owner_id not in self.player_bookmarks:
            self.player_bookmarks[owner_id] = []
        self.player_bookmarks[owner_id].append(bookmark_id)

        # Ensure folder exists
        if owner_id not in self.folders:
            self.folders[owner_id] = {}
        if folder not in self.folders[owner_id]:
            self._create_folder(owner_id, folder)
        self.folders[owner_id][folder].bookmark_ids.append(bookmark_id)

        return bookmark

    def delete_bookmark(self, owner_id: str, bookmark_id: str) -> bool:
        """Delete a bookmark. Only the owner can delete."""
        bookmark = self.bookmarks.get(bookmark_id)
        if not bookmark or bookmark.owner_id != owner_id:
            return False

        # Remove from folder
        if owner_id in self.folders:
            folder_name = bookmark.folder
            if folder_name in self.folders[owner_id]:
                folder = self.folders[owner_id][folder_name]
                if bookmark_id in folder.bookmark_ids:
                    folder.bookmark_ids.remove(bookmark_id)

        # Remove from player index
        if owner_id in self.player_bookmarks:
            if bookmark_id in self.player_bookmarks[owner_id]:
                self.player_bookmarks[owner_id].remove(bookmark_id)

        # Remove bookmark
        del self.bookmarks[bookmark_id]

        return True

    def edit_bookmark(
        self,
        owner_id: str,
        bookmark_id: str,
        label: Optional[str] = None,
        memo: Optional[str] = None,
        folder: Optional[str] = None
    ) -> bool:
        """Edit a bookmark's label, memo, or folder."""
        bookmark = self.bookmarks.get(bookmark_id)
        if not bookmark or bookmark.owner_id != owner_id:
            return False

        if label is not None:
            bookmark.label = label
        if memo is not None:
            bookmark.memo = memo
        if folder is not None:
            # Move to new folder
            old_folder = bookmark.folder
            if owner_id in self.folders and old_folder in self.folders[owner_id]:
                old = self.folders[owner_id][old_folder]
                if bookmark_id in old.bookmark_ids:
                    old.bookmark_ids.remove(bookmark_id)

            bookmark.folder = folder
            if owner_id not in self.folders:
                self.folders[owner_id] = {}
            if folder not in self.folders[owner_id]:
                self._create_folder(owner_id, folder)
            self.folders[owner_id][folder].bookmark_ids.append(bookmark_id)

        return True

    def get_bookmarks(
        self,
        owner_id: str,
        system_id: Optional[str] = None,
        folder: Optional[str] = None
    ) -> List[Bookmark]:
        """
        Get bookmarks for a player, optionally filtered by system or folder.
        """
        bookmark_ids = self.player_bookmarks.get(owner_id, [])
        bookmarks = []

        for bm_id in bookmark_ids:
            bm = self.bookmarks.get(bm_id)
            if not bm:
                continue
            if system_id and bm.system_id != system_id:
                continue
            if folder and bm.folder != folder:
                continue
            bookmarks.append(bm)

        return bookmarks

    def get_bookmark(self, bookmark_id: str) -> Optional[Bookmark]:
        """Get a specific bookmark by ID."""
        return self.bookmarks.get(bookmark_id)

    def get_folders(self, owner_id: str) -> List[str]:
        """Get all folder names for a player."""
        if owner_id in self.folders:
            return list(self.folders[owner_id].keys())
        return []

    def create_folder(self, owner_id: str, folder_name: str) -> bool:
        """Create a new bookmark folder."""
        if owner_id not in self.folders:
            self.folders[owner_id] = {}
        if folder_name in self.folders[owner_id]:
            return False  # Already exists
        self._create_folder(owner_id, folder_name)
        return True

    def delete_folder(self, owner_id: str, folder_name: str) -> bool:
        """
        Delete a bookmark folder. Bookmarks inside are moved to 'Personal Locations'.
        """
        if owner_id not in self.folders or folder_name not in self.folders[owner_id]:
            return False
        if folder_name == "Personal Locations":
            return False  # Can't delete default folder

        folder = self.folders[owner_id][folder_name]

        # Move bookmarks to default folder
        if "Personal Locations" not in self.folders[owner_id]:
            self._create_folder(owner_id, "Personal Locations")

        default_folder = self.folders[owner_id]["Personal Locations"]
        for bm_id in folder.bookmark_ids:
            bm = self.bookmarks.get(bm_id)
            if bm:
                bm.folder = "Personal Locations"
                default_folder.bookmark_ids.append(bm_id)

        del self.folders[owner_id][folder_name]
        return True

    def share_bookmark_with_corp(self, owner_id: str, bookmark_id: str) -> bool:
        """Share a bookmark with corporation members."""
        bookmark = self.bookmarks.get(bookmark_id)
        if not bookmark or bookmark.owner_id != owner_id:
            return False
        bookmark.shared_with_corp = True
        return True

    def get_corp_bookmarks(
        self,
        corp_member_ids: List[str],
        system_id: Optional[str] = None
    ) -> List[Bookmark]:
        """Get all bookmarks shared with the corporation."""
        shared = []
        for member_id in corp_member_ids:
            bookmark_ids = self.player_bookmarks.get(member_id, [])
            for bm_id in bookmark_ids:
                bm = self.bookmarks.get(bm_id)
                if bm and bm.shared_with_corp:
                    if system_id and bm.system_id != system_id:
                        continue
                    shared.append(bm)
        return shared

    def _create_folder(self, owner_id: str, folder_name: str):
        """Internal method to create a folder."""
        self._folder_counter += 1
        folder = BookmarkFolder(
            folder_id=f"folder_{self._folder_counter}",
            owner_id=owner_id,
            name=folder_name,
        )
        self.folders[owner_id][folder_name] = folder
