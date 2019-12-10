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

#ifndef TrenchBroom_Model_Forward_h
#define TrenchBroom_Model_Forward_h

#include <string>

namespace TrenchBroom {
    namespace Model {
        using IdType = size_t;

        class Object;

        class Node;
        class AttributableNode;
        class World;
        class Layer;
        class Group;
        class Entity;
        class Brush;
        class BrushFace;

        enum class VisibilityState;
        enum class LockState;

        class NodeVisitor;
        class ConstNodeVisitor;

        class EntityAttribute;
        using AttributeName = std::string;
        using AttributeValue = std::string;

        class Game;

        class NodeSnapshot;
        class BrushFaceSnapshot;

        using IssueType = int;
        class Issue;
        class IssueQuickFix;
        class IssueGenerator;

        enum class ExportFormat;

        class ModelFactory;

        class Hit;
    }
}

#endif
