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

- (void)renderPoints:(NSArray *)points z:(float)z {
    NSEnumerator* pointEn = [points objectEnumerator];
    Vector2f* point2D;
    Vector3f* point3D = [[Vector3f alloc] init];
    [point3D setZ:z]; // slight offset from face
    
    while ((point2D = [pointEn nextObject])) {
        [point3D setX:[point2D x]];
        [point3D setY:[point2D y]];
        Vector3f* w = [face worldCoordsOf:point3D];
        glVertex3f([w x], [w y], [w z]);
    }
}

- (void)render {
    glPolygonMode(GL_FRONT, GL_FILL);
    
    glColor4f(0.02, 0.5, 0.9, 0.65);
    glBegin(GL_TRIANGLE_STRIP);
    [self renderPoints:ring z:0.01];
    glEnd();
    
    glColor4f(1, 1, 1, 0.65);
    glBegin(GL_POLYGON);
    [self renderPoints:largeCircle z:0.01];
    glEnd();
    
    glColor4f(0.02, 0.5, 0.9, 0.65);
    glBegin(GL_POLYGON);
    [self renderPoints:smallCircle z:0.02];
    glEnd();
    
    Vector3f* v = [[Vector3f alloc] init];
    [v setZ:0.02];
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
    
    glEnd();
}

- (void)dealloc {
    [face release];
    [super dealloc];
}

@end
