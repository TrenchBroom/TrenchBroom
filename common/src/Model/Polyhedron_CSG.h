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

#pragma once

#include "Polyhedron.h"

#include <vector>

namespace TrenchBroom {
    namespace Model {
        template <typename T, typename FP, typename VP>
        Polyhedron<T,FP,VP> Polyhedron<T,FP,VP>::intersect(Polyhedron other) const {
            if (!polyhedron() || !other.polyhedron()) {
                return Polyhedron();
            }

            for (const Face* currentFace : m_faces) {
                const vm::plane<T,3>& plane = currentFace->plane();
                const ClipResult result = other.clip(plane);
                if (result.empty()) {
                    return Polyhedron();
                }
            }

            return other;
        }

        template <typename T, typename FP, typename VP>
        std::vector<Polyhedron<T,FP,VP>> Polyhedron<T,FP,VP>::subtract(const Polyhedron& subtrahend) const {
            Subtract subtract(*this, subtrahend);
            return subtract.result();
        }

        template <typename T, typename FP, typename VP>
        class Polyhedron<T,FP,VP>::Subtract {
        private:
            const Polyhedron& m_minuend;
            Polyhedron m_subtrahend;

            using Fragments = std::vector<Polyhedron<T,FP,VP>>;
            Fragments m_fragments;

            using PlaneList = std::vector<vm::plane<T,3>>;
        public:
            Subtract(const Polyhedron& minuend, const Polyhedron& subtrahend) :
                m_minuend(minuend),
                m_subtrahend(subtrahend) {
                if (clipSubtrahend()) {
                    subtract();
                } else {
                    // minuend and subtrahend are disjoint
                    m_fragments = { minuend };
                }
            }

            Fragments result() {
                return m_fragments;
            }
        private:
            /**
             * Clips away the parts of m_subtrahend which are not intersecting m_minuend
             * (and therefore aren't useful for the subtraction).
             * This is an optimization that might result in better quality subtractions.
             *
             * If the entire subtrahend is clipped away (i.e. the minuend and subtrahend are disjoint), returns false.
             * Otherwise, returns true.
             */
            bool clipSubtrahend() {
                for (const Face* face : m_minuend.faces()) {
                    const ClipResult result = m_subtrahend.clip(face->plane());
                    if (result.empty()) {
                        return false;
                    }
                }

                return true;
            }

            void subtract() {
                const PlaneList planes = sortPlanes(findSubtrahendPlanes());

                assert(m_fragments.empty());
                doSubtract(Fragments{m_minuend}, std::begin(planes), std::end(planes));
            }

            /**
             * Returns a vector containing the planes of all of the subtrahend's faces.
             */
            PlaneList findSubtrahendPlanes() const {
                PlaneList result;
                result.reserve(m_subtrahend.faceCount());

                for (const auto* face : m_subtrahend.faces()) {
                    result.push_back(face->plane());
                }

                return result;
            }

            static PlaneList sortPlanes(PlaneList planes) {
                auto it = std::begin(planes);
                it = sortPlanes(it, std::end(planes), { vm::vec<T,3>::pos_x(), vm::vec<T,3>::pos_y(), vm::vec<T,3>::pos_z() });
                it = sortPlanes(it, std::end(planes), { vm::vec<T,3>::pos_y(), vm::vec<T,3>::pos_x(), vm::vec<T,3>::pos_z() });
                sortPlanes(it, std::end(planes), { vm::vec<T,3>::pos_z(), vm::vec<T,3>::pos_x(), vm::vec<T,3>::pos_y() });

                return planes;
            }

            static typename PlaneList::iterator sortPlanes(typename PlaneList::iterator begin, typename PlaneList::iterator end, const std::vector<vm::vec<T,3>>& axes) {
                if (begin == end) {
                    return end;
                }

                auto it = begin;
                while (it != end) {
                    auto next = selectPlanes(it, end, axes);
                    if (next == it || next == end) {
                        break; // no further progress
                    }
                    it = next;
                }

                return it;
            }

