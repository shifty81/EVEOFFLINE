"""
Tests for new EVE OFFLINE systems:
- Insurance System
- Bounty System
- NPC Damage Reference
- Bookmark System
- Wing Commander System (NPC Pilots)
"""
import sys
import os
import unittest

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from engine.systems.insurance_system import (
    InsuranceSystem, InsuranceLevel, INSURANCE_TIERS, INSURANCE_DURATION
)
from engine.systems.bounty_system import (
    BountySystem, NPC_BOUNTIES
)
from engine.systems.npc_damage_reference import (
    get_optimal_damage, get_tank_against, get_faction_ewar,
    get_all_factions, get_faction_profile, NPC_FACTION_PROFILES
)
from engine.systems.bookmark_system import BookmarkSystem
from engine.systems.wing_commander_system import (
    WingCommanderSystem, PilotSpecialization, PilotStatus,
    TaskType, TaskPriority, MAX_WING_SIZE, PILOT_COSTS
)


class TestInsuranceSystem(unittest.TestCase):
    """Tests for the ship insurance system"""

    def setUp(self):
        self.insurance = InsuranceSystem()

    def test_get_hull_value_specific_ship(self):
        """Test getting hull value for a specific ship"""
        value = self.insurance.get_hull_value("rifter", "Frigate")
        self.assertEqual(value, 450000)

    def test_get_hull_value_class_fallback(self):
        """Test hull value falls back to ship class default"""
        value = self.insurance.get_hull_value("unknown_ship", "Cruiser")
        self.assertEqual(value, 10000000)

    def test_get_insurance_quote(self):
        """Test getting insurance quotes for different tiers"""
        # Platinum quote
        quote = self.insurance.get_insurance_quote("rifter", "Frigate", InsuranceLevel.PLATINUM)
        self.assertEqual(quote["hull_value"], 450000)
        self.assertAlmostEqual(quote["payout"], 450000)  # 100% payout
        self.assertAlmostEqual(quote["premium"], 225000)  # 50% cost

        # None (free) quote
        quote_free = self.insurance.get_insurance_quote("rifter", "Frigate", InsuranceLevel.NONE)
        self.assertEqual(quote_free["premium"], 0.0)
        self.assertAlmostEqual(quote_free["payout"], 180000)  # 40% payout

    def test_purchase_insurance(self):
        """Test purchasing insurance"""
        policy = self.insurance.purchase_insurance(
            ship_entity_id="ship_1",
            ship_type="rifter",
            ship_class="Frigate",
            owner_id="player_1",
            level=InsuranceLevel.PLATINUM,
            current_time=1000.0,
            wallet_isk=1000000.0
        )
        self.assertIsNotNone(policy)
        self.assertTrue(policy.is_active)
        self.assertEqual(policy.ship_type, "rifter")
        self.assertEqual(policy.level, InsuranceLevel.PLATINUM)

    def test_purchase_insufficient_isk(self):
        """Test that purchasing fails with insufficient ISK"""
        policy = self.insurance.purchase_insurance(
            ship_entity_id="ship_1",
            ship_type="rifter",
            ship_class="Frigate",
            owner_id="player_1",
            level=InsuranceLevel.PLATINUM,
            current_time=1000.0,
            wallet_isk=100.0  # Not enough
        )
        self.assertIsNone(policy)

    def test_claim_insurance(self):
        """Test claiming insurance on ship destruction"""
        # Purchase policy
        self.insurance.purchase_insurance(
            ship_entity_id="ship_1",
            ship_type="rifter",
            ship_class="Frigate",
            owner_id="player_1",
            level=InsuranceLevel.PLATINUM,
            current_time=1000.0,
            wallet_isk=1000000.0
        )

        # Claim
        record = self.insurance.claim_insurance("ship_1", current_time=2000.0)
        self.assertIsNotNone(record)
        self.assertAlmostEqual(record.payout_amount, 450000.0)
        self.assertEqual(record.owner_id, "player_1")

    def test_claim_expired_policy(self):
        """Test that expired policies don't pay out"""
        self.insurance.purchase_insurance(
            ship_entity_id="ship_1",
            ship_type="rifter",
            ship_class="Frigate",
            owner_id="player_1",
            level=InsuranceLevel.PLATINUM,
            current_time=1000.0,
            wallet_isk=1000000.0
        )

        # Claim after expiry (12 weeks + 1 second)
        record = self.insurance.claim_insurance(
            "ship_1", current_time=1000.0 + INSURANCE_DURATION + 1
        )
        self.assertIsNone(record)

    def test_void_policy(self):
        """Test voiding a policy (e.g., ship repackaged)"""
        self.insurance.purchase_insurance(
            ship_entity_id="ship_1",
            ship_type="rifter",
            ship_class="Frigate",
            owner_id="player_1",
            level=InsuranceLevel.GOLD,
            current_time=1000.0,
            wallet_isk=1000000.0
        )

        result = self.insurance.void_policy("ship_1")
        self.assertTrue(result)

        # Claim should fail
        record = self.insurance.claim_insurance("ship_1", current_time=2000.0)
        self.assertIsNone(record)

    def test_claim_history(self):
        """Test claim history tracking"""
        self.insurance.purchase_insurance(
            "ship_1", "rifter", "Frigate", "player_1",
            InsuranceLevel.PLATINUM, 1000.0, 1000000.0
        )
        self.insurance.claim_insurance("ship_1", 2000.0)

        history = self.insurance.get_claim_history("player_1")
        self.assertEqual(len(history), 1)
        self.assertEqual(history[0].ship_type, "rifter")


