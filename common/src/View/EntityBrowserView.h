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

#ifndef __TrenchBroom__EntityBrowserView__
#define __TrenchBroom__EntityBrowserView__

#include "VecMath.h"
#include "Assets/EntityDefinitionManager.h"
#include "Renderer/Vbo.h"
#include "Renderer/VertexSpec.h"
#include "View/CellView.h"
#include "View/GLContextHolder.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class EntityModelManager;
        class PointEntityDefinition;
    }
    
    namespace Renderer {
        class FontDescriptor;
        class MeshRenderer;
        class Transformation;
    }
    
    namespace View {
        typedef String EntityGroupData;
        
        class EntityCellData {
        public:
            Assets::PointEntityDefinition* entityDefinition;
            Renderer::MeshRenderer* modelRenderer;
            Renderer::FontDescriptor fontDescriptor;
            BBox3f bounds;
            
            EntityCellData(Assets::PointEntityDefinition* i_entityDefinition, Renderer::MeshRenderer* i_modelRenderer, const Renderer::FontDescriptor& i_fontDescriptor, const BBox3f& i_bounds);
        };

        class EntityBrowserView : public CellView<EntityCellData, EntityGroupData> {
        private:
            typedef Renderer::VertexSpecs::P2T2::Vertex StringVertex;
            typedef std::map<Renderer::FontDescriptor, StringVertex::List> StringMap;

            Assets::EntityDefinitionManager& m_entityDefinitionManager;
            Assets::EntityModelManager& m_entityModelManager;
            Logger& m_logger;
            Quatf m_rotation;
            
            bool m_group;
            bool m_hideUnused;
            Assets::EntityDefinitionManager::SortOrder m_sortOrder;
            String m_filterText;
            
            Renderer::Vbo m_vbo;
        public:
            EntityBrowserView(wxWindow* parent,
                              wxScrollBar* scrollBar,
                              GLContextHolder::Ptr sharedContext,
                              Assets::EntityDefinitionManager& entityDefinitionManager,
                              Assets::EntityModelManager& entityModelManager,
                              Logger& logger);
            ~EntityBrowserView();
            
            void setSortOrder(const Assets::EntityDefinitionManager::SortOrder sortOrder);
            void setGroup(const bool group);
            void setHideUnused(const bool hideUnused);
            void setFilterText(const String& filterText);
        private:
            void doInitLayout(Layout& layout);
            void doReloadLayout(Layout& layout);

            bool dndEnabled();
            wxString dndData(const Layout::Group::Row::Cell& cell);

            void addEntityToLayout(Layout& layout, Assets::PointEntityDefinition* definition, const Renderer::FontDescriptor& font);
            
            void doClear();
            void doRender(Layout& layout, const float y, const float height);

            void renderBounds(Layout& layout, const float y, const float height);
            void renderModels(Layout& layout, const float y, const float height, Renderer::Transformation& transformation);
            void renderNames(Layout& layout, const float y, const float height, const Mat4x4f& projection);
            void renderGroupTitleBackgrounds(Layout& layout, const float y, const float height);
            void renderStrings(Layout& layout, const float y, const float height);
            StringMap collectStringVertices(Layout& layout, const float y, const float height);
            
            Mat4x4f itemTransformation(const Layout::Group::Row::Cell& cell, const float y, const float height) const;
            
            wxString tooltip(const typename Layout::Group::Row::Cell& cell);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityBrowserView__) */
