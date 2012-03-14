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
#import "EntityClassnameAnchor.h"
#import "GroupManager.h"
#import "TextureIndexBuffer.h"
#import "BoundsRenderer.h"

NSString* const RendererChanged = @"RendererChanged";
NSString* const FaceVboKey = @"FaceVbo";
NSString* const EntityBoundsVboKey = @"EntityBoundsVbo";
NSString* const SelectedEntityBoundsVboKey = @"SelectedEntityBoundsVbo";
TVector4f const EntityBoundsDefaultColor = {0.5f, 0.5f, 0.5f, 1};
TVector4f const EntityBoundsWireframeColor = {0.5f, 0.5f, 0.5f, 0.6f};
TVector4f const EntityClassnameColor = {1, 1, 1, 1};
TVector4f const EdgeDefaultColor = {0.6f, 0.6f, 0.6f, 0.6f};
TVector4f const FaceDefaultColor = {0.2f, 0.2f, 0.2f, 1};
TVector4f const SelectionColor = {1, 0, 0, 1};
TVector4f const SelectionColor2 = {1, 0, 0, 0.35f};
TVector4f const SelectionColor3 = {1, 0, 0, 0.6f};
int const VertexSize = 3 * sizeof(float);
int const ColorSize = 4;
int const TexCoordSize = 2 * sizeof(float);

void initIndexBuffer(TIndexBuffer* buffer, int capacity) {
    buffer->capacity = capacity;
    buffer->items = malloc(buffer->capacity * sizeof(GLuint));
    buffer->count = 0;
}

void freeIndexBuffer(TIndexBuffer* buffer) {
    free(buffer->items);
    buffer->capacity = -1;
    buffer->count = -1;
}

void clearIndexBuffer(TIndexBuffer* buffer) {
    buffer->count = 0;
}

void addIndex(TIndexBuffer* buffer, GLuint index) {
    if (buffer->capacity == buffer->count) {
        buffer->capacity *= 2;
        buffer->items = realloc(buffer->items, buffer->capacity * sizeof(GLuint));
    }
    
    buffer->items[buffer->count++] = index;
}

void writeEntityBounds(id <Entity> entity, VboBlock* block) {
    TVector3f t;
    const TBoundingBox* bounds = [entity bounds];
    EntityDefinition* definition = [entity entityDefinition];
    TVector4f color = definition != nil ? *[definition color] : EntityBoundsDefaultColor;
    color.w = EntityBoundsDefaultColor.w;
    
    int address = block->address;
    uint8_t* vboBuffer = block->vbo->buffer;
    
    // east face
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
    
    // front face
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
    
    // bottom face
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
    
    // west face
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
    
    // back face
    t = bounds->max;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.x = bounds->min.x;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.z = bounds->min.z;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    t.x = bounds->max.x;
    address = writeColor4fAsBytes(&color, vboBuffer, address);
    address = writeVector3f(&t, vboBuffer, address);
    
    // top face
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
}

void writeFaceVertices(id <Face> face, VboBlock* block) {
    TVector2f texCoords, gridCoords;
    
    Texture* texture = [face texture];
    const TVector4f* color = [texture dummy] ? &FaceDefaultColor : [texture color];
    int width = texture != nil ? [texture width] : 1;
    int height = texture != nil ? [texture height] : 1;
    
    int address = block->address;
    uint8_t* vboBuffer = block->vbo->buffer;
    
    const TVertexList* vertices = [face vertices];
    for (int i = 0; i < vertices->count; i++) {
        TVertex* vertex = vertices->items[i];
        [face gridCoords:&gridCoords forVertex:&vertex->position];
        [face texCoords:&texCoords forVertex:&vertex->position];
        texCoords.x /= width;
        texCoords.y /= height;
        
        address = writeVector2f(&gridCoords, vboBuffer, address);
        address = writeVector2f(&texCoords, vboBuffer, address);
        address = writeColor4fAsBytes(&EdgeDefaultColor, vboBuffer, address);
        address = writeColor4fAsBytes(color, vboBuffer, address);
        address = writeVector3f(&vertex->position, vboBuffer, address);
    }
}

