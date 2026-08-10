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

#include "ui/Animation.h"

#include "kd/contracts.h"

#include <algorithm>

namespace tb::ui
{

// AnimationCurve
AnimationCurve::~AnimationCurve() = default;

double AnimationCurve::apply(const double progress) const
{
  return doApply(progress);
}

double FlatAnimationCurve::doApply(const double progress) const
{
  return progress;
}

EaseInEaseOutAnimationCurve::EaseInEaseOutAnimationCurve(const double duration)
{
  m_threshold = duration < 100 + 100 ? 0.5 : 100.0 / duration;
}

double EaseInEaseOutAnimationCurve::doApply(const double progress) const
{
  if (progress < m_threshold)
  {
    return progress * progress / m_threshold;
  }

  if (progress > 1.0 - m_threshold)
  {
    auto temp = 1.0 - progress;
    temp = temp * temp / m_threshold;
    return 1.0 - m_threshold + temp;
  }

  return progress;
}

// Animation

Animation::Type Animation::freeType()
{
  static Type type = 0;
  return type++;
}

Animation::Animation(const Type type, const Curve curve, const double duration)
  : m_type{type}
  , m_curve{createAnimationCurve(curve, duration)}
  , m_duration{duration}
{
  contract_pre(m_duration > 0);
}

Animation::~Animation() = default;

Animation::Type Animation::type() const
{
  return m_type;
}

bool Animation::step(const double delta)
{
  m_elapsed = std::min(m_elapsed + delta, m_duration);
  m_progress = m_elapsed / m_duration;
  return m_elapsed >= m_duration;
}

void Animation::update()
{
  doUpdate(m_progress);
}

std::unique_ptr<AnimationCurve> Animation::createAnimationCurve(
  const Curve curve, const double duration)
{
  switch (curve)
  {
  case Curve::EaseInEaseOut:
    return std::make_unique<EaseInEaseOutAnimationCurve>(duration);
  case Curve::Flat:
    return std::make_unique<FlatAnimationCurve>();
  }
  return nullptr;
}

} // namespace tb::ui
