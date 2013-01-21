/*
 Copyright (C) 2010-2012 Kristian Duske

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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__EntityBrowserCanvas__
#define __TrenchBroom__EntityBrowserCanvas__

#include "Model/EntityDefinitionManager.h"
#include "Renderer/OffscreenRenderer.h"
#include "Renderer/Shader/Shader.h"
#include "Renderer/Text/StringManager.h"
#include "Utility/String.h"
#include "Utility/VecMath.h"
#include "View/CellLayoutGLCanvas.h"

using namespace TrenchBroom::Math;

namespace TrenchBroom {
    namespace Model {
        class PointEntityDefinition;
    }

    namespace Renderer {
        class EntityModelRenderer;
        class ShaderProgram;
    }

    namespace View {
        class DocumentViewHolder;

        class EntityGroupData {
        public:
            String groupName;
            Renderer::Text::StringRendererPtr stringRenderer;

            EntityGroupData(const String& i_groupName, Renderer::Text::StringRendererPtr i_stringRenderer) :
            groupName(i_groupName),
            stringRenderer(i_stringRenderer) {}

            EntityGroupData() :
            groupName(""),
            stringRenderer(NULL) {}
        };

        class EntityCellData {
        public:
            Model::PointEntityDefinition* entityDefinition;
            Renderer::EntityModelRenderer* modelRenderer;
            Renderer::Text::StringRendererPtr stringRenderer;
            BBox bounds;

            EntityCellData(Model::PointEntityDefinition* i_entityDefinition, Renderer::EntityModelRenderer* i_modelRenderer, Renderer::Text::StringRendererPtr i_stringRenderer, const BBox& i_bounds) :
            entityDefinition(i_entityDefinition),
            modelRenderer(i_modelRenderer),
            stringRenderer(i_stringRenderer),
            bounds(i_bounds) {}
        };

        class EntityBrowserCanvas : public CellLayoutGLCanvas<EntityCellData, EntityGroupData> {
        protected:
            DocumentViewHolder& m_documentViewHolder;
            Renderer::OffscreenRenderer m_offscreenRenderer;
            Quat m_rotation;

            typedef std::map<Model::PointEntityDefinition*, Renderer::Text::StringRendererPtr> StringRendererCache;
            typedef std::pair<Model::PointEntityDefinition*, Renderer::Text::StringRendererPtr> StringRendererCacheEntry;
            StringRendererCache m_stringRendererCache;

            bool m_group;
            bool m_hideUnused;
            Model::EntityDefinitionManager::SortOrder m_sortOrder;
            String m_filterText;

            void addEntityToLayout(Layout& layout, Model::PointEntityDefinition* definition, const Renderer::Text::FontDescriptor& font);
            void renderEntityBounds(Renderer::Transformation& transformation, Renderer::ShaderProgram& boundsProgram, const Model::PointEntityDefinition& definition, const BBox& rotatedBounds, const Vec3f& offset, float scale);
            void renderEntityModel(Renderer::Transformation& transformation, Renderer::ShaderProgram& entityModelProgram, Renderer::EntityModelRenderer& renderer, const BBox& rotatedBounds, const Vec3f& offset, float scale);

            virtual void doInitLayout(Layout& layout);
            virtual void doReloadLayout(Layout& layout);
            virtual void doClear();
            virtual void doRender(Layout& layout, float y, float height);
            virtual bool dndEnabled();
            virtual wxImage* dndImage(const Layout::Group::Row::Cell& cell);
            virtual wxDataObject* dndData(const Layout::Group::Row::Cell& cell);
        public:
            EntityBrowserCanvas(wxWindow* parent, wxWindowID windowId, wxScrollBar* scrollBar, DocumentViewHolder& documentViewHolder);
            ~EntityBrowserCanvas();

            inline void setSortOrder(Model::EntityDefinitionManager::SortOrder sortOrder) {
                if (sortOrder == m_sortOrder)
                    return;
                m_sortOrder = sortOrder;
                reload();
                Refresh();
            }

            inline void setGroup(bool group) {
                if (group == m_group)
                    return;
                m_group = group;
                reload();
                Refresh();
            }

            inline void setHideUnused(bool hideUnused) {
                if (hideUnused == m_hideUnused)
                    return;
                m_hideUnused = hideUnused;
                reload();
                Refresh();
            }

            inline void setFilterText(const String& filterText) {
                if (filterText == m_filterText)
                    return;
                m_filterText = filterText;
                reload();
                Refresh();
            }
        };
    }
}

#endif /* defined(__TrenchBroom__EntityBrowserCanvas__) */
