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

#include "MoveTexturesCommand.h"

#include "Model/EditStateManager.h"
#include "Model/Face.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool MoveTexturesCommand::performDo() {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::FaceList& faces = editStateManager.selectedFaces();
            assert(!faces.empty());
            
            Model::FaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::Face& face = **it;
                face.moveTexture(m_distance, m_direction);
            }
            
            return true;
        }
        
        bool MoveTexturesCommand::performUndo() {
            Model::EditStateManager& editStateManager = document().editStateManager();
            const Model::FaceList& faces = editStateManager.selectedFaces();
            assert(!faces.empty());
            
            Model::FaceList::const_iterator it, end;
            for (it = faces.begin(), end = faces.end(); it != end; ++it) {
                Model::Face& face = **it;
                face.moveTexture(-m_distance, m_direction);
            }
            
            return true;
        }
        
        MoveTexturesCommand::MoveTexturesCommand(Model::MapDocument& document, const wxString& name, float distance, const Vec3f& direction) :
        DocumentCommand(MoveTextures, document, true, name, true),
        m_distance(distance),
        m_direction(direction) {}

        MoveTexturesCommand* MoveTexturesCommand::moveTextures(Model::MapDocument& document, const wxString& name, float distance, const Vec3f& direction) {
            return new MoveTexturesCommand(document, name, distance, direction);
        }
    }
}