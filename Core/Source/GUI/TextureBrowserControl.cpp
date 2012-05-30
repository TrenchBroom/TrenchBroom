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

#include "TextureBrowserControl.h"
#include "Controller/Editor.h"
#include "Model/Map/Map.h"
#include "Model/Preferences.h"
#include "Model/Selection.h"
#include "Renderer/RenderUtils.h"
#include "Utilities/Utils.h"
#include "Utilities/VecMath.h"
#include "Gwen/Skin.h"
#include "Gwen/Controls/ScrollControl.h"
#include "Gwen/Utility.h"
#include <vector>

namespace TrenchBroom {
    namespace Gui {
        
        void TextureBrowserPanel::selectionChanged(const Model::SelectionEventData& data) {
            Redraw();
        }

        void TextureBrowserPanel::textureManagerDidChange(Model::Assets::TextureManager& textureManager) {
            reloadTextures();
            Redraw();
        }
        
        void TextureBrowserPanel::preferencesDidChange(const std::string& key) {
            Redraw();
        }

        void TextureBrowserPanel::addTexture(Model::Assets::Texture* texture) {
            if ((!m_hideUnused || texture->usageCount > 0) && (m_filterText.empty() || containsString(texture->name, m_filterText, false))) {
                Gwen::Skin::Base* skin = GetSkin();
                Gwen::Font* actualFont = new Gwen::Font(*m_font);
                FontPtr actualFontPtr(actualFont);
                Gwen::Point actualSize;

                float fixedCellWidth = m_layout.fixedCellWidth();
                if  (m_layout.fixedCellWidth() > 0) {
                    Gwen::Renderer::Base* renderer = skin->GetRender();
                    while (actualFont->size > 5 && (actualSize = renderer->MeasureText(actualFont, texture->name)).x > fixedCellWidth)
                        actualFont->size -= 1.0f;
                } else {
                    actualSize = GetSkin()->GetRender()->MeasureText(m_font, texture->name);
                }

                m_layout.addItem(CellData(texture, actualFontPtr), texture->width, texture->height, static_cast<float>(actualSize.x), actualFont->size + 2);
            }
        }

        void TextureBrowserPanel::reloadTextures() {
            m_layout.clear();
            if (m_group) {
                const vector<Model::Assets::TextureCollection*>& collections = m_editor.textureManager().collections();
                for (unsigned int i = 0; i < collections.size(); i++) {
                    Model::Assets::TextureCollection* collection = collections[i];
                    if (m_group)
                        m_layout.addGroup(collection, m_font->size + 2);
                    
                    std::vector<Model::Assets::Texture*> textures = collection->textures(m_sortCriterion);
                    for (unsigned int j = 0; j < textures.size(); j++)
                        addTexture(textures[j]);
                }
            } else {
                std::vector<Model::Assets::Texture*> textures = m_editor.textureManager().textures(m_sortCriterion);
                for (unsigned int i = 0; i < textures.size(); i++)
                    addTexture(textures[i]);
            }

            const Gwen::Padding& padding = GetPadding();
            int controlHeight = static_cast<int>(m_layout.height()) + padding.top + padding.bottom;
            SetBounds(GetBounds().x, GetBounds().y, GetBounds().w, controlHeight);
        }

        void TextureBrowserPanel::renderTextureBorder(CellRow<CellData>::CellPtr cell) {
            const LayoutBounds& itemBounds = cell->itemBounds();
            
            glBegin(GL_QUADS);
            glVertex3f(itemBounds.left()  - 1.0f, itemBounds.top()    - 1.0f, 0);
            glVertex3f(itemBounds.left()  - 1.0f, itemBounds.bottom() + 1.0f, 0);
            glVertex3f(itemBounds.right() + 1.0f, itemBounds.bottom() + 1.0f, 0);
            glVertex3f(itemBounds.right() + 1.0f, itemBounds.top()    - 1.0f, 0);
            glEnd();
        }

        void TextureBrowserPanel::OnMouseMoved(int x, int y, int deltaX, int deltaY) {
            Gwen::Point local = CanvasPosToLocal(Gwen::Point(x, y));
            CellRow<CellData>::CellPtr cell;
            if (m_layout.cellAt(static_cast<float>(local.x), static_cast<float>(local.y), cell)) {
                DragAndDrop_SetPackage(true, "Texture", cell->item().first);
            }
        }

