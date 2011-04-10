//
//  InfoLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 10.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "InfoLayer.h"
#import <OpenGL/gl.h>
#import <OpenGL/glu.h>
#import "Figure.h"

@implementation InfoLayer

- (void)render:(RenderContext *)renderContext {
    static GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(viewport[0], viewport[0] + viewport[2], viewport[1], viewport[1] + viewport[3], 1, 20);
    
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    gluLookAt(0, 0, -10, 
              0, 0, 1, 
              0, 1, 0);
    
    glDisable(GL_DEPTH_TEST);
    NSEnumerator* figureEn = [figures objectEnumerator];
    id <Figure> figure;
    while ((figure = [figureEn nextObject]))
        [figure render];
    glEnable(GL_DEPTH_TEST);
    
    glPopMatrix();
    
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

@end
