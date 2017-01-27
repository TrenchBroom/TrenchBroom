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

#ifndef GameEngineProfileListBox_h
#define GameEngineProfileListBox_h

#include "View/ControlListBox.h"

namespace TrenchBroom {
    namespace Model {
        class GameEngineConfig;
        class GameEngineProfile;
    }
    
    namespace View {
        class GameEngineProfileListBox : public ControlListBox {
        private:
            const Model::GameEngineConfig& m_config;
        public:
            GameEngineProfileListBox(wxWindow* parent, const Model::GameEngineConfig& config);
            ~GameEngineProfileListBox();
            
            Model::GameEngineProfile* selectedProfile() const;
        private:
            void profilesDidChange();
        private:
            class ProfileItem;
            Item* createItem(wxWindow* parent, const wxSize& margins, size_t index);
        };
    }
}

#endif /* GameEngineProfileListBox_h */
