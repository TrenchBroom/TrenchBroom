//
//  Brush.m
//  TrenchBroom
//
//  Created by Kristian Duske on 30.01.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import "Brush.h"
#import "IdGenerator.h"
#import "Vector3f.h"
#import "Vector3i.h"
#import "Face.h"
#import "Polyhedron.h"
#import "Polygon3D.h"
#import "HalfSpace3D.h"
#import "Line3D.h"
#import "Plane3D.h"
#import "Segment3D.h"

@implementation Brush

- (id)init {
    if (self = [super init]) {
        brushId = [[IdGenerator sharedGenerator] getId];
        faces = [[NSMutableArray alloc] init];
        vertices = [[NSMutableDictionary alloc] init];

        // create cube with maximum dimensions
        Vector3i* p1 = [[Vector3i alloc] initWithX:-4096 y:-4096 z:-4096];
        Vector3i* p2 = [[Vector3i alloc] initWithX:4096 y:4096 z:4096];
        
        NSString* texture = @"";
        
        Face* bottom = [[Face alloc] initOnPlane:XY at:p1 thirdAxisPositive:NO texture:texture];
        [faces addObject:bottom];
        
        Face* left = [[Face alloc] initOnPlane:XZ at:p1 thirdAxisPositive:NO texture:texture];
        [faces addObject:left];
        
        Face* back = [[Face alloc] initOnPlane:YZ at:p1 thirdAxisPositive:NO texture:texture];
        [faces addObject:back];
        
        Face* top = [[Face alloc] initOnPlane:XY at:p2 thirdAxisPositive:YES texture:texture];
        [faces addObject:top];
        
        Face* right = [[Face alloc] initOnPlane:XZ at:p2 thirdAxisPositive:YES texture:texture];
        [faces addObject:right];
        
        Face* front = [[Face alloc] initOnPlane:YZ at:p2 thirdAxisPositive:YES texture:texture];
        [faces addObject:front];
        
        Vector3f* lbb = [[Vector3f alloc] initWithX:-4096 y:-4096 z:-4096];
        Vector3f* lbt = [[Vector3f alloc] initWithX:-4096 y:-4096 z:+4096];
        Vector3f* lfb = [[Vector3f alloc] initWithX:-4096 y:+4096 z:-4096];
        Vector3f* lft = [[Vector3f alloc] initWithX:-4096 y:+4096 z:+4096];
        Vector3f* rbb = [[Vector3f alloc] initWithX:+4096 y:-4096 z:-4096];
        Vector3f* rbt = [[Vector3f alloc] initWithX:+4096 y:-4096 z:+4096];
        Vector3f* rfb = [[Vector3f alloc] initWithX:+4096 y:+4096 z:-4096];
        Vector3f* rft = [[Vector3f alloc] initWithX:+4096 y:+4096 z:+4096];
        
        NSMutableArray* bottomVertices = [[NSMutableArray alloc] initWithObjects:lbb, lfb, rbb, rfb, nil];
        NSMutableArray* leftVertices   = [[NSMutableArray alloc] initWithObjects:lbb, lbt, lfb, lft, nil];
        NSMutableArray* topVertices    = [[NSMutableArray alloc] initWithObjects:lbt, lft, rbt, rft, nil];
        NSMutableArray* rightVertices  = [[NSMutableArray alloc] initWithObjects:rbb, rbt, rfb, rft, nil];
        NSMutableArray* frontVertices  = [[NSMutableArray alloc] initWithObjects:lfb, lft, rfb, rft, nil];
        NSMutableArray* backVertices   = [[NSMutableArray alloc] initWithObjects:lbb, lbt, rbb, rbt, nil];
        
        [vertices setObject:bottomVertices forKey:[bottom getId]];
        [vertices setObject:leftVertices forKey:[left getId]];
        [vertices setObject:topVertices forKey:[top getId]];
        [vertices setObject:rightVertices forKey:[right getId]];
        [vertices setObject:frontVertices forKey:[front getId]];
        [vertices setObject:backVertices forKey:[back getId]];
        
        [lbb release];
        [lbt release];
        [lfb release];
        [lft release];
        [rbb release];
        [rbt release];
        [rfb release];
        [rft release];
        
        [bottomVertices release];
        [leftVertices release];
        [topVertices release];
        [rightVertices release];
        [frontVertices release];
        [backVertices release];
        
        [bottom release];
        [left release];
        [back release];
        [top release];
        [right release];
        [front release];
        
        [p1 release];
        [p2 release];    }
    
    return self;
}

