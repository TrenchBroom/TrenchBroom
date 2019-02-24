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

#include "Assets/EntityDefinitionManager.h"
#include "Assets/Texture.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/MapFacade.h"
#include "Model/Node.h"
#include "Model/NodeCollection.h"
#include "Model/NodeVisitor.h"
#include "Model/PushSelection.h"
#include "Model/World.h"

namespace TrenchBroom {
    namespace Model {
        MatchVisitor::MatchVisitor() :
        m_matches(false) {}

        MatchVisitor::~MatchVisitor() = default;

        bool MatchVisitor::matches() const {
            return m_matches;
        }

        void MatchVisitor::setMatches() {
            m_matches = true;
        }

        void BrushFaceMatchVisitor::visit(const BrushFace& face) {
            if (m_matcher(face)) {
                setMatches();
            }
        }

        void BrushMatchVisitor::visit(const Brush& brush) {
            if (m_matcher(brush)) {
                setMatches();
            }
        }

        TextureNameTagMatcher::TextureNameTagMatcher(String pattern) :
        m_pattern(std::move(pattern)) {}

        std::unique_ptr<TagMatcher> TextureNameTagMatcher::clone() const {
            return std::make_unique<TextureNameTagMatcher>(m_pattern);
        }

        bool TextureNameTagMatcher::matches(const Taggable& taggable) const {
            BrushFaceMatchVisitor visitor([this](const BrushFace& face) {
                const auto& textureName = face.textureName();
                auto begin = std::begin(textureName);

                const auto pos = textureName.find_last_of('/');
                if (pos != String::npos) {
                    std::advance(begin, long(pos)+1);
                }

                return StringUtils::matchesPattern(
                    begin, std::end(textureName),
                    std::begin(m_pattern),
                    std::end(m_pattern),
                    StringUtils::CharEqual<StringUtils::CaseInsensitiveCharCompare>());
            });

            taggable.accept(visitor);
            return visitor.matches();
        }

        SurfaceParmTagMatcher::SurfaceParmTagMatcher(String parameter) :
        m_parameter(std::move(parameter)) {}

        std::unique_ptr<TagMatcher> SurfaceParmTagMatcher::clone() const {
            return std::make_unique<SurfaceParmTagMatcher>(m_parameter);
        }

        bool SurfaceParmTagMatcher::matches(const Taggable& taggable) const {
            BrushFaceMatchVisitor visitor([this](const BrushFace& face) {
                const auto* texture = face.texture();
                if (texture != nullptr) {
                    const auto& surfaceParms = texture->surfaceParms();
                    if (surfaceParms.count(m_parameter) > 0) {
                        return true;
                    }
                }
                return false;
            });

            taggable.accept(visitor);
            return visitor.matches();
        }

        ContentFlagsTagMatcher::ContentFlagsTagMatcher(const int flags) :
        m_flags(flags) {}

        std::unique_ptr<TagMatcher> ContentFlagsTagMatcher::clone() const {
            return std::make_unique<ContentFlagsTagMatcher>(m_flags);
        }

        bool ContentFlagsTagMatcher::matches(const Taggable& taggable) const {
            BrushFaceMatchVisitor visitor([this](const BrushFace& face) {
                return (face.surfaceContents() & m_flags) != 0;
            });

            taggable.accept(visitor);
            return visitor.matches();
        }

        SurfaceFlagsTagMatcher::SurfaceFlagsTagMatcher(const int flags) :
        m_flags(flags) {}

        std::unique_ptr<TagMatcher> SurfaceFlagsTagMatcher::clone() const {
            return std::make_unique<SurfaceFlagsTagMatcher>(m_flags);
        }

        bool SurfaceFlagsTagMatcher::matches(const Taggable& taggable) const {
            BrushFaceMatchVisitor visitor([this](const BrushFace& face) {
                return (face.surfaceFlags() & m_flags) != 0;
            });

            taggable.accept(visitor);
            return visitor.matches();
        }

        EntityClassNameTagMatcher::EntityClassNameTagMatcher(String pattern) :
        m_pattern(std::move(pattern)) {}


        std::unique_ptr<TagMatcher> EntityClassNameTagMatcher::clone() const {
            return std::make_unique<EntityClassNameTagMatcher>(m_pattern);
        }

        bool EntityClassNameTagMatcher::matches(const Taggable& taggable) const {
            BrushMatchVisitor visitor([this](const Brush& brush) {
                const auto* entity = brush.entity();
                if (entity == nullptr) {
                    return false;
                }

                return StringUtils::caseInsensitiveMatchesPattern(entity->classname(), m_pattern);
            });

            taggable.accept(visitor);
            return visitor.matches();
        }

        void EntityClassNameTagMatcher::enable(TagMatcherCallback& callback, MapFacade& facade) const {
            assert(canEnable());

            if (!facade.selectedNodes().hasOnlyBrushes()) {
                return;
            }

            const auto& definitionManager = facade.entityDefinitionManager();
            auto definitions = definitionManager.definitions(m_pattern);
            std::sort(std::begin(definitions), std::end(definitions), [](const auto* lhs, const auto* rhs) {
                return StringUtils::caseInsensitiveCompare(lhs->name(), rhs->name()) < 0;
            });

            const Assets::EntityDefinition* definition = nullptr;
            if (definitions.empty()) {
                return;
            } else if (definitions.size() == 1) {
                definition = definitions.front();
            } else {
                StringList options;
                std::transform(std::begin(definitions), std::end(definitions), std::back_inserter(options),
                    [](const auto* current) { return current->name(); });
                const auto index = callback.selectOption(options);
                if (index >= definitions.size()) {
                    return;
                }
                definition = definitions[index];
            }

            assert(definition != nullptr);

            auto* entity = new Entity();
            entity->addOrUpdateAttribute(AttributeNames::Classname, definition->name());

            const auto brushes = facade.selectedNodes().nodes();

            PushSelection pushSelection(facade);
            facade.deselectAll();
            facade.addNode(entity, facade.currentParent());
            facade.reparentNodes(entity, brushes);
        }

        void EntityClassNameTagMatcher::disable(TagMatcherCallback& callback, MapFacade& facade) const {
            assert(canDisable());

            // entities will be removed automatically when they become empty

            const auto& selectedBrushes = facade.selectedNodes().nodes();
            auto detailBrushes = Model::NodeList{};
            for (auto* brush : selectedBrushes) {
                if (matches(*brush)) {
                    detailBrushes.push_back(brush);
                }
            }

            facade.deselectAll();
            facade.reparentNodes(facade.currentParent(), detailBrushes);
            facade.select(Model::NodeList(std::begin(detailBrushes), std::end(detailBrushes)));
        }

        bool EntityClassNameTagMatcher::canEnable() const {
            return true;
        }

        bool EntityClassNameTagMatcher::canDisable() const {
            return true;
        }
    }
}