class TestBountySystem(unittest.TestCase):
    """Tests for the NPC bounty system"""

    def setUp(self):
        self.bounty = BountySystem()

    def test_get_bounty_value(self):
        """Test getting bounty for known NPC types"""
        value = self.bounty.get_bounty_value("serpentis_frigate")
        self.assertEqual(value, 20000)

    def test_bounty_unknown_npc(self):
        """Test bounty for unknown NPC type is 0"""
        value = self.bounty.get_bounty_value("unknown_npc")
        self.assertEqual(value, 0)

    def test_bounty_security_multiplier(self):
        """Test security status affects bounty value"""
        highsec = self.bounty.get_bounty_value("serpentis_frigate", 1.0)
        lowsec = self.bounty.get_bounty_value("serpentis_frigate", 0.3)
        self.assertGreater(lowsec, highsec)

    def test_process_solo_kill(self):
        """Test processing a solo NPC kill"""
        payments = self.bounty.process_npc_kill(
            npc_type="serpentis_cruiser",
            npc_faction="Serpentis",
            killer_id="player_1",
            current_time=1000.0
        )
        self.assertEqual(len(payments), 1)
        self.assertEqual(payments[0].individual_share, 150000)
        self.assertFalse(payments[0].fleet_split)

    def test_process_fleet_kill(self):
        """Test bounty split among fleet members"""
        payments = self.bounty.process_npc_kill(
            npc_type="serpentis_cruiser",
            npc_faction="Serpentis",
            killer_id="player_1",
            fleet_member_ids=["player_1", "player_2", "player_3"],
            current_time=1000.0
        )
        self.assertEqual(len(payments), 3)
        per_member = 150000 / 3
        for p in payments:
            self.assertTrue(p.fleet_split)
            self.assertAlmostEqual(p.individual_share, per_member)

    def test_total_earned_tracking(self):
        """Test total bounties earned tracking"""
        self.bounty.process_npc_kill(
            "serpentis_frigate", "Serpentis", "player_1", current_time=100.0
        )
        self.bounty.process_npc_kill(
            "serpentis_cruiser", "Serpentis", "player_1", current_time=200.0
        )
        total = self.bounty.get_total_earned("player_1")
        self.assertEqual(total, 20000 + 150000)

    def test_session_earnings(self):
        """Test session earnings tracking"""
        self.bounty.process_npc_kill(
            "serpentis_frigate", "Serpentis", "player_1", current_time=100.0
        )
        self.bounty.process_npc_kill(
            "serpentis_cruiser", "Serpentis", "player_1", current_time=500.0
        )
        # Session started at 200, should only count second kill
        session = self.bounty.get_session_earnings("player_1", 200.0)
        self.assertEqual(session, 150000)


