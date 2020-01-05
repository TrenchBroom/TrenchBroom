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

#include "TextureBrowserView.h"

#include "PreferenceManager.h"
#include "Preferences.h"
#include "Renderer/ActiveShader.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Assets/TextureManager.h"
#include "Renderer/GL.h"
#include "Renderer/FontManager.h"
#include "Renderer/PrimType.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/TextureFont.h"
#include "Renderer/Transformation.h"
#include "Renderer/VertexArray.h"
#include "View/MapDocument.h"

#include <kdl/memory_utils.h>
#include <kdl/skip_iterator.h>
#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>

#include <string>
#include <vector>

#include <QTextStream>
#include <QMenu>

// allow storing std::shared_ptr in QVariant
Q_DECLARE_METATYPE(std::shared_ptr<TrenchBroom::View::TextureCellData>)

namespace TrenchBroom {
    namespace View {
        TextureBrowserView::TextureBrowserView(QScrollBar* scrollBar,
                                               GLContextManager& contextManager,
                                               std::weak_ptr<MapDocument> document) :
        CellView(contextManager, scrollBar),
        m_document(document),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(TextureSortOrder::Name),
        m_selectedTexture(nullptr) {
            auto doc = kdl::mem_lock(m_document);
            doc->textureManager().usageCountDidChange.addObserver(this, &TextureBrowserView::usageCountDidChange);
        }

        TextureBrowserView::~TextureBrowserView() {
            if (!kdl::mem_expired(m_document)) {
                auto doc = kdl::mem_lock(m_document);
                doc->textureManager().usageCountDidChange.removeObserver(this, &TextureBrowserView::usageCountDidChange);
            }
            clear();
        }

        void TextureBrowserView::setSortOrder(const TextureSortOrder sortOrder) {
            if (sortOrder == m_sortOrder) {
                return;
            }
            m_sortOrder = sortOrder;
            invalidate();
            update();
        }

        void TextureBrowserView::setGroup(const bool group) {
            if (group == m_group) {
                return;
            }
            m_group = group;
            invalidate();
            update();
        }

        void TextureBrowserView::setHideUnused(const bool hideUnused) {
            if (hideUnused == m_hideUnused) {
                return;
            }
            m_hideUnused = hideUnused;
            invalidate();
            update();
        }

        void TextureBrowserView::setFilterText(const std::string& filterText) {
            if (filterText == m_filterText) {
                return;
            }
            m_filterText = filterText;
            invalidate();
            update();
        }

        Assets::Texture* TextureBrowserView::selectedTexture() const {
            return m_selectedTexture;
        }

        void TextureBrowserView::setSelectedTexture(Assets::Texture* selectedTexture) {
            if (m_selectedTexture == selectedTexture) {
                return;
            }
            m_selectedTexture = selectedTexture;
            update();
        }

        void TextureBrowserView::usageCountDidChange() {
            invalidate();
            update();
        }

        void TextureBrowserView::doInitLayout(Layout& layout) {
            const float scaleFactor = pref(Preferences::TextureBrowserIconSize);

            layout.setOuterMargin(5.0f);
            layout.setGroupMargin(5.0f);
            layout.setRowMargin(15.0f);
            layout.setCellMargin(10.0f);
            layout.setTitleMargin(2.0f);
            layout.setCellWidth(scaleFactor * 64.0f, scaleFactor * 64.0f);
            layout.setCellHeight(scaleFactor * 64.0f, scaleFactor * 128.0f);
        }

        void TextureBrowserView::doReloadLayout(Layout& layout) {
            const IO::Path& fontPath = pref(Preferences::RendererFontPath());
            int fontSize = pref(Preferences::BrowserFontSize);
            assert(fontSize > 0);

            const Renderer::FontDescriptor font(fontPath, static_cast<size_t>(fontSize));

            if (m_group) {
                for (const Assets::TextureCollection* collection : getCollections()) {
                    layout.addGroup(collection->name(), static_cast<float>(fontSize) + 2.0f);
                    for (Assets::Texture* texture : getTextures(collection))
                        addTextureToLayout(layout, texture, font);
                }
            } else {
                for (Assets::Texture* texture : getTextures())
                    addTextureToLayout(layout, texture, font);
            }
        }

