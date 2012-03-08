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

#import "EdgeTree.h"

#define edgeTreeNodeHeight(node) (node == NULL ? 0 : node->height)
#define edgeTreeNodeBalance(node) (edgeTreeNodeHeight(node->right) - edgeTreeNodeHeight(node->left))

int checkEdgeTreeNode(TEdgeTreeNode* node) {
    if (node == NULL)
        return 0;

    if (node->left != NULL)
        assert(compareV3f(&node->left->position, &node->position) < 0);
    if (node->right != NULL)
        assert(compareV3f(&node->right->position, &node->position) > 0);
    

    int leftHeight = checkEdgeTreeNode(node->left);
    int rightHeight = checkEdgeTreeNode(node->right);
    
    assert(edgeTreeNodeHeight(node->left) == leftHeight);
    assert(edgeTreeNodeHeight(node->right) == rightHeight);
    assert(abs(edgeTreeNodeBalance(node)) <= 1);
    
    return 1 + maxi(leftHeight, rightHeight);
}

void checkEdgeTree(TEdgeTree* tree) {
    checkEdgeTreeNode(tree->root);
}

void switchEdgeTreeNodes(TEdgeTreeNode* node1, TEdgeTreeNode* node2) {
    TEdgeTreeNodeItem* items = node1->items;
    TVector3f position = node1->position;
    
    node1->items = node2->items;
    node1->position = node2->position;
    node2->items = items;
    node2->position = position;
}

void rotateEdgeTreeNodeLeft(TEdgeTreeNode* node) {
    TEdgeTreeNode* left = node->left;
    TEdgeTreeNode* right = node->right;
    
    node->right = right->right;
    right->right = NULL;
    node->left = right;
    right->right = right->left;
    right->left = left;
    
    switchEdgeTreeNodes(node, right);

    right->height = maxi(edgeTreeNodeHeight(right->left), edgeTreeNodeHeight(right->right)) + 1;
    node->height = maxi(edgeTreeNodeHeight(node->left), edgeTreeNodeHeight(node->right)) + 1;
}

void rotateEdgeTreeNodeRight(TEdgeTreeNode* node) {
    TEdgeTreeNode* left = node->left;
    TEdgeTreeNode* right = node->right;

    node->left = left->left;
    left->left = NULL;
    node->right = left;
    left->left = left->right;
    left->right = right;
    
    switchEdgeTreeNodes(node, left);
    
    left->height = maxi(edgeTreeNodeHeight(left->left), edgeTreeNodeHeight(left->right)) + 1;
    node->height = maxi(edgeTreeNodeHeight(node->left), edgeTreeNodeHeight(node->right)) + 1;
}

void rebalanceEdgeTreeNode(TEdgeTreeNode* node) {
    int nodeBalance = edgeTreeNodeBalance(node);
    if (nodeBalance > 1) { // right is higher than left
        int rightBalance = edgeTreeNodeBalance(node->right);
        if (rightBalance == 1) { // single left rotation with node as root
            rotateEdgeTreeNodeLeft(node);
        } else if (rightBalance == -1) { // right rotation with right as root, left rotation with node as root
            rotateEdgeTreeNodeRight(node->right);
            rotateEdgeTreeNodeLeft(node);
        }
        assert(node->left != NULL && node->right != NULL);
    } else if (nodeBalance < -1) { // left is higher than right
        int leftBalance = edgeTreeNodeBalance(node->left);
        if (leftBalance == 1) { // left rotation with left as root, right rotation with node as root
            rotateEdgeTreeNodeLeft(node->left);
            rotateEdgeTreeNodeRight(node);
        } else if (leftBalance == -1) { // single right rotation with node as root
            rotateEdgeTreeNodeRight(node);
        }
        assert(node->left != NULL && node->right != NULL);
    }
}

void freeEdgeTreeNode(TEdgeTreeNode* node) {
    if (node->left != NULL)
        freeEdgeTreeNode(node->left);
    if (node->right != NULL)
        freeEdgeTreeNode(node->right);
    
    TEdgeTreeNodeItem* item = node->items;
    while (item != NULL) {
        TEdgeTreeNodeItem* next = item->next;
        free(item);
        item = next;
    }
    
    free(node);
}

TEdgeTreeNodeItem* newEdgeTreeNodeItem(const TVector3f* position, int count, TEdgeTreeNodeItem* next) {
    TEdgeTreeNodeItem* item = malloc(sizeof(TEdgeTreeNodeItem));
    item->position = *position;
    item->count = 1;
    item->selected = 0;
    item->next = next;
    return item;
}

