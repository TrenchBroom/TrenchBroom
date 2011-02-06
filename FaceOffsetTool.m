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
#import "Math.h"

static NSArray* smallCircle;
static NSArray* largeCircle;
static NSArray* ring;

@implementation FaceOffsetTool

+ (void)initialize {
    smallCircle = [makeCircle(3, 20) retain];
    largeCircle = [makeCircle(10, 32) retain];
    ring = [makeRing(10, 11, 32) retain];
}

- (id)initWithFace:(Face *)theFace {
    if (theFace == nil)
        [NSException raise:NSInvalidArgumentException format:@"face must not be nil"];
    
    if (self = [self init]) {
        face = [theFace retain];
    }
    
    return self;
}

- (void)renderPoints:(NSArray *)points {
    NSEnumerator* pointEn = [points objectEnumerator];
    Vector2f* point;
    while ((point = [pointEn nextObject])) {
        Vector3f* w = [face worldCoordsOf:point];
        glVertex3f([w x], [w y], [w z]);
    }
}

- (void)render {
    
    glPolygonMode(GL_FRONT, GL_FILL);
    
    glColor4f(1, 1, 1, 0.65);
    glBegin(GL_POLYGON);
    [self renderPoints:largeCircle];
    glEnd();
    
    glColor4f(0.02, 0.5, 0.9, 0.65);
    glBegin(GL_POLYGON);
    [self renderPoints:smallCircle];
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    [self renderPoints:ring];
    glEnd();
    
    Vector2f* v = [[Vector2f alloc] init];
    Vector3f* w;
    
    glBegin(GL_TRIANGLES);
    // top arrow
    [v setX:-2];
    [v setY:4];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);
    
    [v setX:0];
    [v setY:9];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);
    
    [v setX:2];
    [v setY:4];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);
    
    /*
    // right arrow
    [v setX:4];
    [v setY:2];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);
    
    [v setX:9];
    [v setY:0];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);

    [v setX:4];
    [v setY:-2];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);

    // bottom arrow
    [v setX:2];
    [v setY:-4];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);
    
    [v setX:0];
    [v setY:-9];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);
    
    [v setX:-2];
    [v setY:-4];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);
    
    // left arrow
    [v setX:-4];
    [v setY:-2];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);
    
    [v setX:-9];
    [v setY:0];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);
    
    [v setX:-4];
    [v setY:2];
    w = [face worldCoordsOf:v];
    glVertex3f([w x], [w y], [w z]);
     */
    
    glEnd();
}

- (void)dealloc {
    [face release];
    [super dealloc];
}

@end
