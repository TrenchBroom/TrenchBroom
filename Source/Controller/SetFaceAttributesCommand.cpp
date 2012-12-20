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

#include "SetFaceAttributesCommand.h"

#include "Model/EditStateManager.h"
#include "Model/Face.h"
#include "Model/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        bool SetFaceAttributesCommand::performDo() {
            makeSnapshots(m_faces);
            
            Model::FaceList::const_iterator faceIt, faceEnd;
            for (faceIt = m_faces.begin(), faceEnd = m_faces.end(); faceIt != faceEnd; ++faceIt) {
                Model::Face& face = **faceIt;
                if (m_setXOffset)
                    face.setXOffset(m_xOffset);
                if (m_setYOffset)
                    face.setYOffset(m_yOffset);
                if (m_setXScale)
                    face.setXScale(m_xScale);
                if (m_setYScale)
                    face.setYScale(m_yScale);
                if (m_setRotation)
                    face.setRotation(m_rotation);
                if (m_setTexture)
                    face.setTexture(m_texture);
            }
            
            return true;
        }
        
        bool SetFaceAttributesCommand::performUndo() {
            restoreSnapshots(m_faces);
            clear();
            
            return true;
        }

        SetFaceAttributesCommand::SetFaceAttributesCommand(Model::MapDocument& document, const Model::FaceList& faces, const wxString& name) :
        SnapshotCommand(Command::SetFaceAttributes, document, name),
        m_faces(faces),
        m_xOffset(0.0f),
        m_yOffset(0.0f),
        m_xScale(0.0f),
        m_yScale(0.0f),
        m_rotation(0.0f),
        m_texture(NULL),
        m_setXOffset(false),
        m_setYOffset(false),
        m_setXScale(false),
        m_setYScale(false),
        m_setRotation(false),
        m_setTexture(false) {}

        void SetFaceAttributesCommand::setTemplate(const Model::Face& face) {
            setXOffset(face.xOffset());
            setYOffset(face.yOffset());
            setXScale(face.xScale());
            setYScale(face.yScale());
            setRotation(face.rotation());
            setTexture(face.texture());
        }
    }
}