void writeFaceIndices(id <Face> face, TIndexBuffer* triangleBuffer, TIndexBuffer* edgeBuffer) {
    int baseIndex = [face vboBlock]->address / (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize);
    int vertexCount = [face vertices]->count;
    
    addIndex(edgeBuffer, baseIndex);
    addIndex(edgeBuffer, baseIndex + 1);
    
    for (int i = 1; i < vertexCount - 1; i++) {
        addIndex(triangleBuffer, baseIndex);
        addIndex(triangleBuffer, baseIndex + i);
        addIndex(triangleBuffer, baseIndex + i + 1);
        
        addIndex(edgeBuffer, baseIndex + i);
        addIndex(edgeBuffer, baseIndex + i + 1);
    }
    
    addIndex(edgeBuffer, baseIndex + vertexCount - 1);
    addIndex(edgeBuffer, baseIndex);
}

@interface Renderer (private)

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
- (void)rebuildSelectedFaceIndexBuffers;
- (void)validate;

- (void)renderEntityModels:(NSArray *)theEntities;
- (void)renderEntityBounds:(const TVector4f *)color vertexCount:(int)theVertexCount;
- (void)renderEdges:(const TVector4f *)color indexBuffer:(const TIndexBuffer *)theIndexBuffer;
- (void)renderFaces:(BOOL)textured selected:(BOOL)selected indexBuffers:(NSDictionary *)theIndexBuffers;

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

