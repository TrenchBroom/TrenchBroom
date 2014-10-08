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

#ifndef __TrenchBroom__NodePredicates__
#define __TrenchBroom__NodePredicates__

#include <stdio.h>

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class Entity;
        class Group;
        class Layer;
        class Node;
        class World;
        
        struct True {
            bool operator()(const Node* node) const;
        };
        
        struct False {
            bool operator()(const Node* node) const;
        };
        
        template <typename P1, typename P2>
        class And {
        private:
            P1 m_p1;
            P2 m_p2;
        public:
            And(const P1& p1, const P2& p2) :
            m_p1(p1),
            m_p2(p2) {}
            
            bool operator()(const World* world) const   { return m_p1(world)  && m_p2(world);  }
            bool operator()(World* world) const         { return m_p1(world)  && m_p2(world);  }
            bool operator()(const Layer* layer) const   { return m_p1(layer)  && m_p2(layer);  }
            bool operator()(Layer* layer) const         { return m_p1(layer)  && m_p2(layer);  }
            bool operator()(const Group* group) const   { return m_p1(group)  && m_p2(group);  }
            bool operator()(Group* group) const         { return m_p1(group)  && m_p2(group);  }
            bool operator()(const Entity* entity) const { return m_p1(entity) && m_p2(entity); }
            bool operator()(Entity* entity) const       { return m_p1(entity) && m_p2(entity); }
            bool operator()(const Brush* brush) const   { return m_p1(brush)  && m_p2(brush);  }
            bool operator()(Brush* brush) const         { return m_p1(brush)  && m_p2(brush);  }
        };
        
        template <typename P1, typename P2>
        class Or {
        private:
            P1 m_p1;
            P2 m_p2;
        public:
            Or(const P1& p1, const P2& p2) :
            m_p1(p1),
            m_p2(p2) {}
            
            bool operator()(const World* world) const   { return m_p1(world)  || m_p2(world);  }
            bool operator()(World* world) const         { return m_p1(world)  || m_p2(world);  }
            bool operator()(const Layer* layer) const   { return m_p1(layer)  || m_p2(layer);  }
            bool operator()(Layer* layer) const         { return m_p1(layer)  || m_p2(layer);  }
            bool operator()(const Group* group) const   { return m_p1(group)  || m_p2(group);  }
            bool operator()(Group* group) const         { return m_p1(group)  || m_p2(group);  }
            bool operator()(const Entity* entity) const { return m_p1(entity) || m_p2(entity); }
            bool operator()(Entity* entity) const       { return m_p1(entity) || m_p2(entity); }
            bool operator()(const Brush* brush) const   { return m_p1(brush)  || m_p2(brush);  }
            bool operator()(Brush* brush) const         { return m_p1(brush)  || m_p2(brush);  }
        };
    }
}

#endif /* defined(__TrenchBroom__NodePredicates__) */
