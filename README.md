🚫 Notice
이 레포지토리는 개인 포트폴리오 용도로 공개된 프로젝트입니다.
따라서 외부의 Pull Request나 Issue는 정중히 사양하며, 확인 없이 닫힐 수 있습니다.

# MyHackSlash
A Top-Down Hack & Slash game built with **Unreal Engine 5.5**.

## Overview
This project implements core hack-and-slash gameplay mechanics, including a combat system powered by the **Gameplay Ability System (GAS)**, AI behavior trees, and an optimized object pooling system.

## Key Features

### ⚔️ Combat System
- **Gameplay Ability System (GAS):** 
  - Characters implement `IAbilitySystemInterface`.
  - Abilities (`HGA_Attack`, `HGA_FireBall`) handle gameplay logic.
  - Attributes and Tags manage state (e.g., cooldowns, damage).
- **Combat Interface:** `HCombatInterface` standardizes damage dealing and receiving.
- **Animation Montages:** Fluid attack sequences and hit reactions.

### 🧠 AI System
- **Behavior Trees:** Modular AI logic with custom tasks, services, and decorators located in `Source/MyHackSlash/AI`.
  - `BTD_AttackInRange`: Checks attack feasibility.
  - `BTS_Detect`: Perception and target acquisition.
  - `BTT_Attack`, `BTT_FindPatrolPos`: Action execution.
- **AI Controller:** Custom `HMonsterAIController` (located in `System`) manages enemy decision-making.

### ⚙️ Systems & Architecture
- **Object Pooling:** `UHObjectPoolManager` (World Subsystem) efficiently manages actor lifecycles to reduce allocation overhead during combat.
- **Enhanced Input:** Modern input handling for player controls.
- **Monster Spawning:** Data-driven spawning system (`HMonsterSpawnerDataAsset`, `HMonsterSpawnManager`).

### 💎 Gem Inventory & Upgrade System
- **Inventory Component:** Manages gem instances with an event-driven UI binding.
- **Auto-Upgrade (3-in-1):** 
  - Automatically merges 3 identical gems (ID & Tier) into a higher-tier gem.
  - **Smart Re-equipping:** If an equipped gem is upgraded, the new gem is automatically re-equipped in the same slot.
  - **Support Gem Inheritance:** Main gems retain their linked support gems even after an upgrade.
- **Data-Driven Design:** Gem properties and progression are defined via `UHGemDataAsset`.

## Project Structure (`Source/MyHackSlash`)

| Directory | Description |
|-----------|-------------|
| `AI` | Behavior Tree nodes and AI controllers. |
| `Attribute` | AttributeSets for health, mana, and combat stats. |
| `DataAsset` | Data definitions for skills, items, and wave configurations. |
| `GAS` | Gameplay Ability implementations (`HGA_Attack`, etc.). |
| `Item` | Interactable items like coins and drops. |
| `Mode` | Game Mode, State, and Player Controller classes. |
| `Skill` | Core logic for the Gem system, Projectiles, and Inventory. |
| `System` | Managers (UI, Pool, Quest, Wave) and global type definitions. |
| `UI` | UserWidgets and ListView data objects. |
| `Unit` | Character base classes (`HBaseCharacter`), Player, and Monsters. |

## Tech Stack
- **Engine:** Unreal Engine 5.5
- **Languages:** C++, Blueprints
- **Plugins/Modules:** 
  - GameplayAbilities (GAS)
  - EnhancedInput
  - Niagara
  - AIModule

## Getting Started
1. Clone the repository.
2. Right-click `MyHackSlash.uproject` and select **Generate Visual Studio project files**.
3. Open `MyHackSlash.sln` in Visual Studio.
4. Build the project (Development Editor / Win64).
5. Open `MyHackSlash.uproject` in Unreal Editor.
