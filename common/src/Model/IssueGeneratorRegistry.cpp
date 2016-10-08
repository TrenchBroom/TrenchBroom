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

#include "IssueGeneratorRegistry.h"

#include "CollectionUtils.h"
#include "Model/IssueGenerator.h"

#include <cassert>

namespace TrenchBroom {
    namespace Model {
        IssueGeneratorRegistry::~IssueGeneratorRegistry() {
            clearGenerators();
        }
        
        const IssueGeneratorList& IssueGeneratorRegistry::registeredGenerators() const {
            return m_generators;
        }

        IssueQuickFixList IssueGeneratorRegistry::quickFixes(const IssueType issueTypes) const {
            IssueQuickFixList result;
            IssueGeneratorList::const_iterator it, end;
            for (it = m_generators.begin(), end = m_generators.end(); it != end; ++it) {
                const IssueGenerator* generator = *it;
                if ((generator->type() & issueTypes) != 0)
                    VectorUtils::append(result, generator->quickFixes());
            }
            return result;
        }

        void IssueGeneratorRegistry::registerGenerator(IssueGenerator* generator) {
            ensure(generator != NULL, "generator is null");
            assert(!VectorUtils::contains(m_generators, generator));
            m_generators.push_back(generator);
        }
        
        void IssueGeneratorRegistry::unregisterAllGenerators() {
            clearGenerators();
        }

        void IssueGeneratorRegistry::clearGenerators() {
            VectorUtils::clearAndDelete(m_generators);
        }
    }
}
