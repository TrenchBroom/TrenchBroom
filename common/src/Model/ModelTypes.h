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

#ifndef TrenchBroom_ModelTypes_h
#define TrenchBroom_ModelTypes_h

#include "StringType.h"

namespace TrenchBroom {
    namespace Model {
        using IdType = size_t;

        class Node;

        class AttributableNode;
        class World;
        class Layer;
        class Group;
        class Entity;
        class Brush;
        class BrushFace;

        class Object;

        // TODO: replace with class based enum
        typedef enum {
            Visibility_Inherited = 1,
            Visibility_Hidden    = 2,
            Visibility_Shown     = 4
        } VisibilityState;

        // TODO: replace with class based enum
        typedef enum {
            Lock_Inherited = 1,
            Lock_Locked    = 2,
            Lock_Unlocked  = 4
        } LockState;

        class NodeVisitor;
        class ConstNodeVisitor;

        using AttributeName = String;
        using AttributeValue = String;

        class BrushFaceSnapshot;

        class NodeSnapshot;

        using IssueType = int;

        class Issue;

        class IssueQuickFix;

        class IssueGenerator;

        class Game;

        // TODO: replace with class based enum
        typedef enum {
            WavefrontObj
        } ExportFormat;
    }
}

#endif
