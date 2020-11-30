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

#include "EntityDefinitionFileSpec.h"


#include <kdl/string_compare.h>

#include <cassert>
#include <string>

namespace TrenchBroom {
    namespace Assets {
        EntityDefinitionFileSpec::EntityDefinitionFileSpec() :
        m_type(Type::Unset),
        m_path("") {}

        EntityDefinitionFileSpec EntityDefinitionFileSpec::parse(const std::string& str) {
            if (kdl::cs::str_is_prefix(str, "external:")) {
                const IO::Path path(str.substr(9));
                return EntityDefinitionFileSpec::external(path);
            }

            if (kdl::cs::str_is_prefix(str, "builtin:")) {
                const IO::Path path(str.substr(8));
                return EntityDefinitionFileSpec::builtin(path);
            }

            // If the location spec is missing, we assume that an absolute path indicates an
            // external file spec, and a relative path indicates a builtin file spec.
            const IO::Path path(str);
            if (path.isAbsolute())
                return EntityDefinitionFileSpec::external(path);
            return EntityDefinitionFileSpec::builtin(path);
        }

        EntityDefinitionFileSpec EntityDefinitionFileSpec::builtin(const IO::Path& path) {
            return EntityDefinitionFileSpec(Type::Builtin, path);
        }

        EntityDefinitionFileSpec EntityDefinitionFileSpec::external(const IO::Path& path) {
            return EntityDefinitionFileSpec(Type::External, path);
        }

        EntityDefinitionFileSpec EntityDefinitionFileSpec::unset() {
            return EntityDefinitionFileSpec();
        }

        bool operator<(const EntityDefinitionFileSpec& lhs, const EntityDefinitionFileSpec& rhs) {
            if (lhs.m_type < rhs.m_type) {
                return true;
            } 
            
            if (lhs.m_type > rhs.m_type) {
                return false;
            }

            return lhs.m_path < rhs.m_path;
        }

        bool operator==(const EntityDefinitionFileSpec& lhs, const EntityDefinitionFileSpec& rhs) {
            return lhs.m_type == rhs.m_type && lhs.m_path == rhs.m_path;
        }

        bool operator!=(const EntityDefinitionFileSpec& lhs, const EntityDefinitionFileSpec& rhs) {
            return !(lhs == rhs);
        }

        bool EntityDefinitionFileSpec::valid() const {
            return m_type != Type::Unset;
        }

        bool EntityDefinitionFileSpec::builtin() const {
            return m_type == Type::Builtin;
        }

        bool EntityDefinitionFileSpec::external() const {
            return m_type == Type::External;
        }

        const IO::Path& EntityDefinitionFileSpec::path() const {
            return m_path;
        }

        std::string EntityDefinitionFileSpec::asString() const {
            if (!valid())
                return "";
            if (builtin())
                return "builtin:" + m_path.asString();
            return "external:" + m_path.asString();
        }

        EntityDefinitionFileSpec::EntityDefinitionFileSpec(const Type type, const IO::Path& path) :
        m_type(type),
        m_path(path) {
            assert(valid());
            assert(!path.isEmpty());
        }
    }
}
