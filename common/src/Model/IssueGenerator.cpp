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
#include "Model/Entity.h"
#include "Model/World.h"

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

        void IssueGenerator::generate(World* world, IssueList& issues) const {
            doGenerate(world, issues);
        }

        void IssueGenerator::generate(LayerNode* layer, IssueList& issues) const {
            doGenerate(layer, issues);
        }

        void IssueGenerator::generate(GroupNode* group, IssueList& issues) const {
            doGenerate(group, issues);
        }

        void IssueGenerator::generate(Entity* entity, IssueList& issues) const {
            doGenerate(entity, issues);
        }

        void IssueGenerator::generate(BrushNode* brush, IssueList& issues) const {
            doGenerate(brush, issues);
        }

        IssueGenerator::IssueGenerator(const IssueType type, const std::string& description) :
        m_type(type),
        m_description(description) {}

        void IssueGenerator::addQuickFix(IssueQuickFix* quickFix) {
            ensure(quickFix != nullptr, "quickFix is null");
            assert(!kdl::vec_contains(m_quickFixes, quickFix));
            m_quickFixes.push_back(quickFix);
        }

        void IssueGenerator::doGenerate(World* world,      IssueList& issues) const { doGenerate(static_cast<AttributableNode*>(world), issues); }
        void IssueGenerator::doGenerate(LayerNode*,            IssueList&) const        {}
        void IssueGenerator::doGenerate(GroupNode*,            IssueList&) const        {}
        void IssueGenerator::doGenerate(Entity* entity,    IssueList& issues) const { doGenerate(static_cast<AttributableNode*>(entity), issues); }
        void IssueGenerator::doGenerate(BrushNode*,            IssueList&) const        {}
        void IssueGenerator::doGenerate(AttributableNode*, IssueList&) const        {}
    }
}
