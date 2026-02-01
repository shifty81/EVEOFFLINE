"""
Test manufacturing/industry system
Tests blueprint research, manufacturing, and inventory management
"""
import sys
import os

# Add parent directory to path
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from engine.core.ecs import World, Entity
from engine.systems.industry_system import (
    IndustrySystem, Blueprint, BlueprintInventory, 
    Inventory, ManufacturingQueue
)


def test_blueprint_creation():
    """Test blueprint creation and initialization"""
    print("\n=== Testing Blueprint Creation ===")
    
    world = World()
    industry = IndustrySystem(world)
    
    # Create entity with blueprint inventory
    entity = world.create_entity()
    bp_inv = BlueprintInventory()
    entity.add_component(bp_inv)
    
    # Add a blueprint (BPO with infinite runs = -1)
    bp = Blueprint(
        blueprint_id="frigate_bp",
        item_type="ship",
        item_id="frigate",
        runs=-1,  # BPO has infinite runs
        material_efficiency=0,
        time_efficiency=0
    )
    bp_inv.blueprints["frigate_bp"] = bp
    
    assert len(bp_inv.blueprints) == 1, "Should have 1 blueprint"
    assert bp_inv.blueprints["frigate_bp"].runs == -1, "BPO should have -1 runs"
    print("✓ Blueprint created successfully")


def test_material_efficiency_research():
    """Test ME research"""
    print("\n=== Testing Material Efficiency Research ===")
    
    world = World()
    industry = IndustrySystem(world)
    
    # Create entity with blueprint
    entity = world.create_entity()
    bp_inv = BlueprintInventory()
    bp = Blueprint(
        blueprint_id="frigate_bp",
        item_type="ship",
        item_id="frigate",
        runs=-1,  # BPO
        material_efficiency=0,
        time_efficiency=0
    )
    bp_inv.blueprints["frigate_bp"] = bp
    entity.add_component(bp_inv)
    
    # Research ME
    initial_me = bp.material_efficiency
    success = industry.research_material_efficiency(entity, "frigate_bp")
    
    assert success, "Research should succeed"
    assert bp.material_efficiency == initial_me + 1, "ME should increase by 1"
    print(f"✓ ME researched: {initial_me} -> {bp.material_efficiency}")
    
    # Research to max level (10)
    for i in range(9):
        industry.research_material_efficiency(entity, "frigate_bp")
    
    assert bp.material_efficiency == 10, "ME should cap at 10"
    
    # Try to exceed max
    success = industry.research_material_efficiency(entity, "frigate_bp")
    assert not success, "Should not research beyond max level"
    print("✓ ME caps at level 10")


def test_time_efficiency_research():
    """Test TE research"""
    print("\n=== Testing Time Efficiency Research ===")
    
    world = World()
    industry = IndustrySystem(world)
    
    # Create entity with blueprint
    entity = world.create_entity()
    bp_inv = BlueprintInventory()
    bp = Blueprint(
        blueprint_id="module_bp",
        item_type="module",
        item_id="module",
        runs=-1,  # BPO
        material_efficiency=0,
        time_efficiency=0
    )
    bp_inv.blueprints["module_bp"] = bp
    entity.add_component(bp_inv)
    
    # Research TE
    initial_te = bp.time_efficiency
    success = industry.research_time_efficiency(entity, "module_bp")
    
    assert success, "Research should succeed"
    assert bp.time_efficiency == initial_te + 1, "TE should increase by 1"
    print(f"✓ TE researched: {initial_te} -> {bp.time_efficiency}")
    
    # Research to max level (20)
    for i in range(19):
        industry.research_time_efficiency(entity, "module_bp")
    
    assert bp.time_efficiency == 20, "TE should cap at 20"
    
    # Try to exceed max
    success = industry.research_time_efficiency(entity, "module_bp")
    assert not success, "Should not research beyond max level"
    print("✓ TE caps at level 20")


