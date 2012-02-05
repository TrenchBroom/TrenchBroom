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

#import "Renderer.h"
#import "MapDocument.h"
#import "Entity.h"
#import "MutableEntity.h"
#import "Brush.h"
#import "Face.h"
#import "VertexData.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "SelectionManager.h"
#import "MutableFace.h"
#import "Camera.h"
#import "Options.h"
#import "Grid.h"
#import "MapDocument.h"
#import "MapWindowController.h"
#import "GLResources.h"
#import "TextureManager.h"
#import "GLFontManager.h"
#import "CursorManager.h"
#import "Texture.h"
#import "Figure.h"
#import "Filter.h"
#import "DefaultFilter.h"
#import "EntityDefinition.h"
#import "GLUtils.h"
#import "ControllerUtils.h"
#import "PreferencesManager.h"
#import "RenderChangeSet.h"
#import "IntData.h"
#import "EntityRenderer.h"
#import "EntityRendererManager.h"
#import "TextRenderer.h"
#import "BoundsRenderer.h"
#import "EntityClassnameAnchor.h"
#import "GroupManager.h"

NSString* const RendererChanged = @"RendererChanged";
NSString* const FaceVboKey = @"FaceVbo";
NSString* const EntityBoundsVboKey = @"EntityBoundsVbo";
NSString* const SelectedEntityBoundsVboKey = @"SelectedEntityBoundsVbo";
TVector4f const EntityBoundsDefaultColor = {1, 1, 1, 0.5f};
TVector4f const EntityBoundsWireframeColor = {1, 1, 1, 0.6f};
TVector4f const EntityClassnameColor = {1, 1, 1, 1};
TVector4f const EdgeDefaultColor = {0.4f, 0.4f, 0.4f, 0.4f};
TVector4f const FaceDefaultColor = {0.2f, 0.2f, 0.2f, 1};
TVector4f const SelectionColor = {1, 0, 0, 1};
TVector4f const SelectionColor2 = {1, 0, 0, 0.2f};
TVector4f const SelectionColor3 = {1, 0, 0, 0.5f};
int const VertexSize = 3 * sizeof(float);
int const ColorSize = 4;
int const TexCoordSize = 2 * sizeof(float);

@interface Renderer (private)

- (void)writeEntityBounds:(id <Entity>)theEntity toBlock:(VBOMemBlock *)theBlock;
- (void)writeFace:(id <Face>)theFace toBlock:(VBOMemBlock *)theBlock;
- (void)writeFace:(id <Face>)theFace toIndexBuffer:(IntData *)theIndexBuffer countBuffer:(IntData *)theCountBuffer;

- (void)validateEntityRendererCache;
- (void)validateDeselection;
- (void)validateSelection;
- (void)validateAddedEntities;
- (void)validateChangedEntities;
- (void)validateAddedBrushes;
- (void)validateChangedBrushes;
- (void)validateChangedFaces;
- (void)validateRemovedBrushes;
- (void)rebuildFaceIndexBuffers;
- (void)validate;

- (void)renderEntityModels:(NSArray *)theEntities;
- (void)renderEntityBounds:(const TVector4f *)color vertexCount:(int)theVertexCount;
- (void)renderEdges:(const TVector4f *)color indexBuffers:(NSDictionary *)theIndexBuffers countBuffers:(NSDictionary *)theCountBuffers;
- (void)renderFaces:(BOOL)textured indexBuffers:(NSDictionary *)theIndexBuffers countBuffers:(NSDictionary *)theCountBuffers;
- (void)renderVertexHandles;

- (void)addBrushes:(NSArray *)theBrushes;
- (void)removeBrushes:(NSArray *)theBrushes;
- (void)addEntities:(NSArray *)theEntities;
- (void)removeEntities:(NSArray *)theEntities;

- (void)facesDidChange:(NSNotification *)notification;
- (void)brushesDidChange:(NSNotification *)notification;
- (void)brushesAdded:(NSNotification *)notification;
- (void)brushesWillBeRemoved:(NSNotification *)notification;
- (void)entitiesAdded:(NSNotification *)notification;
- (void)entitiesWillBeRemoved:(NSNotification *)notification;
- (void)propertiesDidChange:(NSNotification *)notification;

- (void)selectionAdded:(NSNotification *)notification;
- (void)selectionRemoved:(NSNotification *)notification;
- (void)textureManagerChanged:(NSNotification *)notification;
- (void)cameraChanged:(NSNotification *)notification;
- (void)optionsOrGroupsChanged:(NSNotification *)notification;
- (void)gridChanged:(NSNotification *)notification;
- (void)cursorChanged:(NSNotification *)notification;
- (void)preferencesDidChange:(NSNotification *)notification;

- (void)documentCleared:(NSNotification *)notification;
- (void)documentLoaded:(NSNotification *)notification;

@end

@implementation Renderer (private)

- (void)writeEntityBounds:(id <Entity>)theEntity toBlock:(VBOMemBlock *)theBlock {
    TVector3f t;
    TBoundingBox* bounds = [theEntity bounds];
    EntityDefinition* definition = [theEntity entityDefinition];
    TVector4f color = definition != nil ? *[definition color] : EntityBoundsDefaultColor;
    color.w = EntityBoundsDefaultColor.w;
    
    int address = [theBlock address];
    uint8_t* vboBuffer = [[theBlock vbo] buffer];
    
    // bottom side
    t = bounds->min;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.x = bounds->max.x;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.y = bounds->max.y;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.x = bounds->min.x;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    // south side
    t = bounds->min;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.z = bounds->max.z;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.x = bounds->max.x;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.z = bounds->min.z;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    // west side
    t = bounds->min;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.y = bounds->max.y;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.z = bounds->max.z;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.y = bounds->min.y;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    // top side
    t = bounds->max;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.y = bounds->min.y;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.x = bounds->min.x;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.y = bounds->max.y;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    // north side
    t = bounds->max;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.z = bounds->min.z;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.x = bounds->min.x;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.z = bounds->max.z;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    // east side
    t = bounds->max;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.z = bounds->min.z;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.y = bounds->min.y;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.z = bounds->max.z;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    [theBlock setState:BS_USED_VALID];
}

