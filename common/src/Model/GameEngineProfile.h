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

#ifndef GameEngineProfile_h
#define GameEngineProfile_h

#include "Macros.h"
#include "Notifier.h"
#include "IO/Path.h"

#include <memory>
#include <string>

namespace TrenchBroom {
    namespace Model {
        class GameEngineConfig;

        class GameEngineProfile {
        private:
            std::string m_name;
            IO::Path m_path;
            std::string m_parameterSpec;
            GameEngineConfig* m_parent;
        public:
            Notifier<> profileWillBeRemoved;
            Notifier<> profileDidChange;
        public:
            GameEngineProfile(const std::string& name, const IO::Path& path, const std::string& parameterSpec);

            std::unique_ptr<GameEngineProfile> clone() const;

            GameEngineConfig* parent() const;
            void setParent(GameEngineConfig* parent);

            const std::string& name() const;
            const IO::Path& path() const;
            const std::string& parameterSpec() const;

            void setName(const std::string& name);
            void setPath(const IO::Path& path);
            void setParameterSpec(const std::string& parameterSpec);
        private:
            void sendDidChangeNotifications();
        public:
            deleteCopyAndMove(GameEngineProfile)
        };
    }
}

#endif /* GameEngineProfile_h */
