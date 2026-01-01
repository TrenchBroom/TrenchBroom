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

#include "ui/QPreferenceStore.h"

#include "ui/QPreferenceStoreDelegate.h"

namespace tb::ui
{

QPreferenceStore::QPreferenceStore(
  QString preferenceFilePath, std::chrono::milliseconds saveDelay)
  : m_delegate{std::make_unique<QPreferenceStoreDelegate>(
      std::move(preferenceFilePath), saveDelay)}
{
  m_notifierConnection +=
    m_delegate->preferencesWereReloadedNotifier.connect(preferencesWereReloadedNotifier);
}

QPreferenceStore::~QPreferenceStore() = default;


bool QPreferenceStore::load(const std::filesystem::path& path, bool& value)
{
  return m_delegate->load(path, value);
}

bool QPreferenceStore::load(const std::filesystem::path& path, int& value)
{
  return m_delegate->load(path, value);
}

bool QPreferenceStore::load(const std::filesystem::path& path, float& value)
{
  return m_delegate->load(path, value);
}

bool QPreferenceStore::load(const std::filesystem::path& path, std::string& value)
{
  return m_delegate->load(path, value);
}

bool QPreferenceStore::load(
  const std::filesystem::path& path, std::filesystem::path& value)
{
  return m_delegate->load(path, value);
}

bool QPreferenceStore::load(const std::filesystem::path& path, Color& value)
{
  return m_delegate->load(path, value);
}

bool QPreferenceStore::load(const std::filesystem::path& path, QKeySequence& value)
{
  return m_delegate->load(path, value);
}
void QPreferenceStore::save(const std::filesystem::path& path, const bool value)
{
  m_delegate->save(path, value);
}

void QPreferenceStore::save(const std::filesystem::path& path, const float value)
{
  m_delegate->save(path, value);
}

void QPreferenceStore::save(const std::filesystem::path& path, const int value)
{
  m_delegate->save(path, value);
}

void QPreferenceStore::save(const std::filesystem::path& path, const std::string& value)
{
  m_delegate->save(path, value);
}

void QPreferenceStore::save(
  const std::filesystem::path& path, const std::filesystem::path& value)
{
  m_delegate->save(path, value);
}

void QPreferenceStore::save(const std::filesystem::path& path, const Color& value)
{
  m_delegate->save(path, value);
}

void QPreferenceStore::save(const std::filesystem::path& path, const QKeySequence& value)
{
  m_delegate->save(path, value);
}

} // namespace tb::ui
