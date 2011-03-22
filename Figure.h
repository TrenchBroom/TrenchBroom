//
//  Figure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 22.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class VBOBuffer;

@protocol Figure <NSObject>

- (void)updateVBO:(VBOBuffer *)theVbo;

@end
