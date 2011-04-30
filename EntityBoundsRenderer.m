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

static NSString* QuadsBlockKey = @"bounds quads";
static NSString* LinesBlockKey = @"bounds lines";

@interface EntityBoundsRenderer (private)

- (void)validate;

@end

@implementation EntityBoundsRenderer (private)

- (void) validate {
    if ([removedEntities count] > 0) {
        NSEnumerator* entityEn = [removedEntities objectEnumerator];
        id <Entity> entity;
        while ((entity = [entityEn nextObject])) {
            [entity setMemBlock:nil forKey:QuadsBlockKey];
            [entity setMemBlock:nil forKey:LinesBlockKey];
        }
    }

    if ([addedEntities count] > 0) {
        TVector3f c, t;
        float defaultColor[] = {1, 1, 1};
        
        NSEnumerator* entityEn = [addedEntities objectEnumerator];
        id <Entity> entity;
        
        [quads activate];
        [quads mapBuffer];
        while ((entity = [entityEn nextObject])) {
            VBOMemBlock* quadsBlock = [entity memBlockForKey:QuadsBlockKey];
            if (quadsBlock == nil) {
                quadsBlock = [quads allocMemBlock:6 * 4 * (3 + 3) * sizeof(float)];
                [entity setMemBlock:quadsBlock forKey:QuadsBlockKey];
            }
            
            if ([quadsBlock state] != BS_USED_VALID) {
                EntityDefinition* definition = [entity entityDefinition];
                TBoundingBox* bounds = [entity bounds];
                float* color = definition != nil ? [definition color] : defaultColor;
                
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
        [quads pack];
        [quads unmapBuffer];
        [quads deactivate];
    }
}

@end

@implementation EntityBoundsRenderer

- (id)init {
    if (self = [super init]) {
        quads = [[VBOBuffer alloc] initWithTotalCapacity:0xFFFF];
        
        addedEntities = [[NSMutableSet alloc] init];
        removedEntities = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)addEntity:(id <Entity>)entity {
    NSAssert(entity != nil, @"entity must not be nil");
    [addedEntities addObject:entity];
}

- (void)removeEntity:(id <Entity>)entity {
    NSAssert(entity != nil, @"entity must not be nil");
    
    if ([addedEntities containsObject:entity])
        [addedEntities removeObject:entity];
    else
        [removedEntities addObject:entity];
}

- (void)render {
    [self validate];
    
    
    int quadCount = ([quads totalCapacity] - [quads freeCapacity]) / (6 * sizeof(float));
    [quads activate];
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDisable(GL_CULL_FACE);
    glInterleavedArrays(GL_C3F_V3F, 0, 0);
    glDrawArrays(GL_QUADS, 0, quadCount);
    glEnable(GL_CULL_FACE);
    [quads deactivate];
    
}

- (void)dealloc {
    [quads release];
    [addedEntities release];
    [removedEntities release];
    [super dealloc];
}

@end