        void TextureBrowserPanel::OnMouseClickLeft(int x, int y, bool down) {
            if (!down)
                return;
            
            Gwen::Point local = CanvasPosToLocal(Gwen::Point(x, y));
            CellRow<CellData>::CellPtr cell;
            if (m_layout.cellAt(static_cast<float>(local.x), static_cast<float>(local.y), cell)) {
                m_selectedTexture = cell->item().first;
                OnTextureSelected();
            }
        }
        
        void TextureBrowserPanel::OnTextureSelected() {
            m_bCacheTextureDirty = true;
            Redraw();
            onTextureSelected.Call(this);
        }
        
        TextureBrowserPanel::TextureBrowserPanel(Gwen::Controls::Base* parent, Controller::Editor& editor) : Gwen::Controls::Base(parent), m_editor(editor), m_group(false), m_selectedTexture(NULL) {
            m_layout.setGroupMargin(8);
            m_layout.setRowMargin(8);
            m_layout.setCellMargin(8);
            m_layout.setWidth(GetBounds().w);
            m_group = false;
            m_hideUnused = false;
            m_layout.setFixedCellWidth(64);
            m_sortCriterion = Model::Assets::TB_TS_NAME;
            SetFont(GetSkin()->GetDefaultFont());
            reloadTextures();
            
            m_editor.map().selection().selectionAdded                   += new Model::Selection::SelectionEvent::Listener<TextureBrowserPanel>(this, &TextureBrowserPanel::selectionChanged);
            m_editor.map().selection().selectionRemoved                 += new Model::Selection::SelectionEvent::Listener<TextureBrowserPanel>(this, &TextureBrowserPanel::selectionChanged);
            m_editor.textureManager().textureManagerDidChange           += new Model::Assets::TextureManager::TextureManagerEvent::Listener<TextureBrowserPanel>(this, &TextureBrowserPanel::textureManagerDidChange);
            Model::Preferences::sharedPreferences->preferencesDidChange += new Model::Preferences::PreferencesEvent::Listener<TextureBrowserPanel>(this, &TextureBrowserPanel::preferencesDidChange);
        }

        TextureBrowserPanel::~TextureBrowserPanel() {
            m_editor.map().selection().selectionAdded                   -= new Model::Selection::SelectionEvent::Listener<TextureBrowserPanel>(this, &TextureBrowserPanel::selectionChanged);
            m_editor.map().selection().selectionRemoved                 -= new Model::Selection::SelectionEvent::Listener<TextureBrowserPanel>(this, &TextureBrowserPanel::selectionChanged);
            m_editor.textureManager().textureManagerDidChange           -= new Model::Assets::TextureManager::TextureManagerEvent::Listener<TextureBrowserPanel>(this, &TextureBrowserPanel::textureManagerDidChange);
            Model::Preferences::sharedPreferences->preferencesDidChange -= new Model::Preferences::PreferencesEvent::Listener<TextureBrowserPanel>(this, &TextureBrowserPanel::preferencesDidChange);
        }
        
        void TextureBrowserPanel::SetFont(Gwen::Font* font) {
            m_font = font;
        }
        
        Gwen::Font* TextureBrowserPanel::GetFont() {
            return m_font;
        }

        void TextureBrowserPanel::SetPadding(const Gwen::Padding& padding) {
            Base::SetPadding(padding);
            m_layout.setWidth(GetBounds().w - padding.left - padding.right);
        }
        
        void TextureBrowserPanel::OnBoundsChanged( Gwen::Rect oldBounds ) {
            Base::OnBoundsChanged(oldBounds);

            const Gwen::Padding& padding = GetPadding();
            m_layout.setWidth(GetBounds().w - padding.left - padding.right);

            int controlHeight = static_cast<int>(m_layout.height()) + padding.top + padding.bottom;
            SetBounds(GetBounds().x, GetBounds().y, GetBounds().w, controlHeight);
        }

        void TextureBrowserPanel::RenderOver(Gwen::Skin::Base* skin) {
            skin->GetRender()->Flush();
            
            const Gwen::Padding& padding = GetPadding();
            const Gwen::Point& offset = skin->GetRender()->GetRenderOffset();
            const Gwen::Rect& scrollerVisibleRect = ((Gwen::Controls::ScrollControl*)GetParent())->GetVisibleRect();
            const Gwen::Rect& bounds = GetRenderBounds();
            Gwen::Rect visibleRect(bounds.x, -scrollerVisibleRect.y, bounds.w, GetParent()->GetBounds().h);
            
            Model::Map& map = m_editor.map();
            Model::Selection& selection = map.selection();
            Model::Assets::TextureManager& textureManager = m_editor.textureManager();
            
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glTranslatef(offset.x, offset.y, 0);
            glTranslatef(padding.left, padding.top, 0);
            
            glPushAttrib(GL_TEXTURE_BIT);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            float brightness = prefs.brightness();
            float color[4] = {brightness / 2.0f, brightness / 2.0f, brightness / 2.0f, 1.0f};
            
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
            glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);

