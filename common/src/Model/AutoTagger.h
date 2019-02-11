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

#ifndef TRENCHBROOM_AUTOTAGGER_H
#define TRENCHBROOM_AUTOTAGGER_H

#include "Model/Tag.h"

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

        class TagEvaluator {
        protected:
            const Tag& m_tag;
        protected:
            explicit TagEvaluator(const Tag& tag);
        public:
            virtual ~TagEvaluator();
        };

        class NodeTagEvaluator : public TagEvaluator {
        public:
            using TagEvaluator::TagEvaluator;
            ~NodeTagEvaluator() override;

            void update(Node& node) const;
        private:
            class Matcher;
            virtual bool matches(const Node& node) const;
            virtual bool matches(const World& world) const;
            virtual bool matches(const Layer& layer) const;
            virtual bool matches(const Group& group) const;
            virtual bool matches(const Entity& entity) const;
            virtual bool matches(const Brush& brush) const;
        };

        class BrushFaceTagEvaluator : public TagEvaluator {
        public:
            using TagEvaluator::TagEvaluator;
            ~BrushFaceTagEvaluator() override;

            void update(BrushFace& face) const;
        private:
            virtual bool matches(const BrushFace& face) const = 0;
        };

        class TextureNameEvaluator : public BrushFaceTagEvaluator {
        private:
            String m_pattern;
        public:
            TextureNameEvaluator(const Tag& tag, String pattern);
        private:
            bool matches(const BrushFace& face) const override;
        };

        class SurfaceParmEvaluator : public BrushFaceTagEvaluator {
        private:
            String m_parameter;
        public:
            SurfaceParmEvaluator(const Tag& tag, String parameter);
        private:
            bool matches(const BrushFace& face) const override;
        };

        class ContentFlagsEvaluator : public BrushFaceTagEvaluator {
        private:
            int m_flags;
        public:
            ContentFlagsEvaluator(const Tag& tag, int flags);
        private:
            bool matches(const BrushFace& face) const override;
        };

        class SurfaceFlagsEvaluator : public BrushFaceTagEvaluator {
        private:
            int m_flags;
        public:
            SurfaceFlagsEvaluator(const Tag& tag, int flags);
        private:
            bool matches(const BrushFace& face) const override;
        };

        class EntityClassNameEvaluator : public NodeTagEvaluator {
        private:
            String m_pattern;
        public:
            EntityClassNameEvaluator(const Tag& tag, String pattern);
        private:
            bool matches(const Brush& brush) const override;
        };

        class AutoTagger {
        private:
            std::vector<NodeTagEvaluator> m_nodeTagEvaluators;
            std::vector<BrushFaceTagEvaluator> m_faceTagEvaluators;
        public:
            AutoTagger(std::vector<NodeTagEvaluator> nodeTagEvaluators, std::vector<BrushFaceTagEvaluator> faceTagEvaluators);

            void updateTags(Node& node) const;
            void updateTags(BrushFace& face) const;
        };
    }
}


#endif //TRENCHBROOM_AUTOTAGGER_H
