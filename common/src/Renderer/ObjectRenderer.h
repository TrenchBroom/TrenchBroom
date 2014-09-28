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

#ifndef __TrenchBroom__ObjectRenderer__
#define __TrenchBroom__ObjectRenderer__

#include "Color.h"
#include "Model/ModelTypes.h"
#include "Renderer/BrushRenderer.h"

namespace TrenchBroom {
    namespace Renderer {
        class RenderContext;
        
        class ObjectRenderer {
        private:
            BrushRenderer m_brushRenderer;
        public:
            template <typename BrushFilterT>
            ObjectRenderer(const BrushFilterT& brushFilter) :
            m_brushRenderer(brushFilter) {}
        public: // object management
            void addObjects(const Model::NodeList& nodes);
            void addObject(Model::Node* object);
            
            void removeObjects(const Model::NodeList& nodes);
            void removeObject(Model::Node* object);

            void clear();
        public: // configuration
            void setTint(bool tint);
            void setTintColor(const Color& tintColor);
            
            void setRenderOccludedEdges(bool renderOccludedEdges);
            void setTransparencyAlpha(float transparencyAlpha);
            
            void setBrushFaceColor(const Color& brushFaceColor);
            void setBrushEdgeColor(const Color& brushEdgeColor);
            void setOccludedBrushEdgeColor(const Color& occludedEdgeColor);
            
            void setShowHiddenBrushes(bool showHiddenBrushes);
        public: // rendering
            void render(RenderContext& renderContext);
        private:
            ObjectRenderer(const ObjectRenderer&);
            ObjectRenderer& operator=(const ObjectRenderer&);
        };
    }
}

#endif /* defined(__TrenchBroom__ObjectRenderer__) */