- (void)writeFace:(id <Face>)theFace toBlock:(VBOMemBlock *)theBlock {
    TVector2f texCoords, gridCoords;
    
    Texture* texture = [theFace texture];
    int width = texture != nil ? [texture width] : 1;
    int height = texture != nil ? [texture height] : 1;

    int address = [theBlock address];
    uint8_t* vboBuffer = [[theBlock vbo] buffer];

    const TVertexList* vertices = [theFace vertices];
    for (int i = 0; i < vertices->count; i++) {
        TVertex* vertex = vertices->items[i];
        [theFace gridCoords:&gridCoords forVertex:&vertex->position];
        [theFace texCoords:&texCoords forVertex:&vertex->position];
        texCoords.x /= width;
        texCoords.y /= height;
        
        address = writeVector2f(&gridCoords, vboBuffer, address);
        address = writeVector2f(&texCoords, vboBuffer, address);
        address = writeColor4fAsBytes(&EdgeDefaultColor, vboBuffer, address);
        address = writeColor4fAsBytes(&FaceDefaultColor, vboBuffer, address);
        address = writeVector3f(&vertex->position, vboBuffer, address);
    }
    
    [theBlock setState:BS_USED_VALID];
}

- (void)writeFace:(id <Face>)theFace toIndexBuffer:(IntData *)theIndexBuffer countBuffer:(IntData *)theCountBuffer {
    int index = [[theFace memBlock] address] / (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize);
    int count = [theFace vertices]->count;
    
    [theIndexBuffer appendInt:index];
    [theCountBuffer appendInt:count];
}

- (void)validateEntityRendererCache {
    if (!entityRendererCacheValid) {
        [entityRenderers removeAllObjects];

        NSArray* modelEntitiesCopy = [modelEntities copy];
        NSEnumerator* entityEn = [modelEntitiesCopy objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            id <EntityRenderer> renderer = [entityRendererManager entityRendererForEntity:entity mods:mods];
            if (renderer != nil)
                [entityRenderers setObject:renderer forKey:[entity entityId]];
            else
                [modelEntities removeObject:entity];
        }

        NSArray* selectedModelEntitiesCopy = [selectedModelEntities copy];
        entityEn = [selectedModelEntitiesCopy objectEnumerator];
        while ((entity = [entityEn nextObject])) {
            id <EntityRenderer> renderer = [entityRendererManager entityRendererForEntity:entity mods:mods];
            if (renderer != nil)
                [entityRenderers setObject:renderer forKey:[entity entityId]];
            else
                [selectedModelEntities removeObject:entity];
        }
        
        [modelEntitiesCopy release];
        [selectedModelEntitiesCopy release];

        entityRendererCacheValid = YES;
    }
}

- (void)validateDeselection {
    NSArray* deselectedEntities = [changeSet deselectedEntities];
    if ([deselectedEntities count] > 0) {
        [entityBoundsVbo activate];
        [entityBoundsVbo mapBuffer];
        
        NSEnumerator* entityEn = [deselectedEntities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            if (![entity isWorldspawn]) {
                VBOMemBlock* block = [entityBoundsVbo allocMemBlock:6 * 4 * (ColorSize + VertexSize)];
                [self writeEntityBounds:entity toBlock:block];
                [entity setBoundsMemBlock:block];
                [block setState:BS_USED_VALID];
                
                [selectedClassnameRenderer moveStringWithKey:[entity entityId] toTextRenderer:classnameRenderer];

                NSInteger index = [selectedModelEntities indexOfObjectIdenticalTo:entity];
                if (index != NSNotFound) {
                    [selectedModelEntities removeObjectAtIndex:index];
                    [modelEntities addObject:entity];
                }

                entityBoundsVertexCount += 6 * 4;
                selectedEntityBoundsVertexCount -= 6 * 4;
            }
        }
        
        [entityBoundsVbo unmapBuffer];
        [entityBoundsVbo deactivate];

        [selectedEntityBoundsVbo activate];
        [selectedEntityBoundsVbo mapBuffer];
        [selectedEntityBoundsVbo pack];
        [selectedEntityBoundsVbo unmapBuffer];
        [selectedEntityBoundsVbo deactivate];
    }
}

- (void)validateSelection {
    NSArray* selectedEntities = [changeSet selectedEntities];
    if ([selectedEntities count] > 0) {
        [selectedEntityBoundsVbo activate];
        [selectedEntityBoundsVbo mapBuffer];
        
        NSEnumerator* entityEn = [selectedEntities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            if (![entity isWorldspawn]) {
                VBOMemBlock* block = [selectedEntityBoundsVbo allocMemBlock:6 * 4 * (ColorSize + VertexSize)];
                [self writeEntityBounds:entity toBlock:block];
                [entity setBoundsMemBlock:block];
                [block setState:BS_USED_VALID];
                
                [classnameRenderer moveStringWithKey:[entity entityId] toTextRenderer:selectedClassnameRenderer];
                
                NSInteger index = [modelEntities indexOfObjectIdenticalTo:entity];
                if (index != NSNotFound) {
                    [modelEntities removeObjectAtIndex:index];
                    [selectedModelEntities addObject:entity];
                }

                entityBoundsVertexCount -= 6 * 4;
                selectedEntityBoundsVertexCount += 6 * 4;
            }
        }
        
        [selectedEntityBoundsVbo unmapBuffer];
        [selectedEntityBoundsVbo deactivate];

        [entityBoundsVbo activate];
        [entityBoundsVbo mapBuffer];
        [entityBoundsVbo pack];
        [entityBoundsVbo unmapBuffer];
        [entityBoundsVbo deactivate];
    }
}

