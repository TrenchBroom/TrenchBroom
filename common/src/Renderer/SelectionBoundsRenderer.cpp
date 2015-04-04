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

#include "SelectionBoundsRenderer.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderService.h"
#include "Renderer/TextAnchor.h"

namespace TrenchBroom {
    namespace Renderer {
        class SelectionBoundsRenderer::SizeTextAnchor2D : public TextAnchor {
        private:
            const BBox3& m_bounds;
            const Math::Axis::Type m_axis;
            const Camera& m_camera;
        public:
            SizeTextAnchor2D(const BBox3& bounds, const Math::Axis::Type axis, const Renderer::Camera& camera) :
            m_bounds(bounds),
            m_axis(axis),
            m_camera(camera) {}
        private:
            Vec3f basePosition() const {
                const Vec3 half = m_bounds.size() / 2.0;
                Vec3 pos = m_bounds.min;
                pos[m_axis] += half[m_axis];
                return Vec3f(pos);
            }
            
            TextAlignment::Type alignment() const {
                if (m_axis == Math::Axis::AX)
                    return TextAlignment::Top;
                if (m_axis == Math::Axis::AY && m_camera.direction().x() != 0.0f)
                    return TextAlignment::Top;
                return TextAlignment::Right;
            }
            
            Vec2f extraOffsets(TextAlignment::Type alignment, const Vec2f& size) const {
                Vec2f result;
                if (alignment & TextAlignment::Top)
                    result[1] -= 8.0f;
                if (alignment & TextAlignment::Bottom)
                    result[1] += 8.0f;
                if (alignment & TextAlignment::Left)
                    result[0] += 8.0f;
                if (alignment & TextAlignment::Right)
                    result[0] -= 8.0f;
                return result;
            }
        };
        
        class SelectionBoundsRenderer::SizeTextAnchor3D : public TextAnchor {
        private:
            const BBox3& m_bounds;
            const Math::Axis::Type m_axis;
            const Camera& m_camera;
        public:
            SizeTextAnchor3D(const BBox3& bounds, const Math::Axis::Type axis, const Renderer::Camera& camera) :
            m_bounds(bounds),
            m_axis(axis),
            m_camera(camera) {}
        private:
            Vec3f basePosition() const {
                const BBox3::RelativePosition camPos = m_bounds.relativePosition(m_camera.position());
                Vec3 pos;
                const Vec3 half = m_bounds.size() / 2.0;
                
                if (m_axis == Math::Axis::AZ) {
                    if ((camPos[0] == BBox3::RelativePosition::Range_Less && camPos[1] == BBox3::RelativePosition::Range_Less) ||
                        (camPos[0] == BBox3::RelativePosition::Range_Less && camPos[1] == BBox3::RelativePosition::Range_Within)) {
                        pos[0] = m_bounds.min.x();
                        pos[1] = m_bounds.max.y();
                    } else if ((camPos[0] == BBox3::RelativePosition::Range_Less    && camPos[1] == BBox3::RelativePosition::Range_Greater) ||
                               (camPos[0] == BBox3::RelativePosition::Range_Within  && camPos[1] == BBox3::RelativePosition::Range_Greater)) {
                        pos[0] = m_bounds.max.x();
                        pos[1] = m_bounds.max.y();
                    } else if ((camPos[0] == BBox3::RelativePosition::Range_Greater && camPos[1] == BBox3::RelativePosition::Range_Greater) ||
                               (camPos[0] == BBox3::RelativePosition::Range_Greater && camPos[1] == BBox3::RelativePosition::Range_Within)) {
                        pos[0] = m_bounds.max.x();
                        pos[1] = m_bounds.min.y();
                    } else if ((camPos[0] == BBox3::RelativePosition::Range_Within  && camPos[1] == BBox3::RelativePosition::Range_Less) ||
                               (camPos[0] == BBox3::RelativePosition::Range_Greater && camPos[1] == BBox3::RelativePosition::Range_Less)) {
                        pos[0] = m_bounds.min.x();
                        pos[1] = m_bounds.min.y();
                    }
                    
                    pos[2] = m_bounds.min.z() + half.z();
                } else {
                    if (m_axis == Math::Axis::AX) {
                        pos[0] = m_bounds.min.x() + half.x();
                        if (       camPos[0] == BBox3::RelativePosition::Range_Less    && camPos[1] == BBox3::RelativePosition::Range_Less) {
                            pos[1] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.min.y() : m_bounds.max.y();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Less    && camPos[1] == BBox3::RelativePosition::Range_Within) {
                            pos[1] = m_bounds.max.y();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Less    && camPos[1] == BBox3::RelativePosition::Range_Greater) {
                            pos[1] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.max.y() : m_bounds.min.y();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Within  && camPos[1] == BBox3::RelativePosition::Range_Greater) {
                            pos[1] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.max.y() : m_bounds.min.y();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Greater && camPos[1] == BBox3::RelativePosition::Range_Greater) {
                            pos[1] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.max.y() : m_bounds.min.y();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Greater && camPos[1] == BBox3::RelativePosition::Range_Within) {
                            pos[1] = m_bounds.min.y();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Greater && camPos[1] == BBox3::RelativePosition::Range_Less) {
                            pos[1] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.min.y() : m_bounds.max.y();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Within  && camPos[1] == BBox3::RelativePosition::Range_Less) {
                            pos[1] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.min.y() : m_bounds.max.y();
                        }
                    } else {
                        pos[1] = m_bounds.min.y() + half.y();
                        if (       camPos[0] == BBox3::RelativePosition::Range_Less    && camPos[1] == BBox3::RelativePosition::Range_Less) {
                            pos[0] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.min.x() : m_bounds.max.x();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Less    && camPos[1] == BBox3::RelativePosition::Range_Within) {
                            pos[0] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.min.x() : m_bounds.max.x();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Less    && camPos[1] == BBox3::RelativePosition::Range_Greater) {
                            pos[0] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.min.x() : m_bounds.max.x();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Within  && camPos[1] == BBox3::RelativePosition::Range_Greater) {
                            pos[0] = m_bounds.max.x();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Greater && camPos[1] == BBox3::RelativePosition::Range_Greater) {
                            pos[0] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.max.x() : m_bounds.min.x();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Greater && camPos[1] == BBox3::RelativePosition::Range_Within) {
                            pos[0] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.max.x() : m_bounds.min.x();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Greater && camPos[1] == BBox3::RelativePosition::Range_Less) {
                            pos[0] = camPos[2] == BBox3::RelativePosition::Range_Within ? m_bounds.max.x() : m_bounds.min.x();
                        } else if (camPos[0] == BBox3::RelativePosition::Range_Within  && camPos[1] == BBox3::RelativePosition::Range_Less) {
                            pos[0] = m_bounds.min.x();
                        }
                    }
                    
                    if (camPos[2] == BBox3::RelativePosition::Range_Less)
                        pos[2] = m_bounds.min.z();
                    else
                        pos[2] = m_bounds.max.z();
                }
                
                return Vec3f(pos);
            }
            
