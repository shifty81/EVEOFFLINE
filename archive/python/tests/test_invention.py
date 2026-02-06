"""
Tests for the Research & Invention system
"""

import sys
import os
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))

from engine.core.ecs import World
from engine.components.game_components import Inventory, SkillTraining
from engine.systems.invention_system import InventionSystem


def test_basic_invention():
    """Test basic invention from T1 BPC to T2 BPC"""
    print("\n=== Test: Basic Invention ===")
    
    world = World()
    invention_system = InventionSystem(world)
    
    # Create player entity
    player = world.create_entity()
    inventory = Inventory(items={}, cargo_capacity=1000)
    skills = SkillTraining()
    skills.trained_skills = {
        "mechanical_engineering": 3,
        "electronic_engineering": 4,
        "caldari_encryption_methods": 5,
    }
    player.add_component(inventory)
    player.add_component(skills)
    
    # Add T1 BPC
    inventory.items["merlin_t1_bpc"] = {
        "type": "blueprint",
        "name": "Merlin Blueprint I",
        "item_id": "merlin_t1",
        "tech_level": 1,
        "is_original": False,
        "runs": 1,
        "me": 0,
        "te": 0,
        "category": "frigate",
        "invention_skills": ["mechanical_engineering", "electronic_engineering", "caldari_encryption_methods"],
    }
    
    # Add datacores
    inventory.items["datacore_mech"] = {
        "type": "datacore",
        "name": "Mechanical Engineering Datacore",
        "science_skill": "Mechanical Engineering",
    }
    inventory.items["datacore_elec"] = {
        "type": "datacore",
        "name": "Electronic Engineering Datacore",
        "science_skill": "Electronic Engineering",
    }
    
    # Check if can invent
    can_invent, msg = invention_system.can_invent(
        player, "merlin_t1_bpc", "datacore_mech", "datacore_elec"
    )
    print(f"Can invent: {can_invent} - {msg}")
    assert can_invent, f"Should be able to invent: {msg}"
    
    # Calculate success chance
    chance = invention_system.calculate_success_chance(player, "merlin_t1_bpc")
    print(f"Success chance: {chance:.1%}")
    assert 0.0 < chance <= 1.0, "Success chance should be between 0 and 100%"
    
    # Attempt invention (may fail, that's OK - we test the mechanics)
    success, t2_bpc, msg = invention_system.attempt_invention(
        player, "merlin_t1_bpc", "datacore_mech", "datacore_elec"
    )
    print(f"Invention result: {msg}")
    
    # Materials should be consumed regardless
    assert "merlin_t1_bpc" not in inventory.items, "T1 BPC should be consumed"
    assert "datacore_mech" not in inventory.items, "Datacore 1 should be consumed"
    assert "datacore_elec" not in inventory.items, "Datacore 2 should be consumed"
    
    if success:
        assert t2_bpc is not None, "Should have T2 BPC data on success"
        assert t2_bpc["tech_level"] == 2, "Should be Tech II"
        assert t2_bpc["runs"] >= 1, "Should have at least 1 run"
        print(f"Created: {t2_bpc['name']} with {t2_bpc['runs']} runs")
    
    print("✓ Basic invention test passed")


