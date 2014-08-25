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

#include "RenderConfig.h"

#include "Model/ModelFilter.h"

namespace TrenchBroom {
    namespace Renderer {
        RenderConfig::RenderConfig(const Model::ModelFilter& modelFilter) :
        m_modelFilter(modelFilter),
        m_showEntityClassnames(true),
        m_showPointEntityModels(true),
        m_showEntityBounds(true),
        m_faceRenderMode(FaceRenderMode_Textured),
        m_shadeFaces(true),
        m_useFog(false),
        m_showEdges(true) {}

        bool RenderConfig::showEntityClassnames() const {
            return m_showEntityClassnames;
        }
        
        void RenderConfig::setShowEntityClassnames(const bool showEntityClassnames) {
            if (showEntityClassnames == m_showEntityClassnames)
                return;
            m_showEntityClassnames = showEntityClassnames;
            renderConfigDidChangeNotifier();
        }
        
        bool RenderConfig::showPointEntities() const {
            return m_modelFilter.showPointEntities();
        }
        
        bool RenderConfig::showPointEntityModels() const {
            return m_showPointEntityModels;
        }
        
        void RenderConfig::setShowPointEntityModels(const bool showPointEntityModels) {
            if (showPointEntityModels == m_showPointEntityModels)
                return;
            m_showPointEntityModels = showPointEntityModels;
            renderConfigDidChangeNotifier();
        }
        
        bool RenderConfig::showEntityBounds() const {
            return m_showEntityBounds;
        }
        
        void RenderConfig::setShowEntityBounds(const bool showEntityBounds) {
            if (showEntityBounds == m_showEntityBounds)
                return;
            m_showEntityBounds = showEntityBounds;
            renderConfigDidChangeNotifier();
        }
        
        bool RenderConfig::showBrushes() const {
            return m_modelFilter.showBrushes();
        }
        
        bool RenderConfig::showFaces() const {
            return m_faceRenderMode != FaceRenderMode_Skip;
        }

        bool RenderConfig::showTextures() const {
            return m_faceRenderMode == FaceRenderMode_Textured;
        }

        RenderConfig::FaceRenderMode RenderConfig::faceRenderMode() const {
            return m_faceRenderMode;
        }
        
        void RenderConfig::setFaceRenderMode(const FaceRenderMode faceRenderMode) {
            if (faceRenderMode == m_faceRenderMode)
                return;
            m_faceRenderMode = faceRenderMode;
            renderConfigDidChangeNotifier();
        }
        
        bool RenderConfig::shadeFaces() const {
            return m_shadeFaces;
        }
        
        void RenderConfig::setShadeFaces(const bool shadeFaces) {
            if (shadeFaces == m_shadeFaces)
                return;
            m_shadeFaces = shadeFaces;
            renderConfigDidChangeNotifier();
        }
        
        
        bool RenderConfig::useFog() const {
            return m_useFog;
        }
        
        void RenderConfig::setUseFog(const bool useFog) {
            if (useFog == m_useFog)
                return;
            m_useFog = useFog;
            renderConfigDidChangeNotifier();
        }
        
        bool RenderConfig::showEdges() const {
            return m_showEdges;
        }
        
        void RenderConfig::setShowEdges(const bool showEdges) {
            if (showEdges == m_showEdges)
                return;
            m_showEdges = showEdges;
            renderConfigDidChangeNotifier();
        }
    }
}
