/*
 Copyright (C) 2020 MaxED

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

#include "ColorModel.h"

#include <QColorDialog>

#include "PreferenceManager.h"
#include "Preferences.h"
#include "QtUtils.h"

namespace TrenchBroom::View
{
ColorModel::ColorModel(QObject* parent)
  : QAbstractTableModel(parent)
  , m_colorsCount(0)
{
  initialize();
}

void ColorModel::initialize()
{
  for (auto* pref : Preferences::staticPreferences())
  {
    auto* colorPref = dynamic_cast<Preference<Color>*>(pref);
    if (colorPref != nullptr && !colorPref->isReadOnly())
    {
      m_colors.emplace_back(colorPref);
    }
  }

  m_colorsCount = static_cast<int>(m_colors.size());
}

// Reset to default values
void ColorModel::reset()
{
  auto& prefs = PreferenceManager::instance();
  for (auto* pref : m_colors)
  {
    prefs.resetToDefault(*pref);
  }

  emit dataChanged(createIndex(0, 0), createIndex(m_colorsCount - 1, 2));
}

int ColorModel::rowCount(const QModelIndex& /* parent */) const
{
  return m_colorsCount;
}

int ColorModel::columnCount(const QModelIndex& /* parent */) const
{
  return 3; // Color, Context, Description
}

QVariant ColorModel::headerData(
  const int section, const Qt::Orientation orientation, const int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
  {
    switch (section)
    {
    case 0:
      return QString("Color");
    case 1:
      return QString("Context");
    case 2:
      return QString("Description");
      switchDefault();
    }
  }

  return QVariant();
}

QVariant ColorModel::data(const QModelIndex& index, const int role) const
{
  if (!checkIndex(index))
  {
    return QVariant();
  }

  if (role == Qt::DisplayRole)
  {
    const auto* colorPreference = getColorPreference(index.row());

    switch (index.column())
    {
    case 0:
      return QVariant(); // Leave first cell empty
    case 1:
      return QString::fromStdString(colorPreference->path().firstComponent().asString());
    case 2:
      return QString::fromStdString(
        colorPreference->path().deleteFirstComponent().asString(" > "));
      switchDefault();
    }
  }

  // Colorize the first cell background with the associated preference color
  if (role == Qt::BackgroundRole && index.column() == 0)
  {
    auto* colorPreference = getColorPreference(index.row());
    auto color = toQColor(pref(*colorPreference));
    color.setAlpha(255); // Ignore alpha

    return QBrush(color);
  }

  return QVariant();
}

bool ColorModel::setData(
  const QModelIndex& index, const QVariant& value, const int /*role*/)
{
  if (!checkIndex(index) || index.column() != 0)
  {
    return false;
  }

  auto* colorPreference = getColorPreference(index.row());
  const auto color = toQColor(pref(*colorPreference));

  auto newColor = value.value<QColor>();
  newColor.setAlpha(color.alpha()); // Keep initial alpha...

  auto& prefs = PreferenceManager::instance();
  prefs.set(*colorPreference, fromQColor(newColor));

  emit dataChanged(index, index);
  return true;
}

Qt::ItemFlags ColorModel::flags(const QModelIndex& index) const
{
  if (checkIndex(index) && index.column() == 0)
  {
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  }

  return Qt::ItemIsEnabled;
}

void ColorModel::pickColor(const QModelIndex& mi)
{
  if (!checkIndex(mi))
  {
    return;
  }

  // Get current color
  auto* colorPreference = getColorPreference(mi.row());
  auto color = toQColor(pref(*colorPreference));

  // Show dialog
  auto newColor = QColorDialog::getColor(
    color, nullptr, "Select new color", QColorDialog::DontUseNativeDialog);

  // Apply color (QColorDialog::getColor() returns an invalid color if the user cancels
  // the dialog)
  if (newColor.isValid())
  {
    // pickColor() can be called for column 1 or 2 if the user double-clicks those
    // columns, but we always edit column 0 (where the color is displayed)
    setData(index(mi.row(), 0), newColor, Qt::EditRole);
  }
}

Preference<Color>* ColorModel::getColorPreference(const int index) const
{
  assert(index < m_colorsCount);
  return m_colors[static_cast<size_t>(index)];
}

bool ColorModel::checkIndex(const QModelIndex& index) const
{
  return index.isValid() && index.column() < 3 && index.row() < m_colorsCount;
}
} // namespace TrenchBroom::View
