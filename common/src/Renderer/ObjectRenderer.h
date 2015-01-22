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
#include "Renderer/EntityRenderer.h"

namespace TrenchBroom {
    namespace Assets {
        class EntityModelManager;
    }
    
    namespace Renderer {
        class FontManager;
        class RenderBatch;
        
        class ObjectRenderer {
        private:
            EntityRenderer m_entityRenderer;
            BrushRenderer m_brushRenderer;
        public:
            template <typename BrushFilterT>
            ObjectRenderer(Assets::EntityModelManager& entityModelManager, const Model::EditorContext& editorContext, const BrushFilterT& brushFilter) :
            m_entityRenderer(entityModelManager, editorContext),
            m_brushRenderer(brushFilter) {}
        public: // object management
            void addObjects(const Model::NodeList& nodes);
            void addObject(Model::Node* object);
            
            void removeObjects(const Model::NodeList& nodes);
            void removeObject(Model::Node* object);

            void updateObjects(const Model::NodeList& nodes);
            void updateObject(Model::Node* object);
            
            void updateBrushFaces(const Model::BrushFaceList& faces);
            
            void invalidate();
            void clear();
        public: // configuration
            void setOverlayTextColor(const Color& overlayTextColor);
            void setOverlayBackgroundColor(const Color& overlayBackgroundColor);
            
            void setTint(bool tint);
            void setTintColor(const Color& tintColor);
            
            void setShowOccludedObjects(bool showOccludedObjects);
            void setOccludedEdgeColor(const Color& occludedEdgeColor);

            void setTransparencyAlpha(float transparencyAlpha);
            
            void setShowEntityAngles(bool showAngles);
            void setEntityAngleColor(const Color& color);

            void setOverrideEntityBoundsColor(bool overrideEntityBoundsColor);
            void setEntityBoundsColor(const Color& color);
            
            void setBrushFaceColor(const Color& brushFaceColor);
            void setBrushEdgeColor(const Color& brushEdgeColor);
            
            void setShowHiddenObjects(bool showHiddenObjects);
        public: // rendering
            void render(RenderContext& renderContext, RenderBatch& renderBatch);
        private:
            ObjectRenderer(const ObjectRenderer&);
            ObjectRenderer& operator=(const ObjectRenderer&);
        };
    }
}

#endif /* defined(__TrenchBroom__ObjectRenderer__) */
