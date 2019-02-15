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

#include "TagMatcher.h"

#include "Assets/Texture.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        TextureNameTagMatcher::TextureNameTagMatcher(String pattern) :
        m_pattern(std::move(pattern)) {}

        std::unique_ptr<TagMatcher> TextureNameTagMatcher::clone() const {
            return std::make_unique<TextureNameTagMatcher>(m_pattern);
        }

        bool TextureNameTagMatcher::matches(const BrushFace& face) const {
            const auto& textureName = face.textureName();
            auto begin = std::begin(textureName);

            const auto pos = textureName.find_last_of('/');
            if (pos != String::npos) {
                std::advance(begin, long(pos)+1);
            }

            return StringUtils::matchesPattern(begin, std::end(textureName), std::begin(m_pattern), std::end(m_pattern), StringUtils::CharEqual<StringUtils::CaseInsensitiveCharCompare>());
        }

        SurfaceParmTagMatcher::SurfaceParmTagMatcher(String parameter) :
        m_parameter(std::move(parameter)) {}

        std::unique_ptr<TagMatcher> SurfaceParmTagMatcher::clone() const {
            return std::make_unique<SurfaceParmTagMatcher>(m_parameter);
        }

        bool SurfaceParmTagMatcher::matches(const BrushFace& face) const {
            const auto* texture = face.texture();
            if (texture != nullptr) {
                const auto& surfaceParms = texture->surfaceParms();
                if (surfaceParms.count(m_parameter) > 0) {
                    return true;
                }
            }
            return false;
        }

        ContentFlagsTagMatcher::ContentFlagsTagMatcher(const int flags) :
        m_flags(flags) {}

        std::unique_ptr<TagMatcher> ContentFlagsTagMatcher::clone() const {
            return std::make_unique<ContentFlagsTagMatcher>(m_flags);
        }

        bool ContentFlagsTagMatcher::matches(const BrushFace& face) const {
            return (face.surfaceContents() & m_flags) != 0;
        }

        SurfaceFlagsTagMatcher::SurfaceFlagsTagMatcher(const int flags) :
        m_flags(flags) {}

        std::unique_ptr<TagMatcher> SurfaceFlagsTagMatcher::clone() const {
            return std::make_unique<SurfaceFlagsTagMatcher>(m_flags);
        }

        bool SurfaceFlagsTagMatcher::matches(const BrushFace& face) const {
            return (face.surfaceFlags() & m_flags) != 0;
        }

        EntityClassNameTagMatcher::EntityClassNameTagMatcher(String pattern) :
        m_pattern(std::move(pattern)) {}


        std::unique_ptr<TagMatcher> EntityClassNameTagMatcher::clone() const {
            return std::make_unique<EntityClassNameTagMatcher>(m_pattern);
        }

        bool EntityClassNameTagMatcher::matches(const Brush& brush) const {
            const auto* entity = brush.entity();
            if (entity == nullptr) {
                return false;
            }

            return StringUtils::caseInsensitiveMatchesPattern(entity->classname(), m_pattern);
        }
    }
}