def test_blueprint_copy_restrictions():
    """Test that BPCs cannot be researched or copied"""
    print("\n=== Testing BPC Restrictions ===")
    
    world = World()
    industry = IndustrySystem(world)
    
    # Create entity with BPC (limited runs)
    entity = world.create_entity()
    bp_inv = BlueprintInventory()
    bpc = Blueprint(
        blueprint_id="ammo_bpc",
        item_type="ammo",
        item_id="ammo",
        runs=10,  # BPC with 10 runs
        material_efficiency=0,
        time_efficiency=0
    )
    bp_inv.blueprints["ammo_bpc"] = bpc
    entity.add_component(bp_inv)
    
    # Try to research ME on BPC (should fail)
    success = industry.research_material_efficiency(entity, "ammo_bpc")
    assert not success, "Should not research ME on BPC"
    
    # Try to research TE on BPC (should fail)
    success = industry.research_time_efficiency(entity, "ammo_bpc")
    assert not success, "Should not research TE on BPC"
    
    # Try to copy BPC (should fail)
    result = industry.copy_blueprint(entity, "ammo_bpc", runs=5)
    assert not result, "Should not copy BPC"
    
    print("✓ BPC restrictions enforced correctly")


def test_manufacturing_basic():
    """Test basic manufacturing"""
    print("\n=== Testing Basic Manufacturing ===")
    
    world = World()
    industry = IndustrySystem(world)
    
    # Create entity with blueprint and inventory
    entity = world.create_entity()
    
    # Add blueprint with materials defined
    bp_inv = BlueprintInventory()
    bp = Blueprint(
        blueprint_id="item_bp",
        item_type="module",
        item_id="test_item",
        runs=-1,  # BPO
        material_efficiency=0,
        time_efficiency=0,
        materials={"mineral_1": 10, "mineral_2": 5},  # Material requirements
        manufacturing_time=100.0
    )
    bp_inv.blueprints["item_bp"] = bp
    entity.add_component(bp_inv)
    
    # Add inventory with materials
    inventory = Inventory()
    inventory.items["mineral_1"] = 100
    inventory.items["mineral_2"] = 50
    entity.add_component(inventory)
    
    # Add manufacturing queue
    mfg_queue = ManufacturingQueue()
    entity.add_component(mfg_queue)
    
    # Start manufacturing
    job_id = industry.start_manufacturing(
        entity=entity,
        blueprint_id="item_bp",
        quantity=1,
        current_time=0.0
    )
    
    assert job_id is not None, "Should return job ID"
    assert len(mfg_queue.jobs) == 1, "Should have 1 active job"
    
    # Check materials consumed
    assert inventory.items["mineral_1"] == 90, "Should consume 10 mineral_1"
    assert inventory.items["mineral_2"] == 45, "Should consume 5 mineral_2"
    print(f"✓ Manufacturing job started: {job_id}")
    print(f"  Materials consumed: mineral_1=-10, mineral_2=-5")


def test_manufacturing_with_me():
    """Test manufacturing with ME bonus"""
    print("\n=== Testing Manufacturing with ME Bonus ===")
    
    world = World()
    industry = IndustrySystem(world)
    
    # Create entity with researched blueprint
    entity = world.create_entity()
    
    # Add blueprint with ME 10 (10% material reduction)
    bp_inv = BlueprintInventory()
    bp = Blueprint(
        blueprint_id="efficient_bp",
        item_type="module",
        item_id="efficient_item",
        runs=-1,  # BPO
        material_efficiency=10,  # 10% reduction
        time_efficiency=0,
        materials={"mineral": 100},  # Base 100 minerals
        manufacturing_time=100.0
    )
    bp_inv.blueprints["efficient_bp"] = bp
    entity.add_component(bp_inv)
    
    # Add inventory with materials
    inventory = Inventory()
    inventory.items["mineral"] = 1000
    entity.add_component(inventory)
    
    # Add manufacturing queue
    mfg_queue = ManufacturingQueue()
    entity.add_component(mfg_queue)
    
    # Start manufacturing
    job_id = industry.start_manufacturing(
        entity=entity,
        blueprint_id="efficient_bp",
        quantity=1,
        current_time=0.0
    )
    
    assert job_id is not None, "Should start successfully"
    
    # Check that less materials were consumed due to ME
    # 100 * (1 - 0.10) = 90
    materials_remaining = inventory.items["mineral"]
    materials_used = 1000 - materials_remaining
    assert materials_used == 90, f"Should use 90 materials with ME10 (used {materials_used})"
    print(f"✓ ME bonus applied: 100 base -> {materials_used} actual (10% reduction)")