        void TextureBrowserView::addTextureToLayout(Layout& layout, Assets::Texture* texture, const Renderer::FontDescriptor& font) {
            const float maxCellWidth = layout.maxCellWidth();

            const auto& groupName   = texture->collection()->name();
            const auto  textureName = IO::Path(texture->name()).lastComponent().asString();

            const auto textureFont = fontManager().selectFontSize(font, textureName, maxCellWidth, 6);
            const auto groupFont   = fontManager().selectFontSize(font, groupName, maxCellWidth, 6);

            const auto defaultTextHeight = fontManager().font(font).measure(groupName + textureName).y();
            const auto textureNameSize   = fontManager().font(textureFont).measure(textureName);
            const auto groupNameSize     = fontManager().font(groupFont).measure(groupName);

            const auto totalSize = vm::vec2f(vm::max(groupNameSize.x(), textureNameSize.x()), 2.0f * defaultTextHeight + 4.0f);

            const float scaleFactor = pref(Preferences::TextureBrowserIconSize);
            const float scaledTextureWidth = vm::round(scaleFactor * static_cast<float>(texture->width()));
            const float scaledTextureHeight = vm::round(scaleFactor * static_cast<float>(texture->height()));

            auto cellData = std::shared_ptr<TextureCellData>(new TextureCellData{
                texture,
                textureName,
                groupName,
                vm::vec2f((maxCellWidth - textureNameSize.x()) / 2.0f, defaultTextHeight + 3.0f),
                vm::vec2f((maxCellWidth - groupNameSize.x()) / 2.0f, 1.0f),
                textureFont,
                groupFont
            });

            layout.addItem(QVariant::fromValue(cellData),
            scaledTextureWidth,
            scaledTextureHeight,
            maxCellWidth,
            totalSize.y());
        }

        struct TextureBrowserView::CompareByUsageCount {
            kdl::ci::string_less m_less;

            template <typename T>
            bool operator()(const T* lhs, const T* rhs) const {
                if (lhs->usageCount() > rhs->usageCount())
                    return true;
                if (lhs->usageCount() < rhs->usageCount())
                    return false;

                return m_less(lhs->name(), rhs->name());
            }
        };

        struct TextureBrowserView::CompareByName {
            kdl::ci::string_less m_less;

            template <typename T>
            bool operator()(const T* lhs, const T* rhs) const {
                return m_less(lhs->name(), rhs->name());
            }
        };

        struct TextureBrowserView::MatchUsageCount {
            template <typename T>
            bool operator()(const T* t) const {
                return t->usageCount() == 0;
            }
        };

        struct TextureBrowserView::MatchName {
            std::string pattern;

            explicit MatchName(const std::string& i_pattern) : pattern(i_pattern) {}

            bool operator()(const Assets::Texture* texture) const {
                return !kdl::ci::str_contains(texture->name(), pattern);
            }
        };

        std::vector<Assets::TextureCollection*> TextureBrowserView::getCollections() const {
            auto doc = kdl::mem_lock(m_document);
            std::vector<Assets::TextureCollection*> collections = doc->textureManager().collections();
            if (m_hideUnused) {
                kdl::vec_erase_if(collections, MatchUsageCount());
            }
            if (m_sortOrder == TextureSortOrder::Usage) {
                kdl::vec_sort(collections, CompareByUsageCount());
            }
            return collections;
        }

        std::vector<Assets::Texture*> TextureBrowserView::getTextures(const Assets::TextureCollection* collection) const {
            std::vector<Assets::Texture*> textures = collection->textures();
            filterTextures(textures);
            sortTextures(textures);
            return textures;
        }

        std::vector<Assets::Texture*> TextureBrowserView::getTextures() const {
            auto doc = kdl::mem_lock(m_document);
            std::vector<Assets::Texture*> textures = doc->textureManager().textures();
            filterTextures(textures);
            sortTextures(textures);
            return textures;
        }

        void TextureBrowserView::filterTextures(std::vector<Assets::Texture*>& textures) const {
            if (m_hideUnused)
                kdl::vec_erase_if(textures, MatchUsageCount());
            if (!m_filterText.empty())
                kdl::vec_erase_if(textures, MatchName(m_filterText));
        }

