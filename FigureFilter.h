//
//  FigureFilter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@protocol FigureFilter <NSObject>

- (BOOL)passes:(id <Figure>)theFigure;

@end
