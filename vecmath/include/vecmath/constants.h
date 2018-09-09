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

#ifndef TRENCHBROOM_CONSTANTS_H
#define TRENCHBROOM_CONSTANTS_H

namespace vm {
    template <typename T>
    class constants {
    public:
        /**
         * Returns an epsilon value to use for comparisons.
         *
         * @return an epsilon value
         */
        static T almostZero() {
            static const T value = static_cast<T>(0.001);
            return value;
        }

        /**
         * Returns an epsilon value to use for determining the position of a point in relation to a point and
         * vector, e.g. a plane.
         *
         * @return an epsilon value
         */
        static T pointStatusEpsilon() {
            static const T value = static_cast<T>(0.0001); // this is what tyrbsp uses
            return value;
        }

        /**
         * Returns an epsilon value to use for correcting a value that is very close to an integral value.
         *
         * @return an epsilon value
         */
        static T correctEpsilon() {
            static const T value = static_cast<T>(0.001); // this is what QBSP uses
            return value;
        }

        /**
         * Returns an epislon value to use when testing colinearity.
         *
         * @return an epsilon value
         */
        static T colinearEpsilon() {
            static const T value = static_cast<T>(0.00001); // this value seems to hit a sweet spot in relation to the point status epsilon
            return value;
        }

        /**
         * Returns an epsilon value to use when comparing angles in radians.
         *
         * @return an epsilon value
         */
        static T angleEpsilon() {
            static const T value = static_cast<T>(0.00000001); // if abs(sin()) of the angle between two vectors is less than this, they are considered to be parallel or opposite
            return value;
        }

        /**
         * Returns the value of PI.
         *
         * @return the value of PI
         */
        static T pi() {
            static const T value = static_cast<T>(3.141592653589793);
            return value;
        }

        /**
         * Returns the value of 2 * PI.
         *
         * @return the value of 2 * PI
         */
        static T twoPi() {
            static const T value = static_cast<T>(2.0) * pi();
            return value;
        }

        /**
         * Returns the value of PI / 2.
         *
         * @return the value of PI / 2
         */
        static T piOverTwo() {
            static const T value = pi() / static_cast<T>(2.0);
            return value;
        }

        /**
         * Returns the value of PI / 4.
         *
         * @return the value of PI / 4
         */
        static T piOverFour() {
            static const T value = pi() / static_cast<T>(4.0);
            return value;
        }

        /**
         * Returns the value of 3 * PI / 2.
         *
         * @return the value of 3 * PI / 2
         */
        static T threePiOverTwo() {
            static const T value = static_cast<T>(3.0) * pi() / static_cast<T>(2.0);
            return value;
        }

        /**
         * Returns the value of PI / 180.
         *
         * @return the value of PI / 180
         */
        static T piOverStraightAngle() {
            static const T value = pi() / static_cast<T>(180.0);
            return value;
        }

        /**
         * Returns the value of 180 / PI.
         *
         * @return the value of 180 / PI.
         */
        static T straightAngleOverPi() {
            static const T value = static_cast<T>(180.0) / pi();
            return value;
        }

        /**
         * Returns the value of e, the Euler constant.
         *
         * @return the value of e
         */
        static T e() {
            static const T value = static_cast<T>(2.718281828459045);
            return value;
        }
    };

    using Cf = constants<float>;
    using Cd = constants<double>;
}

#endif //TRENCHBROOM_CONSTANTS_H
