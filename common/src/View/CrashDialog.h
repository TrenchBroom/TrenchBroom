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

#ifndef CrashDialog_h
#define CrashDialog_h

#include <QDialog>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    namespace View {
        class CrashDialog : public QDialog {
            Q_OBJECT
        public:
            CrashDialog(const IO::Path& reportPath, const IO::Path& mapPath, const IO::Path& logPath);
        private:
            void createGui(const IO::Path& reportPath, const IO::Path& mapPath, const IO::Path& logPath);
        };
    }
}

#endif /* CrashDialog_h */
