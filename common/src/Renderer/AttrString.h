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

#pragma once

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Renderer {
        class AttrString {
        private:
            enum class Justify {
                Left,
                Right,
                Center
            };
        public:
            class LineFunc {
            public:
                virtual ~LineFunc();
                void process(const std::string& str, Justify justify);
            private:
                virtual void justifyLeft(const std::string& str) = 0;
                virtual void justifyRight(const std::string& str) = 0;
                virtual void center(const std::string& str) = 0;
            };
        private:
            struct Line {
                std::string string;
                Justify justify;
                Line(const std::string& i_string, Justify i_justify);
            };
            using Lines = std::vector<Line>;

            Lines m_lines;
        public:
            AttrString();

            AttrString(const std::string& string);

            bool operator<(const AttrString& other) const;
            int compare(const AttrString& other) const;

            void lines(LineFunc& func) const;

            void appendLeftJustified(const std::string& string);
            void appendRightJustified(const std::string& string);
            void appendCentered(const std::string& string);
        };
    }
}

