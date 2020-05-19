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
#include "Model/TagVisitor.h"

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class BrushNode;
        class BrushFace;
        class ChangeBrushFaceAttributesRequest;
        class Game;
        class MapFacade;

        class MatchVisitor : public ConstTagVisitor {
        private:
            bool m_matches;
        public:
            MatchVisitor();

            bool matches() const;
        protected:
            void setMatches();
        };

        class BrushFaceMatchVisitor : public MatchVisitor {
        private:
            std::function<bool(const BrushFace&)> m_matcher;
        public:
            explicit BrushFaceMatchVisitor(std::function<bool(const BrushFace&)> matcher) :
            m_matcher(std::move(matcher)) {}

            void visit(const BrushFace& face) override;
        };

        class BrushMatchVisitor : public MatchVisitor {
        private:
            std::function<bool(const BrushNode&)> m_matcher;
        public:
            explicit BrushMatchVisitor(std::function<bool(const BrushNode&)> matcher) :
            m_matcher(std::move(matcher)) {}

            void visit(const BrushNode& brush) override;
        };

        class TextureNameTagMatcher : public TagMatcher {
        private:
            std::string m_pattern;
        public:
            explicit TextureNameTagMatcher(const std::string& pattern);
            std::unique_ptr<TagMatcher> clone() const override;
        public:
            bool matches(const Taggable& taggable) const override;
            void enable(TagMatcherCallback& callback, MapFacade& facade) const override;
            bool canEnable() const override;
        private:
            bool matchesTextureName(std::string_view textureName) const;
        };

        class SurfaceParmTagMatcher : public TagMatcher {
        private:
            std::string m_parameter;
        public:
            explicit SurfaceParmTagMatcher(const std::string& parameter);
            std::unique_ptr<TagMatcher> clone() const override;
        private:
            bool matches(const Taggable& taggable) const override;
        };

        class FlagsTagMatcher : public TagMatcher {
        protected:
            using GetFlags = std::function<int(const BrushFace&)>;
            using SetFlags = std::function<void(ChangeBrushFaceAttributesRequest&, int)>;
            using GetFlagNames = std::function<std::vector<std::string>(const Game& game, int)>;
        protected:
            int m_flags;
            GetFlags m_getFlags;
            SetFlags m_setFlags;
            SetFlags m_unsetFlags;
            GetFlagNames m_getFlagNames;
        protected:
            FlagsTagMatcher(int flags, GetFlags getFlags, SetFlags setFlags, SetFlags unsetFlags, GetFlagNames getFlagNames);
        private:
            bool matches(const Taggable& taggable) const override;
            void enable(TagMatcherCallback& callback, MapFacade& facade) const override;
            void disable(TagMatcherCallback& callback, MapFacade& facade) const override;
            bool canEnable() const override;
            bool canDisable() const override;
        };

        class ContentFlagsTagMatcher : public FlagsTagMatcher {
        public:
            explicit ContentFlagsTagMatcher(int flags);
            std::unique_ptr<TagMatcher> clone() const override;
        };

        class SurfaceFlagsTagMatcher : public FlagsTagMatcher {
        public:
            explicit SurfaceFlagsTagMatcher(int flags);
            std::unique_ptr<TagMatcher> clone() const override;
        };

        class EntityClassNameTagMatcher : public TagMatcher {
        private:
            std::string m_pattern;
            /**
             * The texture to set when this tag is enabled.
             */
            std::string m_texture;
        public:
            EntityClassNameTagMatcher(const std::string& pattern, const std::string& texture);
            std::unique_ptr<TagMatcher> clone() const override;
        private:
            bool matches(const Taggable& taggable) const override;
        public:
            void enable(TagMatcherCallback& callback, MapFacade& facade) const override;
            void disable(TagMatcherCallback& callback, MapFacade& facade) const override;
            bool canEnable() const override;
            bool canDisable() const override;
        private:
            bool matchesClassname(const std::string& classname) const;
        };
    }
}


#endif //TRENCHBROOM_TAGMATCHER_H
