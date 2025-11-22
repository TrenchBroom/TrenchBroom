/*
 Copyright (C) 2010 Kristian Duske

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

#include "Macros.h"
#include "el/Forward.h"

#include <iosfwd>
#include <string>

namespace tb
{
namespace mdl
{
struct CompilationConfig;
struct CompilationProfile;
} // namespace mdl

namespace io
{

class CompilationConfigWriter
{
private:
  const mdl::CompilationConfig& m_config;
  std::ostream& m_stream;

public:
  CompilationConfigWriter(const mdl::CompilationConfig& config, std::ostream& stream);

  void writeConfig();

private:
  el::Value writeProfiles(const mdl::CompilationConfig& config) const;
  el::Value writeProfile(const mdl::CompilationProfile& profile) const;

  el::Value writeTasks(const mdl::CompilationProfile& profile) const;

  std::string escape(const std::string& str) const;

  deleteCopyAndMove(CompilationConfigWriter);
};

} // namespace io
} // namespace tb
