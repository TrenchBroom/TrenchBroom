//
//  GeometryLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 07.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "GeometryLayer.h"
#import <OpenGL/gl.h>
#import "TextureManager.h"
#import "Brush.h"
#import "Face.h"
#import "Options.h"
#import "Grid.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "IntData.h"
#import "Filter.h"
#import "GLUtils.h"

@implementation GeometryLayer

- (id)init {
    if ((self = [super init])) {
        faces = [[NSMutableSet alloc] init];
        addedFaces = [[NSMutableSet alloc] init];
        removedFaces = [[NSMutableSet alloc] init];
        indexBuffers = [[NSMutableDictionary alloc] init];
        countBuffers = [[NSMutableDictionary alloc] init];
    }
    
    return self;
}

- (id)initWithVbo:(VBOBuffer *)theVbo textureManager:(TextureManager *)theTextureManager options:(Options *)theOptions {
    NSAssert(theVbo != nil, @"VBO must not be nil");
    NSAssert(theTextureManager != nil, @"texture manager must not be nil");
    
    if ((self = [self init])) {
        sharedVbo = [theVbo retain];
        textureManager = [theTextureManager retain];
        options = [theOptions retain];
    }
    
    return self;
}

- (void)addBrushes:(NSSet *)theBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");

    if ([theBrushes count] == 0)
        return;
    
    NSMutableSet* add = [[NSMutableSet alloc] init];
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [add addObjectsFromArray:[brush faces]];
    
    [self addFaces:add];
    [add release];
}

- (void)removeBrushes:(NSSet *)theBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    
    if ([theBrushes count] == 0)
        return;
    
    NSMutableSet* remove = [[NSMutableSet alloc] init];
    NSEnumerator* brushEn = [theBrushes objectEnumerator];
    id <Brush> brush;
    while ((brush = [brushEn nextObject]))
        [remove addObjectsFromArray:[brush faces]];
    
    [self removeFaces:remove];
    [remove release];
}

- (void)addBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    NSSet* add = [[NSSet alloc] initWithArray:[theBrush faces]];
    [self addFaces:add];
    [add release];
}

- (void)removeBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    NSSet* remove = [[NSSet alloc] initWithArray:[theBrush faces]];
    [self removeFaces:remove];
    [remove release];
}

- (void)addFaces:(NSSet *)theFaces {
    NSAssert(theFaces != nil, @"face set must not be nil");
    [addedFaces unionSet:theFaces];
}

- (void)removeFaces:(NSSet *)theFaces {
    NSAssert(theFaces != nil, @"face set must not be nil");
    NSMutableSet* remaining = [[NSMutableSet alloc] initWithSet:theFaces];
    [remaining minusSet:addedFaces];
    [addedFaces minusSet:theFaces];
    [removedFaces unionSet:remaining];
    [remaining release];
}

- (void)addFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    [addedFaces addObject:theFace];
}

- (void)removeFace:(id <Face>)theFace {
    NSAssert(theFace != nil, @"face must not be nil");
    if ([addedFaces containsObject:theFace])
        [addedFaces removeObject:theFace];
    else
        [removedFaces addObject:theFace];
}

