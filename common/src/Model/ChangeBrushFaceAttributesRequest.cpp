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

#include "ChangeBrushFaceAttributesRequest.h"

#include "Macros.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/BrushFaceHandle.h"
#include "Model/BrushNode.h"

#include <cassert>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        template <typename T>
        static T evaluateValueOp(const T oldValue, const T newValue, const ChangeBrushFaceAttributesRequest::ValueOp op) {
            switch (op) {
                case ChangeBrushFaceAttributesRequest::ValueOp_Set:
                    return newValue;
                case ChangeBrushFaceAttributesRequest::ValueOp_Add:
                    return oldValue + newValue;
                case ChangeBrushFaceAttributesRequest::ValueOp_Mul:
                    return oldValue * newValue;
                case ChangeBrushFaceAttributesRequest::ValueOp_None:
                    return oldValue;
                    switchDefault()
            }
        }

        template <typename T>
        static T valueOrZero(const std::optional<T>& option) {
            if (option) {
                return *option;
            }
            return static_cast<T>(0);
        }

        template <typename T>
        static std::optional<T> evaluateValueOp(const std::optional<T>& oldValue, const std::optional<T>& newValue, const ChangeBrushFaceAttributesRequest::ValueOp op) {
            switch (op) {
                case ChangeBrushFaceAttributesRequest::ValueOp_Set:
                    return newValue;
                case ChangeBrushFaceAttributesRequest::ValueOp_Add:
                    return valueOrZero(oldValue) + valueOrZero(newValue);
                case ChangeBrushFaceAttributesRequest::ValueOp_Mul:
                    return valueOrZero(oldValue) * valueOrZero(newValue);
                case ChangeBrushFaceAttributesRequest::ValueOp_None:
                    return oldValue;
                    switchDefault()
            }
        }

        template <typename T>
        static T evaluateFlagOp(const T oldValue, const T newValue, const ChangeBrushFaceAttributesRequest::FlagOp op) {
            switch (op) {
                case ChangeBrushFaceAttributesRequest::FlagOp_Replace:
                    return newValue;
                case ChangeBrushFaceAttributesRequest::FlagOp_Set:
                    return oldValue | newValue;
                case ChangeBrushFaceAttributesRequest::FlagOp_Unset:
                    return oldValue & ~newValue;
                case ChangeBrushFaceAttributesRequest::FlagOp_None:
                    return oldValue;
                    switchDefault()
            }
        }

        template <typename T>
        static std::optional<T> evaluateFlagOp(const std::optional<T>& oldValue, const std::optional<T>& newValue, const ChangeBrushFaceAttributesRequest::FlagOp op) {
            switch (op) {
                case ChangeBrushFaceAttributesRequest::FlagOp_Replace:
                    return newValue;
                case ChangeBrushFaceAttributesRequest::FlagOp_Set:
                    return valueOrZero(oldValue) | valueOrZero(newValue);
                case ChangeBrushFaceAttributesRequest::FlagOp_Unset:
                    return valueOrZero(oldValue) & ~valueOrZero(newValue);
                case ChangeBrushFaceAttributesRequest::FlagOp_None:
                    return oldValue;
                    switchDefault()
            }
        }

        ChangeBrushFaceAttributesRequest::ChangeBrushFaceAttributesRequest() :
        m_xOffset(0.0f),
        m_yOffset(0.0f),
        m_rotation(0.0f),
        m_xScale(0.0f),
        m_yScale(0.0f),
        m_surfaceFlags(0),
        m_contentFlags(0),
        m_surfaceValue(0.0f),
        m_textureOp(TextureOp_None),
        m_axisOp(AxisOp_None),
        m_xOffsetOp(ValueOp_None),
        m_yOffsetOp(ValueOp_None),
        m_rotationOp(ValueOp_None),
        m_xScaleOp(ValueOp_None),
        m_yScaleOp(ValueOp_None),
        m_surfaceFlagsOp(FlagOp_None),
        m_contentFlagsOp(FlagOp_None),
        m_surfaceValueOp(ValueOp_None),
        m_colorValueOp(ValueOp_None) {}

        void ChangeBrushFaceAttributesRequest::clear() {
            m_textureName = "";
            m_xOffset = m_yOffset = 0.0f;
            m_rotation = 0.0f;
            m_xScale = m_yScale = 1.0f;
            m_surfaceFlags = m_contentFlags = 0;
            m_surfaceValue = 0.0f;
            m_textureOp = TextureOp_None;
            m_axisOp = AxisOp_None;
            m_xOffsetOp = m_yOffsetOp = ValueOp_None;
            m_rotationOp = ValueOp_None;
            m_xScaleOp = m_yScaleOp = ValueOp_None;
            m_surfaceFlagsOp = m_contentFlagsOp = FlagOp_None;
            m_surfaceValueOp = ValueOp_None;
            m_colorValueOp = ValueOp_None;
        }

        const std::string ChangeBrushFaceAttributesRequest::name() const {
            return "Change Face Attributes";
        }

        bool ChangeBrushFaceAttributesRequest::evaluate(BrushFace& brushFace) const {
            auto result = false;

            BrushFaceAttributes attributes = brushFace.attributes();
            
            switch (m_textureOp) {
                case TextureOp_Set:
                    result |= attributes.setTextureName(m_textureName);
                    break;
                case TextureOp_None:
                    break;
                switchDefault();
            }

            result |= attributes.setXOffset(evaluateValueOp(attributes.xOffset(), m_xOffset, m_xOffsetOp));
            result |= attributes.setYOffset(evaluateValueOp(attributes.yOffset(), m_yOffset, m_yOffsetOp));
            result |= attributes.setRotation(evaluateValueOp(attributes.rotation(), m_rotation, m_rotationOp));
            result |= attributes.setXScale(evaluateValueOp(attributes.xScale(), m_xScale, m_xScaleOp));
            result |= attributes.setYScale(evaluateValueOp(attributes.yScale(), m_yScale, m_yScaleOp));
            result |= attributes.setSurfaceFlags(evaluateFlagOp(attributes.surfaceFlags(), m_surfaceFlags, m_surfaceFlagsOp));
            result |= attributes.setSurfaceContents(evaluateFlagOp(attributes.surfaceContents(), m_contentFlags, m_contentFlagsOp));
            result |= attributes.setSurfaceValue(evaluateValueOp(attributes.surfaceValue(), m_surfaceValue, m_surfaceValueOp));
            result |= attributes.setColor(evaluateValueOp(attributes.color(), m_colorValue, m_colorValueOp));

            brushFace.setAttributes(attributes);
            
            switch (m_axisOp) {
                case AxisOp_Reset:
                    brushFace.resetTextureAxes();
                    result |= true;
                    break;
                case AxisOp_None:
                    break;
                case AxisOp_ToParaxial:
                    brushFace.resetTextureAxesToParaxial();
                    result |= true;
                    break;
                case AxisOp_ToParallel:
                    break;
                switchDefault()
            }

            return result;
        }

        void ChangeBrushFaceAttributesRequest::resetAll(const BrushFaceAttributes& defaultFaceAttributes) {
            resetTextureAxes();
            setOffset(vm::vec2f::zero());
            setRotation(0.0f);
            setScale(defaultFaceAttributes.scale());
        }

        void ChangeBrushFaceAttributesRequest::resetAllToParaxial(const BrushFaceAttributes& defaultFaceAttributes) {
            resetTextureAxesToParaxial();
            setOffset(vm::vec2f::zero());
            setRotation(0.0f);
            setScale(defaultFaceAttributes.scale());
        }

        void ChangeBrushFaceAttributesRequest::setTextureName(const std::string& textureName) {
            m_textureName = textureName;
            m_textureOp = TextureOp_Set;
        }

        void ChangeBrushFaceAttributesRequest::resetTextureAxes() {
            m_axisOp = AxisOp_Reset;
        }

        void ChangeBrushFaceAttributesRequest::resetTextureAxesToParaxial() {
            m_axisOp = AxisOp_ToParaxial;
        }

        void ChangeBrushFaceAttributesRequest::resetTextureAxesToParallel() {
            m_axisOp = AxisOp_ToParallel;
        }

        void ChangeBrushFaceAttributesRequest::setOffset(const vm::vec2f& offset) {
            setXOffset(offset.x());
            setYOffset(offset.y());
        }

        void ChangeBrushFaceAttributesRequest::addOffset(const vm::vec2f& offset) {
            addXOffset(offset.x());
            addYOffset(offset.y());
        }

        void ChangeBrushFaceAttributesRequest::mulOffset(const vm::vec2f& offset) {
            mulXOffset(offset.x());
            mulYOffset(offset.y());
        }

        void ChangeBrushFaceAttributesRequest::setXOffset(const float xOffset) {
            m_xOffset = xOffset;
            m_xOffsetOp = ValueOp_Set;
        }

        void ChangeBrushFaceAttributesRequest::addXOffset(const float xOffset) {
            m_xOffset = xOffset;
            m_xOffsetOp = ValueOp_Add;
        }

        void ChangeBrushFaceAttributesRequest::mulXOffset(const float xOffset) {
            m_xOffset = xOffset;
            m_xOffsetOp = ValueOp_Mul;
        }

        void ChangeBrushFaceAttributesRequest::setYOffset(const float yOffset) {
            m_yOffset = yOffset;
            m_yOffsetOp = ValueOp_Set;
        }

        void ChangeBrushFaceAttributesRequest::addYOffset(const float yOffset) {
            m_yOffset = yOffset;
            m_yOffsetOp = ValueOp_Add;
        }

        void ChangeBrushFaceAttributesRequest::mulYOffset(const float yOffset) {
            m_yOffset = yOffset;
            m_yOffsetOp = ValueOp_Mul;
        }

        void ChangeBrushFaceAttributesRequest::setRotation(const float rotation) {
            m_rotation = rotation;
            m_rotationOp = ValueOp_Set;
        }

        void ChangeBrushFaceAttributesRequest::addRotation(const float rotation) {
            m_rotation = rotation;
            m_rotationOp = ValueOp_Add;
        }

        void ChangeBrushFaceAttributesRequest::mulRotation(const float rotation) {
            m_rotation = rotation;
            m_rotationOp = ValueOp_Mul;
        }

        void ChangeBrushFaceAttributesRequest::setScale(const vm::vec2f& scale) {
            setXScale(scale.x());
            setYScale(scale.y());
        }

        void ChangeBrushFaceAttributesRequest::addScale(const vm::vec2f& scale) {
            addXScale(scale.x());
            addYScale(scale.y());
        }

        void ChangeBrushFaceAttributesRequest::mulScale(const vm::vec2f& scale) {
            mulXScale(scale.x());
            mulYScale(scale.y());
        }

        void ChangeBrushFaceAttributesRequest::setXScale(const float xScale) {
            m_xScale = xScale;
            m_xScaleOp = ValueOp_Set;
        }

        void ChangeBrushFaceAttributesRequest::addXScale(const float xScale) {
            m_xScale = xScale;
            m_xScaleOp = ValueOp_Add;
        }

        void ChangeBrushFaceAttributesRequest::mulXScale(const float xScale) {
            m_xScale = xScale;
            m_xScaleOp = ValueOp_Mul;
        }

        void ChangeBrushFaceAttributesRequest::setYScale(const float yScale) {
            m_yScale = yScale;
            m_yScaleOp = ValueOp_Set;
        }

        void ChangeBrushFaceAttributesRequest::addYScale(const float yScale) {
            m_yScale = yScale;
            m_yScaleOp = ValueOp_Add;
        }

        void ChangeBrushFaceAttributesRequest::mulYScale(const float yScale) {
            m_yScale = yScale;
            m_yScaleOp = ValueOp_Mul;
        }

        void ChangeBrushFaceAttributesRequest::setSurfaceFlags(const int surfaceFlags) {
            m_surfaceFlags = surfaceFlags;
            m_surfaceFlagsOp = FlagOp_Set;
        }

        void ChangeBrushFaceAttributesRequest::unsetSurfaceFlags(const int surfaceFlags) {
            m_surfaceFlags = surfaceFlags;
            m_surfaceFlagsOp = FlagOp_Unset;
        }

        void ChangeBrushFaceAttributesRequest::replaceSurfaceFlags(const std::optional<int>& surfaceFlags) {
            m_surfaceFlags = surfaceFlags;
            m_surfaceFlagsOp = FlagOp_Replace;
        }

        void ChangeBrushFaceAttributesRequest::setContentFlags(const int contentFlags) {
            m_contentFlags = contentFlags;
            m_contentFlagsOp = FlagOp_Set;
        }

        void ChangeBrushFaceAttributesRequest::unsetContentFlags(const int contentFlags) {
            m_contentFlags = contentFlags;
            m_contentFlagsOp = FlagOp_Unset;
        }

        void ChangeBrushFaceAttributesRequest::replaceContentFlags(const std::optional<int>& contentFlags) {
            m_contentFlags = contentFlags;
            m_contentFlagsOp = FlagOp_Replace;
        }

        void ChangeBrushFaceAttributesRequest::setSurfaceValue(const std::optional<float>& surfaceValue) {
            m_surfaceValue = surfaceValue;
            m_surfaceValueOp = ValueOp_Set;
        }

        void ChangeBrushFaceAttributesRequest::addSurfaceValue(const float surfaceValue) {
            m_surfaceValue = surfaceValue;
            m_surfaceValueOp = ValueOp_Add;
        }

        void ChangeBrushFaceAttributesRequest::mulSurfaceValue(const float surfaceValue) {
            m_surfaceValue = surfaceValue;
            m_surfaceValueOp = ValueOp_Mul;
        }

        void ChangeBrushFaceAttributesRequest::setColor(const Color& colorValue) {
            m_colorValue = colorValue;
            m_colorValueOp = ValueOp_Set;
        }

        void ChangeBrushFaceAttributesRequest::setAll(const Model::BrushFace& face) {
            setAll(face.attributes());
        }

        void ChangeBrushFaceAttributesRequest::setAllExceptContentFlags(const Model::BrushFace& face) {
            setAllExceptContentFlags(face.attributes());
        }

        void ChangeBrushFaceAttributesRequest::setAll(const Model::BrushFaceAttributes& attributes) {
            setAllExceptContentFlags(attributes);
            replaceContentFlags(attributes.surfaceContents());
        }

        void ChangeBrushFaceAttributesRequest::setAllExceptContentFlags(const Model::BrushFaceAttributes& attributes) {
            setTextureName(attributes.textureName());
            setXOffset(attributes.xOffset());
            setYOffset(attributes.yOffset());
            setRotation(attributes.rotation());
            setXScale(attributes.xScale());
            setYScale(attributes.yScale());
            replaceSurfaceFlags(attributes.surfaceFlags());
            setSurfaceValue(attributes.surfaceValue());
            setColor(attributes.color());
        }
    }
}
