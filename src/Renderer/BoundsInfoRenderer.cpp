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

#include "BoundsInfoRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/RenderContext.h"
#include "Renderer/ShaderManager.h"

namespace TrenchBroom {
    namespace Renderer {
        BoundsInfoRenderer::BoundsInfoSizeTextAnchor::BoundsInfoSizeTextAnchor(const BBox3& bounds, const Math::Axis::Type axis, const Renderer::Camera& camera) :
        m_bounds(bounds),
        m_axis(axis),
        m_camera(camera) {}

        Vec3f BoundsInfoRenderer::BoundsInfoSizeTextAnchor::basePosition() const {
            const BBox3::RelativePosition camPos = m_bounds.relativePosition(m_camera.position());
            Vec3f pos;
            const Vec3f half = m_bounds.size() / 2.0f;
            
            if (m_axis == Math::Axis::AZ) {
                if ((camPos[0] == BBox3::RelativePosition::Less && camPos[1] == BBox3::RelativePosition::Less) ||
                    (camPos[0] == BBox3::RelativePosition::Less && camPos[1] == BBox3::RelativePosition::Within)) {
                    pos[0] = m_bounds.min.x();
                    pos[1] = m_bounds.max.y();
                } else if ((camPos[0] == BBox3::RelativePosition::Less    && camPos[1] == BBox3::RelativePosition::Greater) ||
                           (camPos[0] == BBox3::RelativePosition::Within  && camPos[1] == BBox3::RelativePosition::Greater)) {
                    pos[0] = m_bounds.max.x();
                    pos[1] = m_bounds.max.y();
                } else if ((camPos[0] == BBox3::RelativePosition::Greater && camPos[1] == BBox3::RelativePosition::Greater) ||
                           (camPos[0] == BBox3::RelativePosition::Greater && camPos[1] == BBox3::RelativePosition::Within)) {
                    pos[0] = m_bounds.max.x();
                    pos[1] = m_bounds.min.y();
                } else if ((camPos[0] == BBox3::RelativePosition::Within  && camPos[1] == BBox3::RelativePosition::Less) ||
                           (camPos[0] == BBox3::RelativePosition::Greater && camPos[1] == BBox3::RelativePosition::Less)) {
                    pos[0] = m_bounds.min.x();
                    pos[1] = m_bounds.min.y();
                }
                
                pos[2] = m_bounds.min.z() + half.z();
            } else {
                if (m_axis == Math::Axis::AX) {
                    pos[0] = m_bounds.min.x() + half.x();
                    if (       camPos[0] == BBox3::RelativePosition::Less    && camPos[1] == BBox3::RelativePosition::Less) {
                        pos[1] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.min.y() : m_bounds.max.y();
                    } else if (camPos[0] == BBox3::RelativePosition::Less    && camPos[1] == BBox3::RelativePosition::Within) {
                        pos[1] = m_bounds.max.y();
                    } else if (camPos[0] == BBox3::RelativePosition::Less    && camPos[1] == BBox3::RelativePosition::Greater) {
                        pos[1] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.max.y() : m_bounds.min.y();
                    } else if (camPos[0] == BBox3::RelativePosition::Within  && camPos[1] == BBox3::RelativePosition::Greater) {
                        pos[1] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.max.y() : m_bounds.min.y();
                    } else if (camPos[0] == BBox3::RelativePosition::Greater && camPos[1] == BBox3::RelativePosition::Greater) {
                        pos[1] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.max.y() : m_bounds.min.y();
                    } else if (camPos[0] == BBox3::RelativePosition::Greater && camPos[1] == BBox3::RelativePosition::Within) {
                        pos[1] = m_bounds.min.y();
                    } else if (camPos[0] == BBox3::RelativePosition::Greater && camPos[1] == BBox3::RelativePosition::Less) {
                        pos[1] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.min.y() : m_bounds.max.y();
                    } else if (camPos[0] == BBox3::RelativePosition::Within  && camPos[1] == BBox3::RelativePosition::Less) {
                        pos[1] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.min.y() : m_bounds.max.y();
                    }
                } else {
                    pos[1] = m_bounds.min.y() + half.y();
                    if (       camPos[0] == BBox3::RelativePosition::Less    && camPos[1] == BBox3::RelativePosition::Less) {
                        pos[0] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.min.x() : m_bounds.max.x();
                    } else if (camPos[0] == BBox3::RelativePosition::Less    && camPos[1] == BBox3::RelativePosition::Within) {
                        pos[0] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.min.x() : m_bounds.max.x();
                    } else if (camPos[0] == BBox3::RelativePosition::Less    && camPos[1] == BBox3::RelativePosition::Greater) {
                        pos[0] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.min.x() : m_bounds.max.x();
                    } else if (camPos[0] == BBox3::RelativePosition::Within  && camPos[1] == BBox3::RelativePosition::Greater) {
                        pos[0] = m_bounds.max.x();
                    } else if (camPos[0] == BBox3::RelativePosition::Greater && camPos[1] == BBox3::RelativePosition::Greater) {
                        pos[0] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.max.x() : m_bounds.min.x();
                    } else if (camPos[0] == BBox3::RelativePosition::Greater && camPos[1] == BBox3::RelativePosition::Within) {
                        pos[0] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.max.x() : m_bounds.min.x();
                    } else if (camPos[0] == BBox3::RelativePosition::Greater && camPos[1] == BBox3::RelativePosition::Less) {
                        pos[0] = camPos[2] == BBox3::RelativePosition::Within ? m_bounds.max.x() : m_bounds.min.x();
                    } else if (camPos[0] == BBox3::RelativePosition::Within  && camPos[1] == BBox3::RelativePosition::Less) {
                        pos[0] = m_bounds.min.x();
                    }
                }
                
                if (camPos[2] == BBox3::RelativePosition::Less)
                    pos[2] = m_bounds.min.z();
                else
                    pos[2] = m_bounds.max.z();
            }
            
            return pos;
        }
        
