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

#ifndef CompilationConfigParser_h
#define CompilationConfigParser_h

#include "Macros.h"
#include "EL/EL_Forward.h"
#include "IO/ConfigParserBase.h"

#include <memory>
#include <string>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        class CompilationConfig;
        class CompilationProfile;
        class CompilationTask;
    }

    namespace IO {
        class Path;

        class CompilationConfigParser : public ConfigParserBase {
        public:
            CompilationConfigParser(const char* begin, const char* end, const Path& path);
            explicit CompilationConfigParser(const std::string& str, const Path& path = Path(""));

            Model::CompilationConfig parse();
        private:
            std::vector<std::unique_ptr<Model::CompilationProfile>> parseProfiles(const EL::Value& value) const;
            std::unique_ptr<Model::CompilationProfile> parseProfile(const EL::Value& value) const;

            std::vector<std::unique_ptr<Model::CompilationTask>> parseTasks(const EL::Value& value) const;
            std::unique_ptr<Model::CompilationTask> parseTask(const EL::Value& value) const;
            std::unique_ptr<Model::CompilationTask> parseExportTask(const EL::Value& value) const;
            std::unique_ptr<Model::CompilationTask> parseCopyTask(const EL::Value& value) const;
            std::unique_ptr<Model::CompilationTask> parseToolTask(const EL::Value& value) const;

            deleteCopyAndMove(CompilationConfigParser)
        };
    }
}

#endif /* CompilationConfigParser_h */
