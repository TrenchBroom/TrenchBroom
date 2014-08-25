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

namespace TrenchBroom {
    namespace Renderer {
        RenderContext::RenderContext(const Camera& camera, ShaderManager& shaderManager, const RenderConfig& renderConfig, const bool showGrid, const size_t gridSize) :
        m_camera(camera),
        m_transformation(m_camera.projectionMatrix(), m_camera.viewMatrix()),
        m_shaderManager(shaderManager),
        m_renderConfig(renderConfig),
        m_showGrid(showGrid),
        m_gridSize(gridSize),
        m_hideSelection(false),
        m_tintSelection(true),
        m_showSelectionGuide(ShowSelectionGuide_Hide),
        m_showMouseIndicators(true) {}
        
        const Camera& RenderContext::camera() const {
            return m_camera;
        }

        Transformation& RenderContext::transformation() {
            return m_transformation;
        }

        ShaderManager& RenderContext::shaderManager() {
            return m_shaderManager;
        }

        const RenderConfig& RenderContext::renderConfig() const {
            return m_renderConfig;
        }

        bool RenderContext::showGrid() const {
            return m_showGrid;
        }
        
        size_t RenderContext::gridSize() const {
            return m_gridSize;
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

        bool RenderContext::showMouseIndicators() const {
            return m_showMouseIndicators;
        }
        
        void RenderContext::setHideMouseIndicators() {
            m_showMouseIndicators = false;
        }
    }
}
