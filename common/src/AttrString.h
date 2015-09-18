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

#ifndef TrenchBroom_AttrString
#define TrenchBroom_AttrString

#include "StringUtils.h"

#include <vector>

namespace TrenchBroom {
    class AttrString {
    private:
        typedef enum {
            Justify_Left,
            Justify_Right,
            Justify_Center
        } Justify;
    public:
        class LineFunc {
        public:
            virtual ~LineFunc();
            void process(const String& str, Justify justify);
        private:
            virtual void justifyLeft(const String& str) = 0;
            virtual void justifyRight(const String& str) = 0;
            virtual void center(const String& str) = 0;
        };
    private:
        struct Line {
            String string;
            Justify justify;
            Line(const String& i_string, Justify i_justify);
        };
        typedef std::vector<Line> Lines;
        
        Lines m_lines;
    public:
        AttrString();
        AttrString(const String& string);
        
        bool operator<(const AttrString& other) const;
        int compare(const AttrString& other) const;
        
        void lines(LineFunc& func) const;
        
        void appendLeftJustified(const String& string);
        void appendRightJustified(const String& string);
        void appendCentered(const String& string);
    };
}

#endif /* defined(TrenchBroom_AttrString) */
