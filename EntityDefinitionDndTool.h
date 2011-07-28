//
//  EntityDefinitionDragTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 18.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "DndTool.h"

@class MapWindowController;
@class EntityDefinitionDndFeedbackFigure;
@protocol Entity;

@interface EntityDefinitionDndTool : NSObject <DndTool> {
@private
    MapWindowController* windowController;
    id <Entity> entity;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

@end
