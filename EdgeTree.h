/*
Copyright (C) 2010-2012 Kristian Duske

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
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "VertexData.h"

typedef struct TEdgeTreeNodeItemTag {
    TVector3f position;
    int count;
    int selected;
    struct TEdgeTreeNodeItemTag* next;
} TEdgeTreeNodeItem;

typedef struct TEdgeTreeNodeTag {
    TVector3f position;
    TEdgeTreeNodeItem* items;
    struct TEdgeTreeNodeTag* left;
    struct TEdgeTreeNodeTag* right;
    int height;
} TEdgeTreeNode;

typedef struct  {
    TEdgeTreeNode* root;
    int count;
    int selected;
} TEdgeTree;

void initEdgeTree(TEdgeTree* tree);
void clearEdgeTree(TEdgeTree* tree);
void insertEdgeIntoTree(TEdgeTree* tree, const TEdge* edge);
void removeEdgeFromTree(TEdgeTree* tree, const TEdge* edge);
void selectEdgeInTree(TEdgeTree* tree, const TEdge* edge);
void deselectEdgeInTree(TEdgeTree* tree, const TEdge* edge);
