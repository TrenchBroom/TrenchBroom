/*
 Copyright (C) 2010-2012 Kristian Duske
 
 This file is part of TrenchBroom.
 
 TrenchBroom is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 TrenchBroom is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UndoManager.h"

#include <cassert>
#include "Model/Undo/SnapshotUndoItem.h"

namespace TrenchBroom {
    namespace Model {
        UndoGroup::UndoGroup(const std::string name) : m_name(name) {
        }
        
        UndoGroup::~UndoGroup() {
            while (!m_items.empty()) delete m_items.back(), m_items.pop_back();
        }
        
        void UndoGroup::addItem(UndoItem* item) {
            m_items.push_back(item);
        }

        void UndoGroup::undo() {
            for (std::vector<UndoItem*>::reverse_iterator it = m_items.rbegin(); it != m_items.rend(); ++it)
                (*it)->undo();
        }

        const std::string& UndoGroup::name() {
            return m_name;
        }
        
        bool UndoGroup::empty() const {
            return m_items.empty();
        }

        UndoManager::UndoManager() : m_depth(0), m_currentGroup(NULL), m_state(TB_US_DEFAULT) {
        }
        
        UndoManager::~UndoManager() {
            clear();
        }

        void UndoManager::clear() {
            if (m_currentGroup != NULL) {
                delete m_currentGroup;
                m_currentGroup = NULL;
            }

            while (!m_undoStack.empty()) delete m_undoStack.back(), m_undoStack.pop_back();
            while (!m_redoStack.empty()) delete m_redoStack.back(), m_redoStack.pop_back();
        }
        
        void UndoManager::undo() {
            assert(m_depth == 0);
            if (m_undoStack.empty())
                return;
            
            UndoGroup* group = m_undoStack.back();
            m_undoStack.pop_back();

            m_state = TB_US_UNDO;
            begin(group->name());
            group->undo();
            end();
            m_state = TB_US_DEFAULT;

            undoPerformed(*group);
            delete group;
        }
        
        void UndoManager::redo() {
            assert(m_depth == 0);
            if (m_redoStack.empty())
                return;

            UndoGroup* group = m_redoStack.back();
            m_redoStack.pop_back();
            
            m_state = TB_US_REDO;
            begin(group->name());
            group->undo();
            end();
            m_state = TB_US_DEFAULT;
            
            redoPerformed(*group);
            delete group;
        }
        
        void UndoManager::begin(const std::string& name) {
            if (m_currentGroup == NULL)
                m_currentGroup = new UndoGroup(name);
            m_depth++;
        }
        
        void UndoManager::addItem(UndoItem* item) {
            assert(m_currentGroup != NULL);
            m_currentGroup->addItem(item);
        }

        void UndoManager::addSnapshot(Map& map) {
            assert(m_currentGroup != NULL);
            SnapshotUndoItem* snapshotItem = new SnapshotUndoItem(map);
            m_currentGroup->addItem(snapshotItem);
        }
        
        void UndoManager::end() {
            assert(m_currentGroup != NULL);
            
            m_depth--;
            if (m_depth == 0) {
                if (!m_currentGroup->empty()) {
                    if (m_state == TB_US_UNDO)
                        m_redoStack.push_back(m_currentGroup);
                    else
                        m_undoStack.push_back(m_currentGroup);
                    if (m_state == TB_US_DEFAULT)
                        undoGroupCreated(*m_currentGroup);
                } else {
                    delete m_currentGroup;
                }
                m_currentGroup = NULL;
            }
        }

        bool UndoManager::undoStackEmpty() {
            return m_undoStack.empty();
        }
        
        bool UndoManager::redoStackEmpty() {
            return m_redoStack.empty();
        }

        const std::string& UndoManager::topUndoName() {
            assert(!undoStackEmpty());
            return m_undoStack.back()->name();
        }
        
        const std::string& UndoManager::topRedoName() {
            assert(!redoStackEmpty());
            return m_redoStack.back()->name();
        }
    }
}