        void TextureBrowserView::sortTextures(std::vector<Assets::Texture*>& textures) const {
            switch (m_sortOrder) {
                case TextureSortOrder::Name:
                    kdl::vec_sort(textures, CompareByName());
                    break;
                case TextureSortOrder::Usage:
                    kdl::vec_sort(textures, CompareByUsageCount());
                    break;
            }
        }

        void TextureBrowserView::doClear() {}

        void TextureBrowserView::doRender(Layout& layout, const float y, const float height) {
            auto doc = kdl::mem_lock(m_document);
            doc->textureManager().commitChanges();

            const float viewLeft      = static_cast<float>(0);
            const float viewTop       = static_cast<float>(size().height());
            const float viewRight     = static_cast<float>(size().width());
            const float viewBottom    = static_cast<float>(0);

            const vm::mat4x4f projection = vm::ortho_matrix(-1.0f, 1.0f, viewLeft, viewTop, viewRight, viewBottom);
            const vm::mat4x4f view = vm::view_matrix(vm::vec3f::neg_z(), vm::vec3f::pos_y()) *vm::translation_matrix(vm::vec3f(0.0f, 0.0f, 0.1f));
            const Renderer::Transformation transformation(projection, view);

            glAssert(glDisable(GL_DEPTH_TEST));
            glAssert(glFrontFace(GL_CCW));

            renderBounds(layout, y, height);
            renderTextures(layout, y, height);
            renderNames(layout, y, height);
        }

        bool TextureBrowserView::doShouldRenderFocusIndicator() const {
            return false;
        }

