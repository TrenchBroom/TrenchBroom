//
//  Side.m
//  TrenchBroom
//
//  Created by Kristian Duske on 24.01.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "Side.h"
#import "Edge.h"
#import "Vertex.h"
#import "SideEdge.h"

@implementation Side

- (id)init {
    if (self = [super init]) {
        edges = [[NSMutableArray alloc] init];
        mark = SM_NEW;
    }
    
    return self;
}

- (id)initWithEdges:(NSArray *)theEdges flipped:(BOOL*)flipped {
    if (theEdges == nil)
        [NSException raise:NSInvalidArgumentException format:@"edge array must not be nil"];
    
    if (self = [self init]) {
        for (int i = 0; i < [theEdges count]; i++) {
            Edge* edge = [theEdges objectAtIndex:i];
            SideEdge* sideEdge = [[SideEdge alloc] initWithEdge:edge flipped:flipped[i]];
            [edges addObject:sideEdge];
            [sideEdge release];
        }
    }
    
    return self;
}

- (id)initWithSideEdges:(NSArray *)theEdges {
    if (theEdges == nil)
        [NSException raise:NSInvalidArgumentException format:@"edge array must not be nil"];
    
    if (self = [self init]) {
        [edges addObjectsFromArray:theEdges];
    }
    
    return self;
}

- (SideEdge *)split {
    EEdgeMark previousMark = [[edges lastObject] mark];
    if (previousMark == EM_KEEP)
        mark = SM_KEEP;
    else if (previousMark == EM_DROP)
        mark = SM_DROP;
    else if (previousMark == EM_SPLIT)
        mark = SM_SPLIT;
    
    int splitIndex1, splitIndex2 = -1;

    for (int i = 0; i < [edges count]; i++) {
        SideEdge* sideEdge = [edges objectAtIndex:i];
        EEdgeMark currentMark = [sideEdge mark];
        if (currentMark == EM_SPLIT) {
            if ([[sideEdge startVertex] mark] == VM_KEEP)
                splitIndex1 = i;
            else
                splitIndex2 = i;
        }
        
        if ((mark == SM_KEEP && currentMark != EM_KEEP) || 
            (mark == SM_DROP && currentMark != EM_DROP))
            mark = SM_SPLIT;
        previousMark = currentMark;
    }
    
    if (mark == SM_KEEP || mark == SM_DROP)
        return nil;

    Vertex* startVertex = [[edges objectAtIndex:splitIndex1] endVertex];
    Vertex* endVertex = [[edges objectAtIndex:splitIndex2] startVertex];
    Edge* newEdge = [[Edge alloc] initWithStartVertex:startVertex endVertex:endVertex];
    SideEdge* sideEdge = [[SideEdge alloc] initWithEdge:newEdge flipped:NO];
    if (splitIndex2 > splitIndex1) {
        int num = splitIndex2 - splitIndex1 - 1;
        if (num > 0)
            [edges removeObjectsInRange:NSMakeRange(splitIndex1 + 1, num)];
        [edges insertObject:sideEdge atIndex:splitIndex1 + 1];
    } else {
        int num = [edges count] - splitIndex1 - 1;
        if (num > 0)
            [edges removeObjectsInRange:NSMakeRange(splitIndex1 + 1, num)];
        num = splitIndex2;
        if (num > 0)
            [edges removeObjectsInRange:NSMakeRange(0, num)];
        [edges addObject:sideEdge];
    }
    [sideEdge release];
    
    SideEdge* opposingEdge = [[SideEdge alloc] initWithEdge:newEdge flipped:YES];
    [newEdge release];
    
    return [opposingEdge autorelease];
}

- (ESideMark)mark {
    return mark;
}

- (void)setMark:(ESideMark)theMark {
    mark = theMark;
}

- (NSArray *)vertices {
    NSMutableArray* vertices = [[NSMutableArray alloc] initWithCapacity:[edges count]];
    
    NSEnumerator* eEn = [edges objectEnumerator];
    Edge* edge;
    while ((edge = [eEn nextObject]))
        [vertices addObject:[[edge startVertex] vector]];
    
    return [vertices autorelease];
}

- (NSString *)description {
    NSMutableString* desc = [NSMutableString stringWithFormat:@"side mark: "];
    switch (mark) {
        case SM_KEEP:
            [desc appendFormat:@"SM_KEEP\n"];
            break;
        case SM_DROP:
            [desc appendFormat:@"SM_DROP\n"];
            break;
        case SM_SPLIT:
            [desc appendFormat:@"SM_SPLIT\n"];
            break;
        case SM_NEW:
            [desc appendFormat:@"SM_NEW\n"];
            break;
        case SM_UNKNOWN:
            [desc appendFormat:@"SM_UNKNOWN\n"];
            break;
        default:
            [desc appendFormat:@"invalid\n"];
            break;
    }
    
    NSEnumerator* eEn = [edges objectEnumerator];
    SideEdge* sideEdge;
    while ((sideEdge = [eEn nextObject]))
        [desc appendFormat:@"%@\n", sideEdge];
    
    return desc;
}

- (void)dealloc {
    [edges release];
    [super dealloc];
}

@end
