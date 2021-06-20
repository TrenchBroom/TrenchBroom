/*
 Copyright (C) 2021 Kristian Duske

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

#include <vector>

namespace TrenchBroom {
    class NotifierBase;

    /**
     * Manages one or multiple connections of observer calllbacks to notifiers.
     *
     * All connections are disconnected when an instance of this class is destroyed.
     */
    class NotifierConnection {
    private:
        std::vector<std::tuple<NotifierBase&, size_t>> m_connections;
    public:
        /**
         * Creates a new instance that contains no connections.
         */
        NotifierConnection();

        /**
         * Creates a new instance that contains one connection to the given notifier with the given id.
         */
        NotifierConnection(NotifierBase& notifier, const size_t id);

        NotifierConnection(const NotifierConnection&) = delete;
        NotifierConnection(NotifierConnection&&) noexcept;

        NotifierConnection& operator=(const NotifierConnection&) = delete;
        NotifierConnection& operator=(NotifierConnection&&) noexcept;

        /**
         * Disconnects all connected observers.
         */
        ~NotifierConnection();

        /**
         * Transfers the connections from the given instance to this one. The given instance will lose its connections.
         */
        NotifierConnection& operator+=(NotifierConnection&& other);

        /**
         * Disconnects all connected observers.
         */
        void disconnect();
    };
}
