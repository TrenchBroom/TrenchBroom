//
//  VertexRenderer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 02.08.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "VertexRenderer.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "Brush.h"
#import "Vertex.h"
#import "Math.h"

@interface VertexRenderer (private)

- (BOOL)validate;

@end

@implementation VertexRenderer (private)

- (BOOL)validate {
    BOOL mapped = NO;
    
    if ([removedBrushes count] > 0) {
        NSEnumerator* brushEn = [removedBrushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            VBOMemBlock* block = [blocks objectForKey:[brush brushId]];
            [block free];
            [blocks removeObjectForKey:[brush brushId]];
        }
        
        [vbo activate];
        [vbo mapBuffer];
        [vbo pack];
        mapped = YES;
        
        [removedBrushes removeAllObjects];
    }
    
    if ([addedBrushes count] != 0) {
        if (!mapped) {
            [vbo activate];
            [vbo mapBuffer];
            mapped = YES;
        }
        
        NSEnumerator* brushEn = [addedBrushes objectEnumerator];
        id <Brush> brush;
        while ((brush = [brushEn nextObject])) {
            NSArray* vertices = [brush vertices];
            VBOMemBlock* block = [vbo allocMemBlock:[vertices count] * 6 * 4 * 3 * sizeof(float)];
            
            int offset = 0;
            TVector3f esb, wsb, est, wst, enb, wnb, ent, wnt;
            
            NSEnumerator* vertexEn = [vertices objectEnumerator];
            Vertex* vertex;
            while ((vertex = [vertexEn nextObject])) {
                esb = *[vertex vector];
                esb.x -= handleSize / 2;
                esb.y -= handleSize / 2;
                esb.z -= handleSize / 2;
                
                wsb = esb;
                wsb.x += handleSize;
                
                est = esb;
                est.z += handleSize;
                
                wst = wsb;
                wst.z += handleSize;
                
                enb = esb;
                enb.y += handleSize;
                
                wnb = wsb;
                wnb.y += handleSize;
                
                ent = est;
                ent.y += handleSize;
                
                wnt = wst;
                wnt.y += handleSize;
                
                // south quad
                offset = [block writeVector3f:&esb offset:offset];
                offset = [block writeVector3f:&est offset:offset];
                offset = [block writeVector3f:&wst offset:offset];
                offset = [block writeVector3f:&wsb offset:offset];
                
                // north quad
                offset = [block writeVector3f:&enb offset:offset];
                offset = [block writeVector3f:&wnb offset:offset];
                offset = [block writeVector3f:&wnt offset:offset];
                offset = [block writeVector3f:&ent offset:offset];
                
                // east quad
                offset = [block writeVector3f:&enb offset:offset];
                offset = [block writeVector3f:&ent offset:offset];
                offset = [block writeVector3f:&est offset:offset];
                offset = [block writeVector3f:&esb offset:offset];
                
                // west quad
                offset = [block writeVector3f:&wsb offset:offset];
                offset = [block writeVector3f:&wst offset:offset];
                offset = [block writeVector3f:&wnt offset:offset];
                offset = [block writeVector3f:&wnb offset:offset];
                
                // top quad
                offset = [block writeVector3f:&est offset:offset];
                offset = [block writeVector3f:&ent offset:offset];
                offset = [block writeVector3f:&wnt offset:offset];
                offset = [block writeVector3f:&wst offset:offset];
                
                // bottom quad
                offset = [block writeVector3f:&wsb offset:offset];
                offset = [block writeVector3f:&wnb offset:offset];
                offset = [block writeVector3f:&enb offset:offset];
                offset = [block writeVector3f:&esb offset:offset];
            }
            
            [block setState:BS_USED_VALID];
            [blocks setObject:block forKey:[brush brushId]];
        }
        
        [addedBrushes removeAllObjects];
    }
    
    if (mapped)
        [vbo unmapBuffer];
    return mapped;
}

@end

@implementation VertexRenderer

- (id)initWithHandleSize:(float)theHandleSize {
    NSAssert(theHandleSize > 0, @"handle size must be a positive float");
    
    if ((self = [super init])) {
        handleSize = theHandleSize;
        vbo = [[VBOBuffer alloc] initWithTotalCapacity:0xFFF];
        blocks = [[NSMutableDictionary alloc] init];
        addedBrushes = [[NSMutableSet alloc] init];
        removedBrushes = [[NSMutableSet alloc] init];
    }
    
    return self;
}

- (void)dealloc {
    [addedBrushes release];
    [removedBrushes release];
    [blocks release];
    [vbo release];
    [super dealloc];
}

- (void)addBrushes:(NSSet *)theBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    [addedBrushes unionSet:theBrushes];
}

- (void)removeBrushes:(NSSet *)theBrushes {
    NSAssert(theBrushes != nil, @"brush set must not be nil");
    NSMutableSet* remove = [[NSMutableSet alloc] initWithSet:theBrushes];
    [remove minusSet:addedBrushes];
    [addedBrushes minusSet:theBrushes];
    [removedBrushes unionSet:remove];
    [remove release];
}

- (void)addBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    [addedBrushes addObject:theBrush];
}

- (void)removeBrush:(id <Brush>)theBrush {
    NSAssert(theBrush != nil, @"brush must not be nil");
    if ([addedBrushes containsObject:theBrush])
        [addedBrushes removeObject:theBrush];
    else
        [removedBrushes addObject:theBrush];
}

- (void)render {
    if (![self validate])
        [vbo activate];
    
    int vertexCount = ([vbo totalCapacity] - [vbo freeCapacity]) / (3 * sizeof(float));
    
    glVertexPointer(3, GL_FLOAT, 0, (const GLvoid *)0);
    glDrawArrays(GL_QUADS, 0, vertexCount);

    [vbo deactivate];
}

@end
