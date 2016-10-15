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

#ifndef ConfigParserBase_h
#define ConfigParserBase_h

#include "StringUtils.h"
#include "EL.h"
#include "IO/ELParser.h"
#include "IO/Path.h"

#include <memory>

namespace TrenchBroom {
    namespace IO {
        class ConfigParserBase {
        private:
            ELParser m_parser;
        protected:
            Path m_path;
        protected:
            ConfigParserBase(const char* begin, const char* end, const Path& path);
            ConfigParserBase(const String& str, const Path& path = Path(""));
        public:
            virtual ~ConfigParserBase();
        protected:
            EL::Expression parseConfigFile();
            
            void expectType(const EL::Value& value, EL::ValueType type) const;
            void expectStructure(const EL::Value& value, const String& structure) const;
            void expectMapEntry(const EL::Value& value, const String& key, EL::ValueType type) const;
        };
    }
}

#endif /* ConfigParserBase_h */