            static typename PlaneList::iterator selectPlanes(typename PlaneList::iterator begin, typename PlaneList::iterator end, const std::vector<vm::vec<T,3>>& axes) {
                assert(begin != end);
                assert(!axes.empty());

                vm::vec<T,3> axis = axes.front();
                auto bestIt = end;
                for (auto it = begin; it != end; ++it) {
                    auto newBestIt = selectPlane(it, bestIt, end, axis);

                    // Resolve ambiguities if necessary.
                    for (auto axIt = std::next(std::begin(axes)), axEnd = std::end(axes); newBestIt == end && axIt != axEnd; ++axIt) {
                        const vm::vec<T,3>& altAxis = *axIt;
                        newBestIt = selectPlane(it, bestIt, end, altAxis);
                        if (newBestIt != end) {
                            break;
                        }
                    }

                    if (newBestIt != end) {
                        bestIt = newBestIt;
                    }
                }

                if (bestIt == end) {
                    return end;
                }

                if (vm::abs(dot(bestIt->normal, axis)) < 0.5) {
                    return begin;
                }

                assert(bestIt != end);
                axis = -bestIt->normal;
                std::iter_swap(begin++, bestIt);

                bestIt = end;
                for (auto it = begin; it != end; ++it) {
                    const T bestDot = bestIt != end ? dot(bestIt->normal, axis) : 0.0;
                    const T curDot  = dot(it->normal, axis);

                    if (curDot > bestDot) {
                        bestIt = it;
                    }
                    if (bestDot == 1.0) {
                        break;
                    }
                }

                if (bestIt != end) {
                    std::iter_swap(begin++, bestIt);
                }

                return begin;
            }

            /**
             * From the two given planes, select the one whose normal is closer to the given axis (or its opposite),
             * and return its iterator.
             *
             * @param curIt a plane
             * @param bestIt the current best plane, or an end iterator
             * @param end the end iterator
             * @param axis the axis
             * @return an iterator to the new best plane
             */
            static typename PlaneList::iterator selectPlane(typename PlaneList::iterator curIt, typename PlaneList::iterator bestIt, typename PlaneList::iterator end, const vm::vec<T,3>& axis) {
                const T curDot = vm::dot(curIt->normal, axis);
                if (curDot == 0.0) {
                    return bestIt;
                }
                if (curDot == 1.0) {
                    return curIt;
                }

                const T bestDot = bestIt != end ? vm::dot(bestIt->normal, axis) : 0.0;
                if (vm::abs(curDot) > vm::abs(bestDot)) {
                    return curIt;
                } else if (vm::abs(curDot) < vm::abs(bestDot)) {
                    return bestIt; // implies bestIt != end
                } else {
                    // vm::abs(curDot) == vm::abs(bestDot), resolve ambiguities.

                    assert(bestIt != end); // Because curDot != 0.0, the same is true for bestDot!
                    if (bestDot < 0.0 && curDot > 0.0) {
                        // Prefer best matches pointing towards the direction of the axis, not the opposite.
                        return curIt;
                    }

                    // Could not resolve ambiguities. Caller should try other axes.
                    return end;
                }
            }

            void doSubtract(const Fragments& fragments, typename PlaneList::const_iterator curPlaneIt, typename PlaneList::const_iterator endPlaneIt) {
                if (fragments.empty() || curPlaneIt == endPlaneIt) {
                    // no more fragments to process or all of `minutendFragments`
                    // are now behind all of subtrahendPlanes so they can be discarded.
                    return;
                }

                const auto curPlane = *curPlaneIt;
                const auto curPlaneInv = curPlane.flip();

                // clip the list of minutendFragments into a list of those in front of the
                // currentPlane, and those behind
                Fragments backFragments;

                for (const Polyhedron& fragment : fragments) {
                    // the front fragments go directly into the result set.
                    Polyhedron<T,FP,VP> fragmentInFront = fragment;
                    const auto frontClipResult = fragmentInFront.clip(curPlaneInv);

                    if (!frontClipResult.empty()) { // Polyhedron::clip() keeps the part behind the plane.
                        m_fragments.push_back(std::move(fragmentInFront));
                    }

                    // back fragments need to be clipped by the rest of the subtrahend planes
                    Polyhedron<T,FP,VP> fragmentBehind = fragment;
                    const auto backClipResult = fragmentBehind.clip(curPlane);
                    if (!backClipResult.empty()) {
                        backFragments.push_back(std::move(fragmentBehind));
                    }
                }

                // recursively process the back fragments.
                doSubtract(backFragments, std::next(curPlaneIt), endPlaneIt);
            }
        };
    }
}


