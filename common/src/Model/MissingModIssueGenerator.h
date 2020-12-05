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

#pragma once

#include "Model/IssueGenerator.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class Game;

        class MissingModIssueGenerator : public IssueGenerator {
        private:
            class MissingModIssue;
            class MissingModIssueQuickFix;

            std::weak_ptr<Game> m_game;
            mutable std::vector<std::string> m_lastMods;
        public:
            MissingModIssueGenerator(std::weak_ptr<Game> game);
        private:
            void doGenerate(AttributableNode* node, IssueList& issues) const override;
        };
    }
}