- (void)renderFaces:(BOOL)textured {
    glPolygonMode(GL_FRONT, GL_FILL);
    
    Grid* grid = [options grid];
    if ([grid draw]) {
        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        [grid activateTexture];
        glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
        glClientActiveTexture(GL_TEXTURE1);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 7 * sizeof(float), (const GLvoid *) (0 * sizeof(float)));
    }
     
    glActiveTexture(GL_TEXTURE0);
    if (textured) {
        glEnable(GL_TEXTURE_2D);
        [self setTextureMode];
        glClientActiveTexture(GL_TEXTURE0);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(2, GL_FLOAT, 7 * sizeof(float), (const GLvoid *) (2 * sizeof(float)));
    } else {
        glDisable(GL_TEXTURE_2D);
    }

    glColor4f(0.2, 0.2, 0.2, 1);
    glVertexPointer(3, GL_FLOAT, 7 * sizeof(float), (const GLvoid *) (4 * sizeof(float)));
    
    NSEnumerator* textureNameEn = [indexBuffers keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        Texture* texture = [textureManager textureForName:textureName];
        if (textured) {
            if (texture != nil)
                [texture activate];
            else
                glDisable(GL_TEXTURE_2D);
        }
        
        IntData* indexBuffer = [indexBuffers objectForKey:textureName];
        IntData* countBuffer = [countBuffers objectForKey:textureName];
        
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

- (void)preRenderEdges {
    glColor4f(0.4f, 0.4f, 0.4f, 0.4f);
    glSetEdgeOffset(0.5f);
}

- (void)postRenderEdges {
    glResetEdgeOffset();
}

- (void)setTextureMode {
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

- (BOOL)doRenderFaces {
    return [options isolationMode] == IM_NONE;
}

- (BOOL)doRenderEdges {
    return [options isolationMode] != IM_DISCARD;
}

- (void)renderEdges {
    [self preRenderEdges];
    glDisable(GL_TEXTURE_2D);

    glVertexPointer(3, GL_FLOAT, 7 * sizeof(float), (const GLvoid *) (4 * sizeof(float)));
    
    NSEnumerator* textureNameEn = [indexBuffers keyEnumerator];
    NSString* textureName;
    while ((textureName = [textureNameEn nextObject])) {
        IntData* indexBuffer = [indexBuffers objectForKey:textureName];
        IntData* countBuffer = [countBuffers objectForKey:textureName];
        
        const void* indexBytes = [indexBuffer bytes];
        const void* countBytes = [countBuffer bytes];
        int primCount = [indexBuffer count];
        
        glMultiDrawArrays(GL_LINE_LOOP, indexBytes, countBytes, primCount);
    }

    [self postRenderEdges];
}

- (void)render {
    if ([options renderBrushes]) {
        [self validate];
        [sharedVbo activate];
        switch ([options renderMode]) {
            case RM_TEXTURED:
                if ([self doRenderFaces])
                    [self renderFaces:YES];
                if ([self doRenderEdges])
                    [self renderEdges];
                break;
            case RM_FLAT:
                if ([self doRenderFaces])
                    [self renderFaces:NO];
                if ([self doRenderEdges])
                    [self renderEdges];
                break;
            case RM_WIREFRAME:
                if ([self doRenderEdges])
                    [self renderEdges];
                break;
        }
        [sharedVbo deactivate];
    }
}

- (void)validateFaces:(NSSet *)invalidFaces {
    NSEnumerator* faceEn = [invalidFaces objectEnumerator];
    id <Face> face;
    while ((face = [faceEn nextObject])) {
        id <Brush> brush = [face brush];
        if (filter == nil || [filter isBrushRenderable:brush]) {
            VBOMemBlock* block = [face memBlock];
            NSAssert(block != nil, @"face must have VBO mem block");
            NSAssert([block state] == BS_USED_VALID, @"VBO mem block must be valid");
            
            NSString* textureName = [face texture];
            
            IntData* indexBuffer = [indexBuffers objectForKey:textureName];
            if (indexBuffer == nil) {
                indexBuffer = [[IntData alloc] init];
                [indexBuffers setObject:indexBuffer forKey:textureName];
                [indexBuffer release];
            }
            
            IntData* countBuffer = [countBuffers objectForKey:textureName];
            if (countBuffer == nil) {
                countBuffer = [[IntData alloc] init];
                [countBuffers setObject:countBuffer forKey:textureName];
                [countBuffer release];
            }
            
            int index = [block address] / (7 * sizeof(float));
            int count = [[face vertices] count];
            [indexBuffer appendInt:index];
            [countBuffer appendInt:count];
        }
    }
}

- (void)validate {
    BOOL valid = YES;
    if ([removedFaces count] > 0) {
        int c = [faces count];
        [faces minusSet:removedFaces];
        valid = c == [faces count];
        [removedFaces removeAllObjects];
    }
    
    if ([addedFaces count] > 0) {
        [addedFaces minusSet:faces]; // to be safe
        if (valid)
            [self validateFaces:addedFaces];
        [faces unionSet:addedFaces];
        [addedFaces removeAllObjects];
    }
    
    if (!valid) {
        [indexBuffers removeAllObjects];
        [countBuffers removeAllObjects];
        [self validateFaces:faces];
    }
}

- (void)setFilter:(id <Filter>)theFilter {
    [filter release];
    filter = [theFilter retain];

    [indexBuffers removeAllObjects];
    [countBuffers removeAllObjects];
    [self validateFaces:faces];
}

- (void)dealloc {
    [sharedVbo release];
    [faces release];
    [addedFaces release];
    [removedFaces release];
    [indexBuffers release];
    [countBuffers release];
    [textureManager release];
    [options release];
    [filter release];
    [super dealloc];
}

@end