class TestNPCDamageReference(unittest.TestCase):
    """Tests for the NPC damage type reference"""

    def test_get_optimal_damage_serpentis(self):
        """Test optimal damage against Serpentis"""
        primary, secondary = get_optimal_damage("serpentis")
        self.assertEqual(primary, "kinetic")
        self.assertEqual(secondary, "thermal")

    def test_get_tank_against_blood_raiders(self):
        """Test what to tank against Blood Raiders"""
        primary, secondary = get_tank_against("blood_raiders")
        self.assertEqual(primary, "em")
        self.assertEqual(secondary, "thermal")

    def test_get_faction_ewar(self):
        """Test EWAR types for factions"""
        ewar = get_faction_ewar("guristas")
        self.assertIn("ecm", ewar)

    def test_all_factions_exist(self):
        """Test all expected factions are in the reference"""
        factions = get_all_factions()
        self.assertIn("serpentis", factions)
        self.assertIn("guristas", factions)
        self.assertIn("blood_raiders", factions)
        self.assertIn("sansha", factions)
        self.assertIn("angel_cartel", factions)
        self.assertIn("rogue_drones", factions)

    def test_unknown_faction_defaults(self):
        """Test unknown faction returns safe defaults"""
        primary, secondary = get_optimal_damage("unknown_faction")
        self.assertEqual(primary, "kinetic")
        self.assertEqual(secondary, "thermal")

    def test_faction_profile_complete(self):
        """Test that faction profiles have all required fields"""
        profile = get_faction_profile("serpentis")
        self.assertIsNotNone(profile)
        self.assertEqual(profile.faction_name, "Serpentis Corporation")
        self.assertTrue(len(profile.home_regions) > 0)
        self.assertTrue(len(profile.ewar_types) > 0)


class TestBookmarkSystem(unittest.TestCase):
    """Tests for the bookmark system"""

    def setUp(self):
        self.bookmarks = BookmarkSystem()

    def test_create_bookmark(self):
        """Test creating a bookmark"""
        bm = self.bookmarks.create_bookmark(
            owner_id="player_1",
            label="My Safe Spot",
            system_id="jita",
            x=100.0, y=200.0, z=300.0,
            memo="Safe spot near station",
            current_time=1000.0
        )
        self.assertIsNotNone(bm)
        self.assertEqual(bm.label, "My Safe Spot")
        self.assertEqual(bm.system_id, "jita")

    def test_delete_bookmark(self):
        """Test deleting a bookmark"""
        bm = self.bookmarks.create_bookmark(
            "player_1", "Test BM", "jita", 0, 0, 0
        )
        result = self.bookmarks.delete_bookmark("player_1", bm.bookmark_id)
        self.assertTrue(result)
        self.assertIsNone(self.bookmarks.get_bookmark(bm.bookmark_id))

    def test_cannot_delete_others_bookmark(self):
        """Test that players can't delete other players' bookmarks"""
        bm = self.bookmarks.create_bookmark(
            "player_1", "Test BM", "jita", 0, 0, 0
        )
        result = self.bookmarks.delete_bookmark("player_2", bm.bookmark_id)
        self.assertFalse(result)

    def test_edit_bookmark(self):
        """Test editing a bookmark"""
        bm = self.bookmarks.create_bookmark(
            "player_1", "Old Label", "jita", 0, 0, 0
        )
        result = self.bookmarks.edit_bookmark(
            "player_1", bm.bookmark_id, label="New Label", memo="Updated"
        )
        self.assertTrue(result)
        updated = self.bookmarks.get_bookmark(bm.bookmark_id)
        self.assertEqual(updated.label, "New Label")
        self.assertEqual(updated.memo, "Updated")

    def test_bookmark_limit(self):
        """Test bookmark limit per player"""
        self.bookmarks.max_bookmarks = 5
        for i in range(5):
            bm = self.bookmarks.create_bookmark(
                "player_1", f"BM {i}", "jita", i, 0, 0
            )
            self.assertIsNotNone(bm)

        # 6th should fail
        bm = self.bookmarks.create_bookmark(
            "player_1", "BM 5", "jita", 5, 0, 0
        )
        self.assertIsNone(bm)

    def test_get_bookmarks_by_system(self):
        """Test filtering bookmarks by system"""
        self.bookmarks.create_bookmark("player_1", "Jita BM", "jita", 0, 0, 0)
        self.bookmarks.create_bookmark("player_1", "Rancer BM", "rancer", 0, 0, 0)

        jita_bms = self.bookmarks.get_bookmarks("player_1", system_id="jita")
        self.assertEqual(len(jita_bms), 1)
        self.assertEqual(jita_bms[0].label, "Jita BM")

    def test_folder_management(self):
        """Test bookmark folder creation and management"""
        self.bookmarks.create_folder("player_1", "Combat Sites")
        folders = self.bookmarks.get_folders("player_1")
        self.assertIn("Combat Sites", folders)

    def test_share_with_corp(self):
        """Test sharing bookmarks with corporation"""
        bm = self.bookmarks.create_bookmark(
            "player_1", "Shared Spot", "jita", 100, 200, 300
        )
        result = self.bookmarks.share_bookmark_with_corp("player_1", bm.bookmark_id)
        self.assertTrue(result)

        corp_bms = self.bookmarks.get_corp_bookmarks(
            ["player_1", "player_2"], system_id="jita"
        )
        self.assertEqual(len(corp_bms), 1)


