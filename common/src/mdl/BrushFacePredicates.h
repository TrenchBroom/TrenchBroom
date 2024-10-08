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

#include <utility>

namespace tb::mdl
{
class BrushFace;
class BrushNode;
} // namespace tb::mdl

namespace tb::mdl::BrushFacePredicates
{

struct True
{
  bool operator()(const mdl::BrushNode* brush, const BrushFace& face) const;
};

struct False
{
  bool operator()(const mdl::BrushNode* brush, const BrushFace& face) const;
};

template <typename P>
class Not
{
private:
  P m_p;

public:
  explicit Not(P p)
    : m_p{std::move(p)}
  {
  }

  bool operator()(const mdl::BrushNode* brush, const BrushFace& face) const
  {
    return !m_p(brush, face);
  }
};

template <typename P1, typename P2>
class And
{
private:
  P1 m_p1;
  P2 m_p2;

public:
  And(P1 p1, P2 p2)
    : m_p1{std::move(p1)}
    , m_p2{std::move(p2)}
  {
  }

  bool operator()(const mdl::BrushNode* brush, const BrushFace& face) const
  {
    return m_p1(brush, face) && m_p2(brush, face);
  }
};

template <typename P1, typename P2>
class Or
{
private:
  P1 m_p1;
  P2 m_p2;

public:
  Or(P1 p1, P2 p2)
    : m_p1{std::move(p1)}
    , m_p2{std::move(p2)}
  {
  }

  bool operator()(const mdl::BrushNode* brush, const BrushFace& face) const
  {
    return m_p1(brush, face) || m_p2(brush, face);
  }
};

} // namespace tb::mdl::BrushFacePredicates
