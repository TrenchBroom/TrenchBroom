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

#include "EntityDefinitionFileSpec.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        EntityDefinitionFileSpec::EntityDefinitionFileSpec() :
        m_builtin(false),
        m_path(""),
        m_fullPath("") {}

        EntityDefinitionFileSpec EntityDefinitionFileSpec::builtin(const IO::Path& path, const IO::Path& fullPath) {
            return EntityDefinitionFileSpec(true, path, fullPath);
        }
        
        EntityDefinitionFileSpec EntityDefinitionFileSpec::external(const IO::Path& fullPath) {
            return EntityDefinitionFileSpec(false, fullPath, fullPath);
        }
        
        bool EntityDefinitionFileSpec::builtin() const {
            return m_builtin;
        }
        
        const IO::Path& EntityDefinitionFileSpec::path() const {
            return m_path;
        }
        
        const IO::Path& EntityDefinitionFileSpec::fullPath() const {
            return m_fullPath;
        }

        EntityDefinitionFileSpec::EntityDefinitionFileSpec(const bool builtin, const IO::Path& path, const IO::Path& fullPath) :
        m_builtin(builtin),
        m_path(path),
        m_fullPath(fullPath) {
            assert(builtin ^ path.isAbsolute());
            assert(builtin || path == fullPath);
            assert(fullPath.isAbsolute());
        }
    }
}
