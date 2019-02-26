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
#include "Assets/TextureManager.h"
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Entity.h"
#include "Model/Group.h"
#include "Model/Layer.h"
#include "Model/MapFacade.h"
#include "Model/Node.h"
#include "Model/NodeCollection.h"
#include "Model/NodeVisitor.h"
#include "Model/PushSelection.h"
#include "Model/World.h"

#include <algorithm>
#include <iterator>

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
                return matchesTextureName(face.textureName());
            });

            taggable.accept(visitor);
            return visitor.matches();
        }

        void TextureNameTagMatcher::enable(TagMatcherCallback& callback, MapFacade& facade) const {
            const auto& textureManager = facade.textureManager();
            const auto& allTextures = textureManager.textures();
            auto matchingTextures = Assets::TextureList{};

            std::copy_if(std::begin(allTextures), std::end(allTextures), std::back_inserter(matchingTextures), [this](auto* texture) {
                return matchesTextureName(texture->name());
            });

            std::sort(std::begin(matchingTextures), std::end(matchingTextures), [](const auto* lhs, const auto* rhs) {
                return StringUtils::caseInsensitiveCompare(lhs->name(), rhs->name()) < 0;
            });

            Assets::Texture* texture = nullptr;
            if (matchingTextures.empty()) {
                return;
            } else if (matchingTextures.size() == 1) {
                texture = matchingTextures.front();
            } else {
                StringList options;
                std::transform(std::begin(matchingTextures), std::end(matchingTextures), std::back_inserter(options),
                               [](const auto* current) { return current->name(); });
                const auto index = callback.selectOption(options);
                if (index >= matchingTextures.size()) {
                    return;
                }
                texture = matchingTextures[index];
            }

            assert(texture != nullptr);

            ChangeBrushFaceAttributesRequest request;
            request.setTexture(texture);
            facade.setFaceAttributes(request);
        }

        bool TextureNameTagMatcher::canEnable() const {
            return true;
        }

        bool TextureNameTagMatcher::matchesTextureName(const String& textureName) const {
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

        void ContentFlagsTagMatcher::enable(TagMatcherCallback& callback, MapFacade& facade) const {
            ChangeBrushFaceAttributesRequest request;
            request.setContentFlags(m_flags);
            facade.setFaceAttributes(request);
        }

        void ContentFlagsTagMatcher::disable(TagMatcherCallback& callback, MapFacade& facade) const {
            ChangeBrushFaceAttributesRequest request;
            request.unsetContentFlags(m_flags);
            facade.setFaceAttributes(request);
        }

        bool ContentFlagsTagMatcher::canEnable() const {
            return true;
        }

        bool ContentFlagsTagMatcher::canDisable() const {
            return true;
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

        void SurfaceFlagsTagMatcher::enable(TagMatcherCallback& callback, MapFacade& facade) const {
            ChangeBrushFaceAttributesRequest request;
            request.setSurfaceFlags(m_flags);
            facade.setFaceAttributes(request);
        }

        void SurfaceFlagsTagMatcher::disable(TagMatcherCallback& callback, MapFacade& facade) const {
            ChangeBrushFaceAttributesRequest request;
            request.unsetSurfaceFlags(m_flags);
            facade.setFaceAttributes(request);
        }

        bool SurfaceFlagsTagMatcher::canEnable() const {
            return true;
        }

        bool SurfaceFlagsTagMatcher::canDisable() const {
            return true;
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

                return matchesClassname(entity->classname());
            });

            taggable.accept(visitor);
            return visitor.matches();
        }

        void EntityClassNameTagMatcher::enable(TagMatcherCallback& callback, MapFacade& facade) const {
            if (!facade.selectedNodes().hasOnlyBrushes()) {
                return;
            }

            const auto& definitionManager = facade.entityDefinitionManager();
            const auto& allDefinitions = definitionManager.definitions();
            auto matchingDefinitions = Assets::EntityDefinitionList{};

            std::copy_if(std::begin(allDefinitions), std::end(allDefinitions), std::back_inserter(matchingDefinitions), [this](const auto* definition) {
                return matchesClassname(definition->name());
            });

            std::sort(std::begin(matchingDefinitions), std::end(matchingDefinitions), [](const auto* lhs, const auto* rhs) {
                return StringUtils::caseInsensitiveCompare(lhs->name(), rhs->name()) < 0;
            });

            const Assets::EntityDefinition* definition = nullptr;
            if (matchingDefinitions.empty()) {
                return;
            } else if (matchingDefinitions.size() == 1) {
                definition = matchingDefinitions.front();
            } else {
                StringList options;
                std::transform(std::begin(matchingDefinitions), std::end(matchingDefinitions), std::back_inserter(options),
                    [](const auto* current) { return current->name(); });
                const auto index = callback.selectOption(options);
                if (index >= matchingDefinitions.size()) {
                    return;
                }
                definition = matchingDefinitions[index];
            }

            assert(definition != nullptr);
            facade.createBrushEntity(static_cast<const Assets::BrushEntityDefinition*>(definition));
        }

        void EntityClassNameTagMatcher::disable(TagMatcherCallback& callback, MapFacade& facade) const {
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

        bool EntityClassNameTagMatcher::matchesClassname(const String& classname) const {
            return StringUtils::caseInsensitiveMatchesPattern(classname, m_pattern);
        }
    }
}
