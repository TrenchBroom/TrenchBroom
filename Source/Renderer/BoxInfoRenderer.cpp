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

#include "BoxInfoRenderer.h"

#include "Renderer/Camera.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Shader/ShaderManager.h"
#include "Renderer/SharedResources.h"
#include "Renderer/Text/FontManager.h"
#include "Utility/Preferences.h"
#include "Utility/String.h"

#include <cassert>

namespace TrenchBroom {
    namespace Renderer {
        const Vec3f BoxInfoSizeTextAnchor::basePosition() const {
            BBox::PointPosition camPos = m_bounds.pointPosition(m_camera->position());
            Vec3f pos;
            const Vec3f half = m_bounds.size() / 2.0f;
            
            if (m_axis == Axis::AZ) {
                if ((camPos.x == BBox::PointPosition::Less && camPos.y == BBox::PointPosition::Less) ||
                    (camPos.x == BBox::PointPosition::Less && camPos.y == BBox::PointPosition::Within)) {
                    pos.x = m_bounds.min.x;
                    pos.y = m_bounds.max.y;
                } else if ((camPos.x == BBox::PointPosition::Less    && camPos.y == BBox::PointPosition::Greater) ||
                           (camPos.x == BBox::PointPosition::Within  && camPos.y == BBox::PointPosition::Greater)) {
                    pos.x = m_bounds.max.x;
                    pos.y = m_bounds.max.y;
                } else if ((camPos.x == BBox::PointPosition::Greater && camPos.y == BBox::PointPosition::Greater) ||
                           (camPos.x == BBox::PointPosition::Greater && camPos.y == BBox::PointPosition::Within)) {
                    pos.x = m_bounds.max.x;
                    pos.y = m_bounds.min.y;
                } else if ((camPos.x == BBox::PointPosition::Within  && camPos.y == BBox::PointPosition::Less) ||
                           (camPos.x == BBox::PointPosition::Greater && camPos.y == BBox::PointPosition::Less)) {
                    pos.x = m_bounds.min.x;
                    pos.y = m_bounds.min.y;
                }
                
                pos.z = m_bounds.min.z + half.z;
            } else {
                if (m_axis == Axis::AX) {
                    pos.x = m_bounds.min.x + half.x;
                    if (       camPos.x == BBox::PointPosition::Less    && camPos.y == BBox::PointPosition::Less) {
                        pos.y = camPos.z == BBox::PointPosition::Within ? m_bounds.min.y : m_bounds.max.y;
                    } else if (camPos.x == BBox::PointPosition::Less    && camPos.y == BBox::PointPosition::Within) {
                        pos.y = m_bounds.max.y;
                    } else if (camPos.x == BBox::PointPosition::Less    && camPos.y == BBox::PointPosition::Greater) {
                        pos.y = camPos.z == BBox::PointPosition::Within ? m_bounds.max.y : m_bounds.min.y;
                    } else if (camPos.x == BBox::PointPosition::Within  && camPos.y == BBox::PointPosition::Greater) {
                        pos.y = camPos.z == BBox::PointPosition::Within ? m_bounds.max.y : m_bounds.min.y;
                    } else if (camPos.x == BBox::PointPosition::Greater && camPos.y == BBox::PointPosition::Greater) {
                        pos.y = camPos.z == BBox::PointPosition::Within ? m_bounds.max.y : m_bounds.min.y;
                    } else if (camPos.x == BBox::PointPosition::Greater && camPos.y == BBox::PointPosition::Within) {
                        pos.y = m_bounds.min.y;
                    } else if (camPos.x == BBox::PointPosition::Greater && camPos.y == BBox::PointPosition::Less) {
                        pos.y = camPos.z == BBox::PointPosition::Within ? m_bounds.min.y : m_bounds.max.y;
                    } else if (camPos.x == BBox::PointPosition::Within  && camPos.y == BBox::PointPosition::Less) {
                        pos.y = camPos.z == BBox::PointPosition::Within ? m_bounds.min.y : m_bounds.max.y;
                    }
                } else {
                    pos.y = m_bounds.min.y + half.y;
                    if (       camPos.x == BBox::PointPosition::Less    && camPos.y == BBox::PointPosition::Less) {
                        pos.x = camPos.z == BBox::PointPosition::Within ? m_bounds.min.x : m_bounds.max.x;
                    } else if (camPos.x == BBox::PointPosition::Less    && camPos.y == BBox::PointPosition::Within) {
                        pos.x = camPos.z == BBox::PointPosition::Within ? m_bounds.min.x : m_bounds.max.x;
                    } else if (camPos.x == BBox::PointPosition::Less    && camPos.y == BBox::PointPosition::Greater) {
                        pos.x = camPos.z == BBox::PointPosition::Within ? m_bounds.min.x : m_bounds.max.x;
                    } else if (camPos.x == BBox::PointPosition::Within  && camPos.y == BBox::PointPosition::Greater) {
                        pos.x = m_bounds.max.x;
                    } else if (camPos.x == BBox::PointPosition::Greater && camPos.y == BBox::PointPosition::Greater) {
                        pos.x = camPos.z == BBox::PointPosition::Within ? m_bounds.max.x : m_bounds.min.x;
                    } else if (camPos.x == BBox::PointPosition::Greater && camPos.y == BBox::PointPosition::Within) {
                        pos.x = camPos.z == BBox::PointPosition::Within ? m_bounds.max.x : m_bounds.min.x;
                    } else if (camPos.x == BBox::PointPosition::Greater && camPos.y == BBox::PointPosition::Less) {
                        pos.x = camPos.z == BBox::PointPosition::Within ? m_bounds.max.x : m_bounds.min.x;
                    } else if (camPos.x == BBox::PointPosition::Within  && camPos.y == BBox::PointPosition::Less) {
                        pos.x = m_bounds.min.x;
                    }
                }
                
                if (camPos.z == BBox::PointPosition::Less)
                    pos.z = m_bounds.min.z;
                else
                    pos.z = m_bounds.max.z;
            }
            
            return pos;
        }

