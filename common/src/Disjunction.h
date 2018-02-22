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

#ifndef TRENCHBROOM_DISJUNCTION_H
#define TRENCHBROOM_DISJUNCTION_H

#include <cstddef>

namespace TrenchBroom {
    /**
     * Represents a boolean disjunction where all literals are true. Literals can be added and removed by assigning
     * boolean values. Evaluates to true iff the disjunction contains at least one literal.
     */
    class Disjunction {
    private:
        size_t m_count;
    public:
        /**
         * Creates a new empty disjunction.
         */
        Disjunction();

        /**
         * Adds a new true literal.
         *
         * @return this disjunction
         */
        Disjunction& pushLiteral();

        /**
         * Removes a true literal from the disjunction.
         *
         * If a literal is removed from an empty disjunction, this function does nothing, but an assert will fail in
         * debug mode.

         * @return this disjunction
         */
        Disjunction& popLiteral();

        /**
         * Removes all literals.
         *
         * @return this disjunction
         */
        Disjunction& clearLiterals();

        /**
         * Queries the truth value of this disjunction. If it contains at least one true literal, the result is also true.
         * Otherwise, the result is false.
         *
         * @return the truth value of the disjunction
         */
        operator bool() const;

        /**
         * Helper to temporarily set a true literal to the disjunction. The literal will be removed when the instance
         * of this class is destroyed.
         */
        class TemporarilySetLiteral {
        private:
            Disjunction& m_disjunction;
        public:
            /**
             * Adds a new true literal to the given disjunction.
             *
             * @param disjunction the disjunction
             */
            TemporarilySetLiteral(Disjunction& disjunction);

            /**
             * Removes a true literal from the disjunction.
             */
            ~TemporarilySetLiteral();
        };
    };
}


#endif //TRENCHBROOM_DISJUNCTION_H