def test_invention_with_decryptor():
    """Test invention with decryptor modifiers"""
    print("\n=== Test: Invention with Decryptor ===")
    
    world = World()
    invention_system = InventionSystem(world)
    
    # Create player
    player = world.create_entity()
    inventory = Inventory(items={}, cargo_capacity=1000)
    skills = SkillTraining()
    skills.trained_skills = {
        "molecular_engineering": 5,
        "quantum_physics": 5,
        "gallente_encryption_methods": 5,
    }
    player.add_component(inventory)
    player.add_component(skills)
    
    # Add T1 BPC (module)
    inventory.items["web_t1_bpc"] = {
        "type": "blueprint",
        "name": "Stasis Webifier I Blueprint",
        "item_id": "stasis_web_t1",
        "tech_level": 1,
        "is_original": False,
        "runs": 1,
        "me": 0,
        "te": 0,
        "category": "module",
        "invention_skills": ["molecular_engineering", "quantum_physics", "gallente_encryption_methods"],
    }
    
    # Add datacores
    inventory.items["datacore_mol"] = {
        "type": "datacore",
        "name": "Molecular Engineering Datacore",
        "science_skill": "Molecular Engineering",
    }
    inventory.items["datacore_quantum"] = {
        "type": "datacore",
        "name": "Quantum Physics Datacore",
        "science_skill": "Quantum Physics",
    }
    
    # Add decryptor (Attainment - +80% chance, +4 runs)
    inventory.items["decryptor_att"] = {
        "type": "decryptor",
        "name": "Decryptor - Attainment",
        "probability_modifier": 0.80,
        "runs_modifier": 4,
        "me_modifier": 1,
        "te_modifier": 2,
    }
    
    # Calculate success with decryptor
    chance_with = invention_system.calculate_success_chance(
        player, "web_t1_bpc", "decryptor_att"
    )
    print(f"Success chance with Attainment decryptor: {chance_with:.1%}")
    
    # Should be higher than base (modules are 34% base * skills * 1.8 decryptor)
    assert chance_with > 0.34, "Decryptor should increase success chance"
    
    # Attempt invention
    success, t2_bpc, msg = invention_system.attempt_invention(
        player, "web_t1_bpc", "datacore_mol", "datacore_quantum", "decryptor_att"
    )
    print(f"Invention result: {msg}")
    
    # Decryptor should be consumed
    assert "decryptor_att" not in inventory.items, "Decryptor should be consumed"
    
    if success:
        # Runs should be increased by decryptor
        assert t2_bpc["runs"] >= 14, f"Should have 10 base + 4 decryptor runs, got {t2_bpc['runs']}"
        assert t2_bpc["me"] >= 3, f"Should have 2 base + 1 decryptor ME, got {t2_bpc['me']}"
        assert t2_bpc["te"] >= 6, f"Should have 4 base + 2 decryptor TE, got {t2_bpc['te']}"
        print(f"T2 BPC stats: {t2_bpc['runs']} runs, ME {t2_bpc['me']}, TE {t2_bpc['te']}")
    
    print("✓ Decryptor test passed")


def test_invention_skill_requirements():
    """Test that invention requires proper skills"""
    print("\n=== Test: Skill Requirements ===")
    
    world = World()
    invention_system = InventionSystem(world)
    
    # Create player with insufficient skills
    player = world.create_entity()
    inventory = Inventory(items={}, cargo_capacity=1000)
    skills = SkillTraining()
    skills.trained_skills = {
        "mechanical_engineering": 1,  # Has this one
        # Missing electronic_engineering
    }
    player.add_component(inventory)
    player.add_component(skills)
    
    # Add T1 BPC
    inventory.items["rifter_t1_bpc"] = {
        "type": "blueprint",
        "name": "Rifter Blueprint I",
        "item_id": "rifter_t1",
        "tech_level": 1,
        "is_original": False,
        "runs": 1,
        "me": 0,
        "te": 0,
        "category": "frigate",
        "invention_skills": ["mechanical_engineering", "electronic_engineering"],
    }
    
    # Add datacores
    inventory.items["datacore1"] = {"type": "datacore"}
    inventory.items["datacore2"] = {"type": "datacore"}
    
    # Should fail due to missing skill
    can_invent, msg = invention_system.can_invent(
        player, "rifter_t1_bpc", "datacore1", "datacore2"
    )
    print(f"Can invent without required skill: {can_invent} - {msg}")
    assert not can_invent, "Should not be able to invent without required skills"
    assert "skill" in msg.lower(), "Error message should mention skills"
    
    print("✓ Skill requirement test passed")


def test_invention_from_bpo_fails():
    """Test that you cannot invent from BPO (must use BPC)"""
    print("\n=== Test: Cannot Invent from BPO ===")
    
    world = World()
    invention_system = InventionSystem(world)
    
    player = world.create_entity()
    inventory = Inventory(items={}, cargo_capacity=1000)
    skills = SkillTraining()
    skills.trained_skills = {
        "mechanical_engineering": 5,
        "electronic_engineering": 5,
    }
    player.add_component(inventory)
    player.add_component(skills)
    
    # Add T1 BPO (not BPC)
    inventory.items["catalyst_bpo"] = {
        "type": "blueprint",
        "name": "Catalyst Blueprint",
        "item_id": "catalyst_t1",
        "tech_level": 1,
        "is_original": True,  # This is a BPO
        "me": 10,
        "te": 20,
        "category": "destroyer",
        "invention_skills": ["mechanical_engineering", "electronic_engineering"],
    }
    
    inventory.items["datacore1"] = {"type": "datacore"}
    inventory.items["datacore2"] = {"type": "datacore"}
    
    can_invent, msg = invention_system.can_invent(
        player, "catalyst_bpo", "datacore1", "datacore2"
    )
    print(f"Can invent from BPO: {can_invent} - {msg}")
    assert not can_invent, "Should not be able to invent from BPO"
    assert "bpc" in msg.lower() or "copy" in msg.lower(), "Error should mention BPC requirement"
    
    print("✓ BPO restriction test passed")