void insertEdgeIntoNodeItems(TEdgeTree* tree, TEdgeTreeNode* node, const TVector3f* position) {
    if (node->items == NULL) {
        node->items = newEdgeTreeNodeItem(position, 1, NULL);
        tree->count++;
    } else {
        TEdgeTreeNodeItem* item = node->items;
        int order = compareV3f(position, &item->position);
        if (order < 0) {
            node->items = newEdgeTreeNodeItem(position, 1, node->items);
            tree->count++;
        } else {
            TEdgeTreeNodeItem* previous = NULL;
            while (item != NULL) {
                if (order == 0) {
                    item->count++;
                    break;
                } else if (order < 0) { // cannot happen during the first iteration
                    previous->next = newEdgeTreeNodeItem(position, 1, item);
                    tree->count++;
                    break;
                }
                
                previous = item;
                item = item->next;
            }
            
            if (item == NULL) {
                previous->next = newEdgeTreeNodeItem(position, 1, NULL);
                tree->count++;
            }
        }
    }
}

TEdgeTreeNode* newEdgeTreeNode(const TVector3f* position) {
    TEdgeTreeNode* node = malloc(sizeof(TEdgeTreeNode));
    node->position = *position;
    node->items = NULL;
    node->height = 1;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void insertEdgeIntoNode(TEdgeTree* tree, TEdgeTreeNode* node, const TVector3f* smaller, const TVector3f* larger) {
    int order;
    
    order = compareV3f(smaller, &node->position);
    if (order == 0) {
        insertEdgeIntoNodeItems(tree, node, larger);
    } else {
        if (order < 0) { // insert left
            if (node->left == NULL)
                node->left = newEdgeTreeNode(smaller);
            insertEdgeIntoNode(tree, node->left, smaller, larger);
        } else { // insert right
            if (node->right == NULL)
                node->right = newEdgeTreeNode(smaller);
            insertEdgeIntoNode(tree, node->right, smaller, larger);
        }
        
        node->height = maxi(edgeTreeNodeHeight(node->left), edgeTreeNodeHeight(node->right)) + 1;
        rebalanceEdgeTreeNode(node);
        assert(abs(edgeTreeNodeBalance(node)) <= 1);
    }
    
    assert(abs(edgeTreeNodeBalance(node)) <= 1);
}

BOOL removeEdgeFromNodeItems(TEdgeTree* tree, TEdgeTreeNode* node, const TVector3f* position) {
    TEdgeTreeNodeItem* item;
    int order;

    item = node->items;
    if (item != NULL) {
        order = compareV3f(position, &item->position);
        if (order == 0) {
            item->count--;
            if (item->count == 0) {
                node->items = item->next;
                free(item);
                tree->count--;
            }
        } else if (order > 0) {
            TEdgeTreeNodeItem* previous = item;
            item = item->next;
            while (item != NULL) {
                order = compareV3f(position, &item->position);
                if (order == 0) {
                    item->count--;
                    if (item->count == 0) {
                        previous->next = item->next;
                        free(item);
                        tree->count--;
                        item = NULL;
                    }
                }
            }
        }
    }
    
    return node->items == NULL;
}

TEdgeTreeNode* removeEdgeFromTreeNode(TEdgeTree* tree, TEdgeTreeNode* node, const TVector3f* smaller, const TVector3f* larger) {
    int order;
    TEdgeTreeNode* result;
    
    result = node;
    order = compareV3f(smaller, &node->position);
    if (order == 0) {
        if (removeEdgeFromNodeItems(tree, node, larger)) {
            if (node->left == NULL && node->right == NULL) {
                result = NULL;
                free(node);
            } else if (node->left != NULL && node->right == NULL) {
                result = node->left;
                free(node);
            } else if (node->left == NULL && node->right != NULL) {
                result = node->right;
                free(node);
            } else {
                TEdgeTreeNode* succ = node->right;
                while (succ->left != NULL)
                    succ = succ->left;

                TEdgeTreeNodeItem* items = succ->items;
                TVector3f position = succ->position;
                
                succ->items = NULL;
                node->right = removeEdgeFromTreeNode(tree, node->right, &position, NULL);
                node->items = items;
                node->position = position;

                node->height = maxi(edgeTreeNodeHeight(node->left), edgeTreeNodeHeight(node->right)) + 1;
                rebalanceEdgeTreeNode(node);
            }
        }
    } else {
        if (order < 0)
            node->left = removeEdgeFromTreeNode(tree, node->left, smaller, larger);
        else
            node->right = removeEdgeFromTreeNode(tree, node->right, smaller, larger);

        node->height = maxi(edgeTreeNodeHeight(node->left), edgeTreeNodeHeight(node->right)) + 1;
        rebalanceEdgeTreeNode(node);
    }

    return result;
}

void initEdgeTree(TEdgeTree* tree) {
    tree->root = NULL;
    tree->count = 0;
    tree->selected = 0;
}

void clearEdgeTree(TEdgeTree* tree) {
    if (tree->root != NULL) {
        freeEdgeTreeNode(tree->root);
        tree->root = NULL;
        tree->count = 0;
    }
}

void insertEdgeIntoTree(TEdgeTree* tree, const TEdge* edge) {
    const TVector3f* smaller;
    const TVector3f* larger;
    int order;
    
    order = compareV3f(&edge->startVertex->position, &edge->endVertex->position);
    if (order <= 0) {
        smaller = &edge->startVertex->position;
        larger = &edge->endVertex->position;
    } else {
        smaller = &edge->endVertex->position;
        larger = &edge->startVertex->position;
    }
    
    if (tree->root == NULL)
        tree->root = newEdgeTreeNode(smaller);
    insertEdgeIntoNode(tree, tree->root, smaller, larger);
//    checkEdgeTree(tree);
}

void removeEdgeFromTree(TEdgeTree* tree, const TEdge* edge) {
    const TVector3f* smaller;
    const TVector3f* larger;
    int order;

    if (tree->root == NULL)
        return;
    
    order = compareV3f(&edge->startVertex->position, &edge->endVertex->position);
    if (order <= 0) {
        smaller = &edge->startVertex->position;
        larger = &edge->endVertex->position;
    } else {
        smaller = &edge->endVertex->position;
        larger = &edge->startVertex->position;
    }
    
    tree->root = removeEdgeFromTreeNode(tree, tree->root, smaller, larger);
    if (tree->root != NULL) {
        tree->root->height = maxi(edgeTreeNodeHeight(tree->root->left), edgeTreeNodeHeight(tree->root->right)) + 1;
        rebalanceEdgeTreeNode(tree->root);
    }

    checkEdgeTree(tree);
}

TEdgeTreeNodeItem* findItemInNode(TEdgeTreeNode* node, const TVector3f* smaller, const TVector3f* larger) {
    int order;
    
    order = compareV3f(smaller, &node->position);
    if (order == 0) {
        TEdgeTreeNodeItem* item = node->items;
        while (item != NULL) {
            if (compareV3f(larger, &item->position) == 0)
                return item;
            item = item->next;
        }
    } else if (order < 0 && node->left != NULL) {
        return findItemInNode(node->left, smaller, larger);
    } else if (node->right != NULL) {
        return findItemInNode(node->right, smaller, larger);
    }
    
    return NULL;
}

void selectEdgeInTree(TEdgeTree* tree, const TEdge* edge) {
    const TVector3f* smaller;
    const TVector3f* larger;
    int order;
    
    if (tree->root == NULL)
        return;
    
    order = compareV3f(&edge->startVertex->position, &edge->endVertex->position);
    if (order <= 0) {
        smaller = &edge->startVertex->position;
        larger = &edge->endVertex->position;
    } else {
        smaller = &edge->endVertex->position;
        larger = &edge->startVertex->position;
    }
    
    TEdgeTreeNodeItem* item = findItemInNode(tree->root, smaller, larger);
    if (item != NULL) {
        item->selected++;
        if (item->selected == 1)
            tree->selected++;
    }
}

void deselectEdgeInTree(TEdgeTree* tree, const TEdge* edge) {
    const TVector3f* smaller;
    const TVector3f* larger;
    int order;
    
    if (tree->root == NULL)
        return;
    
    order = compareV3f(&edge->startVertex->position, &edge->endVertex->position);
    if (order <= 0) {
        smaller = &edge->startVertex->position;
        larger = &edge->endVertex->position;
    } else {
        smaller = &edge->endVertex->position;
        larger = &edge->startVertex->position;
    }
    
    TEdgeTreeNodeItem* item = findItemInNode(tree->root, smaller, larger);
    if (item != NULL) {
        item->selected--;
        if (item->selected == 0)
            tree->selected--;
    }
}
