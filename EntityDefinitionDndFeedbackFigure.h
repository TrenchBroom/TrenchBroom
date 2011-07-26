//
//  EntityDefinitionDndFeedbackFigure.h
//  TrenchBroom
//
//  Created by Kristian Duske on 26.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Figure.h"
#import "Math.h"

@class EntityDefinition;

@interface EntityDefinitionDndFeedbackFigure : NSObject <Figure> {
    EntityDefinition* definition;
    TVector3i origin;
    EAxis axis;
    BOOL axisDirection;
}

- (id)initWithEntityDefinition:(EntityDefinition *)theDefinition;
- (void)setAxis:(EAxis)theAxis direction:(BOOL)theAxisDirection;
- (void)setOrigin:(TVector3i *)theOrigin;

@end
