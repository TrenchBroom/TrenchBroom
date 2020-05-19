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

#include "WorldBoundsIssueGenerator.h"

#include "Model/Game.h"
#include "Model/BrushNode.h"
#include "Model/EntityNode.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/MapFacade.h"
#include "Model/WorldNode.h"

#include <kdl/memory_utils.h>

#include <string>
#include <optional>

namespace TrenchBroom {
    namespace Model {
        class WorldBoundsIssueGenerator::WorldBoundsIssue : public Issue {
        public:
            friend class WorldBoundsIssueQuickFix;
        public:
            static const IssueType Type;
        public:
            explicit WorldBoundsIssue(Node* node) :
            Issue(node) {}

            IssueType doGetType() const override {
                return Type;
            }

            std::string doGetDescription() const override {
                return "Object is out of world bounds";
            }
        };

        class WorldBoundsIssueGenerator::WorldBoundsIssueQuickFix : public IssueQuickFix {
        public:
            WorldBoundsIssueQuickFix() :
            IssueQuickFix(WorldBoundsIssue::Type, "Delete objects") {}
        private:
            void doApply(MapFacade* facade, const IssueList& /* issues */) const override {
                facade->deleteObjects();
            }
        };

        const IssueType WorldBoundsIssueGenerator::WorldBoundsIssue::Type = Issue::freeType();

        WorldBoundsIssueGenerator::WorldBoundsIssueGenerator(std::weak_ptr<Game> game, const WorldNode* world) :
        IssueGenerator(WorldBoundsIssue::Type, "Objects out of world bounds"),
        m_game(game),
        m_world(world) {
            addQuickFix(new WorldBoundsIssueQuickFix());
        }

        void WorldBoundsIssueGenerator::generateInternal(Node* node, IssueList& issues) const {
            auto game = kdl::mem_lock(m_game);
            const std::optional<vm::bbox3> bounds = game->extractSoftMapBounds(*m_world);

            if (!bounds.has_value()) {
                return;
            }
            if (!bounds->contains(node->logicalBounds())) {
                issues.push_back(new WorldBoundsIssue(node));
            }
        }

        void WorldBoundsIssueGenerator::doGenerate(EntityNode* entity, IssueList& issues) const {
            generateInternal(entity, issues);
        }

        void WorldBoundsIssueGenerator::doGenerate(BrushNode* brush, IssueList& issues) const {
            generateInternal(brush, issues);
        }
    }
}
