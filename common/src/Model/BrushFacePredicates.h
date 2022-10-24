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

namespace TrenchBroom
{
namespace Model
{
class BrushFace;
class BrushNode;

namespace BrushFacePredicates
{
struct True
{
  bool operator()(const Model::BrushNode* brush, const BrushFace& face) const;
};

struct False
{
  bool operator()(const Model::BrushNode* brush, const BrushFace& face) const;
};

template <typename P>
class Not
{
private:
  P m_p;

public:
  explicit Not(const P& p)
    : m_p(p)
  {
  }

  bool operator()(const Model::BrushNode* brush, const BrushFace& face) const
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
  And(const P1& p1, const P2& p2)
    : m_p1(p1)
    , m_p2(p2)
  {
  }

  bool operator()(const Model::BrushNode* brush, const BrushFace& face) const
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
  Or(const P1& p1, const P2& p2)
    : m_p1(p1)
    , m_p2(p2)
  {
  }

  bool operator()(const Model::BrushNode* brush, const BrushFace& face) const
  {
    return m_p1(brush, face) || m_p2(brush, face);
  }
};
} // namespace BrushFacePredicates
} // namespace Model
} // namespace TrenchBroom
