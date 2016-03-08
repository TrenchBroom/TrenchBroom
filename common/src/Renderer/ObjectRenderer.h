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

#ifndef TrenchBroom_ObjectRenderer
#define TrenchBroom_ObjectRenderer

#include "Color.h"
#include "Model/ModelTypes.h"
#include "Renderer/BrushRenderer.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/GroupRenderer.h"

namespace TrenchBroom {
    namespace Assets {
        class EntityModelManager;
    }
    
    namespace Renderer {
        class FontManager;
        class RenderBatch;
        
        class ObjectRenderer {
        private:
            GroupRenderer m_groupRenderer;
            EntityRenderer m_entityRenderer;
            BrushRenderer m_brushRenderer;
        public:
            template <typename BrushFilterT>
            ObjectRenderer(Assets::EntityModelManager& entityModelManager, const Model::EditorContext& editorContext, const BrushFilterT& brushFilter) :
            m_groupRenderer(editorContext),
            m_entityRenderer(entityModelManager, editorContext),
            m_brushRenderer(brushFilter) {}
        public: // object management
            void setObjects(const Model::GroupList& groups, const Model::EntityList& entities, const Model::BrushList& brushes);
            void invalidate();
            void clear();
            void reloadModels();
        public: // configuration
            void setShowOverlays(bool showOverlays);
            void setOverlayTextColor(const Color& overlayTextColor);
            void setOverlayBackgroundColor(const Color& overlayBackgroundColor);
            
            void setTint(bool tint);
            void setTintColor(const Color& tintColor);
            
            void setShowOccludedObjects(bool showOccludedObjects);
            void setOccludedEdgeColor(const Color& occludedEdgeColor);

            void setTransparencyAlpha(float transparencyAlpha);
            
            void setShowEntityAngles(bool showAngles);
            void setEntityAngleColor(const Color& color);

            void setOverrideGroupBoundsColor(bool overrideGroupBoundsColor);
            void setGroupBoundsColor(const Color& color);
            
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

#endif /* defined(TrenchBroom_ObjectRenderer) */
