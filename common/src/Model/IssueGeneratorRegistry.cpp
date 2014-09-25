/*
 Copyright (C) 2010-2014 Kristian Duske
 
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

#include "IssueGeneratorRegistry.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        IssueGeneratorRegistry::~IssueGeneratorRegistry() {
            clearGenerators();
        }
        
        void IssueGeneratorRegistry::registerGenerator(IssueGenerator* generator) {
            assert(generators != NULL);
            assert(!VectorUtils::contains(m_generators, generator));
            m_generators.push_back(generator);
        }
        
        void IssueGeneratorRegistry::unregisterAllGenerators() {
            clearGenerators();
        }

        void IssueGeneratorRegistry::clearGenerators() {
            VectorUtils::clearAndDelete(m_generators);
        }

        void IssueGeneratorRegistry::doGenerate(const Node* node, IssueList& issues) const {
            IssueGeneratorList::const_iterator it, end;
            for (it = m_generators.begin(), end = m_generators.end(); it != end; ++it) {
                const IssueGenerator* generator = *it;
                generator->generate(node, issues);
            }
        }
    }
}
