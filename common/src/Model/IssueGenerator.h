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

#ifndef TrenchBroom_IssueGenerator
#define TrenchBroom_IssueGenerator

#include "Model/IssueType.h"

#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class AttributableNode;
        class BrushNode;
        class Entity;
        class GroupNode;
        class Issue;
        class IssueQuickFix;
        class LayerNode;
        class WorldNode;

        class IssueGenerator {
        protected:
            using IssueList = std::vector<Issue*>;
            using IssueQuickFixList = std::vector<IssueQuickFix*>;
        private:
            IssueType m_type;
            std::string m_description;
            IssueQuickFixList m_quickFixes;
        public:
            virtual ~IssueGenerator();

            IssueType type() const;
            const std::string& description() const;
            const IssueQuickFixList& quickFixes() const;

            void generate(WorldNode* world,   IssueList& issues) const;
            void generate(LayerNode* layer,   IssueList& issues) const;
            void generate(GroupNode* group,   IssueList& issues) const;
            void generate(Entity* entity, IssueList& issues) const;
            void generate(BrushNode* brush,   IssueList& issues) const;
        protected:
            IssueGenerator(IssueType type, const std::string& description);
            void addQuickFix(IssueQuickFix* quickFix);
        private:
            virtual void doGenerate(WorldNode* world,           IssueList& issues) const;
            virtual void doGenerate(LayerNode* layer,           IssueList& issues) const;
            virtual void doGenerate(GroupNode* group,           IssueList& issues) const;
            virtual void doGenerate(Entity* entity,         IssueList& issues) const;
            virtual void doGenerate(BrushNode* brush,           IssueList& issues) const;
            virtual void doGenerate(AttributableNode* node, IssueList& issues) const;
        };
    }
}

#endif /* defined(TrenchBroom_IssueGenerator) */
