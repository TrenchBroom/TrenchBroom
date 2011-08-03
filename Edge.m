//
//  Edge.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Edge.h"
#import "Side.h"
#import "Vertex.h"
#import "Face.h"
#import "MutableFace.h"
#import "VBOBuffer.h"
#import "VBOMemBlock.h"
#import "IntData.h"
#import "PickingHit.h"

@implementation Edge
- (id)init {
    if (self = [super init]) {
        mark = EM_NEW;
    }
    
    return self;
}

- (id)initWithStartVertex:(Vertex *)theStartVertex endVertex:(Vertex *)theEndVertex {
    if (self = [self init]) {
        // do not retain vertices to avoid circular references and leaking
        startVertex = theStartVertex;
        endVertex = theEndVertex;
        [startVertex addEdge:self];
        [endVertex addEdge:self];
    }
    
    return self;
}

- (Vertex *)startVertex {
    return startVertex;
}

- (Vertex *)endVertex {
    return endVertex;
}

- (Vertex *)opposingVertex:(Vertex *)theVertex {
    if (theVertex == startVertex)
        return endVertex;
    return startVertex;
}

- (id <Face>)leftFace {
    return [leftSide face];
}

- (id <Face>)rightFace {
    return [rightSide face];
}

- (id <Face>)frontFaceForRay:(TRay *)theRay {
    id <Face> leftFace = [leftSide face];
    id <Face> rightFace = [rightSide face];
    return dotV3f([leftFace norm], &theRay->direction) >= 0 ? rightFace : leftFace;
}

- (id <Face>)backFaceForRay:(TRay *)theRay {
    id <Face> leftFace = [leftSide face];
    id <Face> rightFace = [rightSide face];
    return dotV3f([leftFace norm], &theRay->direction) >= 0 ? leftFace : rightFace;
}

- (Side *)leftSide {
    return leftSide;
}

- (Side *)rightSide {
    return rightSide;
}

- (Vertex *)startVertexForSide:(Side *)theSide {
    if (theSide == leftSide)
        return endVertex;
    else if (theSide == rightSide)
        return startVertex;
    
    [NSException raise:NSInvalidArgumentException format:@"given side is neither left nor right side"];
    return nil;
}

- (Vertex *)endVertexForSide:(Side *)theSide {
    if (theSide == leftSide)
        return startVertex;
    else if (theSide == rightSide)
        return endVertex;

    [NSException raise:NSInvalidArgumentException format:@"given side is neither left nor right side"];
    return nil;
}

- (void)setLeftSide:(Side *)theLeftSide {
    leftSide = theLeftSide;
}

- (void)setRightSide:(Side *)theRightSide {
    rightSide = theRightSide;
}

- (void)flip {
    Side* tempSide = leftSide;
    leftSide = rightSide;
    rightSide = tempSide;
    
    Vertex* tempVertex = startVertex;
    startVertex = endVertex;
    endVertex = tempVertex;
}

- (Vertex *)splitAt:(TPlane *)plane {
    NSAssert(mark == EM_SPLIT, @"cannot split edge that is not marked with EM_SPLIT");

    TLine line;
    setLinePoints(&line, [startVertex vector], [endVertex vector]);

    TVector3f newVector;
    float dist = intersectPlaneWithLine(plane, &line);
    linePointAtDistance(&line, dist, &newVector);
    
    Vertex* newVertex = [[Vertex alloc] initWithVector:&newVector];
    [newVertex addEdge:self];

    if ([startVertex mark] == VM_DROP) {
        startVertex = newVertex;
    } else {
        endVertex = newVertex;
    }
    
    return [newVertex autorelease];
}

- (EEdgeMark)mark {
    return mark;
}

- (void)updateMark {
    int keep = 0;
    int drop = 0;
    int undecided = 0;
    
    EVertexMark s = [startVertex mark];
    EVertexMark e = [endVertex mark];
    
    if (s == VM_KEEP)
        keep++;
    else if (s == VM_DROP)
        drop++;
    else if (s == VM_UNDECIDED)
        undecided++;
    else
        [NSException raise:@"InvalidVertexStateException" format:@"invalid start vertex state: %i", s];
    
    if (e == VM_KEEP)
        keep++;
    else if (e == VM_DROP)
        drop++;
    else if (e == VM_UNDECIDED)
        undecided++;
    else
        [NSException raise:@"InvalidVertexStateException" format:@"invalid end vertex state: %i", e];
    
    if (keep == 1 && drop == 1)
        mark = EM_SPLIT;
    else if (keep > 0)
        mark = EM_KEEP;
    else if (drop > 0)
        mark = EM_DROP;
    else
        mark = EM_UNDECIDED;
}

- (void)clearMark {
    mark = EM_UNKNOWN;
}

- (NSString *)description {
    NSMutableString* desc = [NSMutableString stringWithFormat:@"[start: %@, end: %@", startVertex, endVertex];
    switch (mark) {
        case EM_KEEP:
            [desc appendFormat:@", mark: EM_KEEP]"];
            break;
        case EM_DROP:
            [desc appendFormat:@", mark: EM_DROP]"];
            break;
        case EM_SPLIT:
            [desc appendFormat:@", mark: EM_SPLIT]"];
            break;
        case EM_UNDECIDED:
            [desc appendFormat:@", mark: EM_UNDECIDED]"];
            break;
        case EM_NEW:
            [desc appendFormat:@", mark: EM_NEW]"];
            break;
        case EM_UNKNOWN:
            [desc appendFormat:@", mark: EM_UNKNOWN]"];
            break;
        default:
            [desc appendFormat:@", mark: invalid]"];
            break;
    }
    
    return desc;
}

@end
