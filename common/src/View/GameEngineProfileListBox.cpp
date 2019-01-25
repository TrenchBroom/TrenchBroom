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

#include "GameEngineProfileListBox.h"

#include "Model/GameEngineConfig.h"
#include "Model/GameEngineProfile.h"
#include "View/wxUtils.h"

#include <wx/settings.h>
#include <QLabel>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        GameEngineProfileListBox::GameEngineProfileListBox(QWidget* parent, const Model::GameEngineConfig& config)  :
        ControlListBox(parent, true, "Click the '+' button to create a game engine profile."),
        m_config(config) {
            m_config.profilesDidChange.addObserver(this, &GameEngineProfileListBox::profilesDidChange);
            SetItemCount(config.profileCount());
        }
        
        GameEngineProfileListBox::~GameEngineProfileListBox() {
            m_config.profilesDidChange.removeObserver(this, &GameEngineProfileListBox::profilesDidChange);
        }
        
        Model::GameEngineProfile* GameEngineProfileListBox::selectedProfile() const {
            if (GetSelection() == wxNOT_FOUND)
                return nullptr;
            return m_config.profile(static_cast<size_t>(GetSelection()));
        }

        void GameEngineProfileListBox::profilesDidChange() {
            SetItemCount(m_config.profileCount());
        }
        
        class GameEngineProfileListBox::ProfileItem : public Item {
        private:
            Model::GameEngineProfile* m_profile;
            QLabel* m_nameText;
            QLabel* m_pathText;
        public:
            ProfileItem(QWidget* parent, Model::GameEngineProfile* profile, const wxSize& margins) :
            Item(parent),
            m_profile(profile),
            m_nameText(nullptr),
            m_pathText(nullptr) {
                ensure(m_profile != nullptr, "profile is null");
                
                m_nameText = new QLabel(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_END);
                m_pathText = new QLabel(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_MIDDLE);
                
                m_nameText->SetFont(m_nameText->GetFont().Bold());
                m_pathText->SetForegroundColour(makeLighter(m_pathText->GetForegroundColour()));
#ifndef _WIN32
                m_pathText->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
                
                auto* vSizer = new QVBoxLayout();
                vSizer->Add(m_nameText, wxSizerFlags().Expand());
                vSizer->Add(m_pathText, wxSizerFlags().Expand());
                
                auto* hSizer = new QHBoxLayout();
                hSizer->addSpacing(margins.x);
                hSizer->Add(vSizer, wxSizerFlags().Expand().Proportion(1).Border(wxTOP | wxBOTTOM, margins.y));
                hSizer->addSpacing(margins.x);
                
                SetSizer(hSizer);
                
                refresh();
                addObservers();
            }
            
            ~ProfileItem() override {
                if (m_profile != nullptr)
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
                if (m_profile != nullptr) {
                    removeObservers();
                    m_profile = nullptr;
                }
            }
            
            void profileDidChange() {
                refresh();
            }
            
            void refresh() {
                if (m_profile == nullptr) {
                    m_nameText->SetLabel("");
                    m_pathText->SetLabel("");
                } else {
                    m_nameText->SetLabel(m_profile->name());
                    m_pathText->SetLabel(m_profile->path().asString());
                }
                if (m_nameText->GetLabel().IsEmpty())
                    m_nameText->SetLabel("not set");
            }
        private:
            void setDefaultColours(const wxColour& foreground, const wxColour& background) override {
                Item::setDefaultColours(foreground, background);
                m_pathText->SetForegroundColour(makeLighter(m_pathText->GetForegroundColour()));
            }
        };
        
        ControlListBox::Item* GameEngineProfileListBox::createItem(QWidget* parent, const wxSize& margins, const size_t index) {
            Model::GameEngineProfile* profile = m_config.profile(index);
            return new ProfileItem(parent, profile, margins);
        }
    }
}
