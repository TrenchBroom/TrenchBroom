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
                switch (m_xOffsetOp) {
                    case OpSet:
                        face.setXOffset(m_xOffset);
                        break;
                    case OpAdd:
                        face.setXOffset(face.xOffset() + m_xOffset);
                        break;
                    case OpMul:
                        face.setXOffset(face.xOffset() * m_xOffset);
                        break;
                    default:
                        break;
                }
                switch (m_yOffsetOp) {
                    case OpSet:
                        face.setYOffset(m_yOffset);
                        break;
                    case OpAdd:
                        face.setYOffset(face.yOffset() + m_yOffset);
                        break;
                    case OpMul:
                        face.setYOffset(face.yOffset() * m_yOffset);
                        break;
                    default:
                        break;
                }
                switch (m_xScaleOp) {
                    case OpSet:
                        face.setXScale(m_xScale);
                        break;
                    case OpAdd:
                        face.setXScale(face.xScale() + m_xScale);
                        break;
                    case OpMul:
                        face.setXScale(face.xScale() * m_xScale);
                        break;
                    default:
                        break;
                }
                switch (m_yScaleOp) {
                    case OpSet:
                        face.setYScale(m_yScale);
                        break;
                    case OpAdd:
                        face.setYScale(face.yScale() + m_yScale);
                        break;
                    case OpMul:
                        face.setYScale(face.yScale() * m_yScale);
                        break;
                    default:
                        break;
                }
                switch (m_rotationOp) {
                    case OpSet:
                        face.setRotation(m_rotation);
                        break;
                    case OpAdd:
                        face.setRotation(face.rotation() + m_rotation);
                        break;
                    case OpMul:
                        face.setRotation(face.rotation() * m_rotation);
                        break;
                    default:
                        break;
                }
                if (m_setTexture)
                    face.setTexture(m_texture);
            }
            
            if (m_setTexture) {
                m_previousMruTexture = document().mruTexture();
                document().setMruTexture(m_texture);
            }
            
            return true;
        }
        
        bool SetFaceAttributesCommand::performUndo() {
            restoreSnapshots(m_faces);
            clear();
            
            if (m_setTexture)
                document().setMruTexture(m_previousMruTexture);
            
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
        m_previousMruTexture(NULL),
        m_xOffsetOp(OpNone),
        m_yOffsetOp(OpNone),
        m_xScaleOp(OpNone),
        m_yScaleOp(OpNone),
        m_rotationOp(OpNone),
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
