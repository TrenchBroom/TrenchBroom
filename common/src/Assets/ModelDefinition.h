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

#ifndef TrenchBroom_ModelDefinition
#define TrenchBroom_ModelDefinition

#include "EL/Expression.h"
#include "IO/Path.h"
#include "Model/EntityAttributes.h"

#include <iostream>

namespace TrenchBroom {
    namespace Assets {
        struct ModelSpecification {
            IO::Path path;
            size_t skinIndex;
            size_t frameIndex;

            ModelSpecification();
            explicit ModelSpecification(const IO::Path& i_path, size_t i_skinIndex = 0, size_t i_frameIndex = 0);

            bool operator<(const ModelSpecification& rhs) const;
            bool operator>(const ModelSpecification& rhs) const;
            bool operator<=(const ModelSpecification& rhs) const;
            bool operator>=(const ModelSpecification& rhs) const;
            bool operator==(const ModelSpecification& rhs) const;
            bool operator!=(const ModelSpecification& rhs) const;
            int compare(const ModelSpecification& other) const;
        };

        std::ostream& operator<<(std::ostream& stream, const ModelSpecification& spec);

        class ModelDefinition {
        private:
            EL::Expression m_expression;
        public:
            ModelDefinition();
            ModelDefinition(size_t line, size_t column);
            explicit ModelDefinition(const EL::Expression& expression);

            void append(const ModelDefinition& other);

            ModelSpecification modelSpecification(const Model::EntityAttributes& attributes) const;
            ModelSpecification defaultModelSpecification() const;
        private:
            ModelSpecification convertToModel(const EL::Value& value) const;
            IO::Path path(const EL::Value& value) const;
            size_t index(const EL::Value& value) const;
        };
    }
}

#endif /* defined(TrenchBroom_ModelDefinition) */
