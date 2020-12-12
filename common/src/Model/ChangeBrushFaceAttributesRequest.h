/*
 Copyright (C) 2010-2017 Kristian Duske

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

#pragma once

#include "Color.h"

#include <vecmath/forward.h>

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class BrushFaceHandle;
        class BrushFaceAttributes;

        class ChangeBrushFaceAttributesRequest {
        public:
            // TODO: replace with class based enum
            typedef enum {
                AxisOp_None,
                AxisOp_Reset,
                AxisOp_ToParaxial,
                AxisOp_ToParallel
            } AxisOp;

            // TODO: replace with class based enum
            typedef enum {
                ValueOp_None,
                ValueOp_Set,
                ValueOp_Add,
                ValueOp_Mul
            } ValueOp;

            // TODO: replace with class based enum
            typedef enum {
                FlagOp_None,
                FlagOp_Replace,
                FlagOp_Set,
                FlagOp_Unset
            } FlagOp;

            // TODO: replace with class based enum
            typedef enum {
                TextureOp_None,
                TextureOp_Set
            } TextureOp;
        private:
            std::string m_textureName;
            float m_xOffset;
            float m_yOffset;
            float m_rotation;
            float m_xScale;
            float m_yScale;
            int m_surfaceFlags;
            int m_contentFlags;
            float m_surfaceValue;
            Color m_colorValue;

            TextureOp m_textureOp;
            AxisOp m_axisOp;
            ValueOp m_xOffsetOp;
            ValueOp m_yOffsetOp;
            ValueOp m_rotationOp;
            ValueOp m_xScaleOp;
            ValueOp m_yScaleOp;
            FlagOp m_surfaceFlagsOp;
            FlagOp m_contentFlagsOp;
            ValueOp m_surfaceValueOp;
            ValueOp m_colorValueOp;
        public:
            ChangeBrushFaceAttributesRequest();

            void clear();

            const std::string name() const;
            bool evaluate(const std::vector<BrushFaceHandle>& faceHandles) const;
            bool evaluate(BrushFace& brushFace) const;

            void resetAll(const BrushFaceAttributes& defaultFaceAttributes);

            void setTextureName(const std::string& textureName);

            void resetTextureAxes();
            void resetTextureAxesToParaxial();
            void resetTextureAxesToParallel();

            void setOffset(const vm::vec2f& offset);
            void addOffset(const vm::vec2f& offset);
            void mulOffset(const vm::vec2f& offset);

            void setXOffset(float xOffset);
            void addXOffset(float xOffset);
            void mulXOffset(float xOffset);

            void setYOffset(float yOffset);
            void addYOffset(float yOffset);
            void mulYOffset(float yOffset);

            void setRotation(float rotation);
            void addRotation(float rotation);
            void mulRotation(float rotation);

            void setScale(const vm::vec2f& scale);
            void addScale(const vm::vec2f& scale);
            void mulScale(const vm::vec2f& scale);

            void setXScale(float xScale);
            void addXScale(float xScale);
            void mulXScale(float xScale);

            void setYScale(float yScale);
            void addYScale(float yScale);
            void mulYScale(float yScale);

            void setSurfaceFlags(int surfaceFlags);
            void unsetSurfaceFlags(int surfaceFlags);
            void replaceSurfaceFlags(int surfaceFlags);

            void setContentFlags(int contentFlags);
            void unsetContentFlags(int contentFlags);
            void replaceContentFlags(int contentFlags);

            void setSurfaceValue(float surfaceValue);
            void addSurfaceValue(float surfaceValue);
            void mulSurfaceValue(float surfaceValue);

            void setColor(const Color& colorValue);

            void setAll(const Model::BrushFace& face);
            void setAllExceptContentFlags(const Model::BrushFace& face);
            void setAll(const Model::BrushFaceAttributes& attributes);
            void setAllExceptContentFlags(const Model::BrushFaceAttributes& attributes);

            bool collateWith(ChangeBrushFaceAttributesRequest& other);
        };
    }
}

