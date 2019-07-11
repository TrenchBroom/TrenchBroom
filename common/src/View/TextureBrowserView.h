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

#ifndef TrenchBroom_TextureBrowserView
#define TrenchBroom_TextureBrowserView

#include "StringUtils.h"
#include "Assets/TextureManager.h"
#include "Renderer/FontDescriptor.h"
#include "Renderer/Vbo.h"
#include "Renderer/GLVertex.h"
#include "Renderer/GLVertexType.h"
#include "View/CellView.h"
#include "View/ViewTypes.h"

#include <map>

class QScrollBar;

namespace TrenchBroom {
    namespace Assets {
        class Texture;
        class TextureCollection;
    }

    namespace View {
        class GLContextManager;
        using TextureGroupData = String;

        struct TextureCellData {
            Assets::Texture* texture;
            String mainTitle;
            String subTitle;
            vm::vec2f mainTitleOffset;
            vm::vec2f subTitleOffset;
            Renderer::FontDescriptor mainTitleFont;
            Renderer::FontDescriptor subTitleFont;
        };

        class TextureBrowserView : public CellView {
            Q_OBJECT
        public:
            typedef enum {
                SO_Name,
                SO_Usage
            } SortOrder;
        private:
            using TextVertex = Renderer::GLVertexTypes::P2T2C4::Vertex;
            using StringMap = std::map<Renderer::FontDescriptor, TextVertex::List>;

            MapDocumentWPtr m_document;
            bool m_group;
            bool m_hideUnused;
            SortOrder m_sortOrder;
            String m_filterText;

            Assets::Texture* m_selectedTexture;
        public:
            TextureBrowserView(QScrollBar* scrollBar,
                               GLContextManager& contextManager,
                               MapDocumentWPtr document);
            ~TextureBrowserView() override;

            void setSortOrder(SortOrder sortOrder);
            void setGroup(bool group);
            void setHideUnused(bool hideUnused);
            void setFilterText(const String& filterText);

            Assets::Texture* selectedTexture() const;
            void setSelectedTexture(Assets::Texture* selectedTexture);
        private:
            void usageCountDidChange();

            void doInitLayout(Layout& layout) override;
            void doReloadLayout(Layout& layout) override;
            void addTextureToLayout(Layout& layout, Assets::Texture* texture, const Renderer::FontDescriptor& font);

            struct CompareByUsageCount;
            struct CompareByName;
            struct MatchUsageCount;
            struct MatchName;

            Assets::TextureCollectionList getCollections() const;
            Assets::TextureList getTextures(const Assets::TextureCollection* collection) const;
            Assets::TextureList getTextures() const;

            void filterTextures(Assets::TextureList& textures) const;
            void sortTextures(Assets::TextureList& textures) const;

            void doClear() override;
            void doRender(Layout& layout, float y, float height) override;
            bool doShouldRenderFocusIndicator() const override;

            void renderBounds(Layout& layout, float y, float height);
            const Color& textureColor(const Assets::Texture& texture) const;
            void renderTextures(Layout& layout, float y, float height);
            void renderNames(Layout& layout, float y, float height);
            void renderGroupTitleBackgrounds(Layout& layout, float y, float height);
            void renderStrings(Layout& layout, float y, float height);
            StringMap collectStringVertices(Layout& layout, float y, float height);

            void doLeftClick(Layout& layout, float x, float y) override;
            QString tooltip(const Cell& cell) override;

            const TextureCellData& cellData(const Cell& cell) const;
        signals:
            void textureSelected(Assets::Texture* texture);
        };
    }
}

#endif /* defined(TrenchBroom_TextureBrowserView) */