- (void)validateAddedEntities {
    NSArray* addedEntities = [changeSet addedEntities];
    if ([addedEntities count] > 0) {
        [entityBoundsVbo activate];
        [entityBoundsVbo mapBuffer];

        NSEnumerator* entityEn = [addedEntities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            if (![entity isWorldspawn]) {
                VBOMemBlock* block = [entityBoundsVbo allocMemBlock:6 * 4 * (ColorSize + VertexSize)];
                [self writeEntityBounds:entity toBlock:block];
                [entity setBoundsMemBlock:block];
                
                id <EntityRenderer> renderer = [entityRendererManager entityRendererForEntity:entity mods:mods];
                if (renderer != nil) {
                    [entityRenderers setObject:renderer forKey:[entity entityId]];
                    [modelEntities addObject:entity];
                }
                
                entityBoundsVertexCount += 6 * 4;
            }
        }

        [entityBoundsVbo unmapBuffer];
        [entityBoundsVbo deactivate];
        
        [fontManager activate];
        entityEn = [addedEntities objectEnumerator];
        while ((entity = [entityEn nextObject])) {
            if (![entity isWorldspawn]) {
                NSString* classname = [entity classname];
                EntityClassnameAnchor* anchor = [[EntityClassnameAnchor alloc] initWithEntity:entity];
                [classnameRenderer addString:classname forKey:[entity entityId] withFont:[NSFont systemFontOfSize:9] withAnchor:anchor];
                [anchor release];
            }
        }
        [fontManager deactivate];
    }
}

- (void)validateChangedEntities {
    NSArray* changedEntities = [changeSet changedEntities];
    if ([changedEntities count] > 0) {
        [selectedEntityBoundsVbo activate];
        [selectedEntityBoundsVbo mapBuffer];

        NSMutableArray* unselectedEntities = [[NSMutableArray alloc] init];
        NSEnumerator* entityEn = [changedEntities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            if (![entity isWorldspawn]) {
                VBOMemBlock* block = [entity boundsMemBlock];
                if ([block vbo] == entityBoundsVbo)
                    [unselectedEntities addObject:entity];
                else
                    [self writeEntityBounds:entity toBlock:block];
            }
        }
        
        [selectedEntityBoundsVbo unmapBuffer];
        [selectedEntityBoundsVbo deactivate];
        
        if ([unselectedEntities count] > 0) {
            [entityBoundsVbo activate];
            [entityBoundsVbo mapBuffer];
            
            entityEn = [unselectedEntities objectEnumerator];
            while ((entity = [entityEn nextObject])) {
                VBOMemBlock* block = [entity boundsMemBlock];
                [self writeEntityBounds:entity toBlock:block];
            }
            
            [entityBoundsVbo unmapBuffer];
            [entityBoundsVbo deactivate];
        }
    }
}

- (void)validateRemovedEntities {
    NSArray* removedEntities = [changeSet removedEntities];
    if ([removedEntities count] > 0) {
        [entityBoundsVbo activate];
        [entityBoundsVbo mapBuffer];
        
        NSEnumerator* entityEn = [removedEntities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            if (![entity isWorldspawn]) {
                [entity setBoundsMemBlock:nil];
                [entityRenderers removeObjectForKey:[entity entityId]];
                [modelEntities removeObject:entity];
            }
        }

        [entityBoundsVbo pack];
        entityBoundsVertexCount -= 6 * 4 * [removedEntities count];

        [entityBoundsVbo unmapBuffer];
        [entityBoundsVbo deactivate];

        [fontManager activate];
        entityEn = [removedEntities objectEnumerator];
        while ((entity = [entityEn nextObject]))
            if (![entity isWorldspawn])
                [classnameRenderer removeStringForKey:[entity entityId]];
        [fontManager deactivate];
    }
}

- (void)validateAddedBrushes {
    NSArray* addedBrushes = [changeSet addedBrushes];
    if ([addedBrushes count] > 0) {
        [faceVbo activate];
        [faceVbo mapBuffer];

        NSEnumerator* brushEn = [addedBrushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject])) {
                VBOMemBlock* block = [faceVbo allocMemBlock:[face vertices]->count * (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize)];
                [self writeFace:face toBlock:block];
                [face setMemBlock:block];
            }
        }

        [faceVbo unmapBuffer];
        [faceVbo deactivate];
    }
}

- (void)validateChangedBrushes {
    NSArray* changedBrushes = [changeSet changedBrushes];
    if ([changedBrushes count] > 0) {
        [faceVbo activate];
        [faceVbo mapBuffer];
        
        NSEnumerator* brushEn = [changedBrushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject])) {
                int blockSize = [face vertices]->count * (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize);
                VBOMemBlock* block = [face memBlock];
                if ([block capacity] != blockSize) {
                    block = [faceVbo allocMemBlock:blockSize];
                    [face setMemBlock:block];
                }
                
                [self writeFace:face toBlock:block];
            }
        }

        [faceVbo unmapBuffer];
        [faceVbo deactivate];
    }
}

