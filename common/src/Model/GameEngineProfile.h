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

#ifndef GameEngineProfile_h
#define GameEngineProfile_h

#include "Macros.h"
#include "Notifier.h"
#include "StringUtils.h"
#include "IO/Path.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class GameEngineProfile {
        public:
            typedef std::vector<GameEngineProfile*> List;
        private:
            String m_name;
            IO::Path m_path;
            String m_parameterSpec;
        public:
            Notifier0 profileWillBeRemoved;
            Notifier0 profileDidChange;
        public:
            GameEngineProfile(const String& name, const IO::Path& path, const String& parameterSpec);
            
            GameEngineProfile* clone() const;
            
            const String& name() const;
            const IO::Path& path() const;
            const String& parameterSpec() const;
            
            void setName(const String& name);
            void setPath(const IO::Path& path);
            void setParameterSpec(const String& parameterSpec);

            deleteCopyAndAssignment(GameEngineProfile)
        };
    }
}

#endif /* GameEngineProfile_h */
