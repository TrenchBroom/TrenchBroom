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

#include "IssueGenerator.h"

#include "Ensure.h"
#include "Model/IssueQuickFix.h"
#include "Model/EntityNode.h"
#include "Model/WorldNode.h"

#include <kdl/vector_utils.h>

#include <cassert>
#include <string>

namespace TrenchBroom {
    namespace Model {
        IssueGenerator::~IssueGenerator() {
            kdl::vec_clear_and_delete(m_quickFixes);
        }

        IssueType IssueGenerator::type() const {
            return m_type;
        }

        const std::string& IssueGenerator::description() const {
            return m_description;
        }

        const std::vector<IssueQuickFix*>& IssueGenerator::quickFixes() const {
            return m_quickFixes;
        }

        void IssueGenerator::generate(WorldNode* worldNode, IssueList& issues) const {
            doGenerate(worldNode, issues);
        }

        void IssueGenerator::generate(LayerNode* layerNode, IssueList& issues) const {
            doGenerate(layerNode, issues);
        }

        void IssueGenerator::generate(GroupNode* groupNode, IssueList& issues) const {
            doGenerate(groupNode, issues);
        }

        void IssueGenerator::generate(EntityNode* entityNode, IssueList& issues) const {
            doGenerate(entityNode, issues);
        }

        void IssueGenerator::generate(BrushNode* brushNode, IssueList& issues) const {
            doGenerate(brushNode, issues);
        }

        IssueGenerator::IssueGenerator(const IssueType type, const std::string& description) :
        m_type(type),
        m_description(description) {}

        void IssueGenerator::addQuickFix(IssueQuickFix* quickFix) {
            ensure(quickFix != nullptr, "quickFix is null");
            assert(!kdl::vec_contains(m_quickFixes, quickFix));
            m_quickFixes.push_back(quickFix);
        }
 
        void IssueGenerator::doGenerate(WorldNode* worldNode,   IssueList& issues) const { doGenerate(static_cast<AttributableNode*>(worldNode), issues); }
        void IssueGenerator::doGenerate(LayerNode*,             IssueList&) const        {}
        void IssueGenerator::doGenerate(GroupNode*,             IssueList&) const        {}
        void IssueGenerator::doGenerate(EntityNode* entityNode, IssueList& issues) const { doGenerate(static_cast<AttributableNode*>(entityNode), issues); }
        void IssueGenerator::doGenerate(BrushNode*,             IssueList&) const        {}
        void IssueGenerator::doGenerate(AttributableNode*,      IssueList&) const        {}
    }
}
