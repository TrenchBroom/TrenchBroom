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

#include "AutoTagger.h"

#include "Assets/Texture.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Node.h"
#include "Model/NodeVisitor.h"

namespace TrenchBroom {
    namespace Model {
        TagEvaluator::TagEvaluator(const Tag& tag) :
        m_tag(tag) {}

        TagEvaluator::~TagEvaluator() = default;

        NodeTagEvaluator::~NodeTagEvaluator() = default;

        void NodeTagEvaluator::update(Node& node) const {
            if (matches(node)) {
                node.addTag(m_tag);
            } else {
                node.removeTag(m_tag);
            }
        }

        class NodeTagEvaluator::Matcher : public ConstNodeVisitor {
        private:
            const NodeTagEvaluator& m_evaluator;
        public:
            Matcher(const NodeTagEvaluator& evaluator) :
            m_evaluator(evaluator) {}
        private:
            void doVisit(const World* world)   { m_evaluator.matches(*world); }
            void doVisit(const Layer* layer)   { m_evaluator.matches(*layer); }
            void doVisit(const Group* group)   { m_evaluator.matches(*group); }
            void doVisit(const Entity* entity) { m_evaluator.matches(*entity); }
            void doVisit(const Brush* brush)   { m_evaluator.matches(*brush); }
        };

        bool NodeTagEvaluator::matches(const Node& node) const {
            Matcher matcher(*this);
            node.accept(matcher);
        }

        bool NodeTagEvaluator::matches(const World& world) const {
            return false;
        }

        bool NodeTagEvaluator::matches(const Layer& layer) const {
            return false;
        }

        bool NodeTagEvaluator::matches(const Group& group) const {
            return false;
        }

        bool NodeTagEvaluator::matches(const Entity& entity) const {
            return false;
        }

        bool NodeTagEvaluator::matches(const Brush& brush) const {
            return false;
        }

        BrushFaceTagEvaluator::~BrushFaceTagEvaluator() = default;

        void BrushFaceTagEvaluator::update(BrushFace& face) const {
            if (matches(face)) {
                face.addTag(m_tag);
            } else {
                face.removeTag(m_tag);
            }
        }

        TextureNameEvaluator::TextureNameEvaluator(const Tag& tag, String pattern) :
        BrushFaceTagEvaluator(tag),
        m_pattern(std::move(pattern)) {}

        bool TextureNameEvaluator::matches(const BrushFace& face) const {
            const auto& textureName = face.textureName();
            auto begin = std::begin(textureName);

            const auto pos = textureName.find_last_of('/');
            if (pos != String::npos) {
                std::advance(begin, long(pos)+1);
            }

            return StringUtils::matchesPattern(begin, std::end(textureName), std::begin(m_pattern), std::end(m_pattern), StringUtils::CharEqual<StringUtils::CaseInsensitiveCharCompare>());
        }

        SurfaceParmEvaluator::SurfaceParmEvaluator(const Tag& tag, String parameter) :
            BrushFaceTagEvaluator(tag),
            m_parameter(std::move(parameter)) {}

        bool SurfaceParmEvaluator::matches(const BrushFace& face) const {
            const auto* texture = face.texture();
            if (texture != nullptr) {
                const auto& surfaceParms = texture->surfaceParms();
                if (surfaceParms.count(m_parameter) > 0) {
                    return true;
                }
            }
            return false;
        }

        ContentFlagsEvaluator::ContentFlagsEvaluator(const Tag& tag, const int flags) :
        BrushFaceTagEvaluator(tag),
        m_flags(flags) {}

        bool ContentFlagsEvaluator::matches(const BrushFace& face) const {
            return (face.surfaceContents() & m_flags) != 0;
        }

        SurfaceFlagsEvaluator::SurfaceFlagsEvaluator(const Tag& tag, const int flags) :
        BrushFaceTagEvaluator(tag),
        m_flags(flags) {}

        bool SurfaceFlagsEvaluator::matches(const BrushFace& face) const {
            return (face.surfaceFlags() & m_flags) != 0;
        }

        EntityClassNameEvaluator::EntityClassNameEvaluator(const Tag& tag, String pattern) :
        NodeTagEvaluator(tag),
        m_pattern(std::move(pattern)) {}


        bool EntityClassNameEvaluator::matches(const Brush& brush) const {
            const auto* entity = brush.entity();
            if (entity == nullptr) {
                return false;
            }

            return StringUtils::caseInsensitiveMatchesPattern(entity->classname(), m_pattern);
        }

        AutoTagger::AutoTagger(std::vector<NodeTagEvaluator> nodeTagEvaluators,
                               std::vector<BrushFaceTagEvaluator> faceTagEvaluators) :
        m_nodeTagEvaluators(std::move(nodeTagEvaluators)),
        m_faceTagEvaluators(std::move(faceTagEvaluators)) {}

        void AutoTagger::updateTags(Node& node) const {
            for (const auto& evaluator : m_nodeTagEvaluators) {
                evaluator.update(node);
            }
        }

        void AutoTagger::updateTags(BrushFace& face) const {
            for (const auto& evaluator : m_faceTagEvaluators) {
                evaluator.update(face);
            }
        }
    }
}
