//
//  SelectionLayer.m
//  TrenchBroom
//
//  Created by Kristian Duske on 08.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import "SelectionLayer.h"


@implementation SelectionLayer

- (void)renderFaces:(RenderContext *)renderContext {
    switch ([renderContext mode]) {
        case RM_TEXTURED:
            glEnable(GL_TEXTURE_2D);
            glPolygonMode(GL_FRONT, GL_FILL);
            glColor4f(1, 0, 0, 1);
            glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
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

@end