            TextAlignment::Type alignment() const {
                if (m_axis == Math::Axis::AZ)
                    return TextAlignment::Right;
                
                const BBox3::RelativePosition camPos = m_bounds.relativePosition(m_camera.position());
                if (camPos[2] == BBox3::RelativePosition::Range_Less)
                    return TextAlignment::Top;
                return TextAlignment::Bottom;
            }
            
            Vec2f extraOffsets(TextAlignment::Type alignment, const Vec2f& size) const {
                Vec2f result;
                if (alignment & TextAlignment::Top)
                    result[1] -= 8.0f;
                if (alignment & TextAlignment::Bottom)
                    result[1] += 8.0f;
                if (alignment & TextAlignment::Left)
                    result[0] += 8.0f;
                if (alignment & TextAlignment::Right)
                    result[0] -= 8.0f;
                return result;
            }
        };
        
        class SelectionBoundsRenderer::MinMaxTextAnchor3D : public TextAnchor {
        private:
            const BBox3& m_bounds;
            const BBox3::Corner m_minMax;
            const Renderer::Camera& m_camera;
        public:
            MinMaxTextAnchor3D(const BBox3& bounds, const BBox3::Corner minMax, const Renderer::Camera& camera) :
            m_bounds(bounds),
            m_minMax(minMax),
            m_camera(camera) {}
        private:
            Vec3f basePosition() const {
                if (m_minMax == BBox3::Corner_Min)
                    return m_bounds.min;
                return m_bounds.max;
            }
            
