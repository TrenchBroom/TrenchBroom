/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "BrushRenderer.h"

#include "Preferences.h"
#include "PreferenceManager.h"
#include "Renderer/RenderUtils.h"

namespace TrenchBroom {
    namespace Renderer {
        BrushRenderer::BrushRenderer() :
        m_grayScale(false) {}
        
        BrushRenderer::BrushRenderer(Vbo& vbo, const Model::BrushFace::Mesh& faces, const VertexSpecs::P3::Vertex::List& edges) :
        m_grayScale(false),
        m_faceRenderer(vbo, faces, faceColor()),
        m_edgeRenderer(vbo, edges, edgeColor()) {}
        
        BrushRenderer::BrushRenderer(Vbo& vbo, const Model::BrushFace::Mesh& faces, const VertexSpecs::P3C4::Vertex::List& edges) :
        m_grayScale(false),
        m_faceRenderer(vbo, faces, faceColor()),
        m_edgeRenderer(vbo, edges) {}
        
        void BrushRenderer::setGrayScale(const bool grayScale) {
            m_grayScale = grayScale;
        }

        void BrushRenderer::render(RenderContext& context) {
            m_faceRenderer.render(context, m_grayScale);
            glSetEdgeOffset(0.02f);
            m_edgeRenderer.render(context);
            glResetEdgeOffset();
        }

        Color BrushRenderer::faceColor() {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getColor(Preferences::FaceColor);
        }

        Color BrushRenderer::edgeColor() {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getColor(Preferences::EdgeColor);
        }
    }
}
