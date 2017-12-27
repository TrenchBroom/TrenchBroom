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

#ifndef TRENCHBROOM_RECOVERABLEEXCEPTIONS_H
#define TRENCHBROOM_RECOVERABLEEXCEPTIONS_H

#include "Exceptions.h"

#include <functional>

namespace TrenchBroom {
    namespace IO {
        class Path;
    }

    class RecoverableException : public Exception {
    public:
        using Op = std::function<void()>;
    private:
        std::string m_query;
        Op m_op;
    protected:
        RecoverableException(const std::string& str, const std::string& query, const Op& op);
    public:
        const std::string& query() const;
        void recover() const;
    };

    class FileDeletingException : public RecoverableException {
    public:
        FileDeletingException(const std::string& str, const IO::Path& path) noexcept;
    private:
        static std::string getQuery(const IO::Path& path);
        static Op getOp(const IO::Path& path);
    };
}

#endif //TRENCHBROOM_RECOVERABLEEXCEPTIONS_H
