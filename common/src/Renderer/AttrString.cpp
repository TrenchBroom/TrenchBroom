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

#include "AttrString.h"

#include "Macros.h"

#include <algorithm>

namespace TrenchBroom {
    namespace Renderer {
        AttrString::LineFunc::~LineFunc() {}

        void AttrString::LineFunc::process(const std::string& str, const Justify justify) {
            switch (justify) {
                case Justify::Left:
                    justifyLeft(str);
                    break;
                case Justify::Right:
                    justifyRight(str);
                    break;
                case Justify::Center:
                    center(str);
                    break;
                    switchDefault()
            }
        }

        AttrString::Line::Line(const std::string& i_string, Justify i_justify) :
            string(i_string),
            justify(i_justify) {}

        AttrString::AttrString() {}

        AttrString::AttrString(const std::string& string) {
            appendLeftJustified(string);
        }

        bool AttrString::operator<(const AttrString& other) const {
            return compare(other) < 0;
        }

        int AttrString::compare(const AttrString& other) const {
            for (size_t i = 0; i < std::min(m_lines.size(), other.m_lines.size()); ++i) {
                const Line& myLine = m_lines[i];
                const Line& otherLine = other.m_lines[i];

                if (myLine.justify < otherLine.justify)
                    return -1;
                if (myLine.justify > otherLine.justify)
                    return 1;

                const int cmp = myLine.string.compare(otherLine.string);
                if (cmp < 0)
                    return -1;
                if (cmp > 0)
                    return 1;
            }

            if (m_lines.size() < other.m_lines.size())
                return -1;
            if (m_lines.size() > other.m_lines.size())
                return 1;
            return 0;
        }

        void AttrString::lines(LineFunc& func) const {
            for (size_t i = 0; i < m_lines.size(); ++i)
                func.process(m_lines[i].string, m_lines[i].justify);
        }


        void AttrString::appendLeftJustified(const std::string& string) {
            m_lines.push_back(Line(string, Justify::Left));
        }

        void AttrString::appendRightJustified(const std::string& string) {
            m_lines.push_back(Line(string, Justify::Right));
        }

        void AttrString::appendCentered(const std::string& string) {
            m_lines.push_back(Line(string, Justify::Center));
        }
    }
}
