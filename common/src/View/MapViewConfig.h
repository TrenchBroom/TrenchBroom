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

#ifndef TrenchBroom_MapViewConfig
#define TrenchBroom_MapViewConfig

#include "Notifier.h"

namespace TrenchBroom {
    namespace Model {
        class EditorContext;
    }

    namespace View {
        class MapViewConfig {
        public:
            typedef enum {
                FaceRenderMode_Textured,
                FaceRenderMode_Flat,
                FaceRenderMode_Skip
            } FaceRenderMode;
        private:
            const Model::EditorContext& m_editorContext;

            bool m_showEntityClassnames;
            bool m_showPointEntityModels;

            bool m_showGroupBounds;
            bool m_showBrushEntityBounds;
            bool m_showPointEntityBounds;

            FaceRenderMode m_faceRenderMode;
            bool m_shadeFaces;
            bool m_showFog;
            bool m_showEdges;
            bool m_showSoftMapBounds;
        public:
            Notifier<> mapViewConfigDidChangeNotifier;
        public:
            explicit MapViewConfig(const Model::EditorContext& editorContext);

            bool showEntityClassnames() const;
            void setShowEntityClassnames(bool showEntityClassnames);

            bool showPointEntities() const;

            bool showPointEntityModels() const;
            void setShowPointEntityModels(bool showPointEntityModels);

            bool showGroupBounds() const;
            void setShowGroupBounds(bool showGroupBounds);

            bool showBrushEntityBounds() const;
            void setShowBrushEntityBounds(bool showBrushEntityBounds);

            bool showPointEntityBounds() const;
            void setShowPointEntityBounds(bool showPointEntityBounds);

            bool showBrushes() const;

            bool showFaces() const;
            bool showTextures() const;
            FaceRenderMode faceRenderMode() const;
            void setFaceRenderMode(FaceRenderMode faceRenderMode);

            bool shadeFaces() const;
            void setShadeFaces(bool shadeFaces);

            bool showFog() const;
            void setShowFog(bool showFog);

            bool showEdges() const;
            void setShowEdges(bool showEdges);

            bool showSoftMapBounds() const;
            void setShowSoftMapBounds(bool showSoftMapBounds);
        private:
            MapViewConfig(const MapViewConfig& other);
            MapViewConfig& operator=(const MapViewConfig& other);
        };
    }
}

#endif /* defined(TrenchBroom_MapViewConfig) */
