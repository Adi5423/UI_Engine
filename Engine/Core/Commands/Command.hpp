#pragma once

#include <memory>
#include <string>

/**
 * Base interface for all commands in the engine.
 * Implements the Command Pattern used in Unity, Unreal, Godot.
 */
class ICommand
{
public:
    virtual ~ICommand() = default;

    /**
     * Execute the command (perform the action)
     */
    virtual void Execute() = 0;

    /**
     * Undo the command (revert the action)
     */
    virtual void Undo() = 0;

    /**
     * Get a description of the command (for logging)
     */
    virtual std::string GetDescription() const = 0;
};
