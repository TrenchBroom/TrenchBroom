//
//  Renderer2D.h
//  TrenchBroom
//
//  Created by Kristian Duske on 16.03.10.
//  Copyright 2010 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class RenderContext;

@interface Renderer2D : NSObject {

}

- (void)renderWithContext:(RenderContext *)renderContext;

@end