def test_research_agent():
    """Test R&D agent research project"""
    print("\n=== Test: R&D Agent Research ===")
    
    world = World()
    invention_system = InventionSystem(world)
    
    player = world.create_entity()
    inventory = Inventory(items={}, cargo_capacity=1000)
    skills = SkillTraining()
    skills.trained_skills = {
        "mechanical_engineering": 4,
    }
    player.add_component(inventory)
    player.add_component(skills)
    
    # Start research with agent
    success, msg = invention_system.start_research_agent(
        player, "agent_jita_mechengineer", "mechanical_engineering"
    )
    print(f"Start research: {success} - {msg}")
    assert success, f"Should be able to start research: {msg}"
    
    # Check research project was created
    assert hasattr(skills, "research_agents"), "Should have research agents"
    assert "agent_jita_mechengineer" in skills.research_agents, "Should have agent registered"
    
    agent_data = skills.research_agents["agent_jita_mechengineer"]
    assert agent_data["science_skill"] == "mechanical_engineering"
    assert agent_data["skill_level"] == 4
    assert agent_data["points_per_day"] == 200, "Should be 50 RP/day * 4 levels = 200"
    
    print(f"Research project: {agent_data['points_per_day']} RP/day")
    
    # Collect datacores after 10 days
    success, datacores, msg = invention_system.collect_datacores(
        player, "agent_jita_mechengineer", days_elapsed=10
    )
    print(f"Collect datacores: {success} - {datacores} datacores - {msg}")
    assert success, f"Should be able to collect datacores: {msg}"
    assert datacores == 20, f"Should get 20 datacores (2000 RP / 100), got {datacores}"
    
    # Check datacore was added to inventory
    datacore_found = False
    for item_id, item_data in inventory.items.items():
        if item_data.get("type") == "datacore":
            assert item_data["quantity"] == 20
            datacore_found = True
            print(f"Datacore added: {item_data['name']} x{item_data['quantity']}")
            break
    
    assert datacore_found, "Datacore should be in inventory"
    
    print("✓ R&D agent test passed")


def test_multiple_invention_attempts():
    """Test multiple invention attempts to verify randomness"""
    print("\n=== Test: Multiple Invention Attempts ===")
    
    world = World()
    invention_system = InventionSystem(world)
    
    successes = 0
    failures = 0
    attempts = 20
    
    for i in range(attempts):
        # Create fresh player for each attempt
        player = world.create_entity()
        inventory = Inventory(items={}, cargo_capacity=1000)
        skills = SkillTraining()
        skills.trained_skills = {
            "quantum_physics": 5,
            "laser_physics": 5,
            "amarr_encryption_methods": 5,
        }
        player.add_component(inventory)
        player.add_component(skills)
        
        # Add materials
        inventory.items[f"punisher_bpc_{i}"] = {
            "type": "blueprint",
            "name": "Punisher Blueprint I",
            "item_id": "punisher_t1",
            "tech_level": 1,
            "is_original": False,
            "runs": 1,
            "me": 0,
            "te": 0,
            "category": "frigate",
            "invention_skills": ["quantum_physics", "laser_physics", "amarr_encryption_methods"],
        }
        inventory.items[f"dc1_{i}"] = {"type": "datacore"}
        inventory.items[f"dc2_{i}"] = {"type": "datacore"}
        
        success, t2_bpc, msg = invention_system.attempt_invention(
            player, f"punisher_bpc_{i}", f"dc1_{i}", f"dc2_{i}"
        )
        
        if success:
            successes += 1
        else:
            failures += 1
    
    print(f"Results over {attempts} attempts:")
    print(f"  Successes: {successes} ({successes/attempts*100:.1f}%)")
    print(f"  Failures: {failures} ({failures/attempts*100:.1f}%)")
    
    # With good skills (15 total skill levels) and base 30% for frigate:
    # Expected: 30% * (1 + 15*0.01) = 30% * 1.15 = 34.5%
    # We should see some successes and some failures
    assert successes > 0, "Should have at least some successes"
    assert failures > 0, "Should have at least some failures (unless extremely lucky)"
    
    print("✓ Multiple attempts test passed")