            for (unsigned int i = 0; i < m_layout.size(); i++) {
                CellLayout<CellData, GroupData>::CellGroupPtr group = m_layout[i];
                if (group->intersectsY(visibleRect.y, visibleRect.h)) {
                    for (unsigned int j = 0; j < group->size(); j++) {
                        CellGroup<CellData, GroupData>::CellRowPtr row = (*group)[j];
                        if (row->intersectsY(visibleRect.y, visibleRect.h)) {
                            for (unsigned int k = 0; k < row->size(); k++) {
                                CellRow<CellData>::CellPtr cell = (*row)[k];
                                const LayoutBounds& itemBounds = cell->itemBounds();
                                Model::Assets::Texture* texture = cell->item().first;
                                
                                // paint border if necessary
                                bool override = textureManager.texture(texture->name) != texture;
                                if (override) {
                                    glDisable(GL_TEXTURE_2D);
                                    Renderer::glColorV4f(prefs.overriddenTextureColor());
                                    renderTextureBorder(cell);
                                } else if (!selection.mruTextures().empty() && selection.mruTextures().back() == texture) {
                                    glDisable(GL_TEXTURE_2D);
                                    Renderer::glColorV4f(prefs.selectedTextureColor());
                                    renderTextureBorder(cell);
                                } else if (texture->usageCount > 0) {
                                    glDisable(GL_TEXTURE_2D);
                                    Renderer::glColorV4f(prefs.usedTextureColor());
                                    renderTextureBorder(cell);
                                }
                                
                                // render the texture
                                glEnable(GL_TEXTURE_2D);
                                texture->activate();
                                if (override)
                                    glColor4f(1, 1, 1, 0.7f);
                                else
                                    glColor4f(1, 1, 1, 1);
                                glBegin(GL_QUADS);
                                glTexCoord2f(0, 0);
                                glVertex3f(itemBounds.left(), itemBounds.top(), 0);
                                glTexCoord2f(0, 1);
                                glVertex3f(itemBounds.left(), itemBounds.bottom(), 0);
                                glTexCoord2f(1, 1);
                                glVertex3f(itemBounds.right(), itemBounds.bottom(), 0);
                                glTexCoord2f(1, 0);
                                glVertex3f(itemBounds.right(), itemBounds.top(), 0);
                                glEnd();
                                texture->deactivate();
                            }
                        }
                    }
                }
            }
            glPopMatrix();
            glPopAttrib();
            
            for (unsigned int i = 0; i < m_layout.size(); i++) {
                CellLayout<CellData, GroupData>::CellGroupPtr group = m_layout[i];

                skin->GetRender()->SetDrawColor(Gwen::Color(255, 255, 255, 255));
                for (unsigned int j = 0; j < group->size(); j++) {
                    CellGroup<CellData, GroupData>::CellRowPtr row = (*group)[j];
                    for (unsigned int k = 0; k < row->size(); k++) {
                        CellRow<CellData>::CellPtr cell = (*row)[k];
                        const LayoutBounds& titleBounds = cell->titleBounds();
                        if (titleBounds.intersectsY(visibleRect.y, visibleRect.h)) {
                            Model::Assets::Texture* texture = cell->item().first;
                            FontPtr font = cell->item().second;
                            skin->GetRender()->RenderText(font.get(), Gwen::Point(padding.left + titleBounds.left(), padding.top + titleBounds.top() + 1), texture->name);
                        }
                    }
                }

                if (m_group) {
                    const LayoutBounds titleBounds = group->titleBoundsForVisibleRect(visibleRect.y, visibleRect.h);
                    if (titleBounds.intersectsY(visibleRect.y, visibleRect.h)) {
                        // paint background for group title
                        glPushMatrix();
                        glTranslatef(offset.x + padding.left, offset.y + padding.right, 0);
                        glDisable(GL_TEXTURE_2D);
                        glBegin(GL_QUADS);
                        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
                        glVertex3f(titleBounds.left(),  titleBounds.top(),    0);
                        glVertex3f(titleBounds.left(),  titleBounds.bottom(), 0);
                        glVertex3f(titleBounds.right(), titleBounds.bottom(), 0);
                        glVertex3f(titleBounds.right(), titleBounds.top(),    0);
                        glEnd();
                        glPopMatrix();
                        
                        Model::Assets::TextureCollection* collection = group->item();
                        std::vector<std::string> components = pathComponents(collection->name());
                        skin->GetRender()->SetDrawColor(Gwen::Color(255, 255, 255, 255));
                        skin->GetRender()->RenderText(m_font, Gwen::Point(padding.left + titleBounds.left() + 3, padding.top + titleBounds.top() + 1), components.back());
                    }
                }
            }
        }

