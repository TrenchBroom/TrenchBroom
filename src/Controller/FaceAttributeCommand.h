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

#ifndef __TrenchBroom__FaceAttributeCommand__
#define __TrenchBroom__FaceAttributeCommand__

#include "Macros.h"
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
                ValueOp_None,
                ValueOp_Set,
                ValueOp_Add,
                ValueOp_Mul
            } ValueOp;

            typedef enum {
                FlagOp_None,
                FlagOp_Replace,
                FlagOp_Set,
                FlagOp_Unset
            } FlagOp;
            
            View::MapDocumentWPtr m_document;
            const Model::BrushFaceList m_faces;
            Model::Snapshot m_snapshot;

            Assets::Texture* m_texture;
            float m_xOffset;
            float m_yOffset;
            float m_rotation;
            float m_xScale;
            float m_yScale;
            int m_surfaceFlags;
            int m_contentFlags;
            float m_surfaceValue;
            
            bool m_setTexture;
            ValueOp m_xOffsetOp;
            ValueOp m_yOffsetOp;
            ValueOp m_rotationOp;
            ValueOp m_xScaleOp;
            ValueOp m_yScaleOp;
            FlagOp m_surfaceFlagsOp;
            FlagOp m_contentFlagsOp;
            ValueOp m_surfaceValueOp;
        public:
            FaceAttributeCommand(View::MapDocumentWPtr document, const Model::BrushFaceList& faces);
            
            void setTexture(Assets::Texture* texture);

            void setXOffset(float xOffset);
            void addXOffset(float xOffset);
            void mulXOffset(float xOffset);
            
            void setYOffset(float yOffset);
            void addYOffset(float yOffset);
            void mulYOffset(float yOffset);
            
            void setRotation(float rotation);
            void addRotation(float rotation);
            void mulRotation(float rotation);
            
            void setXScale(float xScale);
            void addXScale(float xScale);
            void mulXScale(float xScale);
            
            void setYScale(float yScale);
            void addYScale(float yScale);
            void mulYScale(float yScale);
            
            void replaceSurfaceFlags(int surfaceFlags);
            void setSurfaceFlag(size_t surfaceFlag);
            void unsetSurfaceFlag(size_t surfaceFlag);
            
            void replaceContentFlags(int contentFlags);
            void setContentFlag(size_t contentFlag);
            void unsetContentFlag(size_t contentFlag);
            
            void setSurfaceValue(float surfaceValue);
            void addSurfaceValue(float surfaceValue);
            void mulSurfaceValue(float surfaceValue);
            
            void setAll(const Model::BrushFace& original);

            bool doPerformDo();
            bool doPerformUndo();
            
            template <typename T>
            T evaluate(const T oldValue, const T newValue, const ValueOp op) const {
                switch (op) {
                    case ValueOp_Set:
                        return newValue;
                    case ValueOp_Add:
                        return oldValue + newValue;
                    case ValueOp_Mul:
                        return oldValue * newValue;
                    case ValueOp_None:
                        return oldValue;
                    DEFAULT_SWITCH()
                }
            }
            
            template <typename T>
            T evaluate(const T oldValue, const T newValue, const FlagOp op) const {
                switch (op) {
                    case FlagOp_Replace:
                        return newValue;
                    case FlagOp_Set:
                        return oldValue | newValue;
                    case FlagOp_Unset:
                        return oldValue & ~newValue;
                    case FlagOp_None:
                        return oldValue;
                    DEFAULT_SWITCH()
                }
            }
        };
    }
}

#endif /* defined(__TrenchBroom__FaceAttributeCommand__) */