- (void)validateEntityRendererCache {
    if (!entityRendererCacheValid) {
        [entityRenderers removeAllObjects];

        NSArray* modelEntitiesCopy = [modelEntities copy];
        for (id <Entity> entity in modelEntitiesCopy) {
            id <EntityRenderer> renderer = [entityRendererManager entityRendererForEntity:entity mods:mods];
            if (renderer != nil)
                [entityRenderers setObject:renderer forKey:[entity entityId]];
            else
                [modelEntities removeObject:entity];
        }

        NSArray* selectedModelEntitiesCopy = [selectedModelEntities copy];
        for (id <Entity> entity in selectedModelEntitiesCopy) {
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
        activateVbo(&entityBoundsVbo);
        mapVbo(&entityBoundsVbo);
        
        for (id <Entity> entity in deselectedEntities) {
            if (![entity isWorldspawn] && ![entity isGroup]) {
                VboBlock* block = allocVboBlock(&entityBoundsVbo, 6 * 4 * (ColorSize + VertexSize));
                writeEntityBounds(entity, block);
                [entity setBoundsVboBlock:block];
                
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
        
        unmapVbo(&entityBoundsVbo);
        deactivateVbo(&entityBoundsVbo);

        activateVbo(&selectedEntityBoundsVbo);
        mapVbo(&selectedEntityBoundsVbo);
        packVbo(&selectedEntityBoundsVbo);
        unmapVbo(&selectedEntityBoundsVbo);
        deactivateVbo(&selectedEntityBoundsVbo);
    }
}

- (void)validateSelection {
    NSArray* selectedEntities = [changeSet selectedEntities];
    if ([selectedEntities count] > 0) {
        activateVbo(&selectedEntityBoundsVbo);
        mapVbo(&selectedEntityBoundsVbo);
        
        for (id <Entity> entity in selectedEntities) {
            if (![entity isWorldspawn] && ![entity isGroup]) {
                VboBlock* block = allocVboBlock(&selectedEntityBoundsVbo, 6 * 4 * (ColorSize + VertexSize));
                writeEntityBounds(entity, block);
                [entity setBoundsVboBlock:block];
                
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
        
        unmapVbo(&selectedEntityBoundsVbo);
        deactivateVbo(&selectedEntityBoundsVbo);
        
        activateVbo(&entityBoundsVbo);
        mapVbo(&entityBoundsVbo);
        packVbo(&entityBoundsVbo);
        unmapVbo(&entityBoundsVbo);
        deactivateVbo(&entityBoundsVbo);
    }
}

- (void)validateAddedEntities {
    NSArray* addedEntities = [changeSet addedEntities];
    if ([addedEntities count] > 0) {
        activateVbo(&entityBoundsVbo);
        mapVbo(&entityBoundsVbo);

        for (id <Entity> entity in addedEntities) {
            if (![entity isWorldspawn] && ![entity isGroup]) {
                VboBlock* block = allocVboBlock(&entityBoundsVbo, 6 * 4 * (ColorSize + VertexSize));
                writeEntityBounds(entity, block);
                [entity setBoundsVboBlock:block];
                
                id <EntityRenderer> renderer = [entityRendererManager entityRendererForEntity:entity mods:mods];
                if (renderer != nil) {
                    [entityRenderers setObject:renderer forKey:[entity entityId]];
                    [modelEntities addObject:entity];
                }
                
                entityBoundsVertexCount += 6 * 4;
            }
        }

        unmapVbo(&entityBoundsVbo);
        deactivateVbo(&entityBoundsVbo);
        
        for (id <Entity> entity in addedEntities) {
            if (![entity isWorldspawn] && ![entity isGroup]) {
                NSString* classname = [entity classname];
                EntityClassnameAnchor* anchor = [[EntityClassnameAnchor alloc] initWithEntity:entity];
                [classnameRenderer addString:classname forKey:[entity entityId] withFont:[NSFont systemFontOfSize:9] withAnchor:anchor];
                [anchor release];
            }
        }
    }
}

- (void)validateChangedEntities {
    NSArray* changedEntities = [changeSet changedEntities];
    if ([changedEntities count] > 0) {
        activateVbo(&selectedEntityBoundsVbo);
        mapVbo(&selectedEntityBoundsVbo);

        NSMutableArray* unselectedEntities = [[NSMutableArray alloc] init];
        for (id <Entity> entity in changedEntities) {
            if (![entity isWorldspawn] && ![entity isGroup]) {
                VboBlock* block = [entity boundsVboBlock];
                if (block->vbo == &entityBoundsVbo)
                    [unselectedEntities addObject:entity];
                else
                    writeEntityBounds(entity, block);
                
            }
        }
        
        unmapVbo(&selectedEntityBoundsVbo);
        deactivateVbo(&selectedEntityBoundsVbo);
        
        if ([unselectedEntities count] > 0) {
            activateVbo(&entityBoundsVbo);
            mapVbo(&entityBoundsVbo);
            
            for (id <Entity> entity in unselectedEntities) {
                VboBlock* block = [entity boundsVboBlock];
                writeEntityBounds(entity, block);
            }
            
            unmapVbo(&entityBoundsVbo);
            deactivateVbo(&entityBoundsVbo);
        }
        
        [unselectedEntities release];
    }
}

- (void)validateRemovedEntities {
    NSArray* removedEntities = [changeSet removedEntities];
    if ([removedEntities count] > 0) {
        activateVbo(&entityBoundsVbo);
        mapVbo(&entityBoundsVbo);
        
        for (id <Entity> entity in removedEntities) {
            if (![entity isWorldspawn] && ![entity isGroup]) {
                [entity setBoundsVboBlock:NULL];
                [entityRenderers removeObjectForKey:[entity entityId]];
                [modelEntities removeObjectIdenticalTo:entity];
            }
        }

        packVbo(&entityBoundsVbo);
        entityBoundsVertexCount -= 6 * 4 * [removedEntities count];

        unmapVbo(&entityBoundsVbo);
        deactivateVbo(&entityBoundsVbo);

        [fontManager activate];
        for (id <Entity> entity in removedEntities)
            if (![entity isWorldspawn] && ![entity isGroup])
                [classnameRenderer removeStringForKey:[entity entityId]];
        [fontManager deactivate];
    }
}

- (void)validateAddedBrushes {
    NSArray* addedBrushes = [changeSet addedBrushes];
    if ([addedBrushes count] > 0) {
        activateVbo(&faceVbo);
        mapVbo(&faceVbo);

        for (id <Brush> brush in addedBrushes) {
            for (id <Face> face in [brush faces]) {
                VboBlock* block = allocVboBlock(&faceVbo, [face vertices]->count * (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize));
                writeFaceVertices(face, block);
                [face setVboBlock:block];
            }
        }

        unmapVbo(&faceVbo);
        deactivateVbo(&faceVbo);
    }
}

- (void)validateChangedBrushes {
    NSArray* changedBrushes = [changeSet changedBrushes];
    if ([changedBrushes count] > 0) {
        activateVbo(&faceVbo);
        mapVbo(&faceVbo);
        
        for (id <Brush> brush in changedBrushes) {
            for (id <Face> face in [brush faces]) {
                int blockSize = [face vertices]->count * (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize);
                VboBlock* block = [face vboBlock];
                if (block == NULL || block->capacity != blockSize) {
                    block = allocVboBlock(&faceVbo, blockSize);
                    [face setVboBlock:block];
                }
                
                writeFaceVertices(face, block);
            }
        }

        unmapVbo(&faceVbo);
        deactivateVbo(&faceVbo);
    }
}

- (void)validateRemovedBrushes {
    NSArray* removedBrushes = [changeSet removedBrushes];
    if ([removedBrushes count] > 0) {
        activateVbo(&faceVbo);
        mapVbo(&faceVbo);
        
        for (id <Brush> brush in removedBrushes)
            for (id <Face> face in [brush faces])
                [face setVboBlock:NULL];
        
        unmapVbo(&faceVbo);
        deactivateVbo(&faceVbo);
    }
}


- (void)validateChangedFaces {
    NSArray* changedFaces = [changeSet changedFaces];
    if ([changedFaces count] > 0) {
        activateVbo(&faceVbo);
        mapVbo(&faceVbo);
        
        for (id <Face> face in changedFaces) {
            int blockSize = [face vertices]->count * (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize);
            VboBlock* block = [face vboBlock];
            if (block->capacity != blockSize) {
                block = allocVboBlock(&faceVbo, blockSize);
                [face setVboBlock:block];
            }
            
            writeFaceVertices(face, block);
        }
        
        unmapVbo(&faceVbo);
        deactivateVbo(&faceVbo);
    }
}

- (void)rebuildFaceIndexBuffers {
    NSArray* textureIndexBuffers = [faceIndexBuffers allValues];
    [faceIndexBuffers removeAllObjects];
    clearIndexBuffer(&edgeIndexBuffer);
    
    NSEnumerator* textureIndexBufferEn = [textureIndexBuffers objectEnumerator];
    TextureIndexBuffer* textureIndexBuffer;

    MapDocument* map = [windowController document];
    for (id <Entity> entity in [map entities]) {
        if ([filter entityRenderable:entity]) {
            for (id <Brush> brush in [entity brushes]) {
                if ([filter brushRenderable:brush]) {
                    for (id <Face> face in [brush faces]) {
                        if (![face selected]) {
                            Texture* texture = [face texture];
                            NSString* textureName = [texture name];
                            textureIndexBuffer = [faceIndexBuffers objectForKey:textureName];
                            TIndexBuffer* indexBuffer = NULL;
                            if (textureIndexBuffer == nil) {
                                textureIndexBuffer = [textureIndexBufferEn nextObject];
                                if (textureIndexBuffer == nil) {
                                    textureIndexBuffer = [[TextureIndexBuffer alloc] init];
                                    [faceIndexBuffers setObject:textureIndexBuffer forKey:textureName];
                                    [textureIndexBuffer release];
                                    indexBuffer = [textureIndexBuffer buffer];
                                } else {
                                    [faceIndexBuffers setObject:textureIndexBuffer forKey:textureName];
                                    indexBuffer = [textureIndexBuffer buffer];
                                    clearIndexBuffer(indexBuffer);
                                }
                                
                                [textureIndexBuffer setTexture:texture];
                            } else {
                                indexBuffer = [textureIndexBuffer buffer];
                            }
                            
                            writeFaceIndices(face, indexBuffer, &edgeIndexBuffer);
                        }
                    }
                }
            }
        }
    }
}

- (void)rebuildSelectedFaceIndexBuffers {
    NSArray* textureIndexBuffers = [selectedFaceIndexBuffers allValues];
    [selectedFaceIndexBuffers removeAllObjects];
    clearIndexBuffer(&selectedEdgeIndexBuffer);
    
    MapDocument* map = [windowController document];
    SelectionManager* selectionManager = [map selectionManager];
    
    NSEnumerator* textureIndexBufferEn = [textureIndexBuffers objectEnumerator];
    TextureIndexBuffer* textureIndexBuffer;

    for (id <Brush> brush in [selectionManager selectedBrushes]) {
        for (id <Face> face in [brush faces]) {
            Texture* texture = [face texture];
            NSString* textureName = [texture name];
            textureIndexBuffer = [selectedFaceIndexBuffers objectForKey:textureName];
            TIndexBuffer* indexBuffer = NULL;
            if (textureIndexBuffer == nil) {
                textureIndexBuffer = [textureIndexBufferEn nextObject];
                if (textureIndexBuffer == nil) {
                    textureIndexBuffer = [[TextureIndexBuffer alloc] init];
                    [selectedFaceIndexBuffers setObject:textureIndexBuffer forKey:textureName];
                    [textureIndexBuffer release];
                    indexBuffer = [textureIndexBuffer buffer];
                } else {
                    [selectedFaceIndexBuffers setObject:textureIndexBuffer forKey:textureName];
                    indexBuffer = [textureIndexBuffer buffer];
                    clearIndexBuffer(indexBuffer);
                }

                [textureIndexBuffer setTexture:texture];
            } else {
                indexBuffer = [textureIndexBuffer buffer];
            }

            writeFaceIndices(face, indexBuffer, &selectedEdgeIndexBuffer);
        }
    }
    
    for (id <Face> face in [selectionManager selectedFaces]) {
        Texture* texture = [face texture];
        NSString* textureName = [texture name];
        textureIndexBuffer = [selectedFaceIndexBuffers objectForKey:textureName];
        TIndexBuffer* indexBuffer = NULL;
        if (textureIndexBuffer == nil) {
            textureIndexBuffer = [textureIndexBufferEn nextObject];
            if (textureIndexBuffer == nil) {
                textureIndexBuffer = [[TextureIndexBuffer alloc] init];
                [selectedFaceIndexBuffers setObject:textureIndexBuffer forKey:textureName];
                [textureIndexBuffer release];
                indexBuffer = [textureIndexBuffer buffer];
            } else {
                [selectedFaceIndexBuffers setObject:textureIndexBuffer forKey:textureName];
                indexBuffer = [textureIndexBuffer buffer];
                clearIndexBuffer(indexBuffer);
            }

            [textureIndexBuffer setTexture:texture];
        } else {
            indexBuffer = [textureIndexBuffer buffer];
        }

         writeFaceIndices(face, indexBuffer, &selectedEdgeIndexBuffer);
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
        [[changeSet removedBrushes] count] > 0 ||
        [[changeSet selectedBrushes] count] > 0 ||
        [[changeSet deselectedBrushes] count] > 0 ||
        [[changeSet selectedFaces] count] > 0 ||
        [[changeSet deselectedFaces] count] > 0 ||
        [changeSet filterChanged] ||
        [changeSet textureManagerChanged]) {
        
        [self rebuildFaceIndexBuffers];
    }
    
    if ([[changeSet changedBrushes] count] > 0 ||
        [[changeSet changedFaces] count] > 0 ||
        [[changeSet selectedBrushes] count] > 0 ||
        [[changeSet deselectedBrushes] count] > 0 ||
        [[changeSet selectedFaces] count] > 0 ||
        [[changeSet deselectedFaces] count] > 0 ||
        [changeSet filterChanged] ||
        [changeSet textureManagerChanged]) {
        
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
    for (id <Entity> entity in theEntities) {
        if (filter == nil || [filter entityRenderable:entity]) {
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
    
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    if (color != NULL) {
        glColorV4f(color);
        glVertexPointer(3, GL_FLOAT, ColorSize + VertexSize, (const GLvoid *)(long)ColorSize);
    } else {
        glInterleavedArrays(GL_C4UB_V3F, 0, 0);
    }

    glDrawArrays(GL_QUADS, 0, theVertexCount);

    glPopClientAttrib();
    glResetEdgeOffset();
}

- (void)renderEdges:(const TVector4f *)color indexBuffer:(const TIndexBuffer *)theIndexBuffer {
    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    
    if (color != NULL) {
        glColorV4f(color);
        glVertexPointer(3, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize + ColorSize));
    } else {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize));
        glVertexPointer(3, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize + ColorSize));
    }
    
    glDrawElements(GL_LINES, theIndexBuffer->count, GL_UNSIGNED_INT, theIndexBuffer->items);
    
    glPopClientAttrib();
}
- (void)renderFaces:(BOOL)textured selected:(BOOL)selected indexBuffers:(NSDictionary *)theIndexBuffers {
    glPolygonMode(GL_FRONT, GL_FILL);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    
    Grid* grid = [[windowController options] grid];
    if ([grid draw]) {
        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_2D);
        [grid activateTexture];
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glClientActiveTexture(GL_TEXTURE2);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)0);
    }
    
    if (selected) {
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        [grid activateTexture]; // just a dummy
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        float color[3] = {0.6f, 0.35f, 0.35f};
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
        glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2);
    }
        
    glActiveTexture(GL_TEXTURE0);
    if (textured) {
        glEnable(GL_TEXTURE_2D);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        
        PreferencesManager* preferences = [PreferencesManager sharedManager];
        float brightness = [preferences brightness];
        float color[3] = {brightness / 2, brightness / 2, brightness / 2};
        
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, color);

        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
        
        glTexEnvf(GL_TEXTURE_ENV, GL_RGB_SCALE, 2.0f);
        
        glClientActiveTexture(GL_TEXTURE0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)TexCoordSize);
    } else {
        glDisable(GL_TEXTURE_2D);
    }
    
    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(4, GL_UNSIGNED_BYTE, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)TexCoordSize + TexCoordSize + ColorSize);
    glVertexPointer(3, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize + ColorSize));
    
    for (TextureIndexBuffer* textureIndexBuffer in [theIndexBuffers objectEnumerator]) {
        Texture* texture = [textureIndexBuffer texture];
        if (textured) {
            if (texture != nil)
                [texture activate];
            else
                glDisable(GL_TEXTURE_2D);
        }
        
        TIndexBuffer* indexBuffer = [textureIndexBuffer buffer];
        glDrawElements(GL_TRIANGLES, indexBuffer->count, GL_UNSIGNED_INT, indexBuffer->items);
        
        if (textured) {
            if (texture != nil)
                [texture deactivate];
            else
                glEnable(GL_TEXTURE_2D);
        }
    }
    
    if (textured)
        glDisable(GL_TEXTURE_2D);
    
    if (selected) {
        glActiveTexture(GL_TEXTURE1);
        glDisable(GL_TEXTURE_2D);
    }
    
    if ([grid draw]) {
        glActiveTexture(GL_TEXTURE2);
        [grid deactivateTexture];
        glDisable(GL_TEXTURE_2D);
        glActiveTexture(GL_TEXTURE0);
    }

    glPopClientAttrib();
}