class TestWingCommanderSystem(unittest.TestCase):
    """Tests for the NPC wing commander system"""

    def setUp(self):
        self.wing = WingCommanderSystem()

    def test_hire_pilot(self):
        """Test hiring an NPC pilot"""
        pilot = self.wing.hire_pilot(
            owner_id="player_1",
            name="Marcus Stone",
            specialization=PilotSpecialization.COMBAT,
            skill_level=1,
            wallet_isk=500000.0,
            current_time=1000.0
        )
        self.assertIsNotNone(pilot)
        self.assertEqual(pilot.name, "Marcus Stone")
        self.assertEqual(pilot.specialization, PilotSpecialization.COMBAT)
        self.assertEqual(pilot.status, PilotStatus.AVAILABLE)

    def test_hire_insufficient_isk(self):
        """Test hiring fails with insufficient ISK"""
        pilot = self.wing.hire_pilot(
            "player_1", "Test", PilotSpecialization.COMBAT,
            skill_level=1, wallet_isk=50.0  # Not enough
        )
        self.assertIsNone(pilot)

    def test_wing_size_limit(self):
        """Test max wing size of 26"""
        for i in range(MAX_WING_SIZE):
            pilot = self.wing.hire_pilot(
                "player_1", f"Pilot {i}", PilotSpecialization.COMBAT,
                wallet_isk=10000000.0
            )
            self.assertIsNotNone(pilot)

        # 27th should fail
        pilot = self.wing.hire_pilot(
            "player_1", "Too Many", PilotSpecialization.COMBAT,
            wallet_isk=10000000.0
        )
        self.assertIsNone(pilot)

    def test_assign_ship(self):
        """Test assigning a pilot to a ship"""
        pilot = self.wing.hire_pilot(
            "player_1", "Marcus", PilotSpecialization.COMBAT,
            wallet_isk=500000.0
        )
        result = self.wing.assign_ship(
            "player_1", pilot.pilot_id,
            "ship_123", "Rifter", "Frigate"
        )
        self.assertTrue(result)
        self.assertEqual(pilot.assigned_ship_id, "ship_123")
        self.assertEqual(pilot.status, PilotStatus.ASSIGNED)

    def test_assign_wrong_ship_class(self):
        """Test that miners can't fly battleships"""
        pilot = self.wing.hire_pilot(
            "player_1", "Helia", PilotSpecialization.MINER,
            wallet_isk=500000.0
        )
        result = self.wing.assign_ship(
            "player_1", pilot.pilot_id,
            "ship_456", "Tempest", "Battleship"
        )
        self.assertFalse(result)

    def test_give_order(self):
        """Test giving an order to an NPC pilot"""
        pilot = self.wing.hire_pilot(
            "player_1", "Marcus", PilotSpecialization.COMBAT,
            wallet_isk=500000.0
        )
        self.wing.assign_ship("player_1", pilot.pilot_id,
                              "ship_123", "Rifter", "Frigate")

        task = self.wing.give_order(
            "player_1", pilot.pilot_id,
            TaskType.GUARD,
            current_time=1000.0
        )
        self.assertIsNotNone(task)
        self.assertEqual(pilot.status, PilotStatus.EXECUTING)

    def test_give_wrong_specialization_order(self):
        """Test that combat pilots can't mine"""
        pilot = self.wing.hire_pilot(
            "player_1", "Marcus", PilotSpecialization.COMBAT,
            wallet_isk=500000.0
        )
        self.wing.assign_ship("player_1", pilot.pilot_id,
                              "ship_123", "Rifter", "Frigate")

        task = self.wing.give_order(
            "player_1", pilot.pilot_id,
            TaskType.MINE_HERE
        )
        self.assertIsNone(task)

    def test_order_all(self):
        """Test ordering all pilots at once"""
        for i in range(3):
            p = self.wing.hire_pilot(
                "player_1", f"Pilot {i}", PilotSpecialization.COMBAT,
                wallet_isk=10000000.0
            )
            self.wing.assign_ship("player_1", p.pilot_id,
                                  f"ship_{i}", "Rifter", "Frigate")

        tasks = self.wing.order_all("player_1", TaskType.FOLLOW)
        self.assertEqual(len(tasks), 3)

    def test_create_trade_route(self):
        """Test creating a trade route"""
        route = self.wing.create_trade_route(
            owner_id="player_1",
            name="Jita-Amarr Run",
            stops=[
                {"station_id": "jita_4_4", "action": "buy", "item_id": "tritanium", "quantity": 10000},
                {"station_id": "amarr_trade_hub", "action": "sell", "item_id": "tritanium", "quantity": 10000},
            ]
        )
        self.assertIsNotNone(route)
        self.assertEqual(route.name, "Jita-Amarr Run")
        self.assertTrue(route.loop)

    def test_trade_route_requires_min_stops(self):
        """Test that trade route needs at least 2 stops"""
        route = self.wing.create_trade_route(
            "player_1", "Bad Route",
            stops=[{"station_id": "jita", "action": "buy"}]
        )
        self.assertIsNone(route)

    def test_dismiss_pilot(self):
        """Test dismissing an NPC pilot"""
        pilot = self.wing.hire_pilot(
            "player_1", "Dismissable", PilotSpecialization.HAULER,
            wallet_isk=500000.0
        )
        result = self.wing.dismiss_pilot("player_1", pilot.pilot_id)
        self.assertTrue(result)
        self.assertEqual(self.wing.get_wing_size("player_1"), 0)

    def test_experience_and_level_up(self):
        """Test pilot XP gain and leveling"""
        pilot = self.wing.hire_pilot(
            "player_1", "Trainee", PilotSpecialization.COMBAT,
            skill_level=1, wallet_isk=500000.0
        )
        # Need 10000 XP to level from 1 to 2
        result = self.wing.gain_experience(pilot.pilot_id, 10000)
        self.assertTrue(result["leveled_up"])
        self.assertEqual(result["new_level"], 2)
        self.assertEqual(pilot.skill_level, 2)

    def test_salary_processing(self):
        """Test salary payment processing"""
        pilot = self.wing.hire_pilot(
            "player_1", "Paid Pilot", PilotSpecialization.COMBAT,
            wallet_isk=500000.0, current_time=0.0
        )
        # After 1 hour
        result = self.wing.process_salaries(
            "player_1", current_time=3600.0, wallet_isk=100000.0
        )
        self.assertTrue(result["paid"])

    def test_salary_unpaid_loyalty_drop(self):
        """Test loyalty drops when salaries can't be paid"""
        pilot = self.wing.hire_pilot(
            "player_1", "Unpaid Pilot", PilotSpecialization.COMBAT,
            wallet_isk=500000.0, current_time=0.0
        )
        initial_loyalty = pilot.loyalty
        # After 1 hour with no ISK
        result = self.wing.process_salaries(
            "player_1", current_time=3600.0, wallet_isk=0.0
        )
        self.assertFalse(result["paid"])
        self.assertLess(pilot.loyalty, initial_loyalty)

    def test_wing_status(self):
        """Test getting wing status summary"""
        self.wing.hire_pilot(
            "player_1", "Combat 1", PilotSpecialization.COMBAT,
            wallet_isk=500000.0
        )
        self.wing.hire_pilot(
            "player_1", "Miner 1", PilotSpecialization.MINER,
            wallet_isk=500000.0
        )

        status = self.wing.get_wing_status("player_1")
        self.assertEqual(status["total_pilots"], 2)
        self.assertEqual(status["slots_available"], MAX_WING_SIZE - 2)
        self.assertIn("combat", status["specialization_breakdown"])
        self.assertIn("miner", status["specialization_breakdown"])

    def test_complete_task_advances_queue(self):
        """Test completing a task advances to next in queue"""
        pilot = self.wing.hire_pilot(
            "player_1", "Queued", PilotSpecialization.COMBAT,
            wallet_isk=500000.0
        )
        self.wing.assign_ship("player_1", pilot.pilot_id,
                              "ship_1", "Rifter", "Frigate")

        # Give two orders
        self.wing.give_order("player_1", pilot.pilot_id, TaskType.GUARD)
        self.wing.give_order("player_1", pilot.pilot_id, TaskType.ENGAGE_TARGETS)

        # Complete first task
        self.wing.complete_task(pilot.pilot_id, success=True, current_time=2000.0)

        # Should now be on second task
        self.assertIsNotNone(pilot.current_task)
        self.assertEqual(pilot.current_task.task_type, TaskType.ENGAGE_TARGETS)


if __name__ == "__main__":
    unittest.main()
