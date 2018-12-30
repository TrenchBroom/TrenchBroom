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

#include "BrushContentTypeBuilder.h"

#include "Assets/Texture.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"

namespace TrenchBroom {
    namespace Model {
        BrushContentTypeBuilder::Result::Result(const BrushContentType::FlagType i_contentType, const bool i_transparent) :
        contentType(i_contentType),
        transparent(i_transparent) {}

        BrushContentTypeBuilder::BrushContentTypeBuilder(const BrushContentType::List& contentTypes) :
        m_contentTypes(contentTypes) {}
        
        BrushContentTypeBuilder::Result BrushContentTypeBuilder::buildContentType(const Brush* brush) const {
            BrushContentType::FlagType flags = 0;
            bool transparent = false;
            
            BrushContentType::List::const_iterator it, end;
            for (it = std::begin(m_contentTypes), end = std::end(m_contentTypes); it != end; ++it) {
                const BrushContentType& contentType = *it;
                if (contentType.evaluate(brush)) {
                    flags |= contentType.flagValue();
                    transparent |= contentType.transparent();
                }
            }

            if (!transparent) {
                for (const auto* face : brush->faces()) {
                    const auto* texture = face->texture();
                    transparent = texture != nullptr && texture->transparent();
                    if (transparent) {
                        break;
                    }
                }
            }

            return Result(flags, transparent);
        }
    }
}
