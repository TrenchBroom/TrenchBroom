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

#include "mdl/LoadShaders.h"

#include "Error.h" // IWYU pragma: keep
#include "Logger.h"
#include "SimpleParserStatus.h"
#include "fs/FileSystem.h"
#include "fs/PathInfo.h"
#include "fs/TraversalMode.h"
#include "mdl/GameConfig.h"
#include "mdl/Quake3Shader.h"
#include "mdl/Quake3ShaderParser.h"

#include "kd/result.h"
#include "kd/result_fold.h"
#include "kd/task_manager.h"
#include "kd/vector_utils.h"

#include <fmt/format.h>

#include <vector>

namespace tb::mdl
{

namespace
{

Result<std::vector<Quake3Shader>> loadShader(
  const fs::FileSystem& fs, const std::filesystem::path& path, Logger& logger)
{
  return fs.openFile(path) | kdl::and_then([&](auto file) {
           auto bufferedReader = file->reader().buffer();
           auto parser = Quake3ShaderParser{bufferedReader.stringView()};
           auto status = SimpleParserStatus{logger, path.string()};
           return parser.parse(status);
         })
         | kdl::transform_error([&](const auto& e) {
             logger.warn() << "Skipping malformed shader file " << path << ": " << e.msg;
             return std::vector<Quake3Shader>{};
           });
}

} // namespace

Result<std::vector<Quake3Shader>> loadShaders(
  const fs::FileSystem& fs,
  const MaterialConfig& materialConfig,
  kdl::task_manager& taskManager,
  Logger& logger)
{
  if (fs.pathInfo(materialConfig.shaderSearchPath) != fs::PathInfo::Directory)
  {
    return std::vector<Quake3Shader>{};
  }

  return fs.find(
           materialConfig.shaderSearchPath,
           fs::TraversalMode::Flat,
           fs::makeExtensionPathMatcher({".shader"}))
         | kdl::and_then([&](auto paths) {
             auto tasks =
               paths | std::views::transform([&](const auto& path) {
                 return std::function{[&]() { return loadShader(fs, path, logger); }};
               });
             return taskManager.run_tasks_and_wait(tasks) | kdl::fold;
           })
         | kdl::transform([&](auto nestedShaders) {
             return nestedShaders | std::views::join | kdl::ranges::to<std::vector>();
           })
         | kdl::transform([](auto shaders) {
             auto result = kdl::vec_sort_and_remove_duplicates(
               std::move(shaders), [](const auto& lhs, const auto& rhs) {
                 return lhs.shaderPath < rhs.shaderPath;
               });
             return result;
           })
         | kdl::transform([&](auto shaders) {
             logger.info() << "Found " << shaders.size() << " shaders";
             return shaders;
           });
}

} // namespace tb::mdl
