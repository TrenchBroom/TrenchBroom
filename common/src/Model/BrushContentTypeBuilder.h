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

#ifndef TrenchBroom_BrushContentTypeBuilder
#define TrenchBroom_BrushContentTypeBuilder

#include "SharedPointer.h"
#include "Model/BrushContentType.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BrushContentTypeBuilder {
        public:
            struct Result {
                BrushContentType::FlagType contentType;
                bool transparent;
                Result(BrushContentType::FlagType i_contentType, bool i_transparent);
            };
        private:
            BrushContentType::List m_contentTypes;
        public:
            BrushContentTypeBuilder(const BrushContentType::List& contentTypes = BrushContentType::EmptyList);
            Result buildContentType(const Brush* brush) const;
        };
    }
}

#endif /* defined(TrenchBroom_BrushContentTypeBuilder) */
