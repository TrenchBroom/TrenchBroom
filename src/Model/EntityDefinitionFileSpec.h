/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#ifndef __TrenchBroom__EntityDefinitionFileSpec__
#define __TrenchBroom__EntityDefinitionFileSpec__

#include "IO/Path.h"

namespace TrenchBroom {
    namespace Model {
        class EntityDefinitionFileSpec {
        private:
            bool m_builtin;
            IO::Path m_path;
            IO::Path m_fullPath;
        public:
            EntityDefinitionFileSpec();
            
            static EntityDefinitionFileSpec builtin(const IO::Path& path, const IO::Path& fullPath);
            static EntityDefinitionFileSpec external(const IO::Path& fullPath);
            
            bool builtin() const;
            const IO::Path& path() const;
            const IO::Path& fullPath() const;
        private:
            EntityDefinitionFileSpec(bool builtin, const IO::Path& path, const IO::Path& fullPath);
        };
    }
}

#endif /* defined(__TrenchBroom__EntityDefinitionFileSpec__) */
