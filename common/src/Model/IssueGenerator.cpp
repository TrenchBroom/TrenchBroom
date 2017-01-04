/*
 Copyright (C) 2010-2016 Kristian Duske
 
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

#include "CollectionUtils.h"
#include "Model/IssueQuickFix.h"
#include "Model/Entity.h"
#include "Model/World.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        IssueGenerator::~IssueGenerator() {
            VectorUtils::clearAndDelete(m_quickFixes);
        }
        
        IssueType IssueGenerator::type() const {
            return m_type;
        }
        
        const String& IssueGenerator::description() const {
            return m_description;
        }

        const IssueQuickFixArray& IssueGenerator::quickFixes() const {
            return m_quickFixes;
        }

        void IssueGenerator::generate(World* world, IssueArray& issues) const {
            doGenerate(world, issues);
        }
        
        void IssueGenerator::generate(Layer* layer, IssueArray& issues) const {
            doGenerate(layer, issues);
        }
        
        void IssueGenerator::generate(Group* group, IssueArray& issues) const {
            doGenerate(group, issues);
        }
        
        void IssueGenerator::generate(Entity* entity, IssueArray& issues) const {
            doGenerate(entity, issues);
        }
        
        void IssueGenerator::generate(Brush* brush, IssueArray& issues) const {
            doGenerate(brush, issues);
        }

        IssueGenerator::IssueGenerator(const IssueType type, const String& description) :
        m_type(type),
        m_description(description) {}
        
        void IssueGenerator::addQuickFix(IssueQuickFix* quickFix) {
            ensure(quickFix != NULL, "quickFix is null");
            assert(!VectorUtils::contains(m_quickFixes, quickFix));
            m_quickFixes.push_back(quickFix);
        }

        void IssueGenerator::doGenerate(World* world,           IssueArray& issues) const { doGenerate(static_cast<AttributableNode*>(world), issues); }
        void IssueGenerator::doGenerate(Layer* layer,           IssueArray& issues) const {}
        void IssueGenerator::doGenerate(Group* group,           IssueArray& issues) const {}
        void IssueGenerator::doGenerate(Entity* entity,         IssueArray& issues) const { doGenerate(static_cast<AttributableNode*>(entity), issues); }
        void IssueGenerator::doGenerate(Brush* brush,           IssueArray& issues) const {}
        void IssueGenerator::doGenerate(AttributableNode* node, IssueArray& issues) const {}
    }
}
