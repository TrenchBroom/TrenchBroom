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

#ifndef Polyhedron_Intersect_h
#define Polyhedron_Intersect_h

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP> Polyhedron<T,FP,VP>::intersect(const Polyhedron& other) const {
    Callback c;
    return intersect(other, c);
}

template <typename T, typename FP, typename VP>
Polyhedron<T,FP,VP> Polyhedron<T,FP,VP>::intersect(Polyhedron other, const Callback& callback) const {
    const Face* firstFace = m_faces.front();
    const Face* currentFace = firstFace;
    do {
        const Plane<T,3> plane = callback.plane(currentFace);
        const ClipResult result = other.clip(plane);
        if (result.empty())
            return Polyhedron();
        currentFace = currentFace->next();
    } while (currentFace != firstFace);
    return other;
}

#endif /* Polyhedron_Intersect_h */
