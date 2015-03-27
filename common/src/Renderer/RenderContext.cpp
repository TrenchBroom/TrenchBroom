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

#include "RenderContext.h"
#include "Renderer/Camera.h"
#include "View/MapViewConfig.h"

namespace TrenchBroom {
    namespace Renderer {
        RenderContext::RenderContext(const RenderMode renderMode, const Camera& camera, FontManager& fontManager, ShaderManager& shaderManager) :
        m_renderMode(renderMode),
        m_camera(camera),
        m_transformation(m_camera.projectionMatrix(), m_camera.viewMatrix()),
        m_fontManager(fontManager),
        m_shaderManager(shaderManager),
        m_showTextures(true),
        m_showFaces(true),
        m_showEdges(true),
        m_shadeFaces(true),
        m_showPointEntities(true),
        m_showPointEntityModels(true),
        m_showEntityClassnames(true),
        m_showEntityBounds(true),
        m_showFog(false),
        m_showGrid(true),
        m_gridSize(4),
        m_hideSelection(false),
        m_tintSelection(true),
        m_showSelectionGuide(ShowSelectionGuide_Hide) {}
        
        bool RenderContext::render2D() const {
            return m_renderMode == RenderMode_2D;
        }
        
        bool RenderContext::render3D() const {
            return m_renderMode == RenderMode_3D;
        }

        const Camera& RenderContext::camera() const {
            return m_camera;
        }

        Transformation& RenderContext::transformation() {
            return m_transformation;
        }

        FontManager& RenderContext::fontManager() {
            return m_fontManager;
        }

        ShaderManager& RenderContext::shaderManager() {
            return m_shaderManager;
        }

        bool RenderContext::showTextures() const {
            return m_showTextures;
        }
        
        void RenderContext::setShowTextures(const bool showTextures) {
            m_showTextures = showTextures;
        }

        bool RenderContext::showFaces() const {
            return m_renderMode == RenderMode_3D && m_showFaces;
        }
        
        void RenderContext::setShowFaces(const bool showFaces) {
            m_showFaces = showFaces;
        }

        bool RenderContext::showEdges() const {
            return m_renderMode == RenderMode_2D || m_showEdges;
        }

        void RenderContext::setShowEdges(const bool showEdges) {
            m_showEdges = showEdges;
        }

        bool RenderContext::shadeFaces() const {
            return m_shadeFaces;
        }
        
        void RenderContext::setShadeFaces(const bool shadeFaces) {
            m_shadeFaces = shadeFaces;
        }

        bool RenderContext::showPointEntities() const {
            return m_showPointEntities;
        }
        
        void RenderContext::setShowPointEntities(const bool showPointEntities) {
            m_showPointEntities = showPointEntities;
        }

        bool RenderContext::showPointEntityModels() const {
            return m_showPointEntityModels;
        }
        
        void RenderContext::setShowPointEntityModels(const bool showPointEntityModels) {
            m_showPointEntityModels = showPointEntityModels;
        }

        bool RenderContext::showEntityClassnames() const {
            return m_showEntityClassnames;
        }

        void RenderContext::setShowEntityClassnames(const bool showEntityClassnames) {
            m_showEntityClassnames = showEntityClassnames;
        }

        bool RenderContext::showEntityBounds() const {
            return m_showEntityBounds;
        }

        void RenderContext::setShowEntityBounds(const bool showEntityBounds) {
            m_showEntityBounds = showEntityBounds;
        }

        bool RenderContext::showFog() const {
            return m_showFog;
        }
       
        void RenderContext::setShowFog(const bool showFog) {
            m_showFog = showFog;
        }

        bool RenderContext::showGrid() const {
            return m_showGrid;
        }
        
        void RenderContext::setShowGrid(const bool showGrid) {
            m_showGrid = showGrid;
        }
        
        size_t RenderContext::gridSize() const {
            return m_gridSize;
        }
        
        void RenderContext::setGridSize(const size_t gridSize) {
            m_gridSize = gridSize;
        }

        bool RenderContext::hideSelection() const {
            return m_hideSelection;
        }
        
        void RenderContext::setHideSelection() {
            m_hideSelection = true;
        }
        
        bool RenderContext::tintSelection() const {
            return m_tintSelection;
        }
        
        void RenderContext::clearTintSelection() {
            m_tintSelection = false;
        }
        
        bool RenderContext::showSelectionGuide() const {
            return m_showSelectionGuide == ShowSelectionGuide_Show || m_showSelectionGuide == ShowSelectionGuide_ForceShow;
        }
        
        void RenderContext::setShowSelectionGuide() {
            setShowSelectionGuide(ShowSelectionGuide_Show);
        }
        
        void RenderContext::setHideSelectionGuide() {
            setShowSelectionGuide(ShowSelectionGuide_Hide);
        }
        
        void RenderContext::setForceShowSelectionGuide() {
            setShowSelectionGuide(ShowSelectionGuide_ForceShow);
        }

        void RenderContext::setForceHideSelectionGuide() {
            setShowSelectionGuide(ShowSelectionGuide_ForceHide);
        }
        
        void RenderContext::setShowSelectionGuide(const ShowSelectionGuide showSelectionGuide) {
            switch (showSelectionGuide) {
                case ShowSelectionGuide_Show:
                    if (m_showSelectionGuide == ShowSelectionGuide_Hide)
                        m_showSelectionGuide = ShowSelectionGuide_Show;
                    break;
                case ShowSelectionGuide_Hide:
                    if (m_showSelectionGuide == ShowSelectionGuide_Show)
                        m_showSelectionGuide = ShowSelectionGuide_Hide;
                    break;
                case ShowSelectionGuide_ForceShow:
                    m_showSelectionGuide = ShowSelectionGuide_ForceShow;
                    break;
                case ShowSelectionGuide_ForceHide:
                    if (m_showSelectionGuide != ShowSelectionGuide_ForceShow)
                        m_showSelectionGuide = ShowSelectionGuide_ForceHide;
                    break;
            }
        }
    }
}
