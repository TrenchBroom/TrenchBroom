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

#include "MissingModIssueGenerator.h"

#include "CollectionUtils.h"
#include "Model/AttributableNode.h"
#include "Model/EntityAttributes.h"
#include "Model/Issue.h"
#include "Model/IssueQuickFix.h"
#include "Model/Game.h"
#include "Model/MapFacade.h"
#include "Model/PushSelection.h"

#include <algorithm>
#include <cassert>
#include <iterator>

namespace TrenchBroom {
    namespace Model {
        class MissingModIssueGenerator::MissingModIssue : public Issue {
        public:
            static const IssueType Type;
        private:
            String m_mod;
            String m_message;
        public:
            MissingModIssue(AttributableNode* node, const String& mod, const String& message) :
            Issue(node),
            m_mod(mod),
            m_message(message) {}
        private:
            IssueType doGetType() const override {
                return Type;
            }

            const String doGetDescription() const override {
                return "Mod '" + m_mod + "' could not be used: " + m_message;
            }
        public:
            const String& mod() const {
                return m_mod;
            }
        };

        const IssueType MissingModIssueGenerator::MissingModIssue::Type = Issue::freeType();

        class MissingModIssueGenerator::MissingModIssueQuickFix : public IssueQuickFix {
        public:
            MissingModIssueQuickFix() :
            IssueQuickFix(MissingModIssue::Type, "Remove mod") {}
        private:
            void doApply(MapFacade* facade, const IssueList& issues) const override {
                const PushSelection pushSelection(facade);

                 // If nothing is selected, attribute changes will affect only world.
                facade->deselectAll();

                const StringList oldMods = facade->mods();
                const StringList newMods = removeMissingMods(oldMods, issues);
                facade->setMods(newMods);
            }

            StringList removeMissingMods(StringList mods, const IssueList& issues) const {
                for (const Issue* issue : issues) {
                    if (issue->type() == MissingModIssue::Type) {
                        const MissingModIssue* modIssue = static_cast<const MissingModIssue*>(issue);
                        const String missingMod = modIssue->mod();
                        VectorUtils::erase(mods, missingMod);
                    }
                }
                return mods;
            }
        };

        MissingModIssueGenerator::MissingModIssueGenerator(GameWPtr game) :
        IssueGenerator(MissingModIssue::Type, "Missing mod directory"),
        m_game(game) {
            addQuickFix(new MissingModIssueQuickFix());
        }

        void MissingModIssueGenerator::doGenerate(AttributableNode* node, IssueList& issues) const {
            assert(node != nullptr);

            if (node->classname() != AttributeValues::WorldspawnClassname) {
                return;
            }

            if (expired(m_game)) {
                return;
            }

            GameSPtr game = lock(m_game);
            const StringList mods = game->extractEnabledMods(*node);

            if (mods == m_lastMods) {
                return;
            }

            const IO::Path::List additionalSearchPaths = IO::Path::asPaths(mods);
            const Game::PathErrors errors = game->checkAdditionalSearchPaths(additionalSearchPaths);
            using PathError = Game::PathErrors::value_type;

            std::transform(std::begin(errors), std::end(errors), std::back_inserter(issues), [node](const PathError& error) {
                const IO::Path& searchPath = error.first;
                const String& message = error.second;
                return new MissingModIssue(node, searchPath.asString(), message);
            });

            m_lastMods = mods;
        }
    }
}