- (void)addEntities:(NSArray *)theEntities {
    [changeSet entitiesAdded:theEntities];
    
    NSMutableArray* brushes = [[NSMutableArray alloc] init];
    for (id <Entity> entity in theEntities)
        [brushes addObjectsFromArray:[entity brushes]];
    
    [self addBrushes:brushes];
    [brushes release];
}

- (void)removeEntities:(NSArray *)theEntities {
    [changeSet entitiesRemoved:theEntities];
    
    NSMutableArray* brushes = [[NSMutableArray alloc] init];
    for (id <Entity> entity in theEntities)
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
    
    NSMutableSet* entities = [[NSMutableSet alloc] init];
    for (id <Brush> brush in brushes) {
        id <Entity> entity = [brush entity];
        if (![entity isWorldspawn] && [[entity entityDefinition] type] == EDT_BRUSH)
            [entities addObject:entity];
    }
    
    [changeSet entitiesChanged:[entities allObjects]];
    [entities release];
    
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
    [selectedFaceIndexBuffers removeAllObjects];
    freeAllVboBlocks(&faceVbo);

    entityBoundsVertexCount = 0;
    freeAllVboBlocks(&entityBoundsVbo);
    
    selectedEntityBoundsVertexCount = 0;
    freeAllVboBlocks(&selectedEntityBoundsVbo);
    
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
    [changeSet setTextureManagerChanged:YES];
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)cameraChanged:(NSNotification *)notification {
    [[NSNotificationCenter defaultCenter] postNotificationName:RendererChanged object:self];
}

