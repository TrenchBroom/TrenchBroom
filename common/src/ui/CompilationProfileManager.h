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

#include <QWidget>

#include "mdl/CompilationConfig.h"

#include <memory>

class QAbstractButton;
class QPoint;

namespace tb::mdl
{
struct CompilationProfile;
}

namespace tb::ui
{
class CompilationProfileListBox;
class CompilationProfileEditor;
class MapDocument;

/**
 * Editor widget for all profiles in a compilation config.
 *
 * The UI updates our mdl::CompilationConfig m_config; calling code can
 * read the modified config with `config()` and save it to disk.
 */
class CompilationProfileManager : public QWidget
{
  Q_OBJECT
private:
  mdl::CompilationConfig m_config;
  CompilationProfileListBox* m_profileList = nullptr;
  CompilationProfileEditor* m_profileEditor = nullptr;
  QAbstractButton* m_removeProfileButton = nullptr;

public:
  CompilationProfileManager(
    std::weak_ptr<MapDocument> document,
    mdl::CompilationConfig config,
    QWidget* parent = nullptr);

  const mdl::CompilationProfile* selectedProfile() const;
  const mdl::CompilationConfig& config() const;

private:
  void updateGui();
private slots:
  void addProfile();
  void removeProfile();
  void removeProfile(size_t index);
  void removeProfile(const mdl::CompilationProfile& profile);
  void duplicateProfile(const mdl::CompilationProfile& profile);
  void profileContextMenuRequested(
    const QPoint& globalPos, mdl::CompilationProfile& profile);
  void profileSelectionChanged();
signals:
  /**
   * *Which* profile is selected changed.
   */
  void selectedProfileChanged();
  /**
   * An edit was made to a profile.
   */
  void profileChanged();
};

} // namespace tb::ui