- (void)validateChangedFaces {
    NSArray* changedFaces = [changeSet changedFaces];
    if ([changedFaces count] > 0) {
        [faceVbo activate];
        [faceVbo mapBuffer];
        
        NSEnumerator* faceEn = [changedFaces objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            int blockSize = [face vertices]->count * (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize);
            VBOMemBlock* block = [face memBlock];
            if ([block capacity] != blockSize) {
                block = [faceVbo allocMemBlock:blockSize];
                [face setMemBlock:block];
            }
            
            [self writeFace:face toBlock:block];
        }
        
        [faceVbo unmapBuffer];
        [faceVbo deactivate];
    }
}

- (void)validateRemovedBrushes {
    NSArray* removedBrushes = [changeSet removedBrushes];
    if ([removedBrushes count] > 0) {
        [faceVbo activate];
        [faceVbo mapBuffer];
        
        NSEnumerator* brushEn = [removedBrushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSEnumerator* faceEn = [[brush faces] objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject]))
                [face setMemBlock:nil];
        }
        
        [faceVbo unmapBuffer];
        [faceVbo deactivate];
    }
}

- (void)rebuildFaceIndexBuffers {
    [faceIndexBuffers removeAllObjects];
    [faceCountBuffers removeAllObjects];
    
    MapDocument* map = [windowController document];
    NSEnumerator* entityEn = [[map entities] objectEnumerator];
    id <Entity> entity;
    while  ((entity = [entityEn nextObject])) {
        if ([filter isEntityRenderable:entity]) {
            NSEnumerator* brushEn = [[entity brushes] objectEnumerator];
            id <Brush> brush;
            while ((brush = [brushEn nextObject])) {
                if ([filter isBrushRenderable:brush]) {
                    NSEnumerator* faceEn = [[brush faces] objectEnumerator];
                    id <Face> face;
                    while ((face = [faceEn nextObject])) {
                        NSString* textureName = [[face texture] name];
                        IntData* indexBuffer = [faceIndexBuffers objectForKey:textureName];
                        if (indexBuffer == nil) {
                            indexBuffer = [[IntData alloc] init];
                            [faceIndexBuffers setObject:indexBuffer forKey:textureName];
                            [indexBuffer release];
                        }
                        
                        IntData* countBuffer = [faceCountBuffers objectForKey:textureName];
                        if (countBuffer == nil) {
                            countBuffer = [[IntData alloc] init];
                            [faceCountBuffers setObject:countBuffer forKey:textureName];
                            [countBuffer release];
                        }
                        
                        [self writeFace:face toIndexBuffer:indexBuffer countBuffer:countBuffer];
                    }
                }
            }
        }
    }
}

- (void)rebuildSelectedFaceIndexBuffers {
    [selectedFaceIndexBuffers removeAllObjects];
    [selectedFaceCountBuffers removeAllObjects];
    
    MapDocument* map = [windowController document];
    SelectionManager* selectionManager = [map selectionManager];
    
    NSEnumerator* brushEn = [[selectionManager selectedBrushes] objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject])) {
        NSEnumerator* faceEn = [[brush faces] objectEnumerator];
        id <Face> face;
        while ((face = [faceEn nextObject])) {
            NSString* textureName = [[face texture] name];
            IntData* indexBuffer = [selectedFaceIndexBuffers objectForKey:textureName];
            if (indexBuffer == nil) {
                indexBuffer = [[IntData alloc] init];
                [selectedFaceIndexBuffers setObject:indexBuffer forKey:textureName];
                [indexBuffer release];
            }
            
            IntData* countBuffer = [selectedFaceCountBuffers objectForKey:textureName];
            if (countBuffer == nil) {
                countBuffer = [[IntData alloc] init];
                [selectedFaceCountBuffers setObject:countBuffer forKey:textureName];
                [countBuffer release];
            }
            
            [self writeFace:face toIndexBuffer:indexBuffer countBuffer:countBuffer];
        }
    }
    
    NSEnumerator* faceEn = [[selectionManager selectedFaces] objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        NSString* textureName = [[face texture] name];
        IntData* indexBuffer = [selectedFaceIndexBuffers objectForKey:textureName];
        if (indexBuffer == nil) {
            indexBuffer = [[IntData alloc] init];
            [selectedFaceIndexBuffers setObject:indexBuffer forKey:textureName];
            [indexBuffer release];
        }
        
        IntData* countBuffer = [selectedFaceCountBuffers objectForKey:textureName];
        if (countBuffer == nil) {
            countBuffer = [[IntData alloc] init];
            [selectedFaceCountBuffers setObject:countBuffer forKey:textureName];
            [countBuffer release];
        }
        
        [self writeFace:face toIndexBuffer:indexBuffer countBuffer:countBuffer];
    }
}

