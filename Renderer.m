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

NSString* const RendererChanged = @"RendererChanged";
NSString* const FaceVboKey = @"FaceVbo";
NSString* const EntityBoundsVboKey = @"EntityBoundsVbo";
NSString* const SelectedEntityBoundsVboKey = @"SelectedEntityBoundsVbo";
TVector4f const EntityBoundsDefaultColor = {0.5f, 0.5f, 0.5f, 1};
TVector4f const EntityBoundsWireframeColor = {0.5f, 0.5f, 0.5f, 0.6f};
TVector4f const EntityClassnameColor = {0.5f, 0.5f, 0.5f, 1};
TVector4f const EdgeDefaultColor = {0.4f, 0.4f, 0.4f, 0.4f};
TVector4f const FaceDefaultColor = {0.2f, 0.2f, 0.2f, 1};
TVector4f const SelectionColor = {1, 0, 0, 1};
TVector4f const SelectionColor2 = {1, 0, 0, 0.2f};
TVector4f const SelectionColor3 = {1, 0, 0, 0.5f};
int const VertexSize = 3 * sizeof(float);
int const ColorSize = 4;
int const TexCoordSize = 2 * sizeof(float);

@interface Renderer (private)

- (void)writeEntityBounds:(id <Entity>)theEntity toBlock:(VboBlock *)theBlock;
- (void)writeFaceVertices:(id <Face>)theFace toBlock:(VboBlock *)theBlock;
- (void)writeFaceTriangles:(id <Face>)theFace toIndexBuffer:(IntData *)theIndexBuffer;
- (void)writeFaceEdges:(id <Face>)theFace toIndexBuffer:(IntData *)theIndexBuffer;

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
- (void)renderEdges:(const TVector4f *)color indexBuffer:(IntData *)theIndexBuffer;
- (void)renderFaces:(BOOL)textured indexBuffers:(NSDictionary *)theIndexBuffers;
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

- (void)writeEntityBounds:(id <Entity>)theEntity toBlock:(VboBlock *)theBlock {
    TVector3f t;
    const TBoundingBox* bounds = [theEntity bounds];
    EntityDefinition* definition = [theEntity entityDefinition];
    TVector4f color = definition != nil ? *[definition color] : EntityBoundsDefaultColor;
    color.w = EntityBoundsDefaultColor.w;
    
    int address = theBlock->address;
    uint8_t* vboBuffer = theBlock->vbo->buffer;
    
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

- (void)writeFaceVertices:(id <Face>)theFace toBlock:(VboBlock *)theBlock {
    TVector2f texCoords, gridCoords;
    
    Texture* texture = [theFace texture];
    const TVector4f* color = [texture dummy] ? &FaceDefaultColor : [texture color];
    int width = texture != nil ? [texture width] : 1;
    int height = texture != nil ? [texture height] : 1;

    int address = theBlock->address;
    uint8_t* vboBuffer = theBlock->vbo->buffer;

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
        address = writeColor4fAsBytes(color, vboBuffer, address);
        address = writeVector3f(&vertex->position, vboBuffer, address);
    }
}

- (void)writeFaceTriangles:(id <Face>)theFace toIndexBuffer:(IntData *)theIndexBuffer {
    int baseIndex = [theFace vboBlock]->address / (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize);
    
    for (int i = 1; i < [theFace vertices]->count - 1; i++) {
        [theIndexBuffer appendInt:baseIndex];
        [theIndexBuffer appendInt:baseIndex + i];
        [theIndexBuffer appendInt:baseIndex + i + 1];
    }
}