        void TextureBrowserPanel::setHideUnused(bool hideUnused) {
            if (m_hideUnused == hideUnused)
                return;
            m_hideUnused = hideUnused;
            reloadTextures();
        }
        
        void TextureBrowserPanel::setGroup(bool group) {
            if (m_group == group)
                return;
            m_group = group;
            reloadTextures();
        }

        void TextureBrowserPanel::setSortCriterion(Model::Assets::ETextureSortCriterion criterion) {
            if (m_sortCriterion == criterion)
                return;
            m_sortCriterion = criterion;
            reloadTextures();
        }

        void TextureBrowserPanel::setFixedCellWidth(float fixedCellWidth) {
            m_layout.setFixedCellWidth(fixedCellWidth);
        }

        void TextureBrowserPanel::setFilterText(const std::string& filterText) {
            if (m_filterText.compare(filterText) == 0)
                return;
            m_filterText = filterText;
            reloadTextures();
        }
        
        
        Model::Assets::Texture* TextureBrowserPanel::selectedTexture() {
            return m_selectedTexture;
        }

        void TextureBrowserControl::onTextureSelectedInBrowserPanel(Gwen::Controls::Base* control) {
            onTextureSelected.Call(this);
        }

        TextureBrowserControl::TextureBrowserControl(Gwen::Controls::Base* parent, Controller::Editor& editor) : Gwen::Controls::Base(parent), m_editor(editor) {
            m_browserScroller = new Gwen::Controls::ScrollControl(this);
            m_browserScroller->Dock(Gwen::Pos::Fill);
            m_browserScroller->SetScroll(false, true);
            
            m_browserPanel = new TextureBrowserPanel(m_browserScroller, editor);
            m_browserPanel->Dock(Gwen::Pos::Top);
            m_browserPanel->SetPadding(Gwen::Padding(5, 5, 5, 5));
            m_browserPanel->onTextureSelected.Add(this, &TextureBrowserControl::onTextureSelectedInBrowserPanel);
        }

        void TextureBrowserControl::Render(Gwen::Skin::Base* skin) {
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            const Vec4f& backgroundColor = prefs.backgroundColor();
            Gwen::Color drawColor(static_cast<unsigned char>(Math::fround(backgroundColor.x * 255.0f)),
                                  static_cast<unsigned char>(Math::fround(backgroundColor.y * 255.0f)),
                                  static_cast<unsigned char>(Math::fround(backgroundColor.z * 255.0f)),
                                  static_cast<unsigned char>(Math::fround(backgroundColor.w * 255.0f)));
            
            skin->DrawBox(this);
            skin->GetRender()->SetDrawColor(drawColor);
            skin->GetRender()->DrawFilledRect(GetRenderBounds());
        }
        
        void TextureBrowserControl::setHideUnused(bool hideUnused) {
            m_browserPanel->setHideUnused(hideUnused);
        }

        void TextureBrowserControl::setGroup(bool group) {
            m_browserPanel->setGroup(group);
        }
    
        void TextureBrowserControl::setSortCriterion(Model::Assets::ETextureSortCriterion criterion) {
            m_browserPanel->setSortCriterion(criterion);
        }
        
        void TextureBrowserControl::setFixedCellWidth(float fixedCellWidth) {
            m_browserPanel->setFixedCellWidth(fixedCellWidth);
        }
        
        void TextureBrowserControl::setFilterText(const std::string& filterText) {
            m_browserPanel->setFilterText(filterText);
        }
        
        Model::Assets::Texture* TextureBrowserControl::selectedTexture() {
            return m_browserPanel->selectedTexture();
        }
    }
}