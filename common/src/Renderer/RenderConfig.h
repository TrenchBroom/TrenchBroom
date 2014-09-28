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

#ifndef __TrenchBroom__RenderConfig__
#define __TrenchBroom__RenderConfig__

#include "Notifier.h"

namespace TrenchBroom {
    namespace View {
        class EditorContext;
    }
    
    namespace Renderer {
        class RenderConfig {
        public:
            typedef enum {
                FaceRenderMode_Textured,
                FaceRenderMode_Flat,
                FaceRenderMode_Skip
            } FaceRenderMode;
        private:
            const View::EditorContext& m_editorContext;
            
            bool m_showEntityClassnames;
            bool m_showPointEntityModels;
            bool m_showEntityBounds;
            
            FaceRenderMode m_faceRenderMode;
            bool m_shadeFaces;
            bool m_useFog;
            bool m_showEdges;
        public:
            Notifier0 renderConfigDidChangeNotifier;
        public:
            RenderConfig(const View::EditorContext& editorContext);
            
            bool showEntityClassnames() const;
            void setShowEntityClassnames(bool showEntityClassnames);
            
            bool showPointEntities() const;
            
            bool showPointEntityModels() const;
            void setShowPointEntityModels(bool showPointEntityModels);
            
            bool showEntityBounds() const;
            void setShowEntityBounds(bool showEntityBounds);
            
            bool showBrushes() const;
            
            bool showFaces() const;
            bool showTextures() const;
            FaceRenderMode faceRenderMode() const;
            void setFaceRenderMode(FaceRenderMode faceRenderMode);
            
            bool shadeFaces() const;
            void setShadeFaces(bool shadeFaces);
            
            bool useFog() const;
            void setUseFog(bool useFog);
            
            bool showEdges() const;
            void setShowEdges(bool showEdges);
        private:
            RenderConfig(const RenderConfig& other);
            RenderConfig& operator=(const RenderConfig& other);
        };
    }
}

#endif /* defined(__TrenchBroom__RenderConfig__) */