- (BOOL)cutFace:(Face *)face withFace:(Face *)cuttingFace newSegment:(Segment3D **)newSegment {
    NSArray* currentVertices = [self verticesForFace:face];
    HalfSpace3D* halfSpace = [cuttingFace halfSpace];
    
    Vector3f* newSegmentStart = nil;
    Vector3f* newSegmentEnd = nil;
    
    Vector3f* prevVert = [currentVertices lastObject];
    BOOL prevContained = [halfSpace containsPoint:prevVert];
    Vector3f* curVert;
    BOOL curContained;
    BOOL outside = !prevContained;
    
    NSMutableArray* newVertices = [[NSMutableArray alloc] init];
    NSEnumerator* vertEn = [currentVertices objectEnumerator];
    while ((curVert = [vertEn nextObject])) {
        curContained = [halfSpace containsPoint:curVert];
        
        if (prevContained)
            [newVertices addObject:prevVert];
        
        if (prevContained ^ curContained) {
            Line3D* line = [[Line3D alloc] initWithPoint1:prevVert point2:curVert];
            Vector3f* newVert = [[halfSpace boundary] intersectWithLine:line];
            [newVertices addObject:newVert];
            [line release];
            
            if (newSegmentStart == nil)
                newSegmentStart = newVert;
            else if (newSegmentEnd == nil)
                newSegmentEnd = newVert;
        }
        
        prevVert = curVert;
        prevContained = curContained;
        outside &= !prevContained;
    }
    
    [vertices setObject:newVertices forKey:[face getId]];

    if (outside)
        return NO;
    
    if (newSegmentStart != nil && newSegmentEnd != nil)
        *newSegment = [Segment3D segmentWithStartVertex:newSegmentStart endVertex:newSegmentEnd];
    
    return YES;
}

- (BOOL)cutWithFace:(Face *)cuttingFace {
    NSMutableArray* segments = [[NSMutableArray alloc] init];
    @try {
        int i = 0;
        while (i < [faces count]) {
            Face* face = [faces objectAtIndex:i];
            Segment3D* segment = nil;
            if ([self cutFace:face withFace:cuttingFace newSegment:&segment]) {
                if (segment != nil)
                    [segments addObject:segment];
                i++;
            } else {
                [vertices removeObjectForKey:[face getId]];
                [faces removeObjectAtIndex:i];
            }
            
        }
        
        if ([segments count] == 0)
            return NO;
        
        // sort the new segments and add their start vertices in order to newVerts
        NSMutableArray* newVertices = [[NSMutableArray alloc] init];
        for (int i = 0; i < [segments count]; i++) {
            Segment3D* segment = [segments objectAtIndex:i];
            [newVertices addObject:[segment startVertex]];
            for (int j = i + 1; j < [segments count]; j++) {
                Segment3D* candidate = [segments objectAtIndex:j];
                if ([[segment endVertex] isEqualToVector:[candidate startVertex]]) {
                    if (i + 1 != j)
                        [segments exchangeObjectAtIndex:i + 1 withObjectAtIndex:j];
                    break;
                } else if ([[segment endVertex] isEqualToVector:[candidate endVertex]]) {
                    [candidate flip];
                    if (i + 1 != j)
                        [segments exchangeObjectAtIndex:i + 1 withObjectAtIndex:j];
                    break;
                }
            }
        }

        [vertices setObject:newVertices forKey:[cuttingFace getId]];
        [newVertices release];
        return YES;
    } @finally {
        [segments release];
    }
    
}

- (Face *)createFaceWithPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 texture:(NSString *)texture {
    Face* face = [[Face alloc] initWithPoint1:point1 point2:point2 point3:point3 texture:texture];

    if (![self cutWithFace:face]) {
        NSLog(@"Face %@ did not touch brush", face);
        [face release];
        return nil;
    }

    [faces addObject:face];
    return [face autorelease];
}

- (NSNumber *)getId {
    return brushId;
}
         
- (NSArray *)faces {
    return faces;
}

- (NSArray *)verticesForFace:(Face *)face {
    if (face == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];

    NSArray* result = [vertices objectForKey:[face getId]];
    if (result == nil)
        [NSException raise:NSInvalidArgumentException format:@"unknown face %@", face];
    
    return result;
}

- (void)dealloc {
    [vertices release];
    [faces release];
    [super dealloc];
}

@end
