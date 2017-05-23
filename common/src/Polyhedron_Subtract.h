/*
 Copyright (C) 2010-2017 Kristian Duske
 
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

#ifndef Polyhedron_Subtract_h
#define Polyhedron_Subtract_h

template <typename T, typename FP, typename VP>
class Polyhedron<T,FP,VP>::Fragment {
private:
    Polyhedron m_polyhedron;
public:
    Fragment(const V& initialVertex) :
    m_polyhedron({ initialVertex }) {}
    
    const Polyhedron& polyhedron() const {
        return m_polyhedron;
    }
};

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SubtractResult Polyhedron<T,FP,VP>::subtract(const Polyhedron& subtrahend) const {
    Callback c;
    return subtract(subtrahend, c);
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::SubtractResult Polyhedron<T,FP,VP>::subtract(Polyhedron subtrahend, Callback& callback) const {
    
    if (!subtrahend.clip(*this, callback).success())
        return SubtractResult();
    
    FragmentList fragments;
    fragments = createFragments(subtrahend, callback);
    fragments = partitionFragments(fragments, callback);
    
    SubtractResult result;
    std::transform(std::begin(fragments), std::end(fragments), std::back_inserter(result),
                   [](const Fragment& fragment) {
                       return fragment.polyhedron();
                   });
    
    return result;
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::FragmentList Polyhedron<T,FP,VP>::createFragments(const Polyhedron& subtrahend, const Callback& callback) const {


    const typename V::Set minuendVertices = vertexPositionSet();
    const typename V::Set subtrahendVertices = subtrahend.vertexPositionSet();
    
    typename V::Set vertices = SetUtils::merge(minuendVertices, subtrahendVertices);
    FragmentList fragments { Fragment(SetUtils::popFront(vertices)) };
    
    while (!vertices.empty()) {
        const V vertex = SetUtils::popFront(vertices);
        
        
    }
}

template <typename T, typename FP, typename VP>
typename Polyhedron<T,FP,VP>::FragmentList Polyhedron<T,FP,VP>::partitionFragments(const FragmentList& fragments, const Callback& callback) const {
    
}

#endif /* Polyhedron_Subtract_h */
