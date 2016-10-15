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

#include "GameEngineProfile.h"

namespace TrenchBroom {
    namespace Model {
        GameEngineProfile::GameEngineProfile(const String& name, const IO::Path& path) :
        m_name(name),
        m_path(path) {}
        
        GameEngineProfile* GameEngineProfile::clone() const {
            return new GameEngineProfile(m_name, m_path);
        }

        const String& GameEngineProfile::name() const {
            return m_name;
        }
        
        const IO::Path& GameEngineProfile::path() const {
            return m_path;
        }
        
        void GameEngineProfile::setName(const String& name) {
            m_name = name;
            profileDidChange();
        }
        
        void GameEngineProfile::setPath(const IO::Path& path) {
            m_path = path;
            profileDidChange();
        }
    }
}
