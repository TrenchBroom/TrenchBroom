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

#include "Renderer/GL.h"
#include "PreferenceManager.h"
#include "Preferences.h"
#include "Assets/Texture.h"
#include "Assets/TextureCollection.h"
#include "Renderer/FontManager.h"
#include "Renderer/Shaders.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/TextureFont.h"
#include "Renderer/VertexArray.h"
#include "View/TextureSelectedCommand.h"

#include <vecmath/vec.h>
#include <vecmath/mat.h>
#include <vecmath/mat_ext.h>

namespace TrenchBroom {
    namespace View {
        TextureCellData::TextureCellData(Assets::Texture* i_texture, const Renderer::FontDescriptor& i_fontDescriptor) :
        texture(i_texture),
        fontDescriptor(i_fontDescriptor) {}

        TextureBrowserView::TextureBrowserView(wxWindow* parent,
                                               wxScrollBar* scrollBar,
                                               GLContextManager& contextManager,
                                               Assets::TextureManager& textureManager) :
        CellView(parent, contextManager, scrollBar),
        m_textureManager(textureManager),
        m_group(false),
        m_hideUnused(false),
        m_sortOrder(SO_Name),
        m_selectedTexture(nullptr) {
            m_textureManager.usageCountDidChange.addObserver(this, &TextureBrowserView::usageCountDidChange);
        }
        
        TextureBrowserView::~TextureBrowserView() {
            m_textureManager.usageCountDidChange.removeObserver(this, &TextureBrowserView::usageCountDidChange);
            clear();
        }

        void TextureBrowserView::setSortOrder(const SortOrder sortOrder) {
            if (sortOrder == m_sortOrder)
                return;
            m_sortOrder = sortOrder;
            invalidate();
            Refresh();
        }
        
        void TextureBrowserView::setGroup(const bool group) {
            if (group == m_group)
                return;
            m_group = group;
            invalidate();
            Refresh();
        }
        
        void TextureBrowserView::setHideUnused(const bool hideUnused) {
            if (hideUnused == m_hideUnused)
                return;
            m_hideUnused = hideUnused;
            invalidate();
            Refresh();
        }
        
        void TextureBrowserView::setFilterText(const String& filterText) {
            if (filterText == m_filterText)
                return;
            m_filterText = filterText;
            invalidate();
            Refresh();
        }

        Assets::Texture* TextureBrowserView::selectedTexture() const {
            return m_selectedTexture;
        }
        
        void TextureBrowserView::setSelectedTexture(Assets::Texture* selectedTexture) {
            if (m_selectedTexture == selectedTexture)
                return;
            m_selectedTexture = selectedTexture;
            Refresh();
        }

        void TextureBrowserView::usageCountDidChange() {
            invalidate();
            Refresh();
        }

        void TextureBrowserView::doInitLayout(Layout& layout) {
            const float scaleFactor = pref(Preferences::TextureBrowserIconSize);
            
            layout.setOuterMargin(5.0f);
            layout.setGroupMargin(5.0f);
            layout.setRowMargin(5.0f);
            layout.setCellMargin(5.0f);
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
                    layout.addGroup(collection->name(), fontSize + 2.0f);
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
            const Renderer::FontDescriptor actualFont = fontManager().selectFontSize(font, texture->name(), maxCellWidth, 5);
            const vm::vec2f actualSize = fontManager().font(actualFont).measure(texture->name());
            
            const float scaleFactor = pref(Preferences::TextureBrowserIconSize);
            const size_t scaledTextureWidth = static_cast<size_t>(vm::round(scaleFactor * static_cast<float>(texture->width())));
            const size_t scaledTextureHeight = static_cast<size_t>(vm::round(scaleFactor * static_cast<float>(texture->height())));
            
            layout.addItem(TextureCellData(texture, actualFont),
                           scaledTextureWidth,
                           scaledTextureHeight,
                           actualSize.x(),
                           font.size() + 2.0f);
        }

        struct TextureBrowserView::CompareByUsageCount {
            StringUtils::CaseInsensitiveStringLess m_less;

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
            StringUtils::CaseInsensitiveStringLess m_less;
            
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
            String pattern;
            
            MatchName(const String& i_pattern) : pattern(i_pattern) {}
            
            bool operator()(const Assets::Texture* texture) const {
                return !StringUtils::containsCaseInsensitive(texture->name(), pattern);
            }
        };

        Assets::TextureCollectionList TextureBrowserView::getCollections() const {
            Assets::TextureCollectionList collections = m_textureManager.collections();
            if (m_hideUnused)
                VectorUtils::eraseIf(collections, MatchUsageCount());
            if (m_sortOrder == SO_Usage)
                VectorUtils::sort(collections, CompareByUsageCount());
            return collections;
        }
        
