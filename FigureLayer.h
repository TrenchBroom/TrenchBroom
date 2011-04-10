//
//  FigureLayer.h
//  TrenchBroom
//
//  Created by Kristian Duske on 10.04.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "Layer.h"

@protocol Figure;

@interface FigureLayer : NSObject <Layer> {
    NSMutableSet* figures;
}

- (void)addFigure:(id <Figure>)theFigure;
- (void)removeFigure:(id <Figure>)theFigure;

@end
