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
        class EntityNode;
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

            void generate(WorldNode* worldNode,   IssueList& issues) const;
            void generate(LayerNode* layerNode,   IssueList& issues) const;
            void generate(GroupNode* groupNode,   IssueList& issues) const;
            void generate(EntityNode* entityNode, IssueList& issues) const;
            void generate(BrushNode* brushNode,   IssueList& issues) const;
        protected:
            IssueGenerator(IssueType type, const std::string& description);
            void addQuickFix(IssueQuickFix* quickFix);
        private:
            virtual void doGenerate(WorldNode* worldNode,           IssueList& issues) const;
            virtual void doGenerate(LayerNode* layerNode,           IssueList& issues) const;
            virtual void doGenerate(GroupNode* groupNode,           IssueList& issues) const;
            virtual void doGenerate(EntityNode* entityNode,         IssueList& issues) const;
            virtual void doGenerate(BrushNode* brushNode,           IssueList& issues) const;
            virtual void doGenerate(AttributableNode* node, IssueList& issues) const;
        };
    }
}

#endif /* defined(TrenchBroom_IssueGenerator) */
