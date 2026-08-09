/*
 Copyright (C) 2023 Kristian Duske

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

#include "ui/DrawShapeToolExtensionPage.h"

#include <vector>

namespace tb::ui
{
class MapDocument;
class DrawShapeToolParameters;

class DrawShapeToolAxisAlignedShapeExtensionPage : public DrawShapeToolExtensionPage
{
public:
  explicit DrawShapeToolAxisAlignedShapeExtensionPage(
    DrawShapeToolParameters& parameters, QWidget* parent = nullptr);

private:
  DrawShapeToolParameters& m_parameters;
};

class DrawShapeToolCircularShapeExtensionPage
  : public DrawShapeToolAxisAlignedShapeExtensionPage
{
public:
  explicit DrawShapeToolCircularShapeExtensionPage(
    DrawShapeToolParameters& parameters, QWidget* parent = nullptr);

private:
  DrawShapeToolParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolCylinderShapeExtensionPage
  : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolCylinderShapeExtensionPage(
    MapDocument& document,
    DrawShapeToolParameters& parameters,
    QWidget* parent = nullptr);

private:
  DrawShapeToolParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolConeShapeExtensionPage : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolConeShapeExtensionPage(
    MapDocument& document,
    DrawShapeToolParameters& parameters,
    QWidget* parent = nullptr);

private:
  DrawShapeToolParameters& m_parameters;
  Q_OBJECT
};

class DrawShapeToolIcoSphereShapeExtensionPage : public DrawShapeToolExtensionPage
{
public:
  explicit DrawShapeToolIcoSphereShapeExtensionPage(
    MapDocument& document,
    DrawShapeToolParameters& parameters,
    QWidget* parent = nullptr);

private:
  DrawShapeToolParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolUvSphereShapeExtensionPage
  : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolUvSphereShapeExtensionPage(
    MapDocument& document,
    DrawShapeToolParameters& parameters,
    QWidget* parent = nullptr);

private:
  DrawShapeToolParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolStairsExtensionPage : public DrawShapeToolExtensionPage
{
public:
  explicit DrawShapeToolStairsExtensionPage(
    MapDocument& document,
    DrawShapeToolParameters& parameters,
    QWidget* parent = nullptr);

private:
  DrawShapeToolParameters& m_parameters;

  Q_OBJECT
};

class DrawShapeToolArchShapeExtensionPage : public DrawShapeToolCircularShapeExtensionPage
{
public:
  explicit DrawShapeToolArchShapeExtensionPage(
    MapDocument& document,
    DrawShapeToolParameters& parameters,
    QWidget* parent = nullptr);

private:
  DrawShapeToolParameters& m_parameters;

  Q_OBJECT
};

std::vector<DrawShapeToolExtensionPage*> createDrawShapeToolExtensionPages(
  MapDocument& document, DrawShapeToolParameters& parameters, QWidget* parent = nullptr);

} // namespace tb::ui
