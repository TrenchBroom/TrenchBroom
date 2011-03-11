//
//  Figure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 11.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class RenderContext;
@class IntData;

@protocol Figure <NSObject>

- (id)object;
- (NSString *)texture;
- (void)prepare:(RenderContext *)renderContext;
- (void)getIndex:(IntData *)theIndexBuffer count:(IntData *)theCountBuffer;

- (void)invalidate;

@end
