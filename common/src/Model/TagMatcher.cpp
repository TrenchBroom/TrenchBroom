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
#include "Model/Brush.h"
#include "Model/BrushFace.h"
#include "Model/ChangeBrushFaceAttributesRequest.h"
#include "Model/Game.h"
#include "Model/GameConfig.h"
#include "Model/Group.h"
#include "Model/MapFacade.h"
#include "Model/NodeCollection.h"
#include "Model/World.h"

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

        void BrushMatchVisitor::visit(const Brush& brush) {
            if (m_matcher(brush)) {
                setMatches();
            }
        }

        TextureNameTagMatcher::TextureNameTagMatcher(const std::string& pattern) :
        m_pattern(pattern) {}

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
            auto matchingTextures = std::vector<Assets::Texture*>{};

            std::copy_if(std::begin(allTextures), std::end(allTextures), std::back_inserter(matchingTextures), [this](auto* texture) {
                return matchesTextureName(texture->name());
            });

            std::sort(std::begin(matchingTextures), std::end(matchingTextures), [](const auto* lhs, const auto* rhs) {
                return kdl::ci::str_compare(lhs->name(), rhs->name()) < 0;
            });

            Assets::Texture* texture = nullptr;
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
            request.setTexture(texture);
            facade.setFaceAttributes(request);
        }

        bool TextureNameTagMatcher::canEnable() const {
            return true;
        }

        bool TextureNameTagMatcher::matchesTextureName(std::string_view textureName) const {
            const auto pos = textureName.find_last_of('/');
            if (pos != std::string::npos) {
                textureName = textureName.substr(pos + 1);
            }

            return kdl::ci::str_matches_glob(textureName, m_pattern);
        }

        SurfaceParmTagMatcher::SurfaceParmTagMatcher(const std::string& parameter) :
        m_parameter(parameter) {}

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
            [](const BrushFace& face) { return face.surfaceContents(); },
            [](ChangeBrushFaceAttributesRequest& request, const int flags) { request.setContentFlags(flags); },
            [](ChangeBrushFaceAttributesRequest& request, const int flags) { request.unsetContentFlags(flags); },
            [](const Game& game, const int flags) { return game.contentFlags().flagNames(flags); }
        ) {}

        std::unique_ptr<TagMatcher> ContentFlagsTagMatcher::clone() const {
            return std::make_unique<ContentFlagsTagMatcher>(m_flags);
        }

        SurfaceFlagsTagMatcher::SurfaceFlagsTagMatcher(const int i_flags) :
        FlagsTagMatcher(i_flags,
            [](const BrushFace& face) { return face.surfaceFlags(); },
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
                const auto& textureManager = facade.textureManager();
                auto* texture = textureManager.texture(m_texture);
                if (texture != nullptr) {
                    ChangeBrushFaceAttributesRequest request;
                    request.setTexture(texture);
                    facade.setFaceAttributes(request);
                }
            }

        }

        void EntityClassNameTagMatcher::disable(TagMatcherCallback& /* callback */, MapFacade& facade) const {
            // entities will be removed automatically when they become empty

            const auto& selectedBrushes = facade.selectedNodes().nodes();
            auto detailBrushes = std::vector<Node*> {};
            for (auto* brush : selectedBrushes) {
                if (matches(*brush)) {
                    detailBrushes.push_back(brush);
                }
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