            TextAlignment::Type alignment() const {
                const BBox3::RelativePosition camPos = m_bounds.relativePosition(m_camera.position());
                if (m_minMax == BBox3::Corner_Min) {
                    if ((camPos[1] == BBox3::RelativePosition::Range_Less) ||
                        (camPos[1] == BBox3::RelativePosition::Range_Within &&
                         camPos[0] != BBox3::RelativePosition::Range_Less))
                        return TextAlignment::Top | TextAlignment::Right;
                    return TextAlignment::Top | TextAlignment::Left;
                }
                if ((camPos[1] == BBox3::RelativePosition::Range_Less) ||
                    (camPos[1] == BBox3::RelativePosition::Range_Within &&
                     camPos[0] != BBox3::RelativePosition::Range_Less))
                    return TextAlignment::Bottom | TextAlignment::Left;
                return TextAlignment::Bottom | TextAlignment::Right;
            }
            
            Vec2f extraOffsets(TextAlignment::Type alignment, const Vec2f& size) const {
                Vec2f result;
                if (alignment & TextAlignment::Top)
                    result[1] -= 8.0f;
                if (alignment & TextAlignment::Bottom)
                    result[1] += 8.0f;
                if (alignment & TextAlignment::Left)
                    result[0] += 8.0f;
                if (alignment & TextAlignment::Right)
                    result[0] -= 8.0f;
                return result;
            }
        };

        SelectionBoundsRenderer::SelectionBoundsRenderer(const BBox3& bounds) :
        m_bounds(bounds) {}

        void SelectionBoundsRenderer::render(RenderContext& renderContext, RenderBatch& renderBatch) {
            renderBounds(renderContext, renderBatch);
            renderSize(renderContext, renderBatch);
            // renderMinMax(renderContext, renderBatch);
        }

        void SelectionBoundsRenderer::renderBounds(RenderContext& renderContext, RenderBatch& renderBatch) {
            RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::SelectionBoundsColor));
            renderService.renderBounds(m_bounds);
        }
        
        void SelectionBoundsRenderer::renderSize(RenderContext& renderContext, RenderBatch& renderBatch) {
            if (renderContext.render2D())
                renderSize2D(renderContext, renderBatch);
            else
                renderSize3D(renderContext, renderBatch);
        }
        
        void SelectionBoundsRenderer::renderSize2D(RenderContext& renderContext, RenderBatch& renderBatch) {
            static const String labels[3] = { "X", "Y", "Z" };
            StringStream buffer;
            
            RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::InfoOverlayTextColor));
            renderService.setBackgroundColor(pref(Preferences::WeakInfoOverlayBackgroundColor));
            
            const Camera& camera = renderContext.camera();
            const Vec3f& direction = camera.direction();
            
            const Vec3 size = m_bounds.size().corrected();
            for (size_t i = 0; i < 3; ++i) {
                if (direction[i] == 0.0f) {
                    buffer << labels[i] << ": " << size[i];
                    renderService.renderStringOnTop(buffer.str(), SizeTextAnchor2D(m_bounds, i, camera));
                    buffer.str("");
                }
            }
        }
        
        void SelectionBoundsRenderer::renderSize3D(RenderContext& renderContext, RenderBatch& renderBatch) {
            static const String labels[3] = { "X", "Y", "Z" };
            StringStream buffer;
            
            RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::InfoOverlayTextColor));
            renderService.setBackgroundColor(pref(Preferences::WeakInfoOverlayBackgroundColor));
            
            const Vec3 size = m_bounds.size().corrected();
            for (size_t i = 0; i < 3; ++i) {
                buffer << labels[i] << ": " << size[i];
                
                renderService.renderStringOnTop(buffer.str(), SizeTextAnchor3D(m_bounds, i, renderContext.camera()));
                
                buffer.str("");
            }
        }

        void SelectionBoundsRenderer::renderMinMax(RenderContext& renderContext, RenderBatch& renderBatch) {
            StringStream buffer;

            RenderService renderService(renderContext, renderBatch);
            renderService.setForegroundColor(pref(Preferences::InfoOverlayTextColor));
            renderService.setBackgroundColor(pref(Preferences::WeakInfoOverlayBackgroundColor));

            buffer << "Min: " << m_bounds.min.corrected().asString();
            renderService.renderStringOnTop(buffer.str(), MinMaxTextAnchor3D(m_bounds, BBox3::Corner_Min, renderContext.camera()));
            buffer.str("");
            
            buffer << "Max: " << m_bounds.max.corrected().asString();
            renderService.renderStringOnTop(buffer.str(), MinMaxTextAnchor3D(m_bounds, BBox3::Corner_Max, renderContext.camera()));
        }
    }
}
