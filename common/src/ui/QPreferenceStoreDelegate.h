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

#include <QJsonValue>
#include <QKeySequence>
#include <QObject>
#include <QString>

#include "Color.h"
#include "Notifier.h"
#include "ui/QPreferenceStoreUtils.h"

#include <chrono>
#include <filesystem>
#include <vector>

class QFileSystemWatcher;
class QTimer;

namespace tb::ui
{

class QPreferenceStoreDelegate : public QObject
{
private:
  Q_OBJECT

  QString m_preferenceFilePath;

  QTimer* m_saveTimer = nullptr;
  QFileSystemWatcher* m_fileSystemWatcher = nullptr;

  /**
   * This should always be in sync with what is on disk.
   * Preference objects may have different values if there are unsaved changes.
   * There may also be values in here we don't know how to deserialize; we write them back
   * to disk.
   */
  PreferenceValues m_cache;

  /**
   * If true, don't try to read/write preferences anymore.
   * This gets set to true if there is a JSON parse error, so
   * we don't clobber the file if the user makes a mistake while editing it by hand.
   */
  bool m_fileReadWriteDisabled = false;

public:
  Notifier<const std::vector<std::filesystem::path>&> preferencesWereReloadedNotifier;

  explicit QPreferenceStoreDelegate(
    QString preferenceFilePath, std::chrono::milliseconds saveDelay);

  ~QPreferenceStoreDelegate() override;

  bool load(const std::filesystem::path& path, bool& value);
  bool load(const std::filesystem::path& path, int& value);
  bool load(const std::filesystem::path& path, float& value);
  bool load(const std::filesystem::path& path, std::string& value);
  bool load(const std::filesystem::path& path, std::filesystem::path& value);
  bool load(const std::filesystem::path& path, Color& value);
  bool load(const std::filesystem::path& path, QKeySequence& value);

  void save(const std::filesystem::path& path, bool value);
  void save(const std::filesystem::path& path, float value);
  void save(const std::filesystem::path& path, int value);
  void save(const std::filesystem::path& path, const std::string& value);
  void save(const std::filesystem::path& path, const std::filesystem::path& value);
  void save(const std::filesystem::path& path, const Color& value);
  void save(const std::filesystem::path& path, const QKeySequence& value);

private:
  void loadCache();
  void triggerSaveChanges();
  void saveChangesImmediately();

  void showErrorAndDisableFileReadWrite(const QString& reason, const QString& suggestion);
};

} // namespace tb::ui
