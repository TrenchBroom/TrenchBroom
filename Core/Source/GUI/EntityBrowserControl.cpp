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

#include "EntityBrowserControl.h"

#include "Gwen/Controls/ScrollControl.h"
#include "Gwen/Skin.h"

#include "GL/Glee.h"

#include "Controller/Editor.h"
#include "Model/Map/EntityDefinition.h"
#include "Model/Map/Map.h"
#include "Model/Preferences.h"
#include "Renderer/EntityRendererManager.h"
#include "Renderer/EntityRenderer.h"
#include "Renderer/MapRenderer.h"
#include "Utilities/VecMath.h"

namespace TrenchBroom {
    namespace Gui {
        void EntityBrowserPanel::reloadEntityDefinitions() {
            m_layout.clear();
            
            Model::EntityDefinitionManager& defManager = m_editor.map().entityDefinitionManager();
            const vector<Model::EntityDefinitionPtr> definitions = defManager.definitions();
            
            for (unsigned int i = 0; i < definitions.size(); i++) {
                Model::EntityDefinitionPtr definition = definitions[i];
                if (definition->type != Model::TB_EDP_BASE) {
                    Gwen::Skin::Base* skin = GetSkin();
                    Gwen::Font* actualFont = new Gwen::Font(*m_font);
                    FontPtr actualFontPtr(actualFont);
                    Gwen::Point actualSize;
                    
                    float fixedCellWidth = m_layout.fixedCellWidth();
                    if  (m_layout.fixedCellWidth() > 0) {
                        Gwen::Renderer::Base* renderer = skin->GetRender();
                        while (actualFont->size > 5 && (actualSize = renderer->MeasureText(actualFont, definition->name)).x > fixedCellWidth)
                            actualFont->size -= 1.0f;
                    } else {
                        actualSize = GetSkin()->GetRender()->MeasureText(m_font, definition->name);
                    }
                    
                    Vec3f size = definition->bounds.size();
                    if (size.x < m_layout.fixedCellWidth() && size.z < m_layout.fixedCellWidth()) {
                        float f = Math::fmax(size.x, size.z);
                        size *= m_layout.fixedCellWidth() / f;
                    }
                    m_layout.addItem(CellData(definition, actualFontPtr), size.x, size.z, static_cast<float>(actualSize.x), actualFont->size + 2);
                }
            }
            
            const Gwen::Padding& padding = GetPadding();
            int controlHeight = static_cast<int>(m_layout.height()) + padding.top + padding.bottom;
            SetBounds(GetBounds().x, GetBounds().y, GetBounds().w, controlHeight);
        }

        EntityBrowserPanel::EntityBrowserPanel(Gwen::Controls::Base* parent, Controller::Editor& editor) : Base(parent), m_editor(editor) {
            m_layout.setCellMargin(8);
            m_layout.setRowMargin(8);
            m_layout.setGroupMargin(8);
            m_layout.setFixedCellWidth(128);
            m_layout.setWidth(GetBounds().w);
            SetFont(GetSkin()->GetDefaultFont());
            reloadEntityDefinitions();
        }
        
        EntityBrowserPanel::~EntityBrowserPanel() {}

        void EntityBrowserPanel::SetFont(Gwen::Font* font) {
            m_font = font;
        }
        
        Gwen::Font* EntityBrowserPanel::GetFont() {
            return m_font;
        }
        
        void EntityBrowserPanel::SetPadding(const Gwen::Padding& padding) {
            Base::SetPadding(padding);
            m_layout.setWidth(GetBounds().w - padding.left - padding.right);
        }
        
        void EntityBrowserPanel::OnBoundsChanged( Gwen::Rect oldBounds ) {
            Base::OnBoundsChanged(oldBounds);
            
            const Gwen::Padding& padding = GetPadding();
            m_layout.setWidth(GetBounds().w - padding.left - padding.right);
            
            int controlHeight = static_cast<int>(m_layout.height()) + padding.top + padding.bottom;
            SetBounds(GetBounds().x, GetBounds().y, GetBounds().w, controlHeight);
        }
        
