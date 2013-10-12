/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "FaceAttributeCommand.h"

#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType FaceAttributeCommand::Type = Command::freeType();

        FaceAttributeCommand::FaceAttributeCommand(View::MapDocumentPtr document, const Model::BrushFaceList& faces) :
        Command(Type, "Change face attributes", true, true),
        m_document(document),
        m_faces(faces),
        m_texture(NULL),
        m_xOffset(0.0f),
        m_yOffset(0.0f),
        m_rotation(0.0f),
        m_xScale(0.0f),
        m_yScale(0.0f),
        m_setTexture(false),
        m_xOffsetOp(OpNone),
        m_yOffsetOp(OpNone),
        m_rotationOp(OpNone),
        m_xScaleOp(OpNone),
        m_yScaleOp(OpNone),
        m_setSurfaceContents(false),
        m_setSurfaceFlags(false),
        m_setSurfaceValue(false) {}
        
        void FaceAttributeCommand::setTexture(Assets::FaceTexture* texture) {
            m_texture = texture;
            m_setTexture = true;
        }
        
        void FaceAttributeCommand::setXOffset(const float xOffset) {
            m_xOffset = xOffset;
            m_xOffsetOp = OpSet;
        }
        
        void FaceAttributeCommand::addXOffset(const float xOffset) {
            m_xOffset = xOffset;
            m_xOffsetOp = OpAdd;
        }
        
        void FaceAttributeCommand::mulXOffset(const float xOffset) {
            m_xOffset = xOffset;
            m_xOffsetOp = OpMul;
        }
        
        void FaceAttributeCommand::setYOffset(const float yOffset) {
            m_yOffset = yOffset;
            m_yOffsetOp = OpSet;
        }
        
        void FaceAttributeCommand::addYOffset(const float yOffset) {
            m_yOffset = yOffset;
            m_yOffsetOp = OpAdd;
        }
        
        void FaceAttributeCommand::mulYOffset(const float yOffset) {
            m_yOffset = yOffset;
            m_yOffsetOp = OpMul;
        }
        
        void FaceAttributeCommand::setRotation(const float rotation) {
            m_rotation = rotation;
            m_rotationOp = OpSet;
        }
        
        void FaceAttributeCommand::addRotation(const float rotation) {
            m_rotation = rotation;
            m_rotationOp = OpAdd;
        }
        
        void FaceAttributeCommand::mulRotation(const float rotation) {
            m_rotation = rotation;
            m_rotationOp = OpMul;
        }

        void FaceAttributeCommand::setXScale(const float xScale) {
            m_xScale = xScale;
            m_xScaleOp = OpSet;
        }
        
        void FaceAttributeCommand::addXScale(const float xScale) {
            m_xScale = xScale;
            m_xScaleOp = OpAdd;
        }
        
        void FaceAttributeCommand::mulXScale(const float xScale) {
            m_xScale = xScale;
            m_xScaleOp = OpMul;
        }
        
        void FaceAttributeCommand::setYScale(const float yScale) {
            m_yScale = yScale;
            m_yScaleOp = OpSet;
        }
        
        void FaceAttributeCommand::addYScale(const float yScale) {
            m_yScale = yScale;
            m_yScaleOp = OpAdd;
        }
        
        void FaceAttributeCommand::mulYScale(const float yScale) {
            m_yScale = yScale;
            m_yScaleOp = OpMul;
        }
        
        void FaceAttributeCommand::setSurfaceContents(const size_t surfaceContents) {
            m_surfaceContents = surfaceContents;
            m_setSurfaceContents = true;
        }
        
        void FaceAttributeCommand::setSurfaceFlags(const size_t surfaceFlags) {
            m_surfaceFlags = surfaceFlags;
            m_setSurfaceFlags = true;
        }
        
        void FaceAttributeCommand::setSurfaceValue(const float surfaceValue) {
            m_surfaceValue = surfaceValue;
            m_setSurfaceValue = true;
        }

        void FaceAttributeCommand::setAll(const Model::BrushFace& original) {
            setTexture(original.texture());
            setXOffset(original.xOffset());
            setYOffset(original.yOffset());
            setRotation(original.rotation());
            setXScale(original.xScale());
            setYScale(original.yScale());
            setSurfaceContents(original.surfaceContents());
            setSurfaceFlags(original.surfaceFlags());
            setSurfaceValue(original.surfaceValue());
        }

        bool FaceAttributeCommand::doPerformDo() {
            m_snapshot = Model::Snapshot(m_faces);
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                m_document->faceWillChangeNotifier(face);
                if (m_setTexture)
                    face->setTexture(m_texture);
                face->setXOffset(evaluate(face->xOffset(), m_xOffset, m_xOffsetOp));
                face->setYOffset(evaluate(face->yOffset(), m_yOffset, m_yOffsetOp));
                face->setRotation(evaluate(face->rotation(), m_rotation, m_rotationOp));
                face->setXScale(evaluate(face->xScale(), m_xScale, m_xScaleOp));
                face->setYScale(evaluate(face->yScale(), m_yScale, m_yScaleOp));
                if (m_setSurfaceContents)
                    face->setSurfaceContents(m_surfaceContents);
                if (m_setSurfaceFlags)
                    face->setSurfaceFlags(m_surfaceFlags);
                if (m_setSurfaceValue)
                    face->setSurfaceValue(m_surfaceValue);
                m_document->faceDidChangeNotifier(face);
            }
            return true;
        }
        
        bool FaceAttributeCommand::doPerformUndo() {
            m_document->faceWillChangeNotifier(m_faces.begin(), m_faces.end());
            m_snapshot.restore(m_document->worldBounds());
            m_document->faceDidChangeNotifier(m_faces.begin(), m_faces.end());
            return true;
        }
    }
}