- (void)optionsOrGroupsChanged:(NSNotification *)notification {
    Options* options = [windowController options];
    MapDocument* map = [windowController document];
    SelectionManager* selectionManager = [map selectionManager];
    Camera* camera = [windowController camera];
    GroupManager* groupManager = [map groupManager];
    
    [filter release];
    filter = [[DefaultFilter alloc] initWithSelectionManager:selectionManager groupManager:groupManager camera:camera options:options];
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
         // retain so that it only gets released after we release the entity renderer cache
        entityRendererManager = [[glResources entityRendererManager] retain];
        textureManager = [glResources textureManager];
        fontManager = [glResources fontManager];
        
        mods = [modListFromWorldspawn([map worldspawn:YES]) retain];
        entityRendererCacheValid = YES;
        
        initVbo(&faceVbo, GL_ARRAY_BUFFER, 0xFFFF);
        initVbo(&entityBoundsVbo, GL_ARRAY_BUFFER, 0xFFFF);
        initVbo(&selectedEntityBoundsVbo, GL_ARRAY_BUFFER, 0xFFFF);
        
        initIndexBuffer(&edgeIndexBuffer, 0xFFF);
        initIndexBuffer(&selectedEdgeIndexBuffer, 0xFFF);
        
        faceIndexBuffers = [[NSMutableDictionary alloc] init];
        selectedFaceIndexBuffers = [[NSMutableDictionary alloc] init];
        classnameRenderer = [[TextRenderer alloc] initWithFontManager:fontManager camera:[windowController camera] fadeDistance:400];
        selectedClassnameRenderer = [[TextRenderer alloc] initWithFontManager:fontManager camera:[windowController camera] fadeDistance:2000];
        entityRenderers = [[NSMutableDictionary alloc] init];
        modelEntities = [[NSMutableArray alloc] init];
        selectedModelEntities = [[NSMutableArray alloc] init];

        Camera* camera = [windowController camera];
        Options* options = [windowController options];
        Grid* grid = [options grid];
        SelectionManager* selectionManager = [map selectionManager];
        GroupManager* groupManager = [map groupManager];
        
        selectionBoundsRenderer = [[BoundsRenderer alloc] initWithCamera:camera fontManager:fontManager];
                                   
        filter = [[DefaultFilter alloc] initWithSelectionManager:selectionManager groupManager:groupManager camera:camera options:options];
        
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
    [selectedFaceIndexBuffers release];
    freeIndexBuffer(&edgeIndexBuffer);
    freeIndexBuffer(&selectedEdgeIndexBuffer);
    [classnameRenderer release];
    [selectedClassnameRenderer release];
    [entityRenderers release];
    [modelEntities release];
    [selectedModelEntities release];
    [entityRendererManager release];
    [filter release];
    [mods release];
    freeVbo(&faceVbo);
    freeVbo(&entityBoundsVbo);
    freeVbo(&selectedEntityBoundsVbo);
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

- (void)updateFeedbackFigure:(id <Figure>)theFigure {
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
        activateVbo(&faceVbo);
        glEnableClientState(GL_VERTEX_ARRAY);
        
        switch ([options renderMode]) {
            case RM_TEXTURED:
                if ([options isolationMode] == IM_NONE)
                    [self renderFaces:YES selected:NO indexBuffers:faceIndexBuffers];
                [self renderFaces:YES selected:YES indexBuffers:selectedFaceIndexBuffers];
                break;
            case RM_FLAT:
                if ([options isolationMode] == IM_NONE)
                    [self renderFaces:NO selected:NO indexBuffers:faceIndexBuffers];
                [self renderFaces:NO selected:YES indexBuffers:selectedFaceIndexBuffers];
                break;
            case RM_WIREFRAME:
                break;
        }
        
        if ([options isolationMode] != IM_DISCARD) {
            glSetEdgeOffset(0.1f);
            [self renderEdges:NULL indexBuffer:&edgeIndexBuffer];
            glResetEdgeOffset();
        }
        
        if ([[windowController selectionManager] hasSelection]) {
            glDisable(GL_DEPTH_TEST);
            [self renderEdges:&SelectionColor2 indexBuffer:&selectedEdgeIndexBuffer];
            
            glSetEdgeOffset(0.2f);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            [self renderEdges:&SelectionColor indexBuffer:&selectedEdgeIndexBuffer];
            glDepthFunc(GL_LESS);
            glResetEdgeOffset();
        }
        
        glDisableClientState(GL_VERTEX_ARRAY);
        deactivateVbo(&faceVbo);
    }

    if ([options renderEntities]) {
        if ([options isolationMode] == IM_NONE) {
            activateVbo(&entityBoundsVbo);
            glEnableClientState(GL_VERTEX_ARRAY);
            [self renderEntityBounds:NULL vertexCount:entityBoundsVertexCount];
            glDisableClientState(GL_VERTEX_ARRAY);
            deactivateVbo(&entityBoundsVbo);

            [self renderEntityModels:modelEntities];

            if ([options renderEntityClassnames]) {
                [fontManager activate];
                [classnameRenderer renderColor:&EntityClassnameColor];
                [fontManager deactivate];
            }
            
        } else if ([options isolationMode] == IM_WIREFRAME) {
            activateVbo(&entityBoundsVbo);
            glEnableClientState(GL_VERTEX_ARRAY);
            [self renderEntityBounds:&EntityBoundsWireframeColor vertexCount:entityBoundsVertexCount];
            glDisableClientState(GL_VERTEX_ARRAY);
            deactivateVbo(&entityBoundsVbo);
        }
        
        if ([[windowController selectionManager] hasSelection]) {
            [fontManager activate];
            [selectedClassnameRenderer renderColor:&SelectionColor];
            [fontManager deactivate];

            activateVbo(&selectedEntityBoundsVbo);
            glEnableClientState(GL_VERTEX_ARRAY);
            
            glDisable(GL_CULL_FACE);
            glDisable(GL_DEPTH_TEST);
            [self renderEntityBounds:&SelectionColor2 vertexCount:selectedEntityBoundsVertexCount];
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            [self renderEntityBounds:&SelectionColor vertexCount:selectedEntityBoundsVertexCount];
            glDepthFunc(GL_LESS);
            glEnable(GL_CULL_FACE);
            
            glDisableClientState(GL_VERTEX_ARRAY);
            deactivateVbo(&selectedEntityBoundsVbo);
            
            [self renderEntityModels:selectedModelEntities];
        }
    }
    
    if ([options renderSizeGuides] && [[windowController selectionManager] hasSelection]) {
        glDisable(GL_DEPTH_TEST);
        [selectionBoundsRenderer renderColor:&SelectionColor];
    }
    
    if ([feedbackFigures count] > 0) {
        glDisable(GL_DEPTH_TEST);
        for (id <Figure> figure in feedbackFigures)
            [figure render:filter];
        glEnable(GL_DEPTH_TEST);
    }
}

@end
