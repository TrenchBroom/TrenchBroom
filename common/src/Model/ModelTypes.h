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

#include "TrenchBroom.h"
#include "StringType.h"
#include "SharedPointer.h"

#include <vecmath/polygon.h>

// FIXME: this header must not pull in std headers
#include <map>
#include <set>
#include <vector>

namespace TrenchBroom {
    namespace Model {
        using IdType = size_t;

        class Node;
        using NodeSet = std::set<Node*>;
        using NodeList = std::vector<Node*>;
        static const NodeList EmptyNodeList(0);

        using NodeMap = std::map<Node*, Node*>;
        using ParentChildrenMap = std::map<Node*, NodeList>;

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

        using VisibilityMap = std::map<Node*, VisibilityState>;
        using LockStateMap = std::map<Node*, LockState>;

        class NodeVisitor;
        class ConstNodeVisitor;

        class World;

        class AttributableNode;
        using AttributableNodeSet = std::set<AttributableNode*>;
        static const AttributableNodeSet EmptyAttributableNodeSet;
        using AttributableNodeList = std::vector<AttributableNode*>;
        static const AttributableNodeList EmptyAttributableNodeList(0);

        class Layer;
        using LayerList = std::vector<Layer*>;
        static const LayerList EmptyLayerList(0);

        class Group;
        using GroupList = std::vector<Group*>;
        static const GroupList EmptyGroupList(0);
        using GroupSet = std::set<Group*>;
        using GroupNameMap = std::map<Group*, String>;

        class Entity;
        using EntityList = std::vector<Entity*>;
        static const EntityList EmptyEntityList(0);
        using EntitySet = std::set<Entity*>;

        class Brush;
        using BrushList = std::vector<Brush*>;
        static const BrushList EmptyBrushList(0);
        using BrushSet = std::set<Brush*>;
        static const BrushSet EmptyBrushSet;

        class Object;

        class BrushFace;
        using BrushFaceSet = std::set<BrushFace*>;
        static const BrushFaceSet EmptyBrushFaceSet;
        using BrushFaceList = std::vector<BrushFace*>;
        static const BrushFaceList EmptyBrushFaceList(0);

        using AttributeName = String;
        using AttributeNameList = std::vector<AttributeName>;
        using AttributeNameSet = std::set<AttributeName>;
        using AttributeValue = String;
        using AttributeValueList = std::vector<AttributeValue>;

        using VertexToBrushesMap = std::map<vm::vec3, BrushSet>;
        using EdgeToBrushesMap = std::map<vm::segment3, BrushSet>;
        using FaceToBrushesMap = std::map<vm::polygon3, BrushSet>;
        using VertexToFacesMap = std::map<vm::vec3, BrushFaceSet>;
        using BrushVerticesMap = std::map<Model::Brush*, std::vector<vm::vec3>>;
        using BrushEdgesMap = std::map<Model::Brush*, std::vector<vm::segment3>>;
        using BrushFacesMap = std::map<Model::Brush*, std::vector<vm::polygon3>>;

        class BrushFaceSnapshot;
        using BrushFaceSnapshotList = std::vector<BrushFaceSnapshot*>;

        class NodeSnapshot;
        using NodeSnapshotList = std::vector<NodeSnapshot*>;

        using IssueType = int;

        class Issue;
        using IssueList = std::vector<Issue*>;
        static const IssueList EmptyIssueList(0);

        class IssueQuickFix;
        using IssueQuickFixList = std::vector<IssueQuickFix*>;

        class IssueGenerator;
        using IssueGeneratorList = std::vector<IssueGenerator*>;

        class Game;
        using GameSPtr = std::shared_ptr<Game>;
        using GameWPtr = std::weak_ptr<Game>;

        // TODO: replace with class based enum
        typedef enum {
            WavefrontObj
        } ExportFormat;
    }
}

#endif