- (void)validate {
    [self validateEntityRendererCache];
    
    [self validateAddedEntities];
    [self validateAddedBrushes];
    
    [self validateSelection];
    
    [self validateChangedEntities];
    [self validateChangedBrushes];
    [self validateChangedFaces];
    
    [self validateDeselection];
    
    [self validateRemovedEntities];
    [self validateRemovedBrushes];

    if ([[changeSet addedBrushes] count] > 0 ||
        [[changeSet changedBrushes] count] > 0 ||
        [[changeSet changedFaces] count] > 0 ||
        [[changeSet removedBrushes] count] > 0 ||
        [[changeSet selectedBrushes] count] > 0 ||
        [[changeSet deselectedBrushes] count] > 0 ||
        [[changeSet selectedFaces] count] > 0 ||
        [[changeSet deselectedFaces] count] > 0 ||
        [changeSet filterChanged]) {
        
        [self rebuildFaceIndexBuffers];
        [self rebuildSelectedFaceIndexBuffers];
    }
    
    TBoundingBox selectionBounds;
    SelectionManager* selectionManager = [windowController selectionManager];
    [selectionManager selectionBounds:&selectionBounds];
    [selectionBoundsRenderer setBounds:&selectionBounds];
    
    [changeSet clear];
}

- (void)renderEntityModels:(NSArray *)theEntities {
    [entityRendererManager activate];
    
    glMatrixMode(GL_MODELVIEW);
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject])) {
        if (filter == nil || [filter isEntityRenderable:entity]) {
            id <EntityRenderer> entityRenderer = [entityRenderers objectForKey:[entity entityId]];
            
            glPushMatrix();
            [entityRenderer renderWithEntity:entity];
            glPopMatrix();
        }
    }
    
    glDisable(GL_TEXTURE_2D);
    [entityRendererManager deactivate];
}

- (void)renderEntityBounds:(const TVector4f *)color vertexCount:(int)theVertexCount {
    glSetEdgeOffset(0.5f);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);
    
    if (color != NULL) {
        glColor4f(color->x, color->y, color->z, color->w);
        glVertexPointer(3, GL_FLOAT, ColorSize + VertexSize, (const GLvoid *)(long)ColorSize);
    } else {
        glInterleavedArrays(GL_C4UB_V3F, 0, 0);
    }

    glDrawArrays(GL_QUADS, 0, theVertexCount);
    if (color == NULL)
        glDisableClientState(GL_COLOR_ARRAY);
    glEnable(GL_CULL_FACE);
    glResetEdgeOffset();
}

- (void)renderEdges:(const TVector4f *)color indexBuffers:(NSDictionary *)theIndexBuffers countBuffers:(NSDictionary *)theCountBuffers {
    glDisable(GL_TEXTURE_2D);
    
    if (color != NULL) {
        glColor4f(color->x, color->y, color->z, color->w);
        glVertexPointer(3, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize + ColorSize));
    } else {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize));
        glVertexPointer(3, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize + ColorSize));
    }
    
    NSEnumerator* textureNameEn = [theIndexBuffers keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        IntData* indexBuffer = [theIndexBuffers objectForKey:textureName];
        IntData* countBuffer = [theCountBuffers objectForKey:textureName];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        int primCount = [indexBuffer count];
        
        glMultiDrawArrays(GL_LINE_LOOP, indexBytes, countBytes, primCount);
    }
    
    if (color == NULL)
        glDisableClientState(GL_COLOR_ARRAY);
}

- (void)renderFaces:(BOOL)textured indexBuffers:(NSDictionary *)theIndexBuffers countBuffers:(NSDictionary *)theCountBuffers {
    glPolygonMode(GL_FRONT, GL_FILL);
    
    Grid* grid = [[windowController options] grid];
    if ([grid draw]) {
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        [grid activateTexture];
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glClientActiveTexture(GL_TEXTURE1);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)0);
    }
    
    glActiveTexture(GL_TEXTURE0);
    if (textured) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        glClientActiveTexture(GL_TEXTURE0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)TexCoordSize);
    } else {
        glDisable(GL_TEXTURE_2D);
    }
    
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)TexCoordSize + TexCoordSize + ColorSize);
    glVertexPointer(3, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize + ColorSize));
    
    NSEnumerator* textureNameEn = [theIndexBuffers keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        Texture* texture = [textureManager textureForName:textureName];
        if (textured) {
            if (texture != nil)
                [texture activate];
            else
                glDisable(GL_TEXTURE_2D);
        }
        
        IntData* indexBuffer = [theIndexBuffers objectForKey:textureName];
        IntData* countBuffer = [theCountBuffers objectForKey:textureName];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        int primCount = [indexBuffer count];
        
        glMultiDrawArrays(GL_POLYGON, indexBytes, countBytes, primCount);
        
        if (textured) {
            if (texture != nil)
                [texture deactivate];
            else
                glEnable(GL_TEXTURE_2D);
        }
    }
    
    if (textured) {
        glDisable(GL_TEXTURE_2D);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    
    if ([grid draw]) {
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
        glClientActiveTexture(GL_TEXTURE1);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        
        glActiveTexture(GL_TEXTURE0);
        glClientActiveTexture(GL_TEXTURE0);
    }
    
    glDisableClientState(GL_COLOR_ARRAY);
    glDisable(GL_POLYGON_OFFSET_FILL);
}

