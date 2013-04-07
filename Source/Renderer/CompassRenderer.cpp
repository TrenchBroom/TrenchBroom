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

#include "CompassRenderer.h"

#include "Renderer/IndexedVertexArray.h"
#include "Renderer/RenderContext.h"
#include "Renderer/RenderUtils.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexArray.h"
#include "Utility/VecMath.h"

#include <algorithm>
#include <cassert>

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Renderer {
        const float CompassRenderer::m_shaftLength = 32.0f;
        const float CompassRenderer::m_shaftRadius = 3.0f;
        const float CompassRenderer::m_headLength = 12.0f;
        const float CompassRenderer::m_headRadius = 6.0f;

        CompassRenderer::CompassRenderer() :
        m_fans(NULL),
        m_strip(NULL) {}
        
        CompassRenderer::~CompassRenderer() {
            delete m_fans;
            m_fans = NULL;
            delete m_strip;
            m_strip = NULL;
        }

        void CompassRenderer::render(Vbo& vbo, RenderContext& context) {
            SetVboState activateVbo(vbo, Vbo::VboActive);
            if (m_fans == NULL) {
                assert(m_strip == NULL);

                Vec3f::List head;
                Vec3f::List shaft;
                Vec3f::List topCap;
                Vec3f::List bottomCap;
                
                cylinder(m_shaftLength, m_shaftRadius, m_segments, shaft);
                for (size_t i = 0; i < shaft.size(); i++)
                    shaft[i].z -= m_shaftLength / 2.0f;
                
                cone(m_headLength, m_headRadius, m_segments, head);
                for (size_t i = 0; i < head.size(); i++)
                    head[i].z += m_shaftLength / 2.0f;
                
                circle(m_headRadius, m_segments, topCap);
                std::reverse(topCap.begin(), topCap.end());
                for (size_t i = 0; i < topCap.size(); i++)
                    topCap[i].z += m_shaftLength / 2.0f;

                circle(m_shaftRadius, m_segments, bottomCap);
                std::reverse(bottomCap.begin(), bottomCap.end());
                for (size_t i = 0; i < bottomCap.size(); i++)
                    bottomCap[i].z -= m_shaftLength / 2.0f;
                
                m_strip = new VertexArray(vbo, GL_TRIANGLE_FAN, static_cast<unsigned int>(shaft.size()),
                                          Attribute::position3f(),
                                          0);
                m_strip->addAttributes(shaft);

                m_fans = new IndexedVertexArray(vbo, GL_TRIANGLE_STRIP, static_cast<unsigned int>(head.size() + topCap.size() + bottomCap.size()),
                                                Attribute::position3f(),
                                                0);
                SetVboState mapVbo(vbo, Vbo::VboMapped);
                m_fans->addAttributes(head);
                m_fans->endPrimitive();
                m_fans->addAttributes(topCap);
                m_fans->endPrimitive();
                m_fans->addAttributes(bottomCap);
                m_fans->endPrimitive();
            }
            
            // activate shader
            // set color
            // render Z
            // set color
            // set rotation matrix
            // render X
            // set color
            // set rotation matrix
            // render Y
        }
    }
}