def test_inventory_capacity():
    """Test inventory capacity limits"""
    print("\n=== Testing Inventory Capacity ===")
    
    inventory = Inventory(capacity=100.0)
    
    # Add items
    success = inventory.add_item("item_1", 50, volume_per_item=1.0)
    assert success, "Should add items when there's space"
    assert inventory.used_capacity == 50.0, "Should track capacity"
    
    # Try to add more than capacity
    success = inventory.add_item("item_2", 60, volume_per_item=1.0)
    assert not success, "Should reject when over capacity"
    assert inventory.used_capacity == 50.0, "Capacity should not change"
    
    # Add within remaining capacity
    success = inventory.add_item("item_2", 30, volume_per_item=1.0)
    assert success, "Should add when within capacity"
    assert inventory.used_capacity == 80.0, "Should track capacity correctly"
    
    print(f"✓ Inventory capacity managed: {inventory.used_capacity}/{inventory.capacity}")


def test_manufacturing_failure_insufficient_materials():
    """Test manufacturing fails with insufficient materials"""
    print("\n=== Testing Manufacturing with Insufficient Materials ===")
    
    world = World()
    industry = IndustrySystem(world)
    
    # Create entity with blueprint
    entity = world.create_entity()
    
    bp_inv = BlueprintInventory()
    bp = Blueprint(
        blueprint_id="item_bp",
        item_type="module",
        item_id="test_item",
        runs=-1,  # BPO
        materials={"mineral": 100}
    )
    bp_inv.blueprints["item_bp"] = bp
    entity.add_component(bp_inv)
    
    # Insufficient inventory
    inventory = Inventory()
    inventory.items["mineral"] = 50  # Only have 50, need 100
    entity.add_component(inventory)
    
    mfg_queue = ManufacturingQueue()
    entity.add_component(mfg_queue)
    
    # Try to manufacture with insufficient materials
    job_id = industry.start_manufacturing(
        entity=entity,
        blueprint_id="item_bp",
        quantity=1,
        current_time=0.0
    )
    
    # Should fail
    assert job_id is None, "Should fail with insufficient materials"
    assert len(mfg_queue.jobs) == 0, "Should not create job"
    assert inventory.items["mineral"] == 50, "Should not consume materials"
    print("✓ Handled insufficient materials correctly")


def test_bpc_runs_consumption():
    """Test that BPC runs are consumed during manufacturing"""
    print("\n=== Testing BPC Runs Consumption ===")
    
    world = World()
    industry = IndustrySystem(world)
    
    # Create entity with BPC
    entity = world.create_entity()
    
    bp_inv = BlueprintInventory()
    bpc = Blueprint(
        blueprint_id="ammo_bpc",
        item_type="ammo",
        item_id="ammo",
        runs=3,  # BPC with 3 runs
        materials={"mineral": 10}
    )
    bp_inv.blueprints["ammo_bpc"] = bpc
    entity.add_component(bp_inv)
    
    # Add inventory
    inventory = Inventory()
    inventory.items["mineral"] = 1000
    entity.add_component(inventory)
    
    mfg_queue = ManufacturingQueue()
    entity.add_component(mfg_queue)
    
    # Manufacture 2 items
    job_id = industry.start_manufacturing(entity, "ammo_bpc", quantity=2, current_time=0.0)
    
    assert job_id is not None, "Should succeed"
    assert bpc.runs == 1, f"Should have 1 run left (has {bpc.runs})"
    
    # Try to manufacture 2 more (should fail - only 1 run left)
    job_id2 = industry.start_manufacturing(entity, "ammo_bpc", quantity=2, current_time=0.0)
    assert job_id2 is None, "Should fail with insufficient runs"
    
    print("✓ BPC runs consumed correctly")


if __name__ == "__main__":
    print("=" * 60)
    print("Testing Manufacturing/Industry System")
    print("=" * 60)
    
    test_blueprint_creation()
    test_material_efficiency_research()
    test_time_efficiency_research()
    test_blueprint_copy_restrictions()
    test_manufacturing_basic()
    test_manufacturing_with_me()
    test_inventory_capacity()
    test_manufacturing_failure_insufficient_materials()
    test_bpc_runs_consumption()
    
    print("\n" + "=" * 60)
    print("✅ ALL MANUFACTURING TESTS PASSED!")
    print("=" * 60)
