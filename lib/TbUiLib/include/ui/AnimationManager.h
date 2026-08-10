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

#include <QElapsedTimer>
#include <QObject>

#include "ui/Animation.h"

#include <map>
#include <memory>
#include <vector>

class QTimer;

namespace tb::ui
{

class AnimationManager : public QObject
{
  Q_OBJECT
private:
  static const int AnimationUpdateRateHz;

private:
  /**
   * To measure how much time to run the animation for in onTimerTick()
   */
  QElapsedTimer m_elapsedTimer;
  QTimer* m_timer;

  std::map<Animation::Type, std::vector<std::unique_ptr<Animation>>> m_animations;

public:
  explicit AnimationManager(QObject* parent);
  void runAnimation(std::unique_ptr<Animation> animation, bool replace);

private:
  void onTimerTick();
};

} // namespace tb::ui
