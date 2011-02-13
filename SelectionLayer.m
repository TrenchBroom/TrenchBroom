//
//  SelectionLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SelectionLayer.h"
#import <OpenGL/gl.h>
#import "FaceFigure.h"
#import "Vector3f.h"
#import "Options.h"

@implementation SelectionLayer

- (void)renderFaces:(RenderContext *)renderContext {
    switch ([[renderContext options] renderMode]) {
        case RM_TEXTURED:
            glEnable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT, GL_FILL);
            glColor4f(1, 0, 0, 1);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
            [self renderTextured:renderContext];

            glDisable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT, GL_LINE);
            [self renderWireframe:renderContext];
            glEnable(GL_DEPTH_TEST);
            break;
        case RM_FLAT:
            break;
        case RM_WIREFRAME:
            glDisable(GL_TEXTURE_2D);
            glDisable(GL_DEPTH_TEST);
            glPolygonMode(GL_FRONT, GL_LINE);
            glColor4f(1, 0, 0, 0.5);
            [self renderWireframe:renderContext];
            glEnable(GL_DEPTH_TEST);
            break;
    }
}

- (void)render:(RenderContext*)renderContext {
    [super render:renderContext];

    if ([[renderContext options] drawGrid]) {
        int gridSize = [[renderContext options] gridSize];
        
        NSEnumerator* figureEn = [faceFigures objectEnumerator];
        FaceFigure* figure;
        while ((figure = [figureEn nextObject])) {
            Face* face = [figure face];
            NSArray* gridVertices = [face gridWithSize:gridSize];
            NSEnumerator* vertexEn = [gridVertices objectEnumerator];
            Vector3f* vertex;
            
            glDisable(GL_TEXTURE_2D);
            glColor4f(1, 0, 0, 0.5);
            glBegin(GL_LINES);
            while ((vertex = [vertexEn nextObject]))
                glVertex3f([vertex x], [vertex y], [vertex z]);
            glEnd();
        }
    }
}

@end