        void EntityBrowserPanel::RenderOver(Gwen::Skin::Base* skin) {
            skin->GetRender()->Flush();
            
            const Gwen::Padding& padding = GetPadding();
            const Gwen::Point& offset = skin->GetRender()->GetRenderOffset();
            const Gwen::Rect& scrollerVisibleRect = ((Gwen::Controls::ScrollControl*)GetParent())->GetVisibleRect();
            const Gwen::Rect& bounds = GetRenderBounds();
            Gwen::Rect visibleRect(bounds.x, -scrollerVisibleRect.y, bounds.w, GetParent()->GetBounds().h);
            
            glMatrixMode(GL_MODELVIEW);
            glPushMatrix();
            glTranslatef(offset.x, offset.y, 0);
            glTranslatef(padding.left, padding.top, 0);
            
            Model::Preferences& prefs = *Model::Preferences::sharedPreferences;
            float brightness = prefs.brightness();
            float color[4] = {brightness / 2.0f, brightness / 2.0f, brightness / 2.0f, 1.0f};
            
            glPushAttrib(GL_TEXTURE_BIT);
            glEnable(GL_TEXTURE_2D);
            glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
            glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
            glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
            glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
            glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);

            Renderer::EntityRendererManager& rendererManager = m_editor.renderer()->entityRendererManager();
            rendererManager.activate();
            
            for (unsigned int i = 0; i < m_layout.size(); i++) {
                CellLayout<CellData, GroupData>::CellGroupPtr group = m_layout[i];
                if (group->intersectsY(visibleRect.y, visibleRect.h)) {
                    for (unsigned int j = 0; j < group->size(); j++) {
                        CellGroup<CellData, GroupData>::CellRowPtr row = (*group)[j];
                        if (row->intersectsY(visibleRect.y, visibleRect.h)) {
                            for (unsigned int k = 0; k < row->size(); k++) {
                                CellRow<CellData>::CellPtr cell = (*row)[k];
                                const LayoutBounds& itemBounds = cell->itemBounds();
                                Model::EntityDefinitionPtr definition = cell->item().first;
                                
                                Renderer::EntityRenderer* renderer = rendererManager.entityRenderer(*definition, m_editor.map().mods());
                                if (renderer != NULL) {
                                    glPushMatrix();
                                    Vec3f size = definition->bounds.size();
                                    float scale;
                                    if (size.x > size.z) scale = itemBounds.width() / size.x;
                                    else scale = itemBounds.height() / size.z;
                                    renderer->render(Vec3f(itemBounds.left(), itemBounds.top(), 0.0f), 0.0f, scale);
                                    glPopMatrix();
                                }

                            }
                        }
                    }
                }
            }
            rendererManager.deactivate();

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
                            Model::EntityDefinitionPtr definition = cell->item().first;
                            FontPtr font = cell->item().second;
                            skin->GetRender()->RenderText(font.get(), Gwen::Point(padding.left + titleBounds.left(), padding.top + titleBounds.top() + 1), definition->name);
                        }
                    }
                }
            }
        }
        
        EntityBrowserControl::EntityBrowserControl(Gwen::Controls::Base* parent, Controller::Editor& editor) : Base(parent), m_editor(editor) {
            m_browserScroller = new Gwen::Controls::ScrollControl(this);
            m_browserScroller->Dock(Gwen::Pos::Fill);
            m_browserScroller->SetScroll(false, true);
            
            m_browserPanel = new EntityBrowserPanel(m_browserScroller, m_editor);
            m_browserPanel->Dock(Gwen::Pos::Top);
            m_browserPanel->SetPadding(Gwen::Padding(5, 5, 5, 5));
        }
        
        EntityBrowserControl::~EntityBrowserControl() {}

        void EntityBrowserControl::Render(Gwen::Skin::Base* skin) {
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
    }
}