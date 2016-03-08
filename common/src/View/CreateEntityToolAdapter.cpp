/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreateEntityToolAdapter.h"

#include "View/CreateEntityTool.h"
#include "View/InputState.h"

#include <cassert>

namespace TrenchBroom {
    namespace View {
        CreateEntityToolAdapter::CreateEntityToolAdapter(CreateEntityTool* tool) :
        m_tool(tool) {
            assert(m_tool != NULL);
        }
        
        CreateEntityToolAdapter::~CreateEntityToolAdapter() {}
        
        Tool* CreateEntityToolAdapter::doGetTool() {
            return m_tool;
        }
        
        bool CreateEntityToolAdapter::doDragEnter(const InputState& inputState, const String& payload) {
            const StringList parts = StringUtils::split(payload, ':');
            if (parts.size() != 2)
                return false;
            if (parts[0] != "entity")
                return false;
            
            if (m_tool->createEntity(parts[1])) {
                doUpdateEntityPosition(inputState);
                return true;
            }
            return false;
        }
        
        bool CreateEntityToolAdapter::doDragMove(const InputState& inputState) {
            doUpdateEntityPosition(inputState);
            return true;
        }
        
        void CreateEntityToolAdapter::doDragLeave(const InputState& inputState) {
            m_tool->removeEntity();
        }
        
        bool CreateEntityToolAdapter::doDragDrop(const InputState& inputState) {
            m_tool->commitEntity();
            return true;
        }
        
        bool CreateEntityToolAdapter::doCancel() {
            return false;
        }

        CreateEntityToolAdapter2D::CreateEntityToolAdapter2D(CreateEntityTool* tool) :
        CreateEntityToolAdapter(tool) {}
        
        void CreateEntityToolAdapter2D::doUpdateEntityPosition(const InputState& inputState) {
            m_tool->updateEntityPosition2D(inputState.pickRay());
        }
        
        CreateEntityToolAdapter3D::CreateEntityToolAdapter3D(CreateEntityTool* tool) :
        CreateEntityToolAdapter(tool) {}
        
        void CreateEntityToolAdapter3D::doUpdateEntityPosition(const InputState& inputState) {
            m_tool->updateEntityPosition3D(inputState.pickRay(), inputState.pickResult());
        }
    }
}
