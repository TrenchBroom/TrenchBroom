/*
 Copyright (C) 2010-2017 Kristian Duske

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

#include "MapViewConfig.h"

#include "Model/EditorContext.h"

namespace TrenchBroom {
    namespace View {
        MapViewConfig::MapViewConfig(const Model::EditorContext& editorContext) :
        m_editorContext(editorContext),
        m_showEntityClassnames(true),
        m_showPointEntityModels(true),
        m_showGroupBounds(true),
        m_showBrushEntityBounds(true),
        m_showPointEntityBounds(true),
        m_faceRenderMode(FaceRenderMode_Textured),
        m_shadeFaces(true),
        m_showFog(false),
        m_showEdges(true),
        m_showSoftMapBounds(true) {}

        bool MapViewConfig::showEntityClassnames() const {
            return m_showEntityClassnames;
        }

        void MapViewConfig::setShowEntityClassnames(const bool showEntityClassnames) {
            if (showEntityClassnames == m_showEntityClassnames)
                return;
            m_showEntityClassnames = showEntityClassnames;
            mapViewConfigDidChangeNotifier();
        }

        bool MapViewConfig::showPointEntities() const {
            return m_editorContext.showPointEntities();
        }

        bool MapViewConfig::showPointEntityModels() const {
            return m_showPointEntityModels;
        }

        void MapViewConfig::setShowPointEntityModels(const bool showPointEntityModels) {
            if (showPointEntityModels == m_showPointEntityModels)
                return;
            m_showPointEntityModels = showPointEntityModels;
            mapViewConfigDidChangeNotifier();
        }

        bool MapViewConfig::showGroupBounds() const {
            return m_showGroupBounds;
        }

        void MapViewConfig::setShowGroupBounds(const bool showGroupBounds) {
            if (showGroupBounds == m_showGroupBounds) {
                return;
            }
            m_showGroupBounds = showGroupBounds;
            mapViewConfigDidChangeNotifier();
        }

        bool MapViewConfig::showBrushEntityBounds() const {
            return m_showBrushEntityBounds;
        }

        void MapViewConfig::setShowBrushEntityBounds(const bool showBrushEntityBounds) {
            if (showBrushEntityBounds == m_showBrushEntityBounds) {
                return;
            }
            m_showBrushEntityBounds = showBrushEntityBounds;
            mapViewConfigDidChangeNotifier();
        }

        bool MapViewConfig::showPointEntityBounds() const {
            return m_showPointEntityBounds;
        }

        void MapViewConfig::setShowPointEntityBounds(const bool showPointEntityBounds) {
            if (showPointEntityBounds == m_showPointEntityBounds) {
                return;
            }
            m_showPointEntityBounds = showPointEntityBounds;
            mapViewConfigDidChangeNotifier();
        }

        bool MapViewConfig::showBrushes() const {
            return m_editorContext.showBrushes();
        }

        bool MapViewConfig::showFaces() const {
            return m_faceRenderMode != FaceRenderMode_Skip;
        }

        bool MapViewConfig::showTextures() const {
            return m_faceRenderMode == FaceRenderMode_Textured;
        }

        MapViewConfig::FaceRenderMode MapViewConfig::faceRenderMode() const {
            return m_faceRenderMode;
        }

        void MapViewConfig::setFaceRenderMode(const FaceRenderMode faceRenderMode) {
            if (faceRenderMode == m_faceRenderMode)
                return;
            m_faceRenderMode = faceRenderMode;
            mapViewConfigDidChangeNotifier();
        }

        bool MapViewConfig::shadeFaces() const {
            return m_shadeFaces;
        }

        void MapViewConfig::setShadeFaces(const bool shadeFaces) {
            if (shadeFaces == m_shadeFaces)
                return;
            m_shadeFaces = shadeFaces;
            mapViewConfigDidChangeNotifier();
        }


        bool MapViewConfig::showFog() const {
            return m_showFog;
        }

        void MapViewConfig::setShowFog(const bool showFog) {
            if (showFog == m_showFog)
                return;
            m_showFog = showFog;
            mapViewConfigDidChangeNotifier();
        }

        bool MapViewConfig::showEdges() const {
            return m_showEdges;
        }

        void MapViewConfig::setShowEdges(const bool showEdges) {
            if (showEdges == m_showEdges)
                return;
            m_showEdges = showEdges;
            mapViewConfigDidChangeNotifier();
        }

        bool MapViewConfig::showSoftMapBounds() const {
            return m_showSoftMapBounds;
        }

        void MapViewConfig::setShowSoftMapBounds(const bool showSoftMapBounds) {
            if (showSoftMapBounds == m_showSoftMapBounds) {
                return;
            }
            m_showSoftMapBounds = showSoftMapBounds;
            mapViewConfigDidChangeNotifier();
        }
    }
}
