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

#include "OverlayRenderer.h"

#include "Renderer/ApplyMatrix.h"
#include "Renderer/CompassRenderer.h"
#include "Renderer/RenderContext.h"
#include "Renderer/Vbo.h"
#include "Utility/VecMath.h"

using namespace TrenchBroom::VecMath;

namespace TrenchBroom {
    namespace Renderer {
        OverlayRenderer::OverlayRenderer() :
        m_vbo(NULL),
        m_compass(NULL) {}

        OverlayRenderer::~OverlayRenderer() {
            delete m_compass;
            m_compass = NULL;
            delete m_vbo;
            m_vbo = NULL;
        }

        void OverlayRenderer::render(RenderContext& context, const float viewWidth, const float viewHeight) {
            if (m_vbo == NULL)
                m_vbo = new Vbo(GL_ARRAY_BUFFER, 0xFFFF);
            if (m_compass == NULL)
                m_compass = new CompassRenderer();

            glClear(GL_DEPTH_BUFFER_BIT);

            Mat4f projection, view;
            setOrtho(projection, 0.0f, 1000.0f, -viewWidth / 2.0f, viewHeight / 2.0f, viewWidth / 2.0f, -viewHeight / 2.0f);
            setView(view, Vec3f::PosY, Vec3f::PosZ);
            translate(view, 500.0f * Vec3f::PosY);
            Renderer::ApplyTransformation ortho(context.transformation(), projection, view);

            Mat4f compassTransformation = Mat4f::Identity;
            translate(compassTransformation, Vec3f(-viewWidth / 2.0f + 50.0f, 0.0f, -viewHeight / 2.0f + 50.0f));
            scale(compassTransformation, 2.0f);
            Renderer::ApplyModelMatrix applyCompassTranslate(context.transformation(), compassTransformation);

            m_compass->render(*m_vbo, context);
        }
    }
}