        const Text::Alignment::Type BoxInfoSizeTextAnchor::alignment() const {
            if (m_axis == Axis::AZ)
                return Text::Alignment::Right;

            BBox::PointPosition camPos = m_bounds.pointPosition(m_camera->position());
            if (camPos.z == BBox::PointPosition::Less)
                return Text::Alignment::Top;
            return Text::Alignment::Bottom;
        }

        BoxInfoSizeTextAnchor::BoxInfoSizeTextAnchor(BBox& bounds, Axis::Type axis, Renderer::Camera& camera) :
        m_bounds(bounds.expanded(2.0f)), // create a bit of a margin for the text label
        m_axis(axis),
        m_camera(&camera) {}
        
        BoxInfoRenderer::BoxInfoRenderer(const BBox& bounds, Text::FontManager& fontManager) :
        m_bounds(bounds),
        m_textRenderer(NULL),
        m_initialized(false) {
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            
            const String& fontName = prefs.getString(Preferences::RendererFontName);
            int fontSize = prefs.getInt(Preferences::RendererFontSize);
            Text::FontDescriptor fontDescriptor(fontName, static_cast<unsigned int>(fontSize));
            
            Text::TexturedFont* font = fontManager.font(fontDescriptor);
            assert(font != NULL);

            m_textRenderer = new Text::TextRenderer<Axis::Type, BoxInfoSizeTextAnchor>(*font);
            m_textRenderer->setFadeDistance(2000.0f);
        }
        
        BoxInfoRenderer::~BoxInfoRenderer() {
            delete m_textRenderer;
            m_textRenderer = NULL;
        }
        
        void BoxInfoRenderer::render(Vbo& vbo, RenderContext& context) {
            assert(m_textRenderer != NULL);
            
            if (!m_initialized) {
                const String labels[3] = {"X", "Y", "Z"};

                const Vec3f size = m_bounds.size().corrected();
                for (unsigned int i = 0; i < 3; i++) {
                    StringStream buffer;
                    buffer << labels[i] << ": " << size[i];
                    
                    BoxInfoSizeTextAnchor anchor(m_bounds, i, context.camera());
                    m_textRenderer->addString(i, buffer.str(), anchor);
                }
                m_initialized = true;
            }
            
            Preferences::PreferenceManager& prefs = Preferences::PreferenceManager::preferences();
            const Color& textColor = prefs.getColor(Preferences::InfoOverlayTextColor);
            const Color& backgroundColor = prefs.getColor(Preferences::InfoOverlayBackgroundColor);
            ShaderProgram& textShader = context.shaderManager().shaderProgram(Shaders::TextShader);
            ShaderProgram& backgroundShader = context.shaderManager().shaderProgram(Shaders::TextBackgroundShader);
            
            glDisable(GL_DEPTH_TEST);
            m_textRenderer->render(context, m_textFilter, textShader, textColor, backgroundShader, backgroundColor);
            glEnable(GL_DEPTH_TEST);
        }
    }
}
