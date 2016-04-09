/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

namespace TrenchBroom {
    namespace View {
        CompilationProfileListBox::CompilationProfileListBox(wxWindow* parent, const Model::CompilationConfig& config)  :
        ImageListBox(parent, "No Profiles Found"),
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

        wxString CompilationProfileListBox::title(const size_t n) const {
            const Model::CompilationProfile* profile = m_config.profile(n);
            return profile->name();
        }
        
        wxString CompilationProfileListBox::subtitle(const size_t n) const {
            const Model::CompilationProfile* profile = m_config.profile(n);
            wxString result;
            result << profile->taskCount() << " tasks";
            return result;
        }
    }
}
