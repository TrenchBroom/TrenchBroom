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

#include "BrushContentTypeBuilder.h"

namespace TrenchBroom {
    namespace Model {
        BrushContentTypeBuilder::BrushContentTypeBuilder(const BrushContentType::List contentTypes) :
        m_contentTypes(contentTypes) {}
        
        BrushContentType::FlagType BrushContentTypeBuilder::buildContentType(const Brush* brush) const {
            BrushContentType::FlagType flags = 0;
            
            BrushContentType::List::const_iterator it, end;
            for (it = m_contentTypes.begin(), end = m_contentTypes.end(); it != end; ++it) {
                const BrushContentType& contentType = *it;
                if (contentType.evaluate(brush))
                    flags |= contentType.flagValue();
            }
            return flags;
        }
    }
}