- (void)renderVertexHandles {
    TVector3f mid;
    
    SelectionManager* selectionManager = [windowController selectionManager];
    if ([selectionManager mode] == SM_BRUSHES) {
        if (vertexHandle == NULL) {
            vertexHandle = gluNewQuadric();
            gluQuadricDrawStyle(vertexHandle, GLU_FILL);
        }
        
        glFrontFace(GL_CCW);
        glPolygonMode(GL_FRONT, GL_FILL);
        glColorV4f(&SelectionColor);

        NSArray* brushes = [selectionManager selectedBrushes];
        NSEnumerator* brushEn = [brushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            const TVertexList* vertices = [brush vertices];
            for (int i = 0; i < vertices->count; i++) {
                TVector3f* vertex = &vertices->items[i]->position;
                glPushMatrix();
                glTranslatef(vertex->x, vertex->y, vertex->z);
                gluSphere(vertexHandle, 2, 12, 12);
                glPopMatrix();
            }
            
            const TEdgeList* edges = [brush edges];
            for (int i = 0; i < edges->count; i++) {
                TEdge* edge = edges->items[i];
                centerOfEdge(edge, &mid);

                glPushMatrix();
                glTranslatef(mid.x, mid.y, mid.z);
                gluSphere(vertexHandle, 1, 12, 12);
                glPopMatrix();
            }
            
            NSArray* faces = [brush faces];
            NSEnumerator* faceEn = [faces objectEnumerator];
            id <Face> face;
            while ((face = [faceEn nextObject])) {
                centerOfVertices([face vertices], &mid);
                glPushMatrix();
                glTranslatef(mid.x, mid.y, mid.z);
                gluSphere(vertexHandle, 1, 12, 12);
                glPopMatrix();
            }
        }

        glFrontFace(GL_CW);
    }
}

- (void)addEntities:(NSArray *)theEntities {
    [changeSet entitiesAdded:theEntities];
    
    NSMutableArray* brushes = [[NSMutableArray alloc] init];
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject]))
        [brushes addObjectsFromArray:[entity brushes]];
    
    [self addBrushes:brushes];
    [brushes release];
}

- (void)removeEntities:(NSArray *)theEntities {
    [changeSet entitiesRemoved:theEntities];
    
    NSMutableArray* brushes = [[NSMutableArray alloc] init];
    NSEnumerator* entityEn = [theEntities objectEnumerator];
    id <Entity> entity;
    while ((entity = [entityEn nextObject]))
        [brushes addObjectsFromArray:[entity brushes]];
    
    [self removeBrushes:brushes];
    [brushes release];
}

- (void)addBrushes:(NSArray *)theBrushes {
    [changeSet brushesAdded:theBrushes];
}

- (void)removeBrushes:(NSArray *)theBrushes {
    [changeSet brushesRemoved:theBrushes];
}

- (void)facesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* faces = [userInfo objectForKey:FacesKey];
    [changeSet facesChanged:faces];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)brushesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* brushes = [userInfo objectForKey:BrushesKey];
    [changeSet brushesChanged:brushes];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)brushesAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* brushes = [userInfo objectForKey:BrushesKey];
    [self addBrushes:brushes];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)brushesWillBeRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* brushes = [userInfo objectForKey:BrushesKey];
    [self removeBrushes:brushes];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)entitiesAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:EntitiesKey];
    [self addEntities:entities];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)entitiesWillBeRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:EntitiesKey];
    [self removeEntities:entities];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)propertiesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:EntitiesKey];
    [changeSet entitiesChanged:entities];

    id <Entity> worldspawn = [[windowController document] worldspawn:YES];
    if ([entities containsObject:worldspawn]) {
        NSArray* newMods = modListFromWorldspawn(worldspawn);
        if (![newMods isEqualToArray:mods]) {
            entityRendererCacheValid = NO;
            [mods release];
            mods = [newMods retain];
        }
    }
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)documentCleared:(NSNotification *)notification {
    [faceIndexBuffers removeAllObjects];
    [faceCountBuffers removeAllObjects];
    [selectedFaceIndexBuffers removeAllObjects];
    [selectedFaceCountBuffers removeAllObjects];
    [faceVbo freeAllBlocks];

    entityBoundsVertexCount = 0;
    [entityBoundsVbo freeAllBlocks];
    
    selectedEntityBoundsVertexCount = 0;
    [selectedEntityBoundsVbo freeAllBlocks];
    
    [classnameRenderer removeAllStrings];
    [selectedClassnameRenderer removeAllStrings];
    
    [modelEntities removeAllObjects];
    [selectedModelEntities removeAllObjects];
    [entityRenderers removeAllObjects];
    entityRendererCacheValid = YES;

    [feedbackFigures removeAllObjects];
    
    [changeSet clear];
}

- (void)documentLoaded:(NSNotification *)notification {
    MapDocument* map = [notification object];
    [self addEntities:[map entities]];
}

- (void)selectionAdded:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:SelectionEntities];
    NSArray* brushes = [userInfo objectForKey:SelectionBrushes];
    NSArray* faces = [userInfo objectForKey:SelectionFaces];
    
    if (entities != nil)
        [changeSet entitiesSelected:entities];
    
    if (brushes != nil)
        [changeSet brushesSelected:brushes];
    
    if (faces != nil)
        [changeSet facesSelected:faces];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)selectionRemoved:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    NSArray* entities = [userInfo objectForKey:SelectionEntities];
    NSArray* brushes = [userInfo objectForKey:SelectionBrushes];
    NSArray* faces = [userInfo objectForKey:SelectionFaces];
    
    if (entities != nil)
        [changeSet entitiesDeselected:entities];
    
    if (brushes != nil)
        [changeSet brushesDeselected:brushes];
    
    if (faces != nil)
        [changeSet facesDeselected:faces];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)textureManagerChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)cameraChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)optionsOrGroupsChanged:(NSNotification *)notification {
    Options* options = [windowController options];
    MapDocument* map = [windowController document];
    SelectionManager* selectionManager = [map selectionManager];
    GroupManager* groupManager = [map groupManager];
    
    [filter release];
    filter = [[DefaultFilter alloc] initWithSelectionManager:selectionManager groupManager:groupManager options:options];
    [changeSet setFilterChanged:YES];
    
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)gridChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)cursorChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)preferencesDidChange:(NSNotification *)notification {
    NSDictionary* userInfo = [notification userInfo];
    if (DefaultsQuakePath != [userInfo objectForKey:DefaultsKey])
        return;
    
    /*
     [entityLayer refreshRendererCache];
     [selectionLayer refreshRendererCache];
     */
}

