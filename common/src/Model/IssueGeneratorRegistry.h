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

#ifndef TrenchBroom_IssueGeneratorRegistry
#define TrenchBroom_IssueGeneratorRegistry

#include "Model/ModelTypes.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        class IssueGenerator;
        
        class IssueGeneratorRegistry {
        private:
            IssueGeneratorList m_generators;
        public:
            ~IssueGeneratorRegistry();
            
            const IssueGeneratorList& registeredGenerators() const;
            IssueQuickFixList quickFixes(IssueType issueTypes) const;
            
            void registerGenerator(IssueGenerator* generator);
            void unregisterAllGenerators();
        private:
            void clearGenerators();
        };
    }
}

#endif /* defined(TrenchBroom_IssueGeneratorRegistry) */