        Alignment::Type BoundsInfoRenderer::BoundsInfoSizeTextAnchor::alignment() const {
            if (m_axis == Math::Axis::AZ)
                return Alignment::Right;
            
            const BBox3::RelativePosition camPos = m_bounds.relativePosition(m_camera.position());
            if (camPos[2] == BBox3::RelativePosition::Less)
                return Alignment::Top;
            return Alignment::Bottom;
        }
        
        Vec2f BoundsInfoRenderer::BoundsInfoSizeTextAnchor::extraOffsets(const Alignment::Type a) const {
            Vec2f result;
            if (a & Alignment::Top)
                result[1] -= 8.0f;
            if (a & Alignment::Bottom)
                result[1] += 8.0f;
            if (a & Alignment::Left)
                result[0] += 8.0f;
            if (a & Alignment::Right)
                result[0] -= 8.0f;
            return result;
        }

        BoundsInfoRenderer::BoundsInfoMinMaxTextAnchor::BoundsInfoMinMaxTextAnchor(const BBox3& bounds, const EMinMax minMax, const Renderer::Camera& camera) :
        m_bounds(bounds),
        m_minMax(minMax),
        m_camera(camera) {}

        Vec3f BoundsInfoRenderer::BoundsInfoMinMaxTextAnchor::basePosition() const {
            if (m_minMax == BoxMin)
                return m_bounds.min;
            return m_bounds.max;
        }
        
        Alignment::Type BoundsInfoRenderer::BoundsInfoMinMaxTextAnchor::alignment() const {
            const BBox3::RelativePosition camPos = m_bounds.relativePosition(m_camera.position());
            if (m_minMax == BoxMin) {
                if (camPos[1] == BBox3::RelativePosition::Less || (camPos[1] == BBox3::RelativePosition::Within && camPos[0] != BBox3::RelativePosition::Less))
                    return Alignment::Top | Alignment::Right;
                return Alignment::Top | Alignment::Left;
            }
            if (camPos[1] == BBox3::RelativePosition::Less || (camPos[1] == BBox3::RelativePosition::Within && camPos[0] != BBox3::RelativePosition::Less))
                return Alignment::Bottom | Alignment::Left;
            return Alignment::Bottom | Alignment::Right;
        }
        
        Vec2f BoundsInfoRenderer::BoundsInfoMinMaxTextAnchor::extraOffsets(const Alignment::Type a) const {
            Vec2f result;
            if (a & Alignment::Top)
                result[1] -= 8.0f;
            if (a & Alignment::Bottom)
                result[1] += 8.0f;
            if (a & Alignment::Left)
                result[0] += 8.0f;
            if (a & Alignment::Right)
                result[0] -= 8.0f;
            return result;
        }

        BoundsInfoRenderer::BoundsInfoRenderer(TextureFont& font) :
        m_bounds(0.0, 0.0),
        m_textRenderer(font),
        m_textColorProvider(Preferences::InfoOverlayTextColor, Preferences::InfoOverlayBackgroundColor),
        m_valid(false) {
            m_textRenderer.setFadeDistance(2000.0f);
        }
        
        void BoundsInfoRenderer::setBounds(const BBox3& bounds) {
            m_bounds = bounds;
            m_valid = false;
        }
        
        void BoundsInfoRenderer::render(RenderContext& renderContext) {
            if (!m_valid)
                validate(renderContext);

            m_textRenderer.render(renderContext, m_textFilter, m_textColorProvider, Shaders::ColoredTextShader, Shaders::TextBackgroundShader);
        }

        void BoundsInfoRenderer::validate(RenderContext& renderContext) {
            static const String labels[3] = { "X", "Y", "Z" };
            const Vec3 size = m_bounds.size().corrected();

            StringStream buffer;
            for (size_t i = 0; i < 3; ++i) {
                buffer << labels[i] << ": " << size[i];
                
                TextAnchor::Ptr anchor(new BoundsInfoSizeTextAnchor(m_bounds, i, renderContext.camera()));
                m_textRenderer.addString(i, buffer.str(), anchor);

                buffer.str("");
            }
            
            buffer << "Min: " << m_bounds.min.asString();
            TextAnchor::Ptr minAnchor(new BoundsInfoMinMaxTextAnchor(m_bounds, BoundsInfoMinMaxTextAnchor::BoxMin, renderContext.camera()));
            m_textRenderer.addString(3, buffer.str(), minAnchor);
            buffer.str("");
            
            buffer << "Max: " << m_bounds.max.asString();
            TextAnchor::Ptr maxAnchor(new BoundsInfoMinMaxTextAnchor(m_bounds, BoundsInfoMinMaxTextAnchor::BoxMax, renderContext.camera()));
            m_textRenderer.addString(4, buffer.str(), maxAnchor);
            
            m_valid = true;
        }
    }
}
