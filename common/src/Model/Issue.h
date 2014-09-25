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

#ifndef __TrenchBroom__Issue__
#define __TrenchBroom__Issue__

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        
        class Issue {
        protected:
            Node* const m_node;
        public:
            virtual ~Issue();

            size_t lineNumber() const;
            const String& description() const;
            
            bool hidden() const;
            void setHidden(bool hidden);
        protected:
            Issue(Node* node);
            static IssueType freeType();
        private:
            IssueType type() const;
        private: // subclassing interface
            virtual IssueType doGetType() const = 0;
            virtual const String& doGetDescription() const = 0;
        };
    }
}

#endif /* defined(__TrenchBroom__Issue__) */
