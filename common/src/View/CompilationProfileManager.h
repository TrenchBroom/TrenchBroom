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

#include "Model/CompilationConfig.h"

#include <memory>

#include <QWidget>

class QAbstractButton;
class QPoint;

namespace TrenchBroom
{
namespace Model
{
class CompilationProfile;
}

namespace View
{
class CompilationProfileListBox;
class CompilationProfileEditor;
class MapDocument;

/**
 * Editor widget for all profiles in a compilation config.
 *
 * The UI updates our Model::CompilationConfig m_config; calling code can
 * read the modified config with `config()` and save it to disk.
 */
class CompilationProfileManager : public QWidget
{
  Q_OBJECT
private:
  Model::CompilationConfig m_config;
  CompilationProfileListBox* m_profileList{nullptr};
  CompilationProfileEditor* m_profileEditor{nullptr};
  QAbstractButton* m_removeProfileButton{nullptr};

public:
  CompilationProfileManager(
    std::weak_ptr<MapDocument> document,
    Model::CompilationConfig config,
    QWidget* parent = nullptr);

  const Model::CompilationProfile* selectedProfile() const;
  const Model::CompilationConfig& config() const;

private:
  void updateGui();
private slots:
  void addProfile();
  void removeProfile();
  void removeProfile(size_t index);
  void removeProfile(Model::CompilationProfile& profile);
  void duplicateProfile(Model::CompilationProfile& profile);
  void profileContextMenuRequested(
    const QPoint& globalPos, Model::CompilationProfile& profile);
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
} // namespace View
} // namespace TrenchBroom
