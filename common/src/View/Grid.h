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

#ifndef TrenchBroom_Grid
#define TrenchBroom_Grid

#include "Macros.h"
#include "TrenchBroom.h"
#include "VecMath.h"
#include "Notifier.h"
#include "Model/ModelTypes.h"

namespace TrenchBroom {
    namespace View {
        class Grid {
        public:
            static const size_t MaxSize = 8;
        private:
            size_t m_size;
            bool m_snap;
            bool m_visible;
        public:
            Notifier0 gridDidChangeNotifier;
        public:
            Grid(const size_t size);
            
            size_t size() const;
            void setSize(const size_t size);
            void incSize();
            void decSize();
            size_t actualSize() const;
            FloatType angle() const;
        
            bool visible() const;
            void toggleVisible();
            
            bool snap() const;
            void toggleSnap();
            
            template <typename T>
            T snapAngle(const T a) const {
                if (!snap())
                    return a;
                return angle() * Math::round(a / angle());
            }
        public: // Snap scalars.
            template <typename T>
            T snap(const T f) const {
                return snap(f, SnapDir_None);
            }
            
            template <typename T>
            T offset(const T f) const {
                if (!snap())
                    return static_cast<T>(0.0);
                return f - snap(f);
            }
            
            template <typename T>
            T snapUp(const T f, const bool skip) const {
                return snap(f, SnapDir_Up, skip);
            }
            
            template <typename T>
            T snapDown(const T f, const bool skip) const {
                return snap(f, SnapDir_Down, skip);
            }
        private:
            typedef enum {
                SnapDir_None,
                SnapDir_Up,
                SnapDir_Down
            } SnapDir;
            
            template <typename T>
            T snap(const T f, const SnapDir snapDir, const bool skip = false) const {
                if (!snap())
                    return f;

                const T actSize = static_cast<T>(actualSize());
                switch (snapDir) {
                    case SnapDir_None:
                        return actSize * Math::round(f / actSize);
                    case SnapDir_Up: {
                        const T s = actSize * std::ceil(f / actSize);
                        return (skip && Math::eq(s, f)) ? s + actualSize() : s;
                    }
                    case SnapDir_Down: {
                        const T s = actSize * std::floor(f / actSize);
                        return (skip && Math::eq(s, f)) ? s - actualSize() : s;
                    }
					switchDefault()
                }
            }
        public: // Snap vectors.
            template <typename T, size_t S>
            Vec<T,S> snap(const Vec<T,S>& p) const {
                return snap(p, SnapDir_None);
            }
            
            template <typename T, size_t S>
            Vec<T,S> offset(const Vec<T,S>& p) const {
                if (!snap())
                    return Vec<T,S>::Null;
                return p - snap(p);
            }
            
            template <typename T, size_t S>
            Vec<T,S> snapUp(const Vec<T,S>& p, const bool skip = false) const {
                return snap(p, SnapDir_Up, skip);
            }
            
            template <typename T, size_t S>
            Vec<T,S> snapDown(const Vec<T,S>& p, const bool skip = false) const {
                return snap(p, SnapDir_Down, skip);
            }
        private:
            template <typename T, size_t S>
            Vec<T,S> snap(const Vec<T,S>& p, const SnapDir snapDir, const bool skip = false) const {
                if (!snap())
                    return p;
                Vec<T,S> result;
                for (size_t i = 0; i < S; ++i)
                    result[i] = snap(p[i], snapDir, skip);
                return result;
            }
        public: // Snap towards an arbitrary direction.
            template <typename T, size_t S>
            Vec<T,S> snapTowards(const Vec<T,S>& p, const Vec<T,S>& d, const bool skip = false) const {
                if (!snap())
                    return p;
                Vec3 result;
                for (size_t i = 0; i < S; ++i) {
                    if (    Math::pos(d[i]))    result[i] = snapUp(p[i], skip);
                    else if(Math::neg(d[i]))    result[i] = snapDown(p[i], skip);
                    else                        result[i] = snap(p[i]);
                }
                return result;
            }
        public: // Snapping on a plane! Surprise, motherfucker!
            template <typename T>
            Vec<T,3> snap(const Vec<T,3>& p, const Plane<T,3>& onPlane) const {
                return snap(p, onPlane, SnapDir_None, false);
            }
            
            template <typename T>
            Vec<T,3> snapUp(const Vec<T,3>& p, const Plane<T,3>& onPlane, const bool skip = false) const {
                return snap(p, onPlane, SnapDir_Up, skip);
            }
            
            template <typename T>
            Vec<T,3> snapDown(const Vec<T,3>& p, const Plane<T,3>& onPlane, const bool skip = false) const {
                return snap(p, onPlane, SnapDir_Down, skip);
            }
        private:
            template <typename T>
            Vec<T,3> snap(const Vec<T,3>& p, const Plane<T,3>& onPlane, const SnapDir snapDir, const bool skip = false) const {
                Vec<T,3> result;
                switch(onPlane.normal.firstComponent()) {
                    case Math::Axis::AX:
                        result[1] = snap(p.y(), snapDir, skip);
                        result[2] = snap(p.z(), snapDir, skip);
                        result[0] = onPlane.xAt(result.yz());
                        break;
                    case Math::Axis::AY:
                        result[0] = snap(p.x(), snapDir, skip);
                        result[2] = snap(p.z(), snapDir, skip);
                        result[1] = onPlane.yAt(result.xz());
                        break;
                    case Math::Axis::AZ:
                        result[0] = snap(p.x(), snapDir, skip);
                        result[1] = snap(p.y(), snapDir, skip);
                        result[2] = onPlane.zAt(result.xy());
                        break;
                }
                return result;
            }
        public:
            FloatType intersectWithRay(const Ray3& ray, const size_t skip) const;
            
            Vec3 moveDeltaForPoint(const Vec3& point, const BBox3& worldBounds, const Vec3& delta) const;
            Vec3 moveDeltaForBounds(const Plane3& dragPlane, const BBox3& bounds, const BBox3& worldBounds, const Ray3& ray, const Vec3& position) const;
            Vec3 moveDelta(const BBox3& bounds, const BBox3& worldBounds, const Vec3& delta) const;
            Vec3 moveDelta(const Vec3& point, const BBox3& worldBounds, const Vec3& delta) const;
            Vec3 moveDelta(const Vec3& delta) const;
            Vec3 moveDelta(const Model::BrushFace* face, const Vec3& delta) const;
            Vec3 combineDeltas(const Vec3& delta1, const Vec3& delta2) const;
            Vec3 referencePoint(const BBox3& bounds) const;
        };
    }
}

#endif /* defined(TrenchBroom_Grid) */