def test_invention_without_materials():
    """Test that invention fails without required materials"""
    print("\n=== Test: Missing Materials ===")
    
    world = World()
    invention_system = InventionSystem(world)
    
    player = world.create_entity()
    inventory = Inventory(items={}, cargo_capacity=1000)
    skills = SkillTraining()
    skills.trained_skills = {
        "mechanical_engineering": 3,
        "electronic_engineering": 3,
    }
    player.add_component(inventory)
    player.add_component(skills)
    
    # Only add BPC, no datacores
    inventory.items["tristan_bpc"] = {
        "type": "blueprint",
        "name": "Tristan Blueprint I",
        "item_id": "tristan_t1",
        "tech_level": 1,
        "is_original": False,
        "runs": 1,
        "category": "frigate",
        "invention_skills": ["mechanical_engineering", "electronic_engineering"],
    }
    
    can_invent, msg = invention_system.can_invent(
        player, "tristan_bpc", "datacore1", "datacore2"
    )
    print(f"Can invent without datacores: {can_invent} - {msg}")
    assert not can_invent, "Should not be able to invent without datacores"
    assert "not found" in msg.lower(), "Error should mention missing items"
    
    print("✓ Missing materials test passed")


def test_skill_bonus_scaling():
    """Test that higher skills improve success chance"""
    print("\n=== Test: Skill Bonus Scaling ===")
    
    world = World()
    invention_system = InventionSystem(world)
    
    # Test with low skills
    player_low = world.create_entity()
    inventory_low = Inventory(items={}, cargo_capacity=1000)
    skills_low = SkillTraining()
    skills_low.trained_skills = {
        "mechanical_engineering": 1,
        "electronic_engineering": 1,
        "minmatar_encryption_methods": 1,
    }
    player_low.add_component(inventory_low)
    player_low.add_component(skills_low)
    
    inventory_low.items["test_bpc_low"] = {
        "type": "blueprint",
        "tech_level": 1,
        "is_original": False,
        "category": "frigate",
        "invention_skills": ["mechanical_engineering", "electronic_engineering", "minmatar_encryption_methods"],
    }
    
    chance_low = invention_system.calculate_success_chance(player_low, "test_bpc_low")
    print(f"Success chance with level 1 skills: {chance_low:.1%}")
    
    # Test with high skills
    player_high = world.create_entity()
    inventory_high = Inventory(items={}, cargo_capacity=1000)
    skills_high = SkillTraining()
    skills_high.trained_skills = {
        "mechanical_engineering": 5,
        "electronic_engineering": 5,
        "minmatar_encryption_methods": 5,
    }
    player_high.add_component(inventory_high)
    player_high.add_component(skills_high)
    
    inventory_high.items["test_bpc_high"] = {
        "type": "blueprint",
        "tech_level": 1,
        "is_original": False,
        "category": "frigate",
        "invention_skills": ["mechanical_engineering", "electronic_engineering", "minmatar_encryption_methods"],
    }
    
    chance_high = invention_system.calculate_success_chance(player_high, "test_bpc_high")
    print(f"Success chance with level 5 skills: {chance_high:.1%}")
    
    # High skills should give much better chance
    # Low: 30% * (1 + 3*0.01) = 30.9%
    # High: 30% * (1 + 15*0.01) = 34.5%
    assert chance_high > chance_low, "Higher skills should improve success chance"
    improvement = (chance_high - chance_low) / chance_low * 100
    print(f"Improvement: {improvement:.1f}%")
    
    print("✓ Skill scaling test passed")


def run_all_tests():
    """Run all invention system tests"""
    print("=" * 60)
    print("INVENTION SYSTEM TEST SUITE")
    print("=" * 60)
    
    tests = [
        test_basic_invention,
        test_invention_with_decryptor,
        test_invention_skill_requirements,
        test_invention_from_bpo_fails,
        test_research_agent,
        test_multiple_invention_attempts,
        test_invention_without_materials,
        test_skill_bonus_scaling,
    ]
    
    passed = 0
    failed = 0
    
    for test in tests:
        try:
            test()
            passed += 1
        except AssertionError as e:
            print(f"✗ Test failed: {e}")
            failed += 1
        except Exception as e:
            print(f"✗ Test error: {e}")
            failed += 1
    
    print("\n" + "=" * 60)
    print(f"Test Results: {passed} passed, {failed} failed")
    print("=" * 60)
    
    if failed == 0:
        print("🎉 ALL TESTS PASSED! 🎉")
    
    return failed == 0


if __name__ == "__main__":
    success = run_all_tests()
    sys.exit(0 if success else 1)
