/*
Copyright (C) 2010-2011 Kristian Duske

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

#import "ControllerUtils.h"
#import "PreferencesManager.h"
#import "MutableFace.h"

static const float ArrowBaseWidth = 7;
static const float ArrowBaseLength = 11;
static const float ArrowHeadWidth = 10;
static const float ArrowHeadLength = 5;

BOOL calculateEntityOrigin(EntityDefinition* entityDefinition, PickingHitList* hits, NSPoint mousePos, Camera* camera, TVector3i* result) {
    PickingHit* hit = [hits firstHitOfType:HT_FACE ignoreOccluders:YES];
    if (hit != nil) {
        const TVector3f* hitPoint = [hit hitPoint];
        const TBoundingBox* bounds = [entityDefinition bounds];
        TVector3f size;
        sizeOfBounds(bounds, &size);
        
        id <Face> face = [hit object];
        const TVector3f* faceNorm = [face norm];
        
        EAxis lc = strongestComponentV3f(faceNorm);
        BOOL d;
        switch (lc) {
            case A_X:
                if (faceNorm->x > 0) {
                    result->x = hitPoint->x - bounds->min.x;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = YES;
                } else {
                    result->x = hitPoint->x - size.x - bounds->min.x;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = NO;
                }
                break;
            case A_Y:
                if (faceNorm->y > 0) {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - bounds->min.y;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = YES;
                } else {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - size.y - bounds->min.y;
                    result->z = hitPoint->z - bounds->min.z - size.z / 2;
                    d = NO;
                }
                break;
            case A_Z:
                if (faceNorm->z > 0) {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - bounds->min.z;
                    d = YES;
                } else {
                    result->x = hitPoint->x - bounds->min.x - size.x / 2;
                    result->y = hitPoint->y - bounds->min.y - size.y / 2;
                    result->z = hitPoint->z - size.z - bounds->min.z;
                    d = NO;
                }
                break;
        }
        
        return YES;
    } else {
        TVector3f posf = [camera defaultPointAtX:mousePos.x y:mousePos.y];
        roundV3f(&posf, result);
        return NO;
    }
}

NSArray* modListFromWorldspawn(id <Entity> worldspawn) {
    if (![worldspawn isWorldspawn])
        return nil;
    
    NSString* modsStr = [worldspawn propertyForKey:ModsKey];
    NSMutableArray* mods = [[NSMutableArray alloc] init];
    [mods addObject:@"id1"];
    if (modsStr != nil) {
        NSEnumerator* modStrEn = [[modsStr componentsSeparatedByString:@";"] objectEnumerator];
        NSString* mod;
        while ((mod = [modStrEn nextObject])) {
            NSString* cleaned = [mod stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
            if ([cleaned length] > 0)
                [mods addObject:cleaned];
        }
    }
    
    return [mods autorelease];
}

void calculateMoveDelta(Grid* grid, const TBoundingBox* bounds, const TBoundingBox* worldBounds, TVector3f* deltaf, TVector3f* lastPoint) {
    if (deltaf->x > 0) {
        deltaf->x = [grid snapDownToGridf:bounds->max.x + deltaf->x] - bounds->max.x;
        if (deltaf->x <= 0) {
            deltaf->x = 0;
        } else {
            /*
            if (deltaf->x < 1)
                deltaf->x = [grid actualSize];
             */
            if (bounds->max.x + deltaf->x > worldBounds->max.x) {
                deltaf->x = worldBounds->max.x - bounds->max.x;
                deltaf->y = 0;
                deltaf->z = 0;
            } else if (lastPoint != NULL) {
                lastPoint->x += deltaf->x;
            }
        }
    } else if (deltaf->x < 0) {
        deltaf->x = [grid snapUpToGridf:bounds->min.x + deltaf->x] - bounds->min.x;
        if (deltaf->x >= 0) {
            deltaf->x = 0;
        } else {
            /*
            if (deltaf->x > -1)
                deltaf->x = -[grid actualSize];
             */
            if (bounds->min.x + deltaf->x < worldBounds->min.x) {
                deltaf->x = worldBounds->min.x - bounds->min.x;
                deltaf->y = 0;
                deltaf->z = 0;
            } else if (lastPoint != NULL) {
                lastPoint->x += deltaf->x;
            }
        }
    }
    
    if (deltaf->y > 0) {
        deltaf->y = [grid snapDownToGridf:bounds->max.y + deltaf->y] - bounds->max.y;
        if (deltaf->y <= 0) {
            deltaf->y = 0;
        } else {
            /*
            if (deltaf->y < 1)
                deltaf->y = [grid actualSize];
             */
            if (bounds->max.y + deltaf->y > worldBounds->max.y) {
                deltaf->x = 0;
                deltaf->y = worldBounds->max.y - bounds->max.y;
                deltaf->z = 0;
            } else if (lastPoint != NULL) {
                lastPoint->y += deltaf->y;
            }
        }
    } else if (deltaf->y < 0) {
        deltaf->y = [grid snapUpToGridf:bounds->min.y + deltaf->y] - bounds->min.y;
        if (deltaf->y >= 0) {
            deltaf->y = 0;
        } else {
            /*
            if (deltaf->y > -1)
                deltaf->y = -[grid actualSize];
             */
            if (bounds->min.y + deltaf->y < worldBounds->min.y) {
                deltaf->x = 0;
                deltaf->y = worldBounds->min.y - bounds->min.y;
                deltaf->z = 0;
            } else if (lastPoint != NULL) {
                lastPoint->y += deltaf->y;
            }
        }
    }
    
    if (deltaf->z > 0) {
        deltaf->z = [grid snapDownToGridf:bounds->max.z + deltaf->z] - bounds->max.z;
        if (deltaf->z <= 0) {
            deltaf->z = 0;
        } else {
            /*
            if (deltaf->z < 1)
                deltaf->z = [grid actualSize];
             */
            if (bounds->max.z + deltaf->z > worldBounds->max.z) {
                deltaf->x = 0;
                deltaf->y = 0;
                deltaf->z = worldBounds->max.z - bounds->max.z;
            } else if (lastPoint != NULL) {
                lastPoint->z += deltaf->z;
            }
        }
    } else if (deltaf->z < 0) {
        deltaf->z = [grid snapUpToGridf:bounds->min.z + deltaf->z] - bounds->min.z;
        if (deltaf->z >= 0) {
            deltaf->z = 0;
        } else {
            /*
            if (deltaf->z > -1)
                deltaf->z = -[grid actualSize];
             */
            if (bounds->min.z + deltaf->z < worldBounds->min.z) {
                deltaf->x = 0;
                deltaf->y = 0;
                deltaf->z = worldBounds->min.z < bounds->min.z;
            } else if (lastPoint != NULL) {
                lastPoint->z += deltaf->z;
            }
        }
    }
}

