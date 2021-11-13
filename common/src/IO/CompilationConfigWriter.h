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

#pragma once

#include "EL/EL_Forward.h"
#include "Macros.h"

#include <iosfwd>
#include <string>

namespace TrenchBroom {
namespace Model {
class CompilationConfig;
class CompilationProfile;
} // namespace Model

namespace IO {
class CompilationConfigWriter {
private:
  const Model::CompilationConfig& m_config;
  std::ostream& m_stream;

public:
  CompilationConfigWriter(const Model::CompilationConfig& config, std::ostream& stream);

  void writeConfig();

private:
  EL::Value writeProfiles(const Model::CompilationConfig& config) const;
  EL::Value writeProfile(const Model::CompilationProfile* profile) const;

  class WriteCompilationTaskVisitor;
  EL::Value writeTasks(const Model::CompilationProfile* profile) const;

  std::string escape(const std::string& str) const;

  deleteCopyAndMove(CompilationConfigWriter)
};
} // namespace IO
} // namespace TrenchBroom
