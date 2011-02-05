//
//  FaceOffsetTool.m
//  TrenchBroom
//
//  Created by Kristian Duske on 05.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FaceOffsetTool.h"
#import "Face.h"
#import "Vector2f.h"
#import "Vector3f.h"

@implementation FaceOffsetTool

- (id)initWithFace:(Face *)theFace {
    if (theFace == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    
    if (self = [self init]) {
        face = [theFace retain];
    }
    
    return self;
}

- (void)render {
    Vector3f* c = [face center];
    Vector2f* s = [[Vector2f alloc] init];
    Vector3f* w;
    
    glPolygonMode(GL_FRONT, GL_FILL);
    glColor4f(0, 1, 0, 1);
    
    glBegin(GL_QUADS);

    [s setX:0];
    [s setY:10];
    w = [face worldCoordsOf:s];
    [w add:c];
    glVertex3f([w x], [w y], [w z]);

    [s setX:10];
    [s setY:0];
    w = [face worldCoordsOf:s];
    [w add:c];
    glVertex3f([w x], [w y], [w z]);
    
    [s setX:0];
    [s setY:-10];
    w = [face worldCoordsOf:s];
    [w add:c];
    glVertex3f([w x], [w y], [w z]);
    
    [s setX:-10];
    [s setY:0];
    w = [face worldCoordsOf:s];
    [w add:c];
    glVertex3f([w x], [w y], [w z]);
    
    glEnd();
    
    [s release];
}

- (void)dealloc {
    [face release];
    [super dealloc];
}

@end
