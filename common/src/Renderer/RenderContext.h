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

#ifndef __TrenchBroom__RenderContext__
#define __TrenchBroom__RenderContext__

#include "Renderer/Transformation.h"

namespace TrenchBroom {
    namespace Renderer {
        class Camera;
        class ShaderManager;
        
        class RenderContext {
        private:
            typedef enum {
                ShowSelectionGuide_Show,
                ShowSelectionGuide_Hide,
                ShowSelectionGuide_ForceShow,
                ShowSelectionGuide_ForceHide
            } ShowSelectionGuide;
            
            const Camera& m_camera;
            Transformation m_transformation;
            ShaderManager& m_shaderManager;
            
            bool m_gridVisible;
            size_t m_gridSize;
            bool m_hideSelection;
            bool m_tintSelection;
            ShowSelectionGuide m_showSelectionGuide;
        public:
            RenderContext(const Camera& camera, ShaderManager& shaderManager, const bool gridVisible, const size_t gridSize);
            
            const Camera& camera() const;
            Transformation& transformation();
            ShaderManager& shaderManager();
            bool gridVisible() const;
            size_t gridSize() const;
            
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

#endif /* defined(__TrenchBroom__RenderContext__) */
