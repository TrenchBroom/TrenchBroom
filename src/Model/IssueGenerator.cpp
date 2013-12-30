/*
 Copyright (C) 2010-2013 Kristian Duske
 
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

#include "Model/Brush.h"
#include "Model/Entity.h"
#include "Model/Object.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        class Visitor : public ObjectVisitor {
        private:
            const IssueGenerator& m_generator;
            Issue* m_issue;
        public:
            Visitor(const IssueGenerator& generator) :
            m_generator(generator),
            m_issue(NULL) {}
            
            Issue* issue() const {
                return m_issue;
            }
            
            void doVisit(Entity* entity) {
                assert(m_issue == NULL);
                m_issue = m_generator.generate(entity);
            }
            
            void doVisit(Brush* brush) {
                assert(m_issue == NULL);
                m_issue = m_generator.generate(brush);
            }
        };
        
        IssueGenerator::~IssueGenerator() {}
        
        Issue* IssueGenerator::generate(Object* object) const {
            Visitor visitor(*this);
            object->visit(visitor);
            return visitor.issue();
        }

        Issue* IssueGenerator::generate(Entity* entity) const {
            return NULL;
        }
        
        Issue* IssueGenerator::generate(Brush* brush) const {
            return NULL;
        }
    }
}