        void TextureBrowserView::renderBounds(Layout& layout, const float y, const float height) {
            using BoundsVertex = Renderer::GLVertexTypes::P2C4::Vertex;
            std::vector<BoundsVertex> vertices;

            for (size_t i = 0; i < layout.size(); ++i) {
                const Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const Cell& cell = row[k];
                                const LayoutBounds& bounds = cell.itemBounds();
                                const Assets::Texture* texture = cellData(cell).texture;
                                const Color& color = textureColor(*texture);
                                vertices.emplace_back(vm::vec2f(bounds.left() - 2.0f, height - (bounds.top() - 2.0f - y)), color);
                                vertices.emplace_back(vm::vec2f(bounds.left() - 2.0f, height - (bounds.bottom() + 2.0f - y)), color);
                                vertices.emplace_back(vm::vec2f(bounds.right() + 2.0f, height - (bounds.bottom() + 2.0f - y)), color);
                                vertices.emplace_back(vm::vec2f(bounds.right() + 2.0f, height - (bounds.top() - 2.0f - y)), color);
                            }
                        }
                    }
                }
            }

            Renderer::VertexArray vertexArray = Renderer::VertexArray::move(std::move(vertices));
            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::TextureBrowserBorderShader);

            vertexArray.prepare(vboManager());
            vertexArray.render(Renderer::PrimType::Quads);
        }

        const Color& TextureBrowserView::textureColor(const Assets::Texture& texture) const {
            if (&texture == m_selectedTexture)
                return pref(Preferences::TextureBrowserSelectedColor);
            if (texture.usageCount() > 0)
                return pref(Preferences::TextureBrowserUsedColor);
            return pref(Preferences::TextureBrowserDefaultColor);
        }

        void TextureBrowserView::renderTextures(Layout& layout, const float y, const float height) {
            using TextureVertex = Renderer::GLVertexTypes::P2T2::Vertex;

            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::TextureBrowserShader);
            shader.set("ApplyTinting", false);
            shader.set("Texture", 0);
            shader.set("Brightness", pref(Preferences::Brightness));

            size_t num = 0;

            for (size_t i = 0; i < layout.size(); ++i) {
                const Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const Cell& cell = row[k];
                                const LayoutBounds& bounds = cell.itemBounds();
                                const Assets::Texture* texture = cellData(cell).texture;

                                Renderer::VertexArray vertexArray = Renderer::VertexArray::move(std::vector<TextureVertex>({
                                    TextureVertex(vm::vec2f(bounds.left(),  height - (bounds.top() - y)),    vm::vec2f(0.0f, 0.0f)),
                                    TextureVertex(vm::vec2f(bounds.left(),  height - (bounds.bottom() - y)), vm::vec2f(0.0f, 1.0f)),
                                    TextureVertex(vm::vec2f(bounds.right(), height - (bounds.bottom() - y)), vm::vec2f(1.0f, 1.0f)),
                                    TextureVertex(vm::vec2f(bounds.right(), height - (bounds.top() - y)),    vm::vec2f(1.0f, 0.0f))
                                }));

                                shader.set("GrayScale", texture->overridden());
                                texture->activate();

                                vertexArray.prepare(vboManager());
                                vertexArray.render(Renderer::PrimType::Quads);

                                texture->deactivate();

                                ++num;
                            }
                        }
                    }
                }
            }
        }

        void TextureBrowserView::renderNames(Layout& layout, const float y, const float height) {
            renderGroupTitleBackgrounds(layout, y, height);
            renderStrings(layout, y, height);
        }

        void TextureBrowserView::renderGroupTitleBackgrounds(Layout& layout, const float y, const float height) {
            using Vertex = Renderer::GLVertexTypes::P2::Vertex;
            std::vector<Vertex> vertices;

            for (size_t i = 0; i < layout.size(); ++i) {
                const Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    const LayoutBounds titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                    vertices.push_back(Vertex(vm::vec2f(titleBounds.left(), height - (titleBounds.top() - y))));
                    vertices.push_back(Vertex(vm::vec2f(titleBounds.left(), height - (titleBounds.bottom() - y))));
                    vertices.push_back(Vertex(vm::vec2f(titleBounds.right(), height - (titleBounds.bottom() - y))));
                    vertices.push_back(Vertex(vm::vec2f(titleBounds.right(), height - (titleBounds.top() - y))));
                }
            }

            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::VaryingPUniformCShader);
            shader.set("Color", pref(Preferences::BrowserGroupBackgroundColor));

            Renderer::VertexArray vertexArray = Renderer::VertexArray::move(std::move(vertices));

            vertexArray.prepare(vboManager());
            vertexArray.render(Renderer::PrimType::Quads);
        }

        void TextureBrowserView::renderStrings(Layout& layout, const float y, const float height) {
            using StringRendererMap = std::map<Renderer::FontDescriptor, Renderer::VertexArray>;
            StringRendererMap stringRenderers;

            for (const auto& entry : collectStringVertices(layout, y, height)) {
                const auto& descriptor = entry.first;
                const auto& vertices = entry.second;
                stringRenderers[descriptor] = Renderer::VertexArray::ref(vertices);
                stringRenderers[descriptor].prepare(vboManager());
            }

            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::ColoredTextShader);
            shader.set("Texture", 0);

            for (auto& entry : stringRenderers) {
                const auto& descriptor = entry.first;
                auto& vertexArray = entry.second;

                auto& font = fontManager().font(descriptor);
                font.activate();
                vertexArray.render(Renderer::PrimType::Quads);
                font.deactivate();
            }
        }

        TextureBrowserView::StringMap TextureBrowserView::collectStringVertices(Layout& layout, const float y, const float height) {
            Renderer::FontDescriptor defaultDescriptor(pref(Preferences::RendererFontPath()),
                                                       static_cast<size_t>(pref(Preferences::BrowserFontSize)));

            const std::vector<Color> textColor{ pref(Preferences::BrowserTextColor) };
            const std::vector<Color> subTextColor{ pref(Preferences::BrowserSubTextColor) };

            StringMap stringVertices;
            for (size_t i = 0; i < layout.size(); ++i) {
                const auto& group = layout[i];
                if (group.intersectsY(y, height)) {
                    const auto& title = group.item();
                    if (!title.empty()) {
                        const auto titleBounds = layout.titleBoundsForVisibleRect(group, y, height);
                        const auto offset = vm::vec2f(titleBounds.left() + 2.0f, height - (titleBounds.top() - y) - titleBounds.height());

                        auto& font = fontManager().font(defaultDescriptor);
                        const auto quads = font.quads(title, false, offset);
                        const auto titleVertices = TextVertex::toList(
                            quads.size() / 2,
                            kdl::skip_iterator(std::begin(quads), std::end(quads), 0, 2),
                            kdl::skip_iterator(std::begin(quads), std::end(quads), 1, 2),
                            kdl::skip_iterator(std::begin(textColor), std::end(textColor), 0, 0));
                        auto& vertices = stringVertices[defaultDescriptor];
                        vertices.insert(std::end(vertices), std::begin(titleVertices), std::end(titleVertices));
                    }

                    for (size_t j = 0; j < group.size(); ++j) {
                        const auto& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const auto& cell = row[k];
                                const auto titleBounds = cell.titleBounds();
                                const auto& textureFont = fontManager().font(cellData(cell).mainTitleFont);
                                const auto& groupFont   = fontManager().font(cellData(cell).subTitleFont);

                                // y is relative to top, but OpenGL coords are relative to bottom, so invert
                                const auto titleOffset = vm::vec2f(titleBounds.left(), y + height - titleBounds.bottom());

                                const auto textureNameOffset = titleOffset + cellData(cell).mainTitleOffset;
                                const auto groupNameOffset   = titleOffset + cellData(cell).subTitleOffset;

                                const auto& textureName = cellData(cell).mainTitle;
                                const auto& groupName   = cellData(cell).subTitle;

                                const auto textureNameQuads = textureFont.quads(textureName, false, textureNameOffset);
                                const auto groupNameQuads   = groupFont.quads(groupName, false, groupNameOffset);

                                const auto textureNameVertices = TextVertex::toList(
                                    textureNameQuads.size() / 2,
                                    kdl::skip_iterator(std::begin(textureNameQuads), std::end(textureNameQuads), 0, 2),
                                    kdl::skip_iterator(std::begin(textureNameQuads), std::end(textureNameQuads), 1, 2),
                                    kdl::skip_iterator(std::begin(textColor), std::end(textColor), 0, 0));

                                const auto groupNameVertices = TextVertex::toList(
                                    groupNameQuads.size() / 2,
                                    kdl::skip_iterator(std::begin(groupNameQuads), std::end(groupNameQuads), 0, 2),
                                    kdl::skip_iterator(std::begin(groupNameQuads), std::end(groupNameQuads), 1, 2),
                                    kdl::skip_iterator(std::begin(subTextColor), std::end(subTextColor), 0, 0));

                                kdl::vec_append(stringVertices[cellData(cell).mainTitleFont], textureNameVertices);
                                kdl::vec_append(stringVertices[cellData(cell).subTitleFont], groupNameVertices);
                            }
                        }
                    }
                }
            }

            return stringVertices;
        }

        void TextureBrowserView::doLeftClick(Layout& layout, const float x, const float y) {
            const Cell* result = nullptr;
            if (layout.cellAt(x, y, &result)) {
                if (!cellData(*result).texture->overridden()) {
                    auto* texture = cellData(*result).texture;

                    // NOTE: wx had the ability for the textureSelected event to veto the selection, but it
                    // wasn't used.
                    setSelectedTexture(texture);

                    emit textureSelected(texture);

                    update();
                }
            }
        }

        QString TextureBrowserView::tooltip(const Cell& cell) {
            QString tooltip;
            QTextStream ss(&tooltip);
            ss << QString::fromStdString(cellData(cell).texture->name()) << "\n";
            ss << cellData(cell).texture->width() << "x" << cellData(cell).texture->height();
            return tooltip;
        }

        void TextureBrowserView::doContextMenu(Layout& layout, float x, float y, QContextMenuEvent* event) {
            const Cell* result = nullptr;
            if (layout.cellAt(x, y, &result)) {
                if (!cellData(*result).texture->overridden()) {
                    auto* texture = cellData(*result).texture;

                    QMenu menu(this);
                    menu.addAction(tr("Select Faces"), this, [=]() {
                        auto doc = kdl::mem_lock(m_document);
                        doc->selectFacesWithTexture(texture);
                    });
                    menu.exec(event->globalPos());
                }
            }
        }

        const TextureCellData& TextureBrowserView::cellData(const Cell& cell) const {
            QVariant any = cell.item();
            auto ptr = any.value<std::shared_ptr<TextureCellData>>();
            return *ptr;
        }
    }
}
