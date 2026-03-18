# MyHackSlash

A Top-Down Hack & Slash game built with **Unreal Engine 5.5**.

## Overview
This project implements core hack-and-slash gameplay mechanics, including a combat system powered by the **Gameplay Ability System (GAS)**, AI behavior trees, and an optimized object pooling system.

## Key Features

### ⚔️ Combat System
- **Gameplay Ability System (GAS):** 
  - Characters implement `IAbilitySystemInterface`.
  - Abilities (`HGA_Attack`) handle gameplay logic.
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

## Project Structure (`Source/MyHackSlash`)

| Directory | Description |
|-----------|-------------|
| `AI` | Behavior Tree nodes (Decorators, Services, Tasks) and AI interfaces. |
| `DataAsset` | Data definitions for skills and spawning configurations. |
| `GA` | Gameplay Ability implementations (`HGA_Attack`, etc.). |
| `Mode` | Game Mode and Player Controller classes. |
| `Skill` | Combat interfaces and Animation Notifies. |
| `System` | Core subsystems (Object Pooling, Monster Spawning, AI Controller). |
| `Unit` | Character base classes (`HBaseCharacter`), Player, and Monster definitions. |

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
