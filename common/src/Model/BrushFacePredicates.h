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

#ifndef TrenchBroom_BrushFacePredicates
#define TrenchBroom_BrushFacePredicates

namespace TrenchBroom {
    namespace Model {
        class BrushFace;

        namespace BrushFacePredicates {
            struct True {
                bool operator()(const BrushFace* face) const;
            };
            
            struct False {
                bool operator()(const BrushFace* face) const;
            };
            
            template <typename P>
            class Not {
            private:
                P m_p;
            public:
                Not(const P& p) :
                m_p(p) {}
                
                bool operator()(const BrushFace* face) const { return !m_p(face);  }
                bool operator()(BrushFace* face) const       { return !m_p(face);  }
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
                
                bool operator()(const BrushFace* face) const { return m_p1(face) && m_p2(face);  }
                bool operator()(BrushFace* face) const       { return m_p1(face) && m_p2(face);  }
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
                
                bool operator()(const BrushFace* face) const { return m_p1(face) || m_p2(face);  }
                bool operator()(BrushFace* face) const       { return m_p1(face) || m_p2(face);  }
            };
        }
    }
}

#endif /* defined(TrenchBroom_BrushFacePredicates) */
