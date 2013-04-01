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

#ifndef __TrenchBroom__SetFaceAttributesCommand__
#define __TrenchBroom__SetFaceAttributesCommand__

#include "Controller/SnapshotCommand.h"
#include "Model/FaceTypes.h"

namespace TrenchBroom {
    namespace Model {
        class Face;
        class MapDocument;
        class Texture;
    }
    
    namespace Controller {
        class SetFaceAttributesCommand : public SnapshotCommand {
        protected:
            typedef enum {
                OpNone,
                OpSet,
                OpAdd,
                OpMul
            } Operation;
            
            Model::FaceList m_faces;
            
            float m_xOffset;
            float m_yOffset;
            float m_xScale;
            float m_yScale;
            float m_rotation;
            Model::Texture* m_texture;
            Model::Texture* m_previousMruTexture;
            
            Operation m_xOffsetOp;
            Operation m_yOffsetOp;
            Operation m_xScaleOp;
            Operation m_yScaleOp;
            Operation m_rotationOp;
            bool m_setTexture;

            bool performDo();
            bool performUndo();

        public:
            SetFaceAttributesCommand(Model::MapDocument& document, const Model::FaceList& faces, const wxString& name);

            inline void setXOffset(float xOffset) {
                m_xOffset = xOffset;
                m_xOffsetOp = OpSet;
            }
            
            inline void addXOffset(float xOffset) {
                m_xOffset = xOffset;
                m_xOffsetOp = OpAdd;
            }
            
            inline void mulXOffset(float xOffset) {
                m_xOffset = xOffset;
                m_xOffsetOp = OpMul;
            }
            
            inline void setYOffset(float yOffset) {
                m_yOffset = yOffset;
                m_yOffsetOp = OpSet;
            }
            
            inline void addYOffset(float yOffset) {
                m_yOffset = yOffset;
                m_yOffsetOp = OpAdd;
            }
            
            inline void mulYOffset(float yOffset) {
                m_yOffset = yOffset;
                m_yOffsetOp = OpMul;
            }
            
            inline void setXScale(float xScale) {
                m_xScale = xScale;
                m_xScaleOp = OpSet;
            }
            
            inline void addXScale(float xScale) {
                m_xScale = xScale;
                m_xScaleOp = OpAdd;
            }
            
            inline void mulXScale(float xScale) {
                m_xScale = xScale;
                m_xScaleOp = OpMul;
            }
            
            inline void setYScale(float yScale) {
                m_yScale = yScale;
                m_yScaleOp = OpSet;
            }
            
            inline void addYScale(float yScale) {
                m_yScale = yScale;
                m_yScaleOp = OpAdd;
            }
            
            inline void mulYScale(float yScale) {
                m_yScale = yScale;
                m_yScaleOp = OpMul;
            }

            inline void setRotation(float rotation) {
                m_rotation = rotation;
                m_rotationOp = OpSet;
            }

            inline void addRotation(float rotation) {
                m_rotation = rotation;
                m_rotationOp = OpAdd;
            }
            
            inline void mulRotation(float rotation) {
                m_rotation = rotation;
                m_rotationOp = OpMul;
            }
            
            inline void setTexture(Model::Texture* texture) {
                m_texture = texture;
                m_setTexture = true;
            }
            
            void setTemplate(const Model::Face& face);
        };
    }
}

#endif /* defined(__TrenchBroom__SetFaceAttributesCommand__) */
