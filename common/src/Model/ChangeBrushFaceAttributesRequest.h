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

#ifndef TrenchBroom_ChangeBrushFaceAttributesRequest
#define TrenchBroom_ChangeBrushFaceAttributesRequest

#include "StringUtils.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Assets {
        class Texture;
    }

    namespace Model {
        class BrushFaceAttributes;
        
        class ChangeBrushFaceAttributesRequest {
        public:
            typedef enum {
                AxisOp_None,
                AxisOp_Reset,
                AxisOp_ToParaxial,
                AxisOp_ToParallel
            } AxisOp;
            
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
        private:
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
            AxisOp m_axisOp;
            ValueOp m_xOffsetOp;
            ValueOp m_yOffsetOp;
            ValueOp m_rotationOp;
            ValueOp m_xScaleOp;
            ValueOp m_yScaleOp;
            FlagOp m_surfaceFlagsOp;
            FlagOp m_contentFlagsOp;
            ValueOp m_surfaceValueOp;
        public:
            ChangeBrushFaceAttributesRequest();
            
            void clear();
            
            const String name() const;
            void evaluate(const BrushFaceList& faces) const;
            
            void setTexture(Assets::Texture* texture);
            
            void resetTextureAxes();
            void resetTextureAxesToParaxial();
            void resetTextureAxesToParallel();
            
            void setOffset(const Vec2f& offset);
            void addOffset(const Vec2f& offset);
            void mulOffset(const Vec2f& offset);
            
            void setXOffset(float xOffset);
            void addXOffset(float xOffset);
            void mulXOffset(float xOffset);
            
            void setYOffset(float yOffset);
            void addYOffset(float yOffset);
            void mulYOffset(float yOffset);
            
            void setRotation(float rotation);
            void addRotation(float rotation);
            void mulRotation(float rotation);
            
            void setScale(const Vec2f& scale);
            void addScale(const Vec2f& scale);
            void mulScale(const Vec2f& scale);
            
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
            
            void setAll(const Model::BrushFace* face);
            void setAll(const Model::BrushFaceAttributes& attributes);

            bool collateWith(ChangeBrushFaceAttributesRequest& other);
        };
    }
}

#endif /* defined(TrenchBroom_ChangeBrushFaceAttributesRequest) */