        Assets::TextureList TextureBrowserView::getTextures(const Assets::TextureCollection* collection) const {
            Assets::TextureList textures = collection->textures();
            filterTextures(textures);
            sortTextures(textures);
            return textures;
        }
        
        Assets::TextureList TextureBrowserView::getTextures() const {
            Assets::TextureList textures = m_textureManager.textures();
            filterTextures(textures);
            sortTextures(textures);
            return textures;
        }

        void TextureBrowserView::filterTextures(Assets::TextureList& textures) const {
            if (m_hideUnused)
                VectorUtils::eraseIf(textures, MatchUsageCount());
            if (!m_filterText.empty())
                VectorUtils::eraseIf(textures, MatchName(m_filterText));
        }
        
        void TextureBrowserView::sortTextures(Assets::TextureList& textures) const {
            switch (m_sortOrder) {
                case SO_Name:
                    VectorUtils::sort(textures, CompareByName());
                    break;
                case SO_Usage:
                    VectorUtils::sort(textures, CompareByUsageCount());
                    break;
            }
        }

        void TextureBrowserView::doClear() {}
        
        void TextureBrowserView::doRender(Layout& layout, const float y, const float height) {
            m_textureManager.commitChanges();
            
            const float viewLeft      = static_cast<float>(GetClientRect().GetLeft());
            const float viewTop       = static_cast<float>(GetClientRect().GetBottom());
            const float viewRight     = static_cast<float>(GetClientRect().GetRight());
            const float viewBottom    = static_cast<float>(GetClientRect().GetTop());
            
            const vm::mat4x4f projection = vm::orthoMatrix(-1.0f, 1.0f, viewLeft, viewTop, viewRight, viewBottom);
            const vm::mat4x4f view = vm::viewMatrix(vm::vec3f::neg_z, vm::vec3f::pos_y) * translationMatrix(vm::vec3f(0.0f, 0.0f, 0.1f));
            const Renderer::Transformation transformation(projection, view);
            
            Renderer::ActivateVbo activate(vertexVbo());
            
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
            typedef Renderer::VertexSpecs::P2C4::Vertex BoundsVertex;
            BoundsVertex::List vertices;
            
            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                const LayoutBounds& bounds = cell.itemBounds();
                                const Assets::Texture* texture = cell.item().texture;
                                const Color& color = textureColor(*texture);
                                vertices.push_back(BoundsVertex(vm::vec2f(bounds.left() - 2.0f, height - (bounds.top() - 2.0f - y)), color));
                                vertices.push_back(BoundsVertex(vm::vec2f(bounds.left() - 2.0f, height - (bounds.bottom() + 2.0f - y)), color));
                                vertices.push_back(BoundsVertex(vm::vec2f(bounds.right() + 2.0f, height - (bounds.bottom() + 2.0f - y)), color));
                                vertices.push_back(BoundsVertex(vm::vec2f(bounds.right() + 2.0f, height - (bounds.top() - 2.0f - y)), color));
                            }
                        }
                    }
                }
            }

            Renderer::VertexArray vertexArray = Renderer::VertexArray::swap(vertices);
            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::TextureBrowserBorderShader);
            
            Renderer::ActivateVbo activate(vertexVbo());
            vertexArray.prepare(vertexVbo());
            vertexArray.render(GL_QUADS);
        }
        
        const Color& TextureBrowserView::textureColor(const Assets::Texture& texture) const {
            if (&texture == m_selectedTexture)
                return pref(Preferences::TextureBrowserSelectedColor);
            if (texture.usageCount() > 0)
                return pref(Preferences::TextureBrowserUsedColor);
            return pref(Preferences::TextureBrowserDefaultColor);
        }

        void TextureBrowserView::renderTextures(Layout& layout, const float y, const float height) {
            typedef Renderer::VertexSpecs::P2T2::Vertex TextureVertex;
            TextureVertex::List vertices(4);

            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::TextureBrowserShader);
            shader.set("ApplyTinting", false);
            shader.set("Texture", 0);
            shader.set("Brightness", pref(Preferences::Brightness));
            
            size_t num = 0;
            
            Renderer::ActivateVbo activate(vertexVbo());

            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
                if (group.intersectsY(y, height)) {
                    for (size_t j = 0; j < group.size(); ++j) {
                        const Layout::Group::Row& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (size_t k = 0; k < row.size(); ++k) {
                                const Layout::Group::Row::Cell& cell = row[k];
                                const LayoutBounds& bounds = cell.itemBounds();
                                const Assets::Texture* texture = cell.item().texture;
                                
                                vertices[0] = TextureVertex(vm::vec2f(bounds.left(),  height - (bounds.top() - y)),    vm::vec2f(0.0f, 0.0f));
                                vertices[1] = TextureVertex(vm::vec2f(bounds.left(),  height - (bounds.bottom() - y)), vm::vec2f(0.0f, 1.0f));
                                vertices[2] = TextureVertex(vm::vec2f(bounds.right(), height - (bounds.bottom() - y)), vm::vec2f(1.0f, 1.0f));
                                vertices[3] = TextureVertex(vm::vec2f(bounds.right(), height - (bounds.top() - y)),    vm::vec2f(1.0f, 0.0f));

                                Renderer::VertexArray vertexArray = Renderer::VertexArray::copy(vertices);

                                shader.set("GrayScale", texture->overridden());
                                texture->activate();

                                vertexArray.prepare(vertexVbo());
                                vertexArray.render(GL_QUADS);
                                
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
            typedef Renderer::VertexSpecs::P2::Vertex Vertex;
            Vertex::List vertices;
            
            for (size_t i = 0; i < layout.size(); ++i) {
                const Layout::Group& group = layout[i];
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
            
            Renderer::VertexArray vertexArray = Renderer::VertexArray::swap(vertices);

            Renderer::ActivateVbo activate(vertexVbo());
            vertexArray.prepare(vertexVbo());
            vertexArray.render(GL_QUADS);
        }
        
        void TextureBrowserView::renderStrings(Layout& layout, const float y, const float height) {
            using StringRendererMap = std::map<Renderer::FontDescriptor, Renderer::VertexArray>;
            StringRendererMap stringRenderers;
            
            Renderer::ActivateVbo activate(vertexVbo());

            for (const auto& entry : collectStringVertices(layout, y, height)) {
                const auto& descriptor = entry.first;
                const auto& vertices = entry.second;
                stringRenderers[descriptor] = Renderer::VertexArray::ref(vertices);
                stringRenderers[descriptor].prepare(vertexVbo());
            }
            
            Renderer::ActiveShader shader(shaderManager(), Renderer::Shaders::ColoredTextShader);
            shader.set("Texture", 0);

            for (auto& entry : stringRenderers) {
                const auto& descriptor = entry.first;
                auto& vertexArray = entry.second;
                
                auto& font = fontManager().font(descriptor);
                font.activate();
                vertexArray.render(GL_QUADS);
                font.deactivate();
            }
        }
        
        TextureBrowserView::StringMap TextureBrowserView::collectStringVertices(Layout& layout, const float y, const float height) {
            Renderer::FontDescriptor defaultDescriptor(pref(Preferences::RendererFontPath()),
                                                       static_cast<size_t>(pref(Preferences::BrowserFontSize)));
            
            const std::vector<Color> textColor{ pref(Preferences::BrowserTextColor) };

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
                        const auto titleVertices = TextVertex::toList(std::begin(quads), std::begin(quads), std::begin(textColor), quads.size() / 2, 0, 2, 1, 2, 0, 0);
                        auto& vertices = stringVertices[defaultDescriptor];
                        vertices.insert(std::end(vertices), std::begin(titleVertices), std::end(titleVertices));
                    }
                    
                    for (size_t j = 0; j < group.size(); ++j) {
                        const auto& row = group[j];
                        if (row.intersectsY(y, height)) {
                            for (unsigned int k = 0; k < row.size(); k++) {
                                const auto& cell = row[k];
                                const auto titleBounds = cell.titleBounds();
                                const auto offset = vm::vec2f(titleBounds.left(), height - (titleBounds.top() - y) - titleBounds.height());
                                
                                auto& font = fontManager().font(cell.item().fontDescriptor);
                                const auto quads = font.quads(cell.item().texture->name(), false, offset);
                                const auto titleVertices = TextVertex::toList(std::begin(quads), std::begin(quads), std::begin(textColor), quads.size() / 2, 0, 2, 1, 2, 0, 0);
                                auto& vertices = stringVertices[cell.item().fontDescriptor];
                                vertices.insert(std::end(vertices), std::begin(titleVertices), std::end(titleVertices));
                            }
                        }
                    }
                }
            }
            
            return stringVertices;
        }

        void TextureBrowserView::doLeftClick(Layout& layout, const float x, const float y) {
            const Layout::Group::Row::Cell* result = nullptr;
            if (layout.cellAt(x, y, &result)) {
                if (!result->item().texture->overridden()) {
                    auto* texture = result->item().texture;
                    
                    TextureSelectedCommand command;
                    command.SetEventObject(this);
                    command.SetId(GetId());
                    command.setTexture(texture);
                    ProcessEvent(command);
                    
                    if (command.IsAllowed())
                        setSelectedTexture(texture);
                    
                    Refresh();
                }
            }
        }

        wxString TextureBrowserView::tooltip(const Layout::Group::Row::Cell& cell) {
            wxString tooltip;
            tooltip << cell.item().texture->name() << "\n";
            tooltip << cell.item().texture->width() << "x" << cell.item().texture->height();
            return tooltip;
        }
    }
}
