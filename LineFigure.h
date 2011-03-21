//
//  LineFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "Figure.h"

@class VBOBuffer;

@protocol LineFigure <Figure>

- (void)prepareWithVbo:(VBOBuffer *)theVbo;

@end
