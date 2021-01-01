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

        static bool collateTextureOp(ChangeBrushFaceAttributesRequest::TextureOp& myOp, std::string& myTextureName, const ChangeBrushFaceAttributesRequest::TextureOp theirOp, const std::string& theirTextureName) {
            if (theirOp != ChangeBrushFaceAttributesRequest::TextureOp_None) {
                myOp = theirOp;
                myTextureName = theirTextureName;
            }
            return true;
        }

        static bool collateAxisOp(ChangeBrushFaceAttributesRequest::AxisOp& myOp, const ChangeBrushFaceAttributesRequest::AxisOp theirOp) {
            switch (myOp) {
                case ChangeBrushFaceAttributesRequest::AxisOp_None:
                    myOp = theirOp;
                    return true;
                case ChangeBrushFaceAttributesRequest::AxisOp_Reset:
                case ChangeBrushFaceAttributesRequest::AxisOp_ToParaxial:
                case ChangeBrushFaceAttributesRequest::AxisOp_ToParallel:
                    switch (theirOp) {
                        case ChangeBrushFaceAttributesRequest::AxisOp_None:
                            return true;
                        case ChangeBrushFaceAttributesRequest::AxisOp_Reset:
                        case ChangeBrushFaceAttributesRequest::AxisOp_ToParallel:
                        case ChangeBrushFaceAttributesRequest::AxisOp_ToParaxial:
                            myOp = theirOp;
                            return true;
                        switchDefault()
                    }
                switchDefault()
            }
        }

        template <typename T>
        static bool collateValueOp(ChangeBrushFaceAttributesRequest::ValueOp& myOp, T& myValue, const ChangeBrushFaceAttributesRequest::ValueOp theirOp, const T theirValue) {
            switch (myOp) {
                case ChangeBrushFaceAttributesRequest::ValueOp_None:
                    myOp = theirOp;
                    myValue = theirValue;
                    return true;
                case ChangeBrushFaceAttributesRequest::ValueOp_Set:
                    switch (theirOp) {
                        case ChangeBrushFaceAttributesRequest::ValueOp_None:
                            return true;
                        case ChangeBrushFaceAttributesRequest::ValueOp_Set:
                            myValue = theirValue;
                            return true;
                        case ChangeBrushFaceAttributesRequest::ValueOp_Add:
                        case ChangeBrushFaceAttributesRequest::ValueOp_Mul:
                            return false;
                        switchDefault()
                    }
                case ChangeBrushFaceAttributesRequest::ValueOp_Add:
                    switch (theirOp) {
                        case ChangeBrushFaceAttributesRequest::ValueOp_None:
                            return true;
                        case ChangeBrushFaceAttributesRequest::ValueOp_Set:
                            myOp = theirOp;
                            myValue = theirValue;
                            return true;
                        case ChangeBrushFaceAttributesRequest::ValueOp_Add:
                            myValue = myValue + theirValue;
                            return true;
                        case ChangeBrushFaceAttributesRequest::ValueOp_Mul:
                            return false;
                            switchDefault()
                    }
                case ChangeBrushFaceAttributesRequest::ValueOp_Mul:
                    switch (theirOp) {
                        case ChangeBrushFaceAttributesRequest::ValueOp_None:
                            return true;
                        case ChangeBrushFaceAttributesRequest::ValueOp_Set:
                            myOp = theirOp;
                            myValue = theirValue;
                            return true;
                        case ChangeBrushFaceAttributesRequest::ValueOp_Add:
                            return false;
                        case ChangeBrushFaceAttributesRequest::ValueOp_Mul:
                            myValue = myValue * theirValue;
                            return true;
                            switchDefault()
                    }
                    switchDefault()
            }
        }

        template <typename T>
        static bool collateFlagOp(ChangeBrushFaceAttributesRequest::FlagOp& myOp, T& myValue, const ChangeBrushFaceAttributesRequest::FlagOp theirOp, const T theirValue) {
            switch (myOp) {
                case ChangeBrushFaceAttributesRequest::FlagOp_None:
                    myOp = theirOp;
                    myValue = theirValue;
                    return true;
                case ChangeBrushFaceAttributesRequest::FlagOp_Replace:
                    switch (theirOp) {
                        case ChangeBrushFaceAttributesRequest::FlagOp_None:
                            return true;
                        case ChangeBrushFaceAttributesRequest::FlagOp_Replace:
                            myValue = theirValue;
                            return true;
                        case ChangeBrushFaceAttributesRequest::FlagOp_Set:
                        case ChangeBrushFaceAttributesRequest::FlagOp_Unset:
                            return false;
                            switchDefault()
                    }
                case ChangeBrushFaceAttributesRequest::FlagOp_Set:
                    switch (theirOp) {
                        case ChangeBrushFaceAttributesRequest::FlagOp_None:
                            return true;
                        case ChangeBrushFaceAttributesRequest::FlagOp_Replace:
                            myOp = theirOp;
                            myValue = theirValue;
                            return true;
                        case ChangeBrushFaceAttributesRequest::FlagOp_Set:
                            myValue |= theirValue;
                            return true;
                        case ChangeBrushFaceAttributesRequest::FlagOp_Unset:
                            return false;
                            switchDefault()
                    }
                case ChangeBrushFaceAttributesRequest::FlagOp_Unset:
                    switch (theirOp) {
                        case ChangeBrushFaceAttributesRequest::FlagOp_None:
                            return true;
                        case ChangeBrushFaceAttributesRequest::FlagOp_Replace:
                            myOp = theirOp;
                            myValue = theirValue;
                            return true;
                        case ChangeBrushFaceAttributesRequest::FlagOp_Set:
                            return false;
                        case ChangeBrushFaceAttributesRequest::FlagOp_Unset:
                            myValue |= theirValue;
                            return true;
                            switchDefault()
                    }
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

        bool ChangeBrushFaceAttributesRequest::evaluate(const std::vector<BrushFaceHandle>& faceHandles) const {
            auto result = false;
            for (const BrushFaceHandle& faceHandle : faceHandles) {
                BrushNode* node  = faceHandle.node();
                Brush brush = node->brush();
                BrushFace& face = brush.face(faceHandle.faceIndex());

                result |= evaluate(face);
                node->setBrush(std::move(brush));
            }
            return result;
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
                case AxisOp_ToParaxial:
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

        void ChangeBrushFaceAttributesRequest::replaceSurfaceFlags(const int surfaceFlags) {
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

        void ChangeBrushFaceAttributesRequest::replaceContentFlags(const int contentFlags) {
            m_contentFlags = contentFlags;
            m_contentFlagsOp = FlagOp_Replace;
        }

        void ChangeBrushFaceAttributesRequest::setSurfaceValue(const float surfaceValue) {
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

        bool ChangeBrushFaceAttributesRequest::collateWith(ChangeBrushFaceAttributesRequest& other) {
            std::string newTextureName = m_textureName; TextureOp newTextureOp = m_textureOp;
            AxisOp newAxisOp = m_axisOp;

            float newXOffset = m_xOffset;   ValueOp newXOffsetOp = m_xOffsetOp;
            float newYOffset = m_yOffset;   ValueOp newYOffsetOp = m_yOffsetOp;
            float newRotation = m_rotation; ValueOp newRotationOp = m_rotationOp;
            float newXScale = m_xScale;     ValueOp newXScaleOp = m_xScaleOp;
            float newYScale = m_yScale;     ValueOp newYScaleOp = m_yScaleOp;

            int newSurfaceFlags = m_surfaceFlags; FlagOp newSurfaceFlagsOp = m_surfaceFlagsOp;
            int newContentFlags = m_contentFlags; FlagOp newContentFlagsOp = m_contentFlagsOp;
            float newSurfaceValue = m_surfaceValue; ValueOp newSurfaceValueOp = m_surfaceValueOp;

            Color newColorValue = m_colorValue; ValueOp newColorValueOp = m_colorValueOp;

            if (!collateAxisOp(newAxisOp, other.m_axisOp))
                return false;
            if (!collateTextureOp(newTextureOp, newTextureName, other.m_textureOp, other.m_textureName))
                return false;
            if (!collateValueOp(newXOffsetOp, newXOffset, other.m_xOffsetOp, other.m_xOffset))
                return false;
            if (!collateValueOp(newYOffsetOp, newYOffset, other.m_yOffsetOp, other.m_yOffset))
                return false;
            if (!collateValueOp(newRotationOp, newRotation, other.m_rotationOp, other.m_rotation))
                return false;
            if (!collateValueOp(newXScaleOp, newXScale, other.m_xScaleOp, other.m_xScale))
                return false;
            if (!collateValueOp(newYScaleOp, newYScale, other.m_yScaleOp, other.m_yScale))
                return false;

            if (!collateFlagOp(newSurfaceFlagsOp, newSurfaceFlags, other.m_surfaceFlagsOp, other.m_surfaceFlags))
                return false;
            if (!collateFlagOp(newContentFlagsOp, newContentFlags, other.m_contentFlagsOp, other.m_contentFlags))
                return false;
            if (!collateValueOp(newSurfaceValueOp, newSurfaceValue, other.m_surfaceValueOp, other.m_surfaceValue))
                return false;

            if (!collateValueOp(newColorValueOp, newColorValue, other.m_colorValueOp, other.m_colorValue))
                return false;
            
            m_textureName = newTextureName; m_textureOp = newTextureOp;
            m_axisOp = newAxisOp;
            m_xOffset = newXOffset; m_xOffsetOp = newXOffsetOp;
            m_yOffset = newYOffset; m_yOffsetOp = newYOffsetOp;
            m_rotation = newRotation; m_rotationOp = newRotationOp;
            m_xScale = newXScale; m_xScaleOp = newXScaleOp;
            m_yScale = newYScale; m_yScaleOp = newYScaleOp;

            m_surfaceFlags = newSurfaceFlags; m_surfaceFlagsOp = newSurfaceFlagsOp;
            m_contentFlags = newContentFlags; m_contentFlagsOp = newContentFlagsOp;
            m_surfaceValue = newSurfaceValue; m_surfaceValueOp = newSurfaceValueOp;

            m_colorValue = newColorValue; m_colorValueOp = newColorValueOp;

            return true;
        }
    }
}
