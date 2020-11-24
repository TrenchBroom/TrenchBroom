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

#include "Renderer/FontDescriptor.h"
#include "Renderer/GLVertexType.h"
#include "View/CellView.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

class QScrollBar;

namespace TrenchBroom {
    namespace Assets {
        class Texture;
        class TextureCollection;
    }

    namespace View {
        class GLContextManager;
        class MapDocument;
        using TextureGroupData = std::string;

        struct TextureCellData {
            const Assets::Texture* texture;
            std::string mainTitle;
            std::string subTitle;
            vm::vec2f mainTitleOffset;
            vm::vec2f subTitleOffset;
            Renderer::FontDescriptor mainTitleFont;
            Renderer::FontDescriptor subTitleFont;
        };

        enum class TextureSortOrder {
            Name,
            Usage
        };

        class TextureBrowserView : public CellView {
            Q_OBJECT
        private:
            using TextVertex = Renderer::GLVertexTypes::P2T2C4::Vertex;
            using StringMap = std::map<Renderer::FontDescriptor, std::vector<TextVertex>>;

            std::weak_ptr<MapDocument> m_document;
            bool m_group;
            bool m_hideUnused;
            TextureSortOrder m_sortOrder;
            std::string m_filterText;

            const Assets::Texture* m_selectedTexture;
        public:
            TextureBrowserView(QScrollBar* scrollBar,
                               GLContextManager& contextManager,
                               std::weak_ptr<MapDocument> document);
            ~TextureBrowserView() override;

            void setSortOrder(TextureSortOrder sortOrder);
            void setGroup(bool group);
            void setHideUnused(bool hideUnused);
            void setFilterText(const std::string& filterText);

            const Assets::Texture* selectedTexture() const;
            void setSelectedTexture(const Assets::Texture* selectedTexture);

            void revealTexture(const Assets::Texture* texture);
        private:
            void usageCountDidChange();

            void doInitLayout(Layout& layout) override;
            void doReloadLayout(Layout& layout) override;
            void addTextureToLayout(Layout& layout, const Assets::Texture* texture, const std::string& groupName, const Renderer::FontDescriptor& font);

            struct CompareByUsageCount;
            struct CompareByName;
            struct MatchUsageCount;
            struct MatchName;

            const std::vector<Assets::TextureCollection>& getCollections() const;
            std::vector<const Assets::Texture*> getTextures(const Assets::TextureCollection& collection) const;
            std::vector<const Assets::Texture*> getTextures() const;

            void filterTextures(std::vector<const Assets::Texture*>& textures) const;
            void sortTextures(std::vector<const Assets::Texture*>& textures) const;

            void doClear() override;
            void doRender(Layout& layout, float y, float height) override;
            bool doShouldRenderFocusIndicator() const override;
            const Color& getBackgroundColor() override;

            void renderBounds(Layout& layout, float y, float height);
            const Color& textureColor(const Assets::Texture& texture) const;
            void renderTextures(Layout& layout, float y, float height);
            void renderNames(Layout& layout, float y, float height);
            void renderGroupTitleBackgrounds(Layout& layout, float y, float height);
            void renderStrings(Layout& layout, float y, float height);
            StringMap collectStringVertices(Layout& layout, float y, float height);

            void doLeftClick(Layout& layout, float x, float y) override;
            QString tooltip(const Cell& cell) override;
            void doContextMenu(Layout& layout, float x, float y, QContextMenuEvent* event) override;

            const TextureCellData& cellData(const Cell& cell) const;
        signals:
            void textureSelected(const Assets::Texture* texture);
        };
    }
}

#endif /* defined(TrenchBroom_TextureBrowserView) */