float calculateDragDelta(Grid* grid, id<Face> face, const TBoundingBox* worldBounds, const TVector3f* deltaf) {
    float dist = dotV3f(deltaf, [face norm]);
    if (isnan(dist) || dist == 0)
        return NAN;
    
    // compute the rays which the vertices are moved on when the given face is dragged in the given direction
    TVertex** vertices = [face vertices];
    int vertexCount = [face vertexCount];
    
    id <Brush> brush = [face brush];
    TEdge** edges = [brush edges];
    int edgeCount = [brush edgeCount];

    TRay vertexRays[vertexCount];
    int rayIndex = 0;
    
    // look at each edge of the brush and test if exactly one of its end vertices belongs to the given face
    // if such an edge is detected, compute the ray which begins at the face vertex and points at the other
    // vertex
    for (int i = 0; i < edgeCount; i++) {
        int c = 0;
        TRay ray;

        TEdge* e = edges[i];
        for (int j = 0; j < vertexCount; j++) {
            TVertex* v = vertices[j];

            if (v == e->startVertex) {
                c++;
                if (c == 1) {
                    ray.origin = e->startVertex->vector;
                    ray.direction = e->endVertex->vector;
                } else {
                    break;
                }
            } else if (v == e->endVertex) {
                c++;
                if (c == 1) {
                    ray.origin = e->endVertex->vector;
                    ray.direction = e->startVertex->vector;
                } else {
                    break;
                }
            }
        }

        if (c == 1) {
            subV3f(&ray.direction, &ray.origin, &ray.direction);
            normalizeV3f(&ray.direction, &ray.direction);
            
            // depending on the direction of the drag vector, the rays must be inverted to reflect the
            // actual movement of the vertices
            if (dist > 0)
                scaleV3f(&ray.direction, -1, &ray.direction);
            
            vertexRays[rayIndex++] = ray;
        }
    }

    TVector3f vertexDelta;
    int gridSkip = 0;
    float dragDist = FLT_MAX;
    do {
        // Find the smallest drag distance at which the face boundary is actually moved
        // by intersecting the vertex rays with the grid planes.
        // The distance of the vertex to the closest grid plane is then multiplied by the ray
        // direction to yield the vector by which the vertex would be moved if the face was dragged
        // and the drag would snap the vertex onto the previously selected grid plane.
        // This vector is then projected onto the face normal to yield the distance by which the face
        // must be dragged so that the vertex snaps to its closest grid plane.
        // Then, test if the resulting drag distance is smaller than the current candidate and if it is, see if
        // it is large enough so that the face boundary changes when the drag is applied.
        for (int i = 0; i < vertexCount; i++) {
            float vertexDist = [grid intersectWithRay:&vertexRays[i] skip:gridSkip];
            scaleV3f(&vertexRays[i].direction, vertexDist, &vertexDelta);
            float vertexDragDist = dotV3f(&vertexDelta, [face norm]);

            if (fabsf(vertexDragDist) < fabsf(dragDist)) {
                MutableFace* testFace = [[MutableFace alloc] initWithWorldBounds:[face worldBounds] faceTemplate:face];
                [testFace dragBy:vertexDragDist lockTexture:NO];
                if (!equalPlane([face boundary], [testFace boundary]))
                    dragDist = vertexDragDist;
                [testFace release];
            }
        }
        
        // If we didn't find anything, the grid planes were too close to each vertex, so try the next grid planes.
        gridSkip++;
    } while (dragDist == FLT_MAX);
    
    if (fabsf(dragDist) > fabsf(dist))
        return NAN;
    
    return dragDist;
}


void updateMenuWithExecutables(NSMenu* menu, BOOL setIcons, SEL action) {
    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    NSArray* executables = [preferences availableExecutables];

    NSEnumerator* executableEn = [executables objectEnumerator];
    NSString* executable;
    int index = 0;
    
    while ((executable = [executableEn nextObject])) {
        NSString* filename = [executable lastPathComponent];
        NSString* appname = [filename stringByDeletingPathExtension];
        NSMenuItem* menuItem = [[NSMenuItem alloc] initWithTitle:appname action:action keyEquivalent:@""];
        if (setIcons)
            [menuItem setImage:[workspace iconForFile:executable]];
        [menuItem setTag:index++];
        [menu addItem:menuItem];
        [menuItem release];
    }
}