- (void)writeFaceEdges:(id <Face>)theFace toIndexBuffer:(IntData *)theIndexBuffer {
    int baseIndex = [theFace vboBlock]->address / (TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize);
    int vertexCount = [theFace vertices]->count;
    
    for (int i = 0; i < vertexCount - 1; i++) {
        [theIndexBuffer appendInt:baseIndex + i];
        [theIndexBuffer appendInt:baseIndex + i + 1];
    }
    
    [theIndexBuffer appendInt:baseIndex + vertexCount - 1];
    [theIndexBuffer appendInt:baseIndex];
}

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
            if (![entity isWorldspawn]) {
                VboBlock* block = allocVboBlock(&entityBoundsVbo, 6 * 4 * (ColorSize + VertexSize));
                [self writeEntityBounds:entity toBlock:block];
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
            if (![entity isWorldspawn]) {
                VboBlock* block = allocVboBlock(&selectedEntityBoundsVbo, 6 * 4 * (ColorSize + VertexSize));
                [self writeEntityBounds:entity toBlock:block];
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
            if (![entity isWorldspawn]) {
                VboBlock* block = allocVboBlock(&entityBoundsVbo, 6 * 4 * (ColorSize + VertexSize));
                [self writeEntityBounds:entity toBlock:block];
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
            if (![entity isWorldspawn]) {
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
            if (![entity isWorldspawn]) {
                VboBlock* block = [entity boundsVboBlock];
                if (block->vbo == &entityBoundsVbo)
                    [unselectedEntities addObject:entity];
                else
                    [self writeEntityBounds:entity toBlock:block];
                
            }
        }
        
        unmapVbo(&selectedEntityBoundsVbo);
        deactivateVbo(&selectedEntityBoundsVbo);
        
        if ([unselectedEntities count] > 0) {
            activateVbo(&entityBoundsVbo);
            mapVbo(&entityBoundsVbo);
            
            for (id <Entity> entity in unselectedEntities) {
                VboBlock* block = [entity boundsVboBlock];
                [self writeEntityBounds:entity toBlock:block];
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
            if (![entity isWorldspawn]) {
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
            if (![entity isWorldspawn])
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
                [self writeFaceVertices:face toBlock:block];
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
                
                [self writeFaceVertices:face toBlock:block];
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
            
            [self writeFaceVertices:face toBlock:block];
        }
        
        unmapVbo(&faceVbo);
        deactivateVbo(&faceVbo);
    }
}

- (void)rebuildFaceIndexBuffers {
    [faceIndexBuffers removeAllObjects];
    [edgeIndexBuffer release];
    edgeIndexBuffer = [[IntData alloc] init];
    
    NSMutableArray* allFaces = [[NSMutableArray alloc] init];
    
    MapDocument* map = [windowController document];
    for (id <Entity> entity in [map entities])
        if ([filter entityRenderable:entity])
            for (id <Brush> brush in [entity brushes])
                if ([filter brushRenderable:brush])
                    [allFaces addObjectsFromArray:[brush faces]];
    
    SelectionManager* selectionManager = [map selectionManager];
    NSMutableArray* selectedFaces = [[NSMutableArray alloc] initWithArray:[selectionManager selectedFaces]];
    [selectedFaces addObjectsFromArray:[selectionManager selectedBrushFaces]];
    
    [allFaces removeObjectsInArray:selectedFaces];
    [selectedFaces release];
    
    for (id <Face> face in allFaces) {
        NSString* textureName = [[face texture] name];
        IntData* indexBuffer = [faceIndexBuffers objectForKey:textureName];
        if (indexBuffer == nil) {
            indexBuffer = [[IntData alloc] init];
            [faceIndexBuffers setObject:indexBuffer forKey:textureName];
            [indexBuffer release];
        }
        
        [self writeFaceTriangles:face toIndexBuffer:indexBuffer];
        [self writeFaceEdges:face toIndexBuffer:edgeIndexBuffer];
    }
    
    [allFaces release];
}

- (void)rebuildSelectedFaceIndexBuffers {
    [selectedFaceIndexBuffers removeAllObjects];
    [selectedEdgeIndexBuffer release];
    selectedEdgeIndexBuffer = [[IntData alloc] init];
    
    MapDocument* map = [windowController document];
    SelectionManager* selectionManager = [map selectionManager];
    
    for (id <Brush> brush in [selectionManager selectedBrushes]) {
        for (id <Face> face in [brush faces]) {
            NSString* textureName = [[face texture] name];
            IntData* indexBuffer = [selectedFaceIndexBuffers objectForKey:textureName];
            if (indexBuffer == nil) {
                indexBuffer = [[IntData alloc] init];
                [selectedFaceIndexBuffers setObject:indexBuffer forKey:textureName];
                [indexBuffer release];
            }
            
            [self writeFaceTriangles:face toIndexBuffer:indexBuffer];
            [self writeFaceEdges:face toIndexBuffer:selectedEdgeIndexBuffer];
        }
    }
    
    for (id <Face> face in [selectionManager selectedFaces]) {
        NSString* textureName = [[face texture] name];
        IntData* indexBuffer = [selectedFaceIndexBuffers objectForKey:textureName];
        if (indexBuffer == nil) {
            indexBuffer = [[IntData alloc] init];
            [selectedFaceIndexBuffers setObject:indexBuffer forKey:textureName];
            [indexBuffer release];
        }
        
        [self writeFaceTriangles:face toIndexBuffer:indexBuffer];
        [self writeFaceEdges:face toIndexBuffer:selectedEdgeIndexBuffer];
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
        [changeSet filterChanged]) {
        
        [self rebuildFaceIndexBuffers];
    }
    
    if ([[changeSet changedBrushes] count] > 0 ||
        [[changeSet changedFaces] count] > 0 ||
        [[changeSet selectedBrushes] count] > 0 ||
        [[changeSet deselectedBrushes] count] > 0 ||
        [[changeSet selectedFaces] count] > 0 ||
        [[changeSet deselectedFaces] count] > 0 ||
        [changeSet filterChanged]) {
        
        [self rebuildSelectedFaceIndexBuffers];
    }
    
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

- (void)renderEdges:(const TVector4f *)color indexBuffer:(IntData *)theIndexBuffer {
    glDisable(GL_TEXTURE_2D);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

    if (color != NULL) {
        glColor4f(color->x, color->y, color->z, color->w);
        glVertexPointer(3, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize + ColorSize));
    } else {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_UNSIGNED_BYTE, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize));
        glVertexPointer(3, GL_FLOAT, TexCoordSize + TexCoordSize + ColorSize + ColorSize + VertexSize, (const GLvoid *)(long)(TexCoordSize + TexCoordSize + ColorSize + ColorSize));
    }
    
    const void* indexBytes = [theIndexBuffer bytes];
    int primCount = [theIndexBuffer count];
    glDrawElements(GL_LINES, primCount, GL_UNSIGNED_INT, indexBytes);
    
    glPopClientAttrib();
}

- (void)renderFaces:(BOOL)textured indexBuffers:(NSDictionary *)theIndexBuffers {
    glPolygonMode(GL_FRONT, GL_FILL);
    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);
    
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
    
    for (NSString* textureName in theIndexBuffers) {
        Texture* texture = [textureManager textureForName:textureName];
        if (textured) {
            if (texture != nil)
                [texture activate];
            else
                glDisable(GL_TEXTURE_2D);
        }
        
        IntData* indexBuffer = [theIndexBuffers objectForKey:textureName];
        
        const void* indexBytes = [indexBuffer bytes];
        int primCount = [indexBuffer count];
        
        glDrawElements(GL_TRIANGLES, primCount, GL_UNSIGNED_INT, indexBytes);
        
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

    glPopClientAttrib();
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
        
        Camera* camera = [windowController camera];
        
        for (id <Brush> brush in [selectionManager selectedBrushes]) {
            if ([filter brushVerticesPickable:brush]) {
                const TVertexList* vertices = [brush vertices];
                for (int i = 0; i < vertices->count; i++) {
                    TVector3f* vertex = &vertices->items[i]->position;
                    float dist = [camera distanceTo:vertex];
                    
                    glPushMatrix();
                    glTranslatef(vertex->x, vertex->y, vertex->z);
                    glScalef(dist / 300, dist / 300, dist / 300);
                    gluSphere(vertexHandle, 2, 12, 12);
                    glPopMatrix();
                }
                
                const TEdgeList* edges = [brush edges];
                for (int i = 0; i < edges->count; i++) {
                    TEdge* edge = edges->items[i];
                    centerOfEdge(edge, &mid);
                    float dist = [camera distanceTo:&mid];
                    
                    glPushMatrix();
                    glTranslatef(mid.x, mid.y, mid.z);
                    glScalef(dist / 300, dist / 300, dist / 300);
                    gluSphere(vertexHandle, 2, 12, 12);
                    glPopMatrix();
                }
                
                for (id <Face> face in [brush faces]) {
                    centerOfVertices([face vertices], &mid);
                    float dist = [camera distanceTo:&mid];
                    
                    glPushMatrix();
                    glTranslatef(mid.x, mid.y, mid.z);
                    glScalef(dist / 300, dist / 300, dist / 300);
                    gluSphere(vertexHandle, 2, 12, 12);
                    glPopMatrix();
                }
            }
        }

        glFrontFace(GL_CW);
    }
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
        
        faceIndexBuffers = [[NSMutableDictionary alloc] init];
        selectedFaceIndexBuffers = [[NSMutableDictionary alloc] init];
        classnameRenderer = [[TextRenderer alloc] initWithFontManager:fontManager camera:[windowController camera]];
        selectedClassnameRenderer = [[TextRenderer alloc] initWithFontManager:fontManager camera:[windowController camera]];
        entityRenderers = [[NSMutableDictionary alloc] init];
        modelEntities = [[NSMutableArray alloc] init];
        selectedModelEntities = [[NSMutableArray alloc] init];

        Camera* camera = [windowController camera];
        Options* options = [windowController options];
        Grid* grid = [options grid];
        SelectionManager* selectionManager = [map selectionManager];
        GroupManager* groupManager = [map groupManager];
        
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
    [edgeIndexBuffer release];
    [selectedEdgeIndexBuffer release];
    [classnameRenderer release];
    [selectedClassnameRenderer release];
    if (vertexHandle != NULL)
        gluDeleteQuadric(vertexHandle);
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
                    [self renderFaces:YES indexBuffers:faceIndexBuffers];
                [self renderFaces:YES indexBuffers:selectedFaceIndexBuffers];
                break;
            case RM_FLAT:
                if ([options isolationMode] == IM_NONE)
                    [self renderFaces:NO indexBuffers:faceIndexBuffers];
                [self renderFaces:NO indexBuffers:selectedFaceIndexBuffers];
                break;
            case RM_WIREFRAME:
                break;
        }
        
        glSetEdgeOffset(0.5f);
        [self renderEdges:NULL indexBuffer:edgeIndexBuffer];
        glResetEdgeOffset();
        
        if ([[windowController selectionManager] hasSelection]) {
            glSetEdgeOffset(0.6f);
            glDisable(GL_DEPTH_TEST);
            [self renderEdges:&SelectionColor2 indexBuffer:selectedEdgeIndexBuffer];
            glResetEdgeOffset();

            glSetEdgeOffset(0.7f);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            [self renderEdges:&SelectionColor indexBuffer:selectedEdgeIndexBuffer];
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
            
            glDisable(GL_DEPTH_TEST);
            [self renderEntityBounds:&SelectionColor2 vertexCount:selectedEntityBoundsVertexCount];
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL);
            [self renderEntityBounds:&SelectionColor vertexCount:selectedEntityBoundsVertexCount];
            glDepthFunc(GL_LESS);
            
            glDisableClientState(GL_VERTEX_ARRAY);
            deactivateVbo(&selectedEntityBoundsVbo);
            
            [self renderEntityModels:selectedModelEntities];
        }
    }
    
    if ([[windowController selectionManager] hasSelection])
        [self renderVertexHandles];
    
    if ([feedbackFigures count] > 0) {
        glDisable(GL_DEPTH_TEST);
        for (id <Figure> figure in feedbackFigures)
            [figure render];
        glEnable(GL_DEPTH_TEST);
    }
     
    PreferencesManager* preferences = [PreferencesManager sharedManager];
    float brightness = [preferences brightness];
    if(brightness > 1) {
        glBlendFunc(GL_DST_COLOR, GL_ONE);
        glColor3f(brightness - 1, brightness - 1, brightness - 1);
    } else {
        glBlendFunc(GL_ZERO, GL_SRC_COLOR);
        glColor3f(brightness, brightness, brightness);
    }
    glEnable(GL_BLEND);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_QUADS);
    glVertex3i(-1, 1, 0);
    glVertex3i(1, 1, 0);
    glVertex3i(1, -1, 0);
    glVertex3i(-1, -1, 0);
    glEnd();
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    
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