@end

@implementation Renderer

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if ((self = [self init])) {
        windowController = theWindowController;
        
        changeSet = [[RenderChangeSet alloc] init];
        feedbackFigures = [[NSMutableArray alloc] init];
        
        MapDocument* map = [windowController document];
        GLResources* glResources = [map glResources];
        entityRendererManager = [glResources entityRendererManager];
        textureManager = [glResources textureManager];
        fontManager = [glResources fontManager];
        
        mods = [modListFromWorldspawn([map worldspawn:YES]) retain];
        entityRendererCacheValid = YES;
        
        faceVbo = [glResources vboForKey:FaceVboKey];
        entityBoundsVbo = [glResources vboForKey:EntityBoundsVboKey];
        selectedEntityBoundsVbo = [glResources vboForKey:SelectedEntityBoundsVboKey];
        faceIndexBuffers = [[NSMutableDictionary alloc] init];
        faceCountBuffers = [[NSMutableDictionary alloc] init];
        selectedFaceIndexBuffers = [[NSMutableDictionary alloc] init];
        selectedFaceCountBuffers = [[NSMutableDictionary alloc] init];
        classnameRenderer = [[TextRenderer alloc] initWithFontManager:fontManager camera:[windowController camera]];
        selectedClassnameRenderer = [[TextRenderer alloc] initWithFontManager:fontManager camera:[windowController camera]];
        entityRenderers = [[NSMutableDictionary alloc] init];
        modelEntities = [[NSMutableArray alloc] init];
        selectedModelEntities = [[NSMutableArray alloc] init];
        selectionBoundsRenderer = [[BoundsRenderer alloc] initWithCamera:[windowController camera] fontManager:fontManager];
        
        Camera* camera = [windowController camera];
        Options* options = [windowController options];
        Grid* grid = [options grid];
        SelectionManager* selectionManager = [map selectionManager];
        GroupManager* groupManager = [map groupManager];
        
        filter = [[DefaultFilter alloc] initWithSelectionManager:selectionManager groupManager:groupManager options:options];
        
        [self addEntities:[map entities]];
        
        NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
        
        [center addObserver:self selector:@selector(entitiesAdded:) name:EntitiesAdded object:map];
        [center addObserver:self selector:@selector(entitiesWillBeRemoved:) name:EntitiesWillBeRemoved object:map];
        [center addObserver:self selector:@selector(propertiesDidChange:) name:PropertiesDidChange object:map];
        [center addObserver:self selector:@selector(brushesAdded:) name:BrushesAdded object:map];
        [center addObserver:self selector:@selector(brushesWillBeRemoved:) name:BrushesWillBeRemoved object:map];
        [center addObserver:self selector:@selector(brushesDidChange:) name:BrushesDidChange object:map];
        [center addObserver:self selector:@selector(facesDidChange:) name:FacesDidChange object:map];
        [center addObserver:self selector:@selector(documentCleared:) name:DocumentCleared object:map];
        [center addObserver:self selector:@selector(documentLoaded:) name:DocumentLoaded object:map];
        
        [center addObserver:self selector:@selector(selectionAdded:) name:SelectionAdded object:selectionManager];
        [center addObserver:self selector:@selector(selectionRemoved:) name:SelectionRemoved object:selectionManager];
        
        [center addObserver:self selector:@selector(textureManagerChanged:) name:TextureManagerChanged object:textureManager];
        [center addObserver:self selector:@selector(cameraChanged:) name:CameraChanged object:camera];
        [center addObserver:self selector:@selector(optionsOrGroupsChanged:) name:OptionsChanged object:options];
        [center addObserver:self selector:@selector(optionsOrGroupsChanged:) name:GroupsChanged object:groupManager];
        [center addObserver:self selector:@selector(gridChanged:) name:GridChanged object:grid];
        
        CursorManager* cursorManager = [windowController cursorManager];
        [center addObserver:self selector:@selector(cursorChanged:) name:CursorChanged object:cursorManager];
        
        PreferencesManager* preferences = [PreferencesManager sharedManager];
        [center addObserver:self selector:@selector(preferencesDidChange:) name:DefaultsDidChange object:preferences];
    }
    
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [feedbackFigures release];
    [changeSet release];
    [faceIndexBuffers release];
    [faceCountBuffers release];
    [selectedFaceIndexBuffers release];
    [selectedFaceCountBuffers release];
    [classnameRenderer release];
    [selectedClassnameRenderer release];
    if (vertexHandle != NULL)
        gluDeleteQuadric(vertexHandle);
    [entityRenderers release];
    [modelEntities release];
    [selectedModelEntities release];
    [selectionBoundsRenderer release];
    [filter release];
    [mods release];
    [super dealloc];
}

