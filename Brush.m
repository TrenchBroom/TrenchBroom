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
#import "VertexData.h"

@implementation Brush

- (id)init {
    if (self = [super init]) {
        brushId = [[IdGenerator sharedGenerator] getId];
        faces = [[NSMutableArray alloc] init];
        vertexData = [[VertexData alloc] init];
        
        flatColor[0] = (rand() % 255) / 255.0f;
        flatColor[1] = (rand() % 255) / 255.0f;
        flatColor[2] = (rand() % 255) / 255.0f;
    }
    
    return self;
}

- (Face *)createFaceWithPoint1:(Vector3i *)point1 point2:(Vector3i *)point2 point3:(Vector3i *)point3 texture:(NSString *)texture {
    Face* face = [[Face alloc] initWithPoint1:point1 point2:point2 point3:point3 texture:texture];

    NSMutableArray* droppedFaces = nil;
    if (![vertexData cutWithFace:face droppedFaces:&droppedFaces]) {
        NSLog(@"Brush %@ was cut away by face %@", self, face);
        [face release];
        return nil;
    }

    if (droppedFaces != nil) {
        NSEnumerator* droppedFacesEn = [droppedFaces objectEnumerator];
        Face* droppedFace;
        while ((droppedFace = [droppedFacesEn nextObject])) {
            NSLog(@"Face %@ was cut away by face %@", droppedFace, face);
            // TODO speed this up by using a map of face id -> face
            [faces removeObject:droppedFace];
        }
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

    return [vertexData verticesForFace:face];
}

- (float *)flatColor {
    return flatColor;
}

- (void)dealloc {
    [vertexData release];
    [faces release];
    [super dealloc];
}

@end
