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

#include "IO/ConfigParserBase.h"
#include "IO/Path.h"
#include "Model/CompilationConfig.h"
#include "Model/CompilationProfile.h"
#include "Model/CompilationTask.h"

#include <string>

namespace TrenchBroom {
    namespace IO {
        class CompilationConfigParser : public ConfigParserBase {
        public:
            CompilationConfigParser(const char* begin, const char* end, const Path& path);
            CompilationConfigParser(const std::string& str, const Path& path = Path(""));

            Model::CompilationConfig parse();
        private:
            Model::CompilationProfile::List parseProfiles(const EL::Value& value) const;
            Model::CompilationProfile* parseProfile(const EL::Value& value) const;

            Model::CompilationTask::List parseTasks(const EL::Value& value) const;
            Model::CompilationTask* parseTask(const EL::Value& value) const;
            Model::CompilationTask* parseExportTask(const EL::Value& value) const;
            Model::CompilationTask* parseCopyTask(const EL::Value& value) const;
            Model::CompilationTask* parseToolTask(const EL::Value& value) const;

            deleteCopyAndMove(CompilationConfigParser)
        };
    }
}

#endif /* CompilationConfigParser_h */
