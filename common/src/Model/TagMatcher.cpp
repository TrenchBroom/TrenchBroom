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

#include "Assets/EntityDefinition.h"
#include "Assets/EntityDefinitionManager.h"
#include "Assets/Texture.h"
#include "Assets/TextureManager.h"
#include "Model/BrushNode.h"
#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Entity.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/GroupNode.h"
#include "Model/MapFacade.h"
#include "Model/NodeCollection.h"
#include "Model/WorldNode.h"

#include <kdl/string_compare.h>
#include <kdl/vector_utils.h>

#include <vector>

namespace TrenchBroom {
    namespace Model {
        MatchVisitor::MatchVisitor() :
        m_matches(false) {}

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

        void BrushMatchVisitor::visit(const BrushNode& brush) {
            if (m_matcher(brush)) {
                setMatches();
            }
        }

        void TextureTagMatcher::enable(TagMatcherCallback& callback, MapFacade& facade) const {
            const auto& textureManager = facade.textureManager();
            const auto& allTextures = textureManager.textures();
            auto matchingTextures = std::vector<const Assets::Texture*>{};

            std::copy_if(std::begin(allTextures), std::end(allTextures), std::back_inserter(matchingTextures), [this](auto* texture) {
                return matchesTexture(texture);
            });

            std::sort(std::begin(matchingTextures), std::end(matchingTextures), [](const auto* lhs, const auto* rhs) {
                return kdl::ci::str_compare(lhs->name(), rhs->name()) < 0;
            });

            const Assets::Texture* texture = nullptr;
            if (matchingTextures.empty()) {
                return;
            } else if (matchingTextures.size() == 1) {
                texture = matchingTextures.front();
            } else {
                const auto options = kdl::vec_transform(matchingTextures,
                    [](const auto* current) { return current->name(); });
                const auto index = callback.selectOption(options);
                if (index >= matchingTextures.size()) {
                    return;
                }
                texture = matchingTextures[index];
            }

            assert(texture != nullptr);

            ChangeBrushFaceAttributesRequest request;
            request.setTextureName(texture->name());
            facade.setFaceAttributes(request);
        }

        bool TextureTagMatcher::canEnable() const {
            return true;
        }

        TextureNameTagMatcher::TextureNameTagMatcher(const std::string& pattern) :
        m_pattern(pattern) {}

        std::unique_ptr<TagMatcher> TextureNameTagMatcher::clone() const {
            return std::make_unique<TextureNameTagMatcher>(m_pattern);
        }

        bool TextureNameTagMatcher::matches(const Taggable& taggable) const {
            BrushFaceMatchVisitor visitor([this](const BrushFace& face) {
                return matchesTextureName(face.attributes().textureName());
            });

            taggable.accept(visitor);
            return visitor.matches();
        }

        bool TextureNameTagMatcher::matchesTexture(const Assets::Texture* texture) const {
            if (texture == nullptr) {
                return false;
            }
            return matchesTextureName(texture->name());
        }

        bool TextureNameTagMatcher::matchesTextureName(std::string_view textureName) const {
            // If the match pattern doesn't contain a slash, match against
            // only the last component of the texture name.
            if (m_pattern.find('/') == std::string::npos) {
                const auto pos = textureName.find_last_of('/');
                if (pos != std::string::npos) {
                    textureName = textureName.substr(pos + 1);
                }
            }

            return kdl::ci::str_matches_glob(textureName, m_pattern);
        }

        SurfaceParmTagMatcher::SurfaceParmTagMatcher(const std::string& parameter) :
        m_parameters({parameter}) {}

        SurfaceParmTagMatcher::SurfaceParmTagMatcher(const kdl::vector_set<std::string>& parameters) :
        m_parameters(parameters) {}

        std::unique_ptr<TagMatcher> SurfaceParmTagMatcher::clone() const {
            return std::make_unique<SurfaceParmTagMatcher>(m_parameters);
        }

