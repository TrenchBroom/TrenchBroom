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

#include "EL/VariableStore.h"
#include "View/TextOutputAdapter.h"

#include <memory>
#include <string>

namespace TrenchBroom {
    namespace View {
        class MapDocument;

        class CompilationContext {
        private:
            std::weak_ptr<MapDocument> m_document;
            std::unique_ptr<EL::VariableStore> m_variables;

            TextOutputAdapter m_output;
            bool m_test;
        public:
            CompilationContext(std::weak_ptr<MapDocument> document, const EL::VariableStore& variables, const TextOutputAdapter& output, bool test);

            std::shared_ptr<MapDocument> document() const;
            bool test() const;

            std::string interpolate(const std::string& input) const;
            std::string variableValue(const std::string& variableName) const;

            template <typename T>
            CompilationContext& operator<<(const T& t) {
                m_output << t;
                return *this;
            }
        };
    }
}


