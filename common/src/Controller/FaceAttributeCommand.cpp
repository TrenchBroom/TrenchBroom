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

#include "FaceAttributeCommand.h"

#include "View/MapDocument.h"

namespace TrenchBroom {
    namespace Controller {
        const Command::CommandType FaceAttributeCommand::Type = Command::freeType();

        FaceAttributeCommand::FaceAttributeCommand(View::MapDocumentWPtr document, const Model::BrushFaceList& faces) :
        Command(Type, "Change face attributes", true, true),
        m_document(document),
        m_faces(faces),
        m_texture(NULL),
        m_xOffset(0.0f),
        m_yOffset(0.0f),
        m_rotation(0.0f),
        m_xScale(0.0f),
        m_yScale(0.0f),
        m_surfaceFlags(0),
        m_contentFlags(0),
        m_surfaceValue(0.0f),
        m_setTexture(false),
        m_xOffsetOp(ValueOp_None),
        m_yOffsetOp(ValueOp_None),
        m_rotationOp(ValueOp_None),
        m_xScaleOp(ValueOp_None),
        m_yScaleOp(ValueOp_None),
        m_surfaceFlagsOp(FlagOp_None),
        m_contentFlagsOp(FlagOp_None),
        m_surfaceValueOp(ValueOp_None) {}
        
        void FaceAttributeCommand::setTexture(Assets::Texture* texture) {
            m_texture = texture;
            m_setTexture = true;
        }
        
        void FaceAttributeCommand::setXOffset(const float xOffset) {
            m_xOffset = xOffset;
            m_xOffsetOp = ValueOp_Set;
        }
        
        void FaceAttributeCommand::addXOffset(const float xOffset) {
            m_xOffset = xOffset;
            m_xOffsetOp = ValueOp_Add;
        }
        
        void FaceAttributeCommand::mulXOffset(const float xOffset) {
            m_xOffset = xOffset;
            m_xOffsetOp = ValueOp_Mul;
        }
        
        void FaceAttributeCommand::setYOffset(const float yOffset) {
            m_yOffset = yOffset;
            m_yOffsetOp = ValueOp_Set;
        }
        
        void FaceAttributeCommand::addYOffset(const float yOffset) {
            m_yOffset = yOffset;
            m_yOffsetOp = ValueOp_Add;
        }
        
        void FaceAttributeCommand::mulYOffset(const float yOffset) {
            m_yOffset = yOffset;
            m_yOffsetOp = ValueOp_Mul;
        }
        
        void FaceAttributeCommand::setRotation(const float rotation) {
            m_rotation = rotation;
            m_rotationOp = ValueOp_Set;
        }
        
        void FaceAttributeCommand::addRotation(const float rotation) {
            m_rotation = rotation;
            m_rotationOp = ValueOp_Add;
        }
        
        void FaceAttributeCommand::mulRotation(const float rotation) {
            m_rotation = rotation;
            m_rotationOp = ValueOp_Mul;
        }

        void FaceAttributeCommand::setXScale(const float xScale) {
            m_xScale = xScale;
            m_xScaleOp = ValueOp_Set;
        }
        
        void FaceAttributeCommand::addXScale(const float xScale) {
            m_xScale = xScale;
            m_xScaleOp = ValueOp_Add;
        }
        
        void FaceAttributeCommand::mulXScale(const float xScale) {
            m_xScale = xScale;
            m_xScaleOp = ValueOp_Mul;
        }
        
        void FaceAttributeCommand::setYScale(const float yScale) {
            m_yScale = yScale;
            m_yScaleOp = ValueOp_Set;
        }
        
        void FaceAttributeCommand::addYScale(const float yScale) {
            m_yScale = yScale;
            m_yScaleOp = ValueOp_Add;
        }
        
        void FaceAttributeCommand::mulYScale(const float yScale) {
            m_yScale = yScale;
            m_yScaleOp = ValueOp_Mul;
        }
        
        void FaceAttributeCommand::replaceSurfaceFlags(const int surfaceFlags) {
            m_surfaceFlags = surfaceFlags;
            m_surfaceFlagsOp = FlagOp_Replace;
        }
        
