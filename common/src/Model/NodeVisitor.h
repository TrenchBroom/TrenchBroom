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

#ifndef __TrenchBroom__NodeVisitor__
#define __TrenchBroom__NodeVisitor__

#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace Model {
        class BaseNodeVisitor {
        private:
            bool m_cancelled;
            bool m_recursionStopped;
        protected:
            BaseNodeVisitor();
        public:
            virtual ~BaseNodeVisitor();
            bool cancelled() const;
            bool recursionStopped();
        protected:
            void cancel();
            void stopRecursion();
        };
        
        class NodeVisitor : public BaseNodeVisitor {
        protected:
            NodeVisitor();
        public:
            virtual ~NodeVisitor();
            
            void visit(World* world);
            void visit(Layer* layer);
            void visit(Group* group);
            void visit(Entity* entity);
            void visit(Brush* brush);
        private:
            virtual void doVisit(World* world)   = 0;
            virtual void doVisit(Layer* layer)   = 0;
            virtual void doVisit(Group* group)   = 0;
            virtual void doVisit(Entity* entity) = 0;
            virtual void doVisit(Brush* brush)   = 0;
        };
        
        class ConstNodeVisitor : public BaseNodeVisitor {
        protected:
            ConstNodeVisitor();
        public:
            virtual ~ConstNodeVisitor();
            
            void visit(const World* world);
            void visit(const Layer* layer);
            void visit(const Group* group);
            void visit(const Entity* entity);
            void visit(const Brush* brush);
        private:
            virtual void doVisit(const World* world)   = 0;
            virtual void doVisit(const Layer* layer)   = 0;
            virtual void doVisit(const Group* group)   = 0;
            virtual void doVisit(const Entity* entity) = 0;
            virtual void doVisit(const Brush* brush)   = 0;
        };
        
        template <typename T>
        class NodeQuery {
        private:
            bool m_hasResult;
            T m_result;
        public:
            NodeQuery() :
            m_hasResult(false) {}
            
            virtual ~NodeQuery() {}
            
            bool hasResult() const {
                return m_hasResult;
            }
            
            T result() const {
                assert(hasResult());
                return m_result;
            }
        protected:
            void setResult(T result) {
                m_result = result;
                m_hasResult = true;
            }
        };
    }
}

#endif /* defined(__TrenchBroom__NodeVisitor__) */
