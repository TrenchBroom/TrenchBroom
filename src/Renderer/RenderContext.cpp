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
        RenderContext::RenderContext(const Camera& camera, ShaderManager& shaderManager, const bool gridVisible, const size_t gridSize) :
        m_camera(camera),
        m_transformation(m_camera.projectionMatrix(), m_camera.viewMatrix()),
        m_shaderManager(shaderManager),
        m_gridVisible(gridVisible),
        m_gridSize(gridSize),
        m_hideSelection(false),
        m_tintSelection(true),
        m_showSelectionGuide(Hide) {}
        
        const Camera& RenderContext::camera() const {
            return m_camera;
        }

        Transformation& RenderContext::transformation() {
            return m_transformation;
        }

        ShaderManager& RenderContext::shaderManager() {
            return m_shaderManager;
        }

        bool RenderContext::gridVisible() const {
            return m_gridVisible;
        }
        
        size_t RenderContext::gridSize() const {
            return m_gridSize;
        }

        bool RenderContext::tintSelection() const {
            return m_tintSelection;
        }
        
        void RenderContext::clearTintSelection() {
            m_tintSelection = false;
        }

        bool RenderContext::hideSelection() const {
            return m_hideSelection;
        }
        
        void RenderContext::setHideSelection() {
            m_hideSelection = true;
        }

        
        bool RenderContext::showSelectionGuide() const {
            return m_showSelectionGuide == Show || m_showSelectionGuide == ForceShow;
        }
        
        void RenderContext::setShowSelectionGuide() {
            setShowSelectionGuide(Show);
        }
        
        void RenderContext::setHideSelectionGuide() {
            setShowSelectionGuide(Hide);
        }
        
        void RenderContext::setForceShowSelectionGuide() {
            setShowSelectionGuide(ForceShow);
        }

        void RenderContext::setForceHideSelectionGuide() {
            setShowSelectionGuide(ForceHide);
        }
        
        void RenderContext::setShowSelectionGuide(const ShowSelectionGuide showSelectionGuide) {
            switch (showSelectionGuide) {
                case Show:
                    if (m_showSelectionGuide == Hide)
                        m_showSelectionGuide = Show;
                    break;
                case Hide:
                    if (m_showSelectionGuide == Show)
                        m_showSelectionGuide = Hide;
                    break;
                case ForceShow:
                    m_showSelectionGuide = ForceShow;
                    break;
                case ForceHide:
                    if (m_showSelectionGuide != ForceShow)
                        m_showSelectionGuide = ForceHide;
                    break;
            }
        }
    }
}