        void FaceAttributeCommand::setSurfaceFlag(const size_t surfaceFlag) {
            assert(surfaceFlag < sizeof(int) * 8);
            m_surfaceFlags = (1 << surfaceFlag);
            m_surfaceFlagsOp = FlagOp_Set;
        }
        
        void FaceAttributeCommand::unsetSurfaceFlag(const size_t surfaceFlag) {
            assert(surfaceFlag < sizeof(int) * 8);
            m_surfaceFlags = (1 << surfaceFlag);
            m_surfaceFlagsOp = FlagOp_Unset;
        }
        
        void FaceAttributeCommand::replaceContentFlags(const int contentFlags) {
            m_contentFlags = contentFlags;
            m_contentFlagsOp = FlagOp_Replace;
        }
        
        void FaceAttributeCommand::setContentFlag(const size_t contentFlag) {
            assert(contentFlag < sizeof(int) * 8);
            m_contentFlags = (1 << contentFlag);
            m_contentFlagsOp = FlagOp_Set;
        }
        
        void FaceAttributeCommand::unsetContentFlag(const size_t contentFlag) {
            assert(contentFlag < sizeof(int) * 8);
            m_contentFlags = (1 << contentFlag);
            m_contentFlagsOp = FlagOp_Unset;
        }
        
        void FaceAttributeCommand::setSurfaceValue(const float surfaceValue) {
            m_surfaceValue = surfaceValue;
            m_surfaceValueOp = ValueOp_Set;
        }
        
        void FaceAttributeCommand::addSurfaceValue(const float surfaceValue) {
            m_surfaceValue = surfaceValue;
            m_surfaceValueOp = ValueOp_Add;
        }
        
        void FaceAttributeCommand::mulSurfaceValue(const float surfaceValue) {
            m_surfaceValue = surfaceValue;
            m_surfaceValueOp = ValueOp_Mul;
        }

        void FaceAttributeCommand::setAll(const Model::BrushFace& original) {
            setTexture(original.texture());
            setXOffset(original.xOffset());
            setYOffset(original.yOffset());
            setRotation(original.rotation());
            setXScale(original.xScale());
            setYScale(original.yScale());
            replaceSurfaceFlags(original.surfaceFlags());
            replaceContentFlags(original.surfaceContents());
            setSurfaceValue(original.surfaceValue());
        }

        bool FaceAttributeCommand::doPerformDo() {
            View::MapDocumentSPtr document = lock(m_document);
            m_snapshot = Model::Snapshot(m_faces);
            
            Model::BrushFaceList::const_iterator it, end;
            for (it = m_faces.begin(), end = m_faces.end(); it != end; ++it) {
                Model::BrushFace* face = *it;
                document->faceWillChangeNotifier(face);
                if (m_setTexture)
                    face->setTexture(m_texture);
                face->setXOffset(evaluate(face->xOffset(), m_xOffset, m_xOffsetOp));
                face->setYOffset(evaluate(face->yOffset(), m_yOffset, m_yOffsetOp));
                face->setRotation(evaluate(face->rotation(), m_rotation, m_rotationOp));
                face->setXScale(evaluate(face->xScale(), m_xScale, m_xScaleOp));
                face->setYScale(evaluate(face->yScale(), m_yScale, m_yScaleOp));
                face->setSurfaceFlags(evaluate(face->surfaceFlags(), m_surfaceFlags, m_surfaceFlagsOp));
                face->setSurfaceContents(evaluate(face->surfaceContents(), m_contentFlags, m_contentFlagsOp));
                face->setSurfaceValue(evaluate(face->surfaceValue(), m_surfaceValue, m_surfaceValueOp));
                document->faceDidChangeNotifier(face);
            }
            return true;
        }
        
        bool FaceAttributeCommand::doPerformUndo() {
            View::MapDocumentSPtr document = lock(m_document);

            document->faceWillChangeNotifier(m_faces.begin(), m_faces.end());
            m_snapshot.restore(document->worldBounds());
            document->faceDidChangeNotifier(m_faces.begin(), m_faces.end());
            return true;
        }
    }
}
