//
//  EntityBoundsRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 25.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "EntityBoundsRenderer.h"
#import "Entity.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "EntityDefinition.h"
#import "Filter.h"

@interface EntityBoundsRenderer (private)

- (void)validate;

@end

@implementation EntityBoundsRenderer (private)

- (void) validate {
    if (!valid) {
        [quads freeAllBlocks];

        [quads activate];
        [quads mapBuffer];
        
        TVector3f c, t;
        float defaultColor[] = {1, 1, 1};

        NSEnumerator* entityEn = [entities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            if (filter == nil || [filter entityPasses:entity]) {
                VBOMemBlock* quadsBlock = [quads allocMemBlock:6 * 4 * (3 + 3) * sizeof(float)];
                
                EntityDefinition* definition = [entity entityDefinition];
                float* color = definition != nil ? [definition color] : defaultColor;
                
                TBoundingBox* bounds = [entity bounds];
                
                int offset = 0;
                c.x = color[0];
                c.y = color[1];
                c.z = color[2];
                
                // bottom side
                t = bounds->min;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.x = bounds->max.x;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.y = bounds->max.y;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.x = bounds->min.x;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                // south side
                t = bounds->min;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.z = bounds->max.z;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.x = bounds->max.x;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.z = bounds->min.z;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                // west side
                t = bounds->min;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.y = bounds->max.y;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.z = bounds->max.z;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.y = bounds->min.y;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                // top side
                t = bounds->max;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.y = bounds->min.y;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.x = bounds->min.x;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.y = bounds->max.y;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                // north side
                t = bounds->max;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.z = bounds->min.z;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.x = bounds->min.x;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.z = bounds->max.z;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                // east side
                t = bounds->max;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.z = bounds->min.z;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.y = bounds->min.y;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                t.z = bounds->max.z;
                offset = [quadsBlock writeVector3f:&c offset:offset];
                offset = [quadsBlock writeVector3f:&t offset:offset];
                
                [quadsBlock setState:BS_USED_VALID];
            }
        }
        
        [quads unmapBuffer];
        [quads deactivate];
        valid = YES;
    }
}

@end

@implementation EntityBoundsRenderer

- (id)init {
    if (self = [super init]) {
        quads = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        entities = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)addEntity:(id <Entity>)entity {
    NSAssert(entity != nil, @"entity must not be nil");
    [entities addObject:entity];
    valid = NO;
}

- (void)removeEntity:(id <Entity>)entity {
    NSAssert(entity != nil, @"entity must not be nil");
    [entities removeObject:entity];
    valid = NO;
}

- (void)renderWithColor:(BOOL)doRenderWithColor {
    [self validate];
    
    int quadCount = ([quads totalCapacity] - [quads freeCapacity]) / (6 * sizeof(float));
    [quads activate];
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);
    if (doRenderWithColor)
        glInterleavedArrays(GL_C3F_V3F, 0, 0);
    else
        glVertexPointer(3, GL_FLOAT, 6 * sizeof(float), (const GLvoid *) (3 * sizeof(float)));
    glDrawArrays(GL_QUADS, 0, quadCount);
    glEnable(GL_CULL_FACE);
    [quads deactivate];
    
}

- (void)setFilter:(id <Filter>)theFilter {
    if (filter == theFilter)
        return;
    
    [filter release];
    filter = [theFilter retain];
    valid = NO;
}

- (void)dealloc {
    [filter release];
    [quads release];
    [entities release];
    [super dealloc];
}

@end
