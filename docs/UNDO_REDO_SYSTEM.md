# Undo/Redo System Architecture

## Overview
The UI Engine implements a professional, persistent, and crash-safe Undo/Redo system based on the **Command Pattern**. This system ensures that all scene modifications (Creation, Deletion, Transformation, Renaming) are deterministic, reversible, and granular.

## core Architecture

### 1. Command Pattern
Every actionable change in the editor is encapsulated in a class derived from `ICommand`.
- **`Execute()`**: Performs the action (e.g., moving a mesh).
- **`Undo()`**: Reverts the action (e.g., moving it back).
- **`GetDescription()`**: Returns a string for logging (e.g., "Transform Cube").

### 2. Two-Stack History (`CommandHistory`)
We manage two distinct stacks to track state:
- **Undo Stack**: Stores commands that have been executed.
- **Redo Stack**: Stores commands that have been undone.

**Workflow:**
1.  **Execute**: Command Pushed to Undo Stack -> Redo Stack Cleared.
2.  **Undo**: Pop from Undo Stack -> Call `Undo()` -> Push to Redo Stack.
3.  **Redo**: Pop from Redo Stack -> Call `Execute()` -> Push to Undo Stack.

### 3. UUID System (Identity Persistence)
A critical component for crash safety is the **UUID (Universally Unique Identifier) System**.
- **Problem**: In standard ECS, entities are identified by `entt::entity` (integers). If you delete Entity #5 and create a new one, it might recycle ID #5. If you then "Undo" the deletion, the ECS might give you ID #6. Any previous commands referencing ID #5 are now invalid or pointing to the wrong object.
- **Solution**: We assign a distinct 64-bit random **UUID** to every entity upon creation.
- **Implementation**:
    - Commands store the **UUID**, not the raw entity handle.
    - When executing/undoing, the command queries `Scene::GetEntityByUUID(uuid)` to find the correct object.
    - **Result**: Even if the underlying ECS handle changes (e.g., after a delete/restore cycle), the logical entity remains the same, ensuring 100% reliable tracking.

## Detailed Command Implementations

### `ModifyTransformCommand`
Tracks granular movement, rotation, and scaling.
- **Granularity**: Every "Drag -> Release" action in the viewport creates a separate command.
- **State Capture**: Snapshots the *exact* `TransformComponent` (Position, Rotation, Scale) before and after the operation.
- **Logging**: Provides detailed vector logs (e.g., `Pos: (1.0, 2.0, 3.0)`) for verifying exact states.

### `DeleteEntityCommand`
Handles the complex task of removing and restoring entities.
- **Undo Logic**: NOT just a simple "Create". It must restore:
    1.  The Entity with the **ORIGINAL UUID**.
    2.  The `TagComponent` (Name) - critical for the Hierarchy panel.
    3.  The `TransformComponent` (Location).
    4.  The `MeshComponent` (Visuals).
- **Fixes**: Explicitly restores the `TagComponent` to ensure the restored object appears correctly in the Hierarchy view, solving "Ghost Object" bugs.

### `CreateMeshCommand`
- Generates a stable UUID essentially at the moment of command creation, ensuring that the "Created" entity is identical to the one "Redone" later.

## Performance & Optimization

### Memory Management
- **History Limiter**: The `CommandHistory` can be configured with a `MAX_HISTORY_SIZE` (default 500) to prevent infinite memory growth during long sessions.
- **Unique Pointers**: The stacks use `std::unique_ptr`, ensuring automated cleanup of command memory when the history is cleared or overwrites occur.

### Input Debouncing
- **Issue**: Standard `Input::IsKeyPressed` returns true every frame, causing `Ctrl+Z` to trigger ~60 undos per second.
- **Optimization**: Implemented **Key Debouncing** (`m_UndoPressedLastFrame`). The system only accepts the *rising edge* of the key press, guaranteeing exactly **one Undo step per key press**.

## Usage Guide for Developers

### Adding a New Command
1.  Create a class inheriting from `ICommand`.
2.  Implement `Execute()` (apply change) and `Undo()` (revert change).
3.  **Crucial**: Store the target Entity's **UUID**, not its pointer/ID.
4.  In the Editor, create the command and pass it to `m_CommandHistory.ExecuteCommand(std::move(cmd))`.

### API Example
```cpp
// Moving an object programmatically via Undo System
auto cmd = std::make_unique<ModifyTransformCommand>(
    scene,
    targetEntity,
    oldTransform,
    newTransform
);
m_CommandHistory.ExecuteCommand(std::move(cmd));
```

## System Requirements
- **C++17** Standard
- **EnTT** (ECS)
- **GLM** (Math)
- **ImGui** (Editor Interface)
