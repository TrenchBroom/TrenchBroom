//
//  FaceRotationFigure.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "FaceRotationFigure.h"
#import "Math.h"
#import "Face.h"
#import "Vector2f.h"
#import "Vector3f.h"

static NSArray* ring;

@implementation FaceRotationFigure

+ (void)initialize {
    ring = [makeRing(12, 14, 32) retain];
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

- (void)render:(RenderContext *)renderContext {
    glPolygonMode(GL_FRONT, GL_FILL);
    
    glColor4f(0.02, 0.5, 0.9, 0.65);
    glBegin(GL_TRIANGLE_STRIP);
    [self renderPoints:ring z:0.01];
    glEnd();
}

- (void)dealloc {
    [face release];
    [super dealloc];
}
@end
