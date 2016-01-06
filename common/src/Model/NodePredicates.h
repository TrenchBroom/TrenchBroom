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

#ifndef TrenchBroom_NodePredicates
#define TrenchBroom_NodePredicates

#include <stdio.h>

#undef True
#undef False

namespace TrenchBroom {
    namespace Model {
        class Brush;
        class Entity;
        class Group;
        class Layer;
        class Node;
        class Object;
        class World;
        
        namespace NodePredicates {
            struct True {
                bool operator()(const Node* node) const;
            };
            
            struct False {
                bool operator()(const Node* node) const;
            };
            
            template <typename P>
            class Id {
            private:
                P m_p;
            public:
                Id(const P& p) :
                m_p(p) {}
                
                bool operator()(const World* world) const   { return m_p(world);  }
                bool operator()(World* world) const         { return m_p(world);  }
                bool operator()(const Layer* layer) const   { return m_p(layer);  }
                bool operator()(Layer* layer) const         { return m_p(layer);  }
                bool operator()(const Group* group) const   { return m_p(group);  }
                bool operator()(Group* group) const         { return m_p(group);  }
                bool operator()(const Entity* entity) const { return m_p(entity); }
                bool operator()(Entity* entity) const       { return m_p(entity); }
                bool operator()(const Brush* brush) const   { return m_p(brush);  }
                bool operator()(Brush* brush) const         { return m_p(brush);  }
            };
            
            template <typename P>
            class Not {
            private:
                P m_p;
            public:
                Not(const P& p) :
                m_p(p) {}
                
                bool operator()(const World* world) const   { return !m_p(world);  }
                bool operator()(World* world) const         { return !m_p(world);  }
                bool operator()(const Layer* layer) const   { return !m_p(layer);  }
                bool operator()(Layer* layer) const         { return !m_p(layer);  }
                bool operator()(const Group* group) const   { return !m_p(group);  }
                bool operator()(Group* group) const         { return !m_p(group);  }
                bool operator()(const Entity* entity) const { return !m_p(entity); }
                bool operator()(Entity* entity) const       { return !m_p(entity); }
                bool operator()(const Brush* brush) const   { return !m_p(brush);  }
                bool operator()(Brush* brush) const         { return !m_p(brush);  }
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
            
            class EqualsNode {
            private:
                const Node* m_node;
            public:
                EqualsNode(const Node* node) :
                m_node(node) {}
                
                bool operator()(const World* world) const;
                bool operator()(World* world) const;
                bool operator()(const Layer* layer) const;
                bool operator()(Layer* layer) const;
                bool operator()(const Group* group) const;
                bool operator()(Group* group) const;
                bool operator()(const Entity* entity) const;
                bool operator()(Entity* entity) const;
                bool operator()(const Brush* brush) const;
                bool operator()(Brush* brush) const;
            };
            
            class EqualsObject {
            private:
                const Object* m_object;
            public:
                EqualsObject(const Object* object) :
                m_object(object) {}
                
                bool operator()(const World* world) const;
                bool operator()(World* world) const;
                bool operator()(const Layer* layer) const;
                bool operator()(Layer* layer) const;
                bool operator()(const Group* group) const;
                bool operator()(Group* group) const;
                bool operator()(const Entity* entity) const;
                bool operator()(Entity* entity) const;
                bool operator()(const Brush* brush) const;
                bool operator()(Brush* brush) const;
            };
        }
    }
}

#endif /* defined(TrenchBroom_NodePredicates) */
