/*
 Copyright (C) 2021 Eric Wasylishen

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

#include <QObject>

namespace TrenchBroom {
    namespace View {
        /**
         * Helper for delaying actions until control returns to the Qt event loop, and coalescing multiple requests.
         */
        class SignalDelayer : public QObject {
            Q_OBJECT
        private:
            bool m_isQueued;
        public:
            explicit SignalDelayer(QObject* parent = nullptr);
            
        public slots:
            /**
             * Enqueues an action on the Qt event loop that will emit `processSignal()`.
             * 
             * Multiple calls to `queueSignal()` before the signal is emitted will only emit one `processSignal()`.
             */
            void queueSignal();

        signals:
            void processSignal();
        };
    }
}
