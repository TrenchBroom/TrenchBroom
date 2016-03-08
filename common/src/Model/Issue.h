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

#ifndef TrenchBroom_Issue
#define TrenchBroom_Issue

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        
        class Issue {
        private:
            size_t m_seqId;
        protected:
            Node* const m_node;
        public:
            virtual ~Issue();

            size_t seqId() const;
            size_t lineNumber() const;
            const String description() const;
            
            IssueType type() const;
            Node* node() const;
            void addSelectableNodes(Model::NodeList& nodes) const;
            
            bool hidden() const;
            void setHidden(bool hidden);
        protected:
            Issue(Node* node);
            static size_t nextSeqId();
            static IssueType freeType();
        private: // subclassing interface
            virtual IssueType doGetType() const = 0;
            virtual const String doGetDescription() const = 0;
            virtual void doAddSelectableNodes(Model::NodeList& nodes) const;
        };
        
        class EntityIssue : public Issue {
        protected:
            EntityIssue(Node* node);
        protected:
            Entity* entity() const;
        private:
            void doAddSelectableNodes(Model::NodeList& nodes) const;
        };
    }
}

#endif /* defined(TrenchBroom_Issue) */