        bool SurfaceParmTagMatcher::matches(const Taggable& taggable) const {
            BrushFaceMatchVisitor visitor([this](const BrushFace& face) {
                return matchesTexture(face.texture());
            });

            taggable.accept(visitor);
            return visitor.matches();
        }

        bool SurfaceParmTagMatcher::matchesTexture(const Assets::Texture* texture) const {
            if (texture == nullptr) {
                return false;
            }
            const std::set<std::string>& parameters = texture->surfaceParms();
            auto texParams = std::begin(parameters);
            auto tagParams = std::begin(m_parameters);
            auto texParamsEnd = std::end(parameters);
            auto tagParamsEnd = std::end(m_parameters);
            while (texParams != texParamsEnd && tagParams != tagParamsEnd) {
                if (*texParams < *tagParams) {
                    ++texParams;
                } else if (*tagParams < *texParams) {
                    ++tagParams;
                } else {
                    return true;
                }
            }
            return false;
        }

        FlagsTagMatcher::FlagsTagMatcher(const int flags, GetFlags getFlags, SetFlags setFlags, SetFlags unsetFlags, GetFlagNames getFlagNames) :
        m_flags(flags),
        m_getFlags(std::move(getFlags)),
        m_setFlags(std::move(setFlags)),
        m_unsetFlags(std::move(unsetFlags)),
        m_getFlagNames(std::move(getFlagNames)) {}

        bool FlagsTagMatcher::matches(const Taggable& taggable) const {
            BrushFaceMatchVisitor visitor([this](const BrushFace& face) {
                return (m_getFlags(face) & m_flags) != 0;
            });

            taggable.accept(visitor);
            return visitor.matches();
        }

        void FlagsTagMatcher::enable(TagMatcherCallback& callback, MapFacade& facade) const {
            constexpr auto bits = sizeof(decltype(m_flags)) * 8;

            std::vector<size_t> flagIndices;
            for (size_t i = 0; i < bits; ++i) {
                if ((m_flags & (1 << i)) != 0) {
                    flagIndices.push_back(i);
                }
            }

            int flagToSet = 0;
            if (flagIndices.empty()) {
                return;
            } else if (flagIndices.size() == 1) {
                flagToSet = m_flags;
            } else {
                const auto options = m_getFlagNames(*facade.game(), m_flags);
                const auto selectedOptionIndex = callback.selectOption(options);
                if (selectedOptionIndex == options.size()) {
                    return;
                }

                // convert the option index into the index of the flag to set
                size_t currentIndex = 0;
                for (size_t i = 0; i < bits; ++i) {
                    if ((m_flags & (1 << i)) != 0) {
                        // only consider flags which are set to 1
                        if (currentIndex == selectedOptionIndex) {
                            // we found the flag that corresponds to the selected option
                            flagToSet = (1 << i);
                            break;
                        }
                        ++currentIndex;
                    }
                }
            }

            ChangeBrushFaceAttributesRequest request;
            m_setFlags(request, flagToSet);
            facade.setFaceAttributes(request);
        }

        void FlagsTagMatcher::disable(TagMatcherCallback& /* callback */, MapFacade& facade) const {
            ChangeBrushFaceAttributesRequest request;
            m_unsetFlags(request, m_flags);
            facade.setFaceAttributes(request);
        }

        bool FlagsTagMatcher::canEnable() const {
            return true;
        }

        bool FlagsTagMatcher::canDisable() const {
            return true;
        }

        ContentFlagsTagMatcher::ContentFlagsTagMatcher(const int i_flags) :
        FlagsTagMatcher(i_flags,
            [](const BrushFace& face) { return face.attributes().surfaceContents(); },
            [](ChangeBrushFaceAttributesRequest& request, const int flags) { request.setContentFlags(flags); },
            [](ChangeBrushFaceAttributesRequest& request, const int flags) { request.unsetContentFlags(flags); },
            [](const Game& game, const int flags) { return game.contentFlags().flagNames(flags); }
        ) {}

