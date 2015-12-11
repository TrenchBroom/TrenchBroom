/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#ifndef TrenchBroom_EntityDefinitionFileSpec
#define TrenchBroom_EntityDefinitionFileSpec

#include "IO/Path.h"

#include <vector>

namespace TrenchBroom {
    namespace Assets {
        class EntityDefinitionFileSpec {
        public:
            typedef std::vector<EntityDefinitionFileSpec> List;
        private:
            typedef enum {
                Type_Builtin,
                Type_External,
                Type_Unset
            } Type;
            
            Type m_type;
            IO::Path m_path;
        public:
            EntityDefinitionFileSpec();
            
            static EntityDefinitionFileSpec parse(const String& str);
            static EntityDefinitionFileSpec builtin(const IO::Path& path);
            static EntityDefinitionFileSpec external(const IO::Path& path);
            static EntityDefinitionFileSpec unset();
            
            bool operator<(const EntityDefinitionFileSpec& rhs) const;
            bool operator==(const EntityDefinitionFileSpec& rhs) const;

            bool valid() const;
            bool builtin() const;
            bool external() const;
            
            const IO::Path& path() const;
            
            String asString() const;
        private:
            EntityDefinitionFileSpec(Type type, const IO::Path& path);
        };
    }
}

#endif /* defined(TrenchBroom_EntityDefinitionFileSpec) */
