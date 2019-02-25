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

#ifndef TRENCHBROOM_TAGMATCHER_H
#define TRENCHBROOM_TAGMATCHER_H

#include "Model/Tag.h"

#include <memory>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushFace;
        class Node;
        class World;
        class Layer;
        class Group;
        class Entity;
        class Brush;

        class TextureNameTagMatcher : public TagMatcher {
        private:
            String m_pattern;
        public:
            explicit TextureNameTagMatcher(String pattern);
            std::unique_ptr<TagMatcher> clone() const override;
        private:
            bool matches(const BrushFace& face) const override;
        };

        class SurfaceParmTagMatcher : public TagMatcher {
        private:
            String m_parameter;
        public:
            explicit SurfaceParmTagMatcher(String parameter);
            std::unique_ptr<TagMatcher> clone() const override;
        private:
            bool matches(const BrushFace& face) const override;
        };

        class ContentFlagsTagMatcher : public TagMatcher {
        private:
            int m_flags;
        public:
            explicit ContentFlagsTagMatcher(int flags);
            std::unique_ptr<TagMatcher> clone() const override;
        private:
            bool matches(const BrushFace& face) const override;
        };

        class SurfaceFlagsTagMatcher : public TagMatcher {
        private:
            int m_flags;
        public:
            explicit SurfaceFlagsTagMatcher(int flags);
            std::unique_ptr<TagMatcher> clone() const override;
        private:
            bool matches(const BrushFace& face) const override;
        };

        class EntityClassNameTagMatcher : public TagMatcher {
        private:
            String m_pattern;
        public:
            explicit EntityClassNameTagMatcher(String pattern);
            std::unique_ptr<TagMatcher> clone() const override;
        private:
            bool matches(const Brush& brush) const override;
        };
    }
}


#endif //TRENCHBROOM_TAGMATCHER_H
