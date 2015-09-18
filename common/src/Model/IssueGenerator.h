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

#ifndef TrenchBroom_IssueGenerator
#define TrenchBroom_IssueGenerator

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class IssueGenerator {
        private:
            IssueType m_type;
            String m_description;
            IssueQuickFixList m_quickFixes;
        public:
            virtual ~IssueGenerator();
            
            IssueType type() const;
            const String& description() const;
            const IssueQuickFixList& quickFixes() const;
            
            void generate(World* world,   IssueList& issues) const;
            void generate(Layer* layer,   IssueList& issues) const;
            void generate(Group* group,   IssueList& issues) const;
            void generate(Entity* entity, IssueList& issues) const;
            void generate(Brush* brush,   IssueList& issues) const;
        protected:
            IssueGenerator(IssueType type, const String& description);
            void addQuickFix(IssueQuickFix* quickFix);
        private:
            virtual void doGenerate(World* world,   IssueList& issues) const;
            virtual void doGenerate(Layer* layer,   IssueList& issues) const;
            virtual void doGenerate(Group* group,   IssueList& issues) const;
            virtual void doGenerate(Entity* entity, IssueList& issues) const;
            virtual void doGenerate(Brush* brush,   IssueList& issues) const;
        };
    }
}

#endif /* defined(TrenchBroom_IssueGenerator) */
