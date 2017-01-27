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

#include "CompilationProfileListBox.h"

#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "View/wxUtils.h"

#include <wx/settings.h>
#include <wx/stattext.h>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        CompilationProfileListBox::CompilationProfileListBox(wxWindow* parent, const Model::CompilationConfig& config)  :
        ControlListBox(parent, true, "Click the '+' button to create a compilation profile."),
        m_config(config) {
            m_config.profilesDidChange.addObserver(this, &CompilationProfileListBox::profilesDidChange);
            SetItemCount(config.profileCount());
        }

        CompilationProfileListBox::~CompilationProfileListBox() {
            m_config.profilesDidChange.removeObserver(this, &CompilationProfileListBox::profilesDidChange);
        }

        void CompilationProfileListBox::profilesDidChange() {
            SetItemCount(m_config.profileCount());
        }

        class CompilationProfileListBox::ProfileItem : public Item {
        private:
            Model::CompilationProfile* m_profile;
            wxStaticText* m_nameText;
            wxStaticText* m_taskCountText;
        public:
            ProfileItem(wxWindow* parent, Model::CompilationProfile* profile, const wxSize& margins) :
            Item(parent),
            m_profile(profile),
            m_nameText(NULL),
            m_taskCountText(NULL) {
                ensure(m_profile != NULL, "profile is null");

                m_nameText = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_END);
                m_taskCountText = new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_MIDDLE);
                
                m_nameText->SetFont(m_nameText->GetFont().Bold());
                m_taskCountText->SetForegroundColour(makeLighter(m_taskCountText->GetForegroundColour()));
#ifndef _WIN32
                m_taskCountText->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
                
                wxSizer* vSizer = new wxBoxSizer(wxVERTICAL);
                vSizer->Add(m_nameText, wxSizerFlags().Expand());
                vSizer->Add(m_taskCountText, wxSizerFlags().Expand());
                
                wxSizer* hSizer = new wxBoxSizer(wxHORIZONTAL);
                hSizer->AddSpacer(margins.x);
                hSizer->Add(vSizer, wxSizerFlags().Expand().Proportion(1).Border(wxTOP | wxBOTTOM, margins.y));
                hSizer->AddSpacer(margins.x);

                SetSizer(hSizer);
                
                refresh();
                addObservers();
            }
            
            ~ProfileItem() {
                if (m_profile != NULL)
                    removeObservers();
            }
        private:
            void addObservers() {
                m_profile->profileWillBeRemoved.addObserver(this, &ProfileItem::profileWillBeRemoved);
                m_profile->profileDidChange.addObserver(this, &ProfileItem::profileDidChange);
            }
            
            void removeObservers() {
                m_profile->profileWillBeRemoved.removeObserver(this, &ProfileItem::profileWillBeRemoved);
                m_profile->profileDidChange.removeObserver(this, &ProfileItem::profileDidChange);
            }
            
            void profileWillBeRemoved() {
                if (m_profile != NULL) {
                    removeObservers();
                    m_profile = NULL;
                }
            }
            
            void profileDidChange() {
                refresh();
            }
            
            void refresh() {
                if (m_profile == NULL) {
                    m_nameText->SetLabel("");
                    m_taskCountText->SetLabel("");
                } else {
                    m_nameText->SetLabel(m_profile->name());
                    wxString taskCountLabel;
                    taskCountLabel << m_profile->taskCount() << " tasks";
                    m_taskCountText->SetLabel(taskCountLabel);
                }
            }
        private:
            void setDefaultColours(const wxColour& foreground, const wxColour& background) {
                Item::setDefaultColours(foreground, background);
                m_taskCountText->SetForegroundColour(makeLighter(m_taskCountText->GetForegroundColour()));
            }
        };

        ControlListBox::Item* CompilationProfileListBox::createItem(wxWindow* parent, const wxSize& margins, const size_t index) {
            Model::CompilationProfile* profile = m_config.profile(index);
            return new ProfileItem(parent, profile, margins);
        }
    }
}
