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
#include <QLabel>
#include <wx/sizer.h>

namespace TrenchBroom {
    namespace View {
        CompilationProfileListBox::CompilationProfileListBox(QWidget* parent, const Model::CompilationConfig& config)  :
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
            QLabel* m_nameText;
            QLabel* m_taskCountText;
        public:
            ProfileItem(QWidget* parent, Model::CompilationProfile* profile, const wxSize& margins) :
            Item(parent),
            m_profile(profile),
            m_nameText(nullptr),
            m_taskCountText(nullptr) {
                ensure(m_profile != nullptr, "profile is null");

                m_nameText = new QLabel(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_END);
                m_taskCountText = new QLabel(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,  wxST_ELLIPSIZE_MIDDLE);
                
                m_nameText->SetFont(m_nameText->GetFont().Bold());
                m_taskCountText->SetForegroundColour(makeLighter(m_taskCountText->GetForegroundColour()));
#ifndef _WIN32
                m_taskCountText->SetWindowVariant(wxWINDOW_VARIANT_SMALL);
#endif
                
                auto* vSizer = new QVBoxLayout();
                vSizer->Add(m_nameText, wxSizerFlags().Expand());
                vSizer->Add(m_taskCountText, wxSizerFlags().Expand());
                
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
                    m_taskCountText->SetLabel("");
                } else {
                    m_nameText->SetLabel(m_profile->name());
                    QString taskCountLabel;
                    taskCountLabel << m_profile->taskCount() << " tasks";
                    m_taskCountText->SetLabel(taskCountLabel);
                }
            }
        private:
            void setDefaultColours(const wxColour& foreground, const wxColour& background) override {
                Item::setDefaultColours(foreground, background);
                m_taskCountText->SetForegroundColour(makeLighter(m_taskCountText->GetForegroundColour()));
            }
        };

        ControlListBox::Item* CompilationProfileListBox::createItem(QWidget* parent, const wxSize& margins, const size_t index) {
            Model::CompilationProfile* profile = m_config.profile(index);
            return new ProfileItem(parent, profile, margins);
        }
    }
}
