//
//  PolygonFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "Figure.h"

@class IntData;

@protocol PolygonFigure <Figure>

- (void)invalidate;
- (NSString *)texture;
- (void)getIndex:(IntData *)theIndexBuffer count:(IntData *)theCountBuffer;

@end