- (void)addFeedbackFigure:(id <Figure>)theFigure {
    [feedbackFigures addObject:theFigure];
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)removeFeedbackFigure:(id <Figure>)theFigure {
    [feedbackFigures removeObjectIdenticalTo:theFigure];
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)render {
    [self validate];
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_FLAT);
    glResetEdgeOffset();
    
    Options* options = [windowController options];
    if ([options renderOrigin]) {
        glBegin(GL_LINES);
        glColor4f(1, 0, 0, 0.5f);
        glVertex3f(-64, 0, 0);
        glVertex3f(64, 0, 0);
        glColor4f(0, 1, 0, 0.5f);
        glVertex3f(0, -64, 0);
        glVertex3f(0, 64, 0);
        glColor4f(0, 0, 1, 0.5f);
        glVertex3f(0, 0, -64);
        glVertex3f(0, 0, 64);
        glEnd();
    }
    
    if ([options renderBrushes]) {
        [faceVbo activate];
        switch ([options renderMode]) {
            case RM_TEXTURED:
                if ([options isolationMode] == IM_NONE)
                    [self renderFaces:YES indexBuffers:faceIndexBuffers countBuffers:faceCountBuffers];
                [self renderFaces:YES indexBuffers:selectedFaceIndexBuffers countBuffers:selectedFaceCountBuffers];
                break;
            case RM_FLAT:
                if ([options isolationMode] == IM_NONE)
                    [self renderFaces:NO indexBuffers:faceIndexBuffers countBuffers:faceCountBuffers];
                [self renderFaces:NO indexBuffers:selectedFaceIndexBuffers countBuffers:selectedFaceCountBuffers];
                break;
            case RM_WIREFRAME:
                break;
        }
        
        
        glSetEdgeOffset(0.5f);
        [self renderEdges:NULL indexBuffers:faceIndexBuffers countBuffers:faceCountBuffers];
        glResetEdgeOffset();
        
        if ([[windowController selectionManager] hasSelection]) {
            glSetEdgeOffset(0.6f);
            glDisable(GL_DEPTH_TEST);
            [self renderEdges:&SelectionColor2 indexBuffers:selectedFaceIndexBuffers countBuffers:selectedFaceCountBuffers];
            glResetEdgeOffset();

            glSetEdgeOffset(0.7f);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            [self renderEdges:&SelectionColor indexBuffers:selectedFaceIndexBuffers countBuffers:selectedFaceCountBuffers];
            glDepthFunc(GL_LESS);
            glResetEdgeOffset();
        }
        
        [faceVbo deactivate];
    }
    
    if ([options renderEntities]) {
        if ([options isolationMode] == IM_NONE) {
            [entityBoundsVbo activate];
            [self renderEntityBounds:NULL vertexCount:entityBoundsVertexCount];
            [entityBoundsVbo deactivate];
            [self renderEntityModels:modelEntities];
            
            if ([options renderEntityClassnames]) {
                [fontManager activate];
                [classnameRenderer renderColor:&EntityClassnameColor];
                [fontManager deactivate];
            }
        } else if ([options isolationMode] == IM_WIREFRAME) {
            [entityBoundsVbo activate];
            [self renderEntityBounds:&EntityBoundsWireframeColor vertexCount:entityBoundsVertexCount];
            [entityBoundsVbo deactivate];
        }
        
        if ([[windowController selectionManager] hasSelection]) {
            [fontManager activate];
            [selectedClassnameRenderer renderColor:&SelectionColor];
            [fontManager deactivate];

            [selectedEntityBoundsVbo activate];
            glDisable(GL_DEPTH_TEST);
            [self renderEntityBounds:&SelectionColor2 vertexCount:selectedEntityBoundsVertexCount];
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            [self renderEntityBounds:&SelectionColor vertexCount:selectedEntityBoundsVertexCount];
            glDepthFunc(GL_LESS);
            [selectedEntityBoundsVbo deactivate];
            
            [self renderEntityModels:selectedModelEntities];
        }
    }
    
    if ([[windowController selectionManager] hasSelection]) {
        glDisable(GL_DEPTH_TEST);
        [selectionBoundsRenderer renderColor:&SelectionColor3];
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        [selectionBoundsRenderer renderColor:&SelectionColor];
        glDepthFunc(GL_LESS);
        
        [self renderVertexHandles];
    }
    
    if ([feedbackFigures count] > 0) {
        glDisable(GL_DEPTH_TEST);
        NSEnumerator* figureEn = [feedbackFigures objectEnumerator];
        id <Figure> figure;
        while ((figure = [figureEn nextObject]))
            [figure render];
        glEnable(GL_DEPTH_TEST);
    }
    
    /*
    // enable lighting for cursor and compass
    glEnable(GL_LIGHTING);
    glShadeModel(GL_SMOOTH);
    
    GLfloat globalAmbient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globalAmbient);
    
    Camera* camera = [windowController camera];
    
    glEnable(GL_LIGHT0);
    GLfloat ambientLight[] = { 0, 0, 0, 1.0f };
    GLfloat diffuseLight[] = { 1, 1, 1, 1 };
    GLfloat specularLight[] = { 1, 1, 1, 1 };
    GLfloat position[] = { [camera position]->x, [camera position]->y, [camera position]->z, 1.0f };
    
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specularLight);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    
    float specReflection[] = { 1, 1, 1, 1 };
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specReflection);
    glMateriali(GL_FRONT, GL_SHININESS, 50);
    */
    
    CursorManager* cursorManager = [windowController cursorManager];
    [cursorManager render];
    
    /*
    glDisable(GL_LIGHT0);
    glDisable(GL_COLOR_MATERIAL);
    glDisable(GL_LIGHTING);
     */
    
    // brightness
    /*
     float brightness = 0.5f;
     if( brightness > 1 )
     {
     glBlendFunc( GL_DST_COLOR, GL_ONE );
     glColor3f( brightness-1, brightness-1, brightness-1 );
     }
     else
     {
     glBlendFunc( GL_ZERO, GL_SRC_COLOR );
     glColor3f( brightness, brightness, brightness );
     }
     glEnable( GL_BLEND );
     */
}

@end
