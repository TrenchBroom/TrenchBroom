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

#ifndef __TrenchBroom__FaceAttributeCommand__
#define __TrenchBroom__FaceAttributeCommand__

#include "SharedPointer.h"
#include "StringUtils.h"
#include "Controller/Command.h"
#include "Model/ModelTypes.h"
#include "Model/Snapshot.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }
    
    namespace Controller {
        class FaceAttributeCommand : public Command {
        public:
            static const CommandType Type;
            typedef std::tr1::shared_ptr<FaceAttributeCommand> Ptr;
        private:
            typedef enum {
                OpNone,
                OpSet,
                OpAdd,
                OpMul
            } Operation;

            View::MapDocumentPtr m_document;
            const Model::BrushFaceList m_faces;
            Model::Snapshot m_snapshot;

            Assets::Texture* m_texture;
            float m_xOffset;
            float m_yOffset;
            float m_rotation;
            float m_xScale;
            float m_yScale;
            size_t m_surfaceContents;
            size_t m_surfaceFlags;
            float m_surfaceValue;
            
            bool m_setTexture;
            Operation m_xOffsetOp;
            Operation m_yOffsetOp;
            Operation m_rotationOp;
            Operation m_xScaleOp;
            Operation m_yScaleOp;
            bool m_setSurfaceContents;
            bool m_setSurfaceFlags;
            bool m_setSurfaceValue;
        public:
            FaceAttributeCommand(View::MapDocumentPtr document, const Model::BrushFaceList& faces);
            
            void setTexture(Assets::Texture* texture);
            void setXOffset(const float xOffset);
            void addXOffset(const float xOffset);
            void mulXOffset(const float xOffset);
            void setYOffset(const float yOffset);
            void addYOffset(const float yOffset);
            void mulYOffset(const float yOffset);
            void setRotation(const float rotation);
            void addRotation(const float rotation);
            void mulRotation(const float rotation);
            void setXScale(const float xScale);
            void addXScale(const float xScale);
            void mulXScale(const float xScale);
            void setYScale(const float yScale);
            void addYScale(const float yScale);
            void mulYScale(const float yScale);
            void setSurfaceContents(const size_t surfaceContents);
            void setSurfaceFlags(const size_t surfaceFlags);
            void setSurfaceValue(const float surfaceValue);
            void setAll(const Model::BrushFace& original);

            bool doPerformDo();
            bool doPerformUndo();
            
            template <typename T>
            T evaluate(const T oldValue, const T newValue, const Operation op) const {
                switch (op) {
                    case OpSet:
                        return newValue;
                    case OpAdd:
                        return oldValue + newValue;
                    case OpMul:
                        return oldValue * newValue;
                    default:
                        return oldValue;
                }
            }
        };
    }
}

#endif /* defined(__TrenchBroom__FaceAttributeCommand__) */
