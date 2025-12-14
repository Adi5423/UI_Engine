# Command System (Undo/Redo)

## Overview
Professional undo/redo system implemented using the Command Pattern, similar to Unity, Unreal Engine, Godot, and Hazel.

## Features
- ✅ **Full Undo/Redo Support** - All editor operations are undoable
- ✅ **Professional Implementation** - Command Pattern used in AAA engines
- ✅ **Keyboard Shortcuts** - Ctrl+Z (Undo), Ctrl+Y (Redo)
- ✅ **Console Logging** - All commands are logged to console

## Supported Operations

### Scene Operations
- **Create Entity** - Creating cubes or other meshes
- **Delete Entity** - Deleting objects from hierarchy
- **Transform Modification** - Position, Rotation, Scale changes via:
  - Inspector panel drag controls
  - ImGuizmo manipulation (translate/rotate/scale)
- **Rename Entity** - Changing entity names

### Excluded from Undo/Redo
- Camera movement and rotation (as requested)
- Viewport navigation
- UI interactions

## Usage

### Keyboard Shortcuts
- `Ctrl + Z` - Undo last action
- `Ctrl + Y` - Redo last undone action
- `Q/W/E/R` - Gizmo mode shortcuts (None/Translate/Rotate/Scale)

### Creating New Commands

To create a new command, inherit from `ICommand`:

```cpp
class MyCustomCommand : public ICommand
{
public:
    MyCustomCommand(/* parameters */)
        : /* initialization */
    {
    }

    void Execute() override
    {
        // Perform the action
    }

    void Undo() override
    {
        // Revert the action
    }

    std::string GetDescription() const override
    {
        return "My Custom Action";
    }

private:
    // Store data needed for undo
};
```

### Executing Commands

```cpp
// Create command
auto cmd = std::make_unique<MyCustomCommand>(/* params */);

// Execute and add to history
m_CommandHistory.ExecuteCommand(std::move(cmd));
```

## Architecture

### Command Pattern
```
ICommand (Interface)
    ├── Execute()
    ├── Undo()
    └── GetDescription()

CommandHistory
    ├── ExecuteCommand()
    ├── Undo()
    ├── Redo()
    ├── CanUndo()
    └── CanRedo()
```

### Concrete Commands
- `CreateMeshCommand` - Entity creation
- `DeleteEntityCommand` - Entity deletion (stores all component data)
- `ModifyTransformCommand` - Transform changes
- `RenameEntityCommand` - Name changes

## Implementation Details

### Transform Change Detection
Transform changes are detected at three points:
1. **Inspector Panel** - DragFloat3 widgets for Position/Rotation/Scale
2. **ImGuizmo** - 3D gizmo manipulation
3. Both track when editing starts and ends to create a single undo command

### Delete Command
The delete command stores:
- TagComponent
- TransformComponent
- MeshComponent

This allows perfect restoration when undoing a delete.

### Command Logging
All commands are logged to console:
```
[Command] Executed: Create Cube
[Command] Undo: Create Cube
[Command] Redo: Create Cube
```

## How It Works Like Professional Engines

### Unity-Style
- Undo/Redo for all editor operations
- Keyboard shortcuts (Ctrl+Z/Ctrl+Y)
- Command execution immediately updates the scene
- History stack management

### Unreal-Style
- Transaction system (our command pattern)
- Stores before/after state
- Multi-step undo support

### Godot-Style
- Action-based undo/redo
- Non-destructive editing
- Command history

## Future Enhancements

Potential improvements (not implemented):
- Undo history UI panel showing all commands
- Undo/Redo menu items
- Transaction grouping (multiple commands as one)
- Persistent undo history across sessions
- Memory management for large undo stacks
