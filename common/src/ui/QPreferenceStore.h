/*
 Copyright (C) 2025 Kristian Duske
 Copyright (C) 2019 Eric Wasylishen

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

#include <QString>

#include "NotifierConnection.h"
#include "PreferenceStore.h"

#include <chrono>
#include <filesystem>
#include <memory>

class QKeySequence;

namespace tb
{
class Logger;

namespace ui
{
class QPreferenceStoreDelegate;

class QPreferenceStore : public PreferenceStore
{
private:
  // QPreferenceStore cannot inherit from QObject and PreferenceStore, so we use an
  // internal delegate that inherits from QObject
  std::unique_ptr<QPreferenceStoreDelegate> m_delegate;

  NotifierConnection m_notifierConnection;

public:
  explicit QPreferenceStore(
    QString preferenceFilePath,
    std::chrono::milliseconds saveDelay = std::chrono::milliseconds(500));
  ~QPreferenceStore() override;

  bool load(const std::filesystem::path& path, bool& value) override;
  bool load(const std::filesystem::path& path, int& value) override;
  bool load(const std::filesystem::path& path, float& value) override;
  bool load(const std::filesystem::path& path, std::string& value) override;
  bool load(const std::filesystem::path& path, std::filesystem::path& value) override;
  bool load(const std::filesystem::path& path, Color& value) override;
  bool load(const std::filesystem::path& path, QKeySequence& value) override;

  void save(const std::filesystem::path& path, bool value) override;
  void save(const std::filesystem::path& path, float value) override;
  void save(const std::filesystem::path& path, int value) override;
  void save(const std::filesystem::path& path, const std::string& value) override;
  void save(
    const std::filesystem::path& path, const std::filesystem::path& value) override;
  void save(const std::filesystem::path& path, const Color& value) override;
  void save(const std::filesystem::path& path, const QKeySequence& value) override;
};

} // namespace ui
} // namespace tb