        std::unique_ptr<TagMatcher> ContentFlagsTagMatcher::clone() const {
            return std::make_unique<ContentFlagsTagMatcher>(m_flags);
        }

        SurfaceFlagsTagMatcher::SurfaceFlagsTagMatcher(const int i_flags) :
        FlagsTagMatcher(i_flags,
            [](const BrushFace& face) { return face.attributes().surfaceFlags(); },
            [](ChangeBrushFaceAttributesRequest& request, const int flags) { request.setSurfaceFlags(flags); },
            [](ChangeBrushFaceAttributesRequest& request, const int flags) { request.unsetSurfaceFlags(flags); },
            [](const Game& game, const int flags) { return game.surfaceFlags().flagNames(flags); }
        ) {}

        std::unique_ptr<TagMatcher> SurfaceFlagsTagMatcher::clone() const {
            return std::make_unique<SurfaceFlagsTagMatcher>(m_flags);
        }

        EntityClassNameTagMatcher::EntityClassNameTagMatcher(const std::string& pattern, const std::string& texture) :
        m_pattern(pattern),
        m_texture(texture) {}


        std::unique_ptr<TagMatcher> EntityClassNameTagMatcher::clone() const {
            return std::make_unique<EntityClassNameTagMatcher>(m_pattern, m_texture);
        }

        bool EntityClassNameTagMatcher::matches(const Taggable& taggable) const {
            BrushMatchVisitor visitor([this](const BrushNode& brush) {
                if (const auto* entityNode = brush.entity()) {
                    return matchesClassname(entityNode->entity().classname());
                } else {
                    return false;
                }
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
            auto matchingDefinitions = std::vector<Assets::EntityDefinition*>{};

            std::copy_if(std::begin(allDefinitions), std::end(allDefinitions), std::back_inserter(matchingDefinitions), [this](const auto* definition) {
                return definition->type() == Assets::EntityDefinitionType::BrushEntity && matchesClassname(definition->name());
            });

            std::sort(std::begin(matchingDefinitions), std::end(matchingDefinitions), [](const auto* lhs, const auto* rhs) {
                return kdl::ci::str_compare(lhs->name(), rhs->name()) < 0;
            });

            const Assets::EntityDefinition* definition = nullptr;
            if (matchingDefinitions.empty()) {
                return;
            } else if (matchingDefinitions.size() == 1) {
                definition = matchingDefinitions.front();
            } else {
                const auto options = kdl::vec_transform(matchingDefinitions,
                    [](const auto* current) { return current->name(); });
                const auto index = callback.selectOption(options);
                if (index >= matchingDefinitions.size()) {
                    return;
                }
                definition = matchingDefinitions[index];
            }

            assert(definition != nullptr);
            facade.createBrushEntity(static_cast<const Assets::BrushEntityDefinition*>(definition));

            if (!m_texture.empty()) {
                ChangeBrushFaceAttributesRequest request;
                request.setTextureName(m_texture);
                facade.setFaceAttributes(request);
            }

        }

        void EntityClassNameTagMatcher::disable(TagMatcherCallback& /* callback */, MapFacade& facade) const {
            // entities will be removed automatically when they become empty

            const auto selectedBrushes = facade.selectedNodes().nodes();
            auto detailBrushes = std::vector<Node*> {};
            for (auto* brush : selectedBrushes) {
                if (matches(*brush)) {
                    detailBrushes.push_back(brush);
                }
            }

            if (detailBrushes.empty()) {
                return;
            }
            facade.deselectAll();
            facade.reparentNodes(facade.parentForNodes(selectedBrushes), detailBrushes);
            facade.select(std::vector<Node*> (std::begin(detailBrushes), std::end(detailBrushes)));
        }

        bool EntityClassNameTagMatcher::canEnable() const {
            return true;
        }

        bool EntityClassNameTagMatcher::canDisable() const {
            return true;
        }

        bool EntityClassNameTagMatcher::matchesClassname(const std::string& classname) const {
            return kdl::ci::str_matches_glob(classname, m_pattern);
        }
    }
}
