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
        BrushRenderer::BrushRenderer(const Config config) :
        m_config(config) {}

        void BrushRenderer::update(const Model::BrushFace::Mesh& faces, const VertexSpecs::P3::Vertex::List& edges) {
            m_faceRenderer = FaceRenderer(faces, faceColor());
            m_edgeRenderer = EdgeRenderer(edges);
        }
        
        void BrushRenderer::update(const Model::BrushFace::Mesh& faces, const VertexSpecs::P3C4::Vertex::List& edges) {
            m_faceRenderer = FaceRenderer(faces, faceColor());
            m_edgeRenderer = EdgeRenderer(edges);
        }
        
        void BrushRenderer::render(RenderContext& context) {
            if (m_config == BRSelected)
                m_faceRenderer.render(context, grayScale(), tintColor());
            else
                m_faceRenderer.render(context, grayScale());
            
            glSetEdgeOffset(0.02f);
            if (m_config == BRSelected) {
                glDisable(GL_DEPTH_TEST);
                m_edgeRenderer.setColor(occludedEdgeColor());
                m_edgeRenderer.render(context);
                glEnable(GL_DEPTH_TEST);
            }
            m_edgeRenderer.setColor(edgeColor());
            m_edgeRenderer.render(context);
            glResetEdgeOffset();
        }

        bool BrushRenderer::grayScale() const {
            return false;
        }
        
        const Color& BrushRenderer::faceColor() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getColor(Preferences::FaceColor);
        }

        const Color& BrushRenderer::tintColor() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            return prefs.getColor(Preferences::SelectedFaceColor);
        }

        const Color& BrushRenderer::edgeColor() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            switch (m_config) {
                case BRSelected:
                    return prefs.getColor(Preferences::SelectedEdgeColor);
                case BRUnselected:
                default:
                    return prefs.getColor(Preferences::EdgeColor);
            }
        }

        const Color& BrushRenderer::occludedEdgeColor() const {
            PreferenceManager& prefs = PreferenceManager::instance();
            switch (m_config) {
                case BRSelected:
                    return prefs.getColor(Preferences::OccludedSelectedEdgeColor);
                case BRUnselected:
                default:
                    return prefs.getColor(Preferences::EdgeColor);
            }
        }
    }
}
