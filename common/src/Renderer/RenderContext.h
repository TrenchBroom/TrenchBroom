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

#ifndef TrenchBroom_RenderContext
#define TrenchBroom_RenderContext

#include "Renderer/Transformation.h"
#include "Renderer/RenderBatch.h"

namespace TrenchBroom {
    namespace View {
        class MapViewConfig;
    }

    namespace Renderer {
        class Camera;
        class FontManager;
        class Renderable;
        class ShaderManager;
        
        class RenderContext {
        public:
            typedef enum {
                RenderMode_3D,
                RenderMode_2D
            } RenderMode;
        private:
            typedef enum {
                ShowSelectionGuide_Show,
                ShowSelectionGuide_Hide,
                ShowSelectionGuide_ForceShow,
                ShowSelectionGuide_ForceHide
            } ShowSelectionGuide;
            
            // general context for any rendering view
            RenderMode m_renderMode;
            const Camera& m_camera;
            Transformation m_transformation;
            FontManager& m_fontManager;
            ShaderManager& m_shaderManager;

            // settings for any map rendering view
            bool m_showTextures;
            bool m_showFaces;
            bool m_showEdges;
            bool m_shadeFaces;
            
            bool m_showPointEntities;
            bool m_showPointEntityModels;
            bool m_showEntityClassnames;
            bool m_showEntityBounds;
            
            bool m_showFog;
            
            bool m_showGrid;
            size_t m_gridSize;
            
            bool m_hideSelection;
            bool m_tintSelection;
            
            ShowSelectionGuide m_showSelectionGuide;
        public:
            RenderContext(RenderMode renderMode, const Camera& camera, FontManager& fontManager, ShaderManager& shaderManager);

            bool render2D() const;
            bool render3D() const;
            
            const Camera& camera() const;
            Transformation& transformation();
            FontManager& fontManager();
            ShaderManager& shaderManager();
            
            bool showTextures() const;
            void setShowTextures(bool showTextures);

            bool showFaces() const;
            void setShowFaces(bool showFaces);
            
            bool showEdges() const;
            void setShowEdges(bool showEdges);
            
            bool shadeFaces() const;
            void setShadeFaces(bool shadeFaces);
            
            bool showPointEntities() const;
            void setShowPointEntities(bool showPointEntities);
            
            bool showPointEntityModels() const;
            void setShowPointEntityModels(bool showPointEntityModels);
            
            bool showEntityClassnames() const;
            void setShowEntityClassnames(bool showEntityClassnames);
            
            bool showEntityBounds() const;
            void setShowEntityBounds(bool showEntityBounds);
            
            bool showFog() const;
            void setShowFog(bool showFog);
            
            bool showGrid() const;
            void setShowGrid(bool showGrid);
            
            size_t gridSize() const;
            void setGridSize(size_t gridSize);
            
            bool hideSelection() const;
            void setHideSelection();
            
            bool tintSelection() const;
            void clearTintSelection();
            
            bool showSelectionGuide() const;
            void setShowSelectionGuide();
            void setHideSelectionGuide();
            void setForceShowSelectionGuide();
            void setForceHideSelectionGuide();
        private:
            void setShowSelectionGuide(ShowSelectionGuide showSelectionGuide);
        };
    }
}

#endif /* defined(TrenchBroom_RenderContext) */
