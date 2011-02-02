//
//  TextureWindowController.h
//  TrenchBroom
//
//  Created by Kristian Duske on 02.02.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class TextureView;
@class TextureManager;

@interface TextureWindowController : NSWindowController {
    IBOutlet TextureView* textureView;
    NSOpenGLContext* sharedContext;
    TextureManager* textureManager;
}

- (id)initWithWindowNibName:(NSString *)theWindowNibName sharedContext:(NSOpenGLContext *)theSharedContext textureManager:(TextureManager *)theTextureManager;
@end
