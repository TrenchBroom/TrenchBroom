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
 along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __TrenchBroom__MdlParser__
#define __TrenchBroom__MdlParser__

#include "StringUtils.h"
#include "IO/EntityModelParser.h"

#include <vector>

namespace TrenchBroom {
    namespace IO {
        class MdlParser : public EntityModelParser {
        private:
            typedef std::vector<char*> SkinPictureList;
            typedef std::vector<float> SkinTimeList;
            
            struct Skin {
                SkinPictureList pictures;
                SkinTimeList times;
                
                ~Skin();
            };
            
            typedef std::vector<Skin> SkinList;
            
            String m_name;
            const char* m_begin;
            const char* m_end;
        public:
            MdlParser(const String& name, const char* begin, const char* end);
        private:
            Assets::EntityModelCollection* doParseModel();
        };
    }
}

#endif /* defined(__TrenchBroom__MdlParser__) */
