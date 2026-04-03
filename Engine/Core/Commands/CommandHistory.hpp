#pragma once

#include "Command.hpp"
#include <deque>
#include <memory>
#include <iostream>

/**
 * Professional Undo/Redo Manager
 * Uses bounded deque architecture:
 * - UndoStack: Stores executed commands (past) — bounded to MAX_HISTORY_SIZE
 * - RedoStack: Stores undone commands (future)
 * 
 * CRIT-01 FIX: Uses std::deque to enforce MAX_HISTORY_SIZE by evicting oldest commands.
 */
class CommandHistory
{
public:
    CommandHistory() = default;
    ~CommandHistory() = default;

    /**
     * Hard limit to prevent infinite memory growth.
     * Professional engines usually cap this (e.g., 256 or 1000).
     */
    static constexpr size_t MAX_HISTORY_SIZE = 500;

    /**
     * Execute a new command and push to Undo stack.
     * CRITICAL: Clears Redo stack to maintain deterministic history.
     */
    void ExecuteCommand(std::unique_ptr<ICommand> command)
    {
        if (!command) return;

        // 1. Execute
        command->Execute();

        // 2. Push to Undo Stack
        m_UndoStack.push_back(std::move(command));

        // 3. Clear Redo Stack (History divergence)
        m_RedoStack.clear();

        // 4. CRIT-01 FIX: Enforce max size — evict oldest commands
        while (m_UndoStack.size() > MAX_HISTORY_SIZE)
        {
            m_UndoStack.pop_front();
        }
    }

    /**
     * Undo the last operation.
     * Moves command from Undo Stack -> Redo Stack.
     */
    void Undo()
    {
        if (m_UndoStack.empty()) return;

        // Pop from Undo
        auto cmd = std::move(m_UndoStack.back());
        m_UndoStack.pop_back();

        // Undo action
        cmd->Undo();

        // Push to Redo
        m_RedoStack.push_back(std::move(cmd));
    }

    /**
     * Redo the previously undone operation.
     * Moves command from Redo Stack -> Undo Stack.
     */
    void Redo()
    {
        if (m_RedoStack.empty()) return;

        // Pop from Redo
        auto cmd = std::move(m_RedoStack.back());
        m_RedoStack.pop_back();

        // Execute action
        cmd->Execute();

        // Push to Undo
        m_UndoStack.push_back(std::move(cmd));

        // Enforce size again after redo
        while (m_UndoStack.size() > MAX_HISTORY_SIZE)
        {
            m_UndoStack.pop_front();
        }
    }

    bool CanUndo() const { return !m_UndoStack.empty(); }
    bool CanRedo() const { return !m_RedoStack.empty(); }

    void Clear()
    {
        m_UndoStack.clear();
        m_RedoStack.clear();
    }

private:
    std::deque<std::unique_ptr<ICommand>> m_UndoStack;
    std::deque<std::unique_ptr<ICommand>> m_RedoStack;
};
