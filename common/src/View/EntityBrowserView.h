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

#ifndef TrenchBroom_EntityBrowserView
#define TrenchBroom_EntityBrowserView

#include "VecMath.h"
#include "Assets/EntityDefinitionManager.h"
#include "Renderer/VertexSpec.h"
#include "View/CellView.h"
#include "View/ViewTypes.h"

namespace TrenchBroom {
    class Logger;

    namespace Assets {
        class EntityModelManager;
        class PointEntityDefinition;
    }
    
    namespace Renderer {
        class FontDescriptor;
        class TexturedIndexRangeRenderer;
        class Transformation;
    }
    
    namespace View {
        class GLContextManager;
        
        typedef String EntityGroupData;
        
        class EntityCellData {
        private:
            typedef Renderer::TexturedIndexRangeRenderer EntityRenderer;
        public:
            Assets::PointEntityDefinition* entityDefinition;
            EntityRenderer* modelRenderer;
            Renderer::FontDescriptor fontDescriptor;
            BBox3f bounds;
            
            EntityCellData(Assets::PointEntityDefinition* i_entityDefinition, EntityRenderer* i_modelRenderer, const Renderer::FontDescriptor& i_fontDescriptor, const BBox3f& i_bounds);
        };

        class EntityBrowserView : public CellView<EntityCellData, EntityGroupData> {
        private:
            typedef Renderer::TexturedIndexRangeRenderer EntityRenderer;
            
            typedef Renderer::VertexSpecs::P2T2C4::Vertex TextVertex;
            typedef std::map<Renderer::FontDescriptor, TextVertex::List> StringMap;

            Assets::EntityDefinitionManager& m_entityDefinitionManager;
            Assets::EntityModelManager& m_entityModelManager;
            Logger& m_logger;
            Quatf m_rotation;
            
            bool m_group;
            bool m_hideUnused;
            Assets::EntityDefinition::SortOrder m_sortOrder;
            String m_filterText;
        public:
            EntityBrowserView(wxWindow* parent,
                              wxScrollBar* scrollBar,
                              GLContextManager& contextManager,
                              Assets::EntityDefinitionManager& entityDefinitionManager,
                              Assets::EntityModelManager& entityModelManager,
                              Logger& logger);
            ~EntityBrowserView();
        public:
            void setSortOrder(Assets::EntityDefinition::SortOrder sortOrder);
            void setGroup(bool group);
            void setHideUnused(bool hideUnused);
            void setFilterText(const String& filterText);
        private:
            void doInitLayout(Layout& layout);
            void doReloadLayout(Layout& layout);

            bool dndEnabled();
            void dndWillStart();
            void dndDidEnd();
            wxString dndData(const Layout::Group::Row::Cell& cell);

            void addEntityToLayout(Layout& layout, Assets::PointEntityDefinition* definition, const Renderer::FontDescriptor& font);
            
            void doClear();
            void doRender(Layout& layout, float y, float height);
            bool doShouldRenderFocusIndicator() const;

            void renderBounds(Layout& layout, float y, float height);
            
            class MeshFunc;
            void renderModels(Layout& layout, float y, float height, Renderer::Transformation& transformation);
            
            void renderNames(Layout& layout, float y, float height, const Mat4x4f& projection);
            void renderGroupTitleBackgrounds(Layout& layout, float y, float height);
            void renderStrings(Layout& layout, float y, float height);
            StringMap collectStringVertices(Layout& layout, float y, float height);
            
            Mat4x4f itemTransformation(const Layout::Group::Row::Cell& cell, float y, float height) const;
            
            wxString tooltip(const Layout::Group::Row::Cell& cell);
        };
    }
}

#endif /* defined(TrenchBroom_EntityBrowserView) */
