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

#include "NavBar.h"

#include "Controller/Command.h"
#include "Model/Brush.h"
#include "Model/EditStateManager.h"
#include "Model/Entity.h"
#include "Model/EntityDefinition.h"
#include "Model/MapDocument.h"
#include "Utility/List.h"
#include "View/DocumentViewHolder.h"
#include "View/EditorView.h"
#include "View/ViewOptions.h"

#include <wx/dcclient.h>
#include <wx/sizer.h>
#include <wx/srchctrl.h>
#include <wx/stattext.h>

namespace TrenchBroom {
    namespace View {
        wxStaticText* NavBar::makeBreadcrump(const wxString& text, bool link) {
            wxStaticText* staticText = new wxStaticText(m_navPanel, wxID_ANY, text);
#ifdef __APPLE__
            staticText->SetFont(*wxSMALL_FONT);
#endif
            if (link) {
                staticText->SetForegroundColour(wxColour(10, 75, 220)); //wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT));
                staticText->SetCursor(wxCursor(wxCURSOR_HAND));
            }
            return staticText;
        }

        NavBar::NavBar(wxWindow* parent, DocumentViewHolder& documentViewHolder) :
        wxPanel(parent),
        m_documentViewHolder(documentViewHolder),
        m_navPanel(new wxPanel(this, wxID_ANY)),
        m_searchBox(new wxSearchCtrl(this, wxID_ANY)) {
#ifdef __APPLE__
            m_searchBox->SetFont(*wxSMALL_FONT);
            SetBackgroundStyle(wxBG_STYLE_PAINT);
            Bind(wxEVT_PAINT, &NavBar::OnPaint, this);
#endif
            m_searchBox->Bind(wxEVT_COMMAND_TEXT_UPDATED, &NavBar::OnSearchPatternChanged, this);
            
            wxSizer* innerSizer = new wxBoxSizer(wxHORIZONTAL);
            innerSizer->AddSpacer(4);
            innerSizer->Add(m_navPanel, 1, wxEXPAND | wxALIGN_CENTRE_VERTICAL);
            innerSizer->Add(m_searchBox, 0, wxEXPAND | wxALIGN_RIGHT);
#ifdef __APPLE__
            innerSizer->AddSpacer(4);
#endif
            innerSizer->SetItemMinSize(m_searchBox, 200, wxDefaultSize.y);
            
            wxSizer* outerSizer = new wxBoxSizer(wxVERTICAL);
            outerSizer->AddSpacer(2);
            outerSizer->Add(innerSizer, 1, wxEXPAND);
            outerSizer->AddSpacer(2);
            SetSizer(outerSizer);
    }

        void NavBar::OnPaint(wxPaintEvent& event) {
            wxPaintDC dc(this);
            wxRect rect = GetClientRect();
            rect.height -= 1;
            dc.GradientFillLinear(rect, wxColour(211, 211, 211), wxColour(174, 174, 174), wxDOWN);
            dc.SetPen(wxPen(wxColour(67, 67, 67)));
            dc.DrawLine(0, rect.height, rect.width, rect.height);
            dc.DrawLine(rect.width - 1, 0, rect.width - 1, rect.height);
        }

        void NavBar::OnSearchPatternChanged(wxCommandEvent& event) {
            if (!m_documentViewHolder.valid())
                return;
            
            EditorView& editorView = m_documentViewHolder.view();
            editorView.viewOptions().setFilterPattern(m_searchBox->GetValue().ToStdString());
            Controller::Command command(Controller::Command::InvalidateRendererState);
            editorView.OnUpdate(NULL, &command);
        }
        
        void NavBar::updateBreadcrump() {
            m_navPanel->DestroyChildren();
            
            Model::EditStateManager& editStateManager = m_documentViewHolder.document().editStateManager();
            Model::EntitySet entities = Utility::makeSet(editStateManager.selectedEntities());
            const Model::BrushList& brushes = editStateManager.selectedBrushes();
            size_t totalEntityBrushCount = 0;
            
            Model::BrushList::const_iterator brushIt, brushEnd;
            for (brushIt = brushes.begin(), brushEnd = brushes.end(); brushIt != brushEnd; ++brushIt) {
                const Model::Brush& brush = **brushIt;
                Model::Entity* entity = brush.entity();
                entities.insert(entity);
            }
            
            Model::EntitySet::const_iterator entityIt, entityEnd;
            for (entityIt = entities.begin(), entityEnd = entities.end(); entityIt != entityEnd; ++entityIt) {
                const Model::Entity& entity = **entityIt;
                totalEntityBrushCount += entity.brushes().size();
            }
            
            wxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);
            if (entities.empty() && brushes.empty()) {
                sizer->Add(makeBreadcrump(wxT("no selection"), false), 0, wxALIGN_CENTRE_VERTICAL);
            } else {
                if (entities.size() == 1 && (*entities.begin())->worldspawn()) {
                    const Model::Entity& entity = **entities.begin();
                    const String classname = entity.classname() != NULL ? *entity.classname() : Model::Entity::NoClassnameValue;
                    sizer->Add(makeBreadcrump(classname, false), 0, wxALIGN_CENTRE_VERTICAL);
                } else {
                    entityIt = entities.begin();
                    entityEnd = entities.end();
                    
                    const Model::Entity* entity = *entityIt++;
                    const String firstClassname = entity->classname() != NULL ? *entity->classname() : Model::Entity::NoClassnameValue;
                    bool sameClassname = true;
                    while (entityIt != entityEnd && sameClassname) {
                        entity = *entityIt++;
                        const String classname = entity->classname() != NULL ? *entity->classname() : Model::Entity::NoClassnameValue;
                        sameClassname = (classname == firstClassname);
                    }
                    
                    wxString entityString;
                    entityString << entities.size() << " ";
                    if (sameClassname)
                        entityString << firstClassname << " ";
                    entityString << (entities.size() == 1 ? "entity" : "entities");
                    sizer->Add(makeBreadcrump(entityString, false), 0, wxALIGN_CENTRE_VERTICAL);
                }
                if (!brushes.empty()) {
                    sizer->AddSpacer(2);
                    sizer->Add(makeBreadcrump(L"\u00BB", false), 0, wxALIGN_CENTRE_VERTICAL);
                    sizer->AddSpacer(2);
                    
                    wxString brushString;
                    brushString << brushes.size() << "/" << totalEntityBrushCount << (totalEntityBrushCount == 1 ? " brush" : " brushes");
                    sizer->Add(makeBreadcrump(brushString, false), 0, wxALIGN_CENTRE_VERTICAL);
                }
            }
            m_navPanel->SetSizer(sizer);
            
            Layout();
        }
    }
}
