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

#include <QObject>

#include "Result.h"

#include <memory>
#include <string>

class QTextEdit;

namespace tb
{
class VariableTable;

namespace mdl
{
class Map;

struct CompilationProfile;
} // namespace mdl

namespace ui
{
class CompilationRunner;

class CompilationRun : public QObject
{
  Q_OBJECT
private:
  CompilationRunner* m_currentRun{nullptr};

public:
  ~CompilationRun() override;

  bool running() const;
  Result<void> run(
    const mdl::CompilationProfile& profile,
    const mdl::Map& map,
    QTextEdit* currentOutput);
  Result<void> test(
    const mdl::CompilationProfile& profile,
    const mdl::Map& map,
    QTextEdit* currentOutput);
  void terminate();

private:
  bool doIsRunning() const;
  Result<void> run(
    const mdl::CompilationProfile& profile,
    const mdl::Map& map,
    QTextEdit* currentOutput,
    bool test);

private:
  Result<std::string> buildWorkDir(
    const mdl::CompilationProfile& profile, const mdl::Map& map);
  void cleanup();
signals:
  void compilationStarted();
  void compilationEnded();
};

} // namespace ui
} // namespace tb
