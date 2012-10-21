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

#include "RotateTexturesCommand.h"

#include "Model/EditStateManager.h"
#include "Model/Face.h"

#include <cassert>

namespace TrenchBroom {
    namespace Controller {
        bool RotateTexturesCommand::performDo() {
            Model::FaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::Face& face = **it;
                face.rotateTexture(m_angle);
            }
            
            return true;
        }
        
        bool RotateTexturesCommand::performUndo() {
            Model::FaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::Face& face = **it;
                face.rotateTexture(-m_angle);
            }
            
            return true;
        }
        
        RotateTexturesCommand::RotateTexturesCommand(Model::MapDocument& document, const Model::FaceList& faces, const wxString& name, float angle) :
        DocumentCommand(RotateTextures, document, true, name),
        m_faces(faces),
        m_angle(angle) {}

        RotateTexturesCommand* RotateTexturesCommand::rotateClockwise(Model::MapDocument& document, const Model::FaceList& faces, float angle) {
            wxString name = faces.size() == 1 ? wxT("Rotate Texture") : wxT("Rotate Textures");
            return new RotateTexturesCommand(document, faces, name, angle);
        }

        RotateTexturesCommand* RotateTexturesCommand::rotateCounterClockwise(Model::MapDocument& document, const Model::FaceList& faces, float angle) {
            wxString name = faces.size() == 1 ? wxT("Rotate Texture") : wxT("Rotate Textures");
            return new RotateTexturesCommand(document, faces, name, -angle);
        }
    }
}