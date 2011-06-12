//
//  Filter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 17.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@protocol Brush;
@protocol Entity;

@protocol Filter <NSObject>

- (BOOL)isBrushRenderable:(id <Brush>)brush;
- (BOOL)isEntityRenderable:(id <Entity>)entity;
- (BOOL)isBrushPickable:(id <Brush>)brush;
- (BOOL)isEntityPickable:(id <Entity>)entity;

@end
