//
//  DragTool.h
//  TrenchBroom
//
//  Created by Kristian Duske on 18.07.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Math.h"

@class PickingHitList;

@protocol DndTool <NSObject>

- (NSDragOperation)handleDraggingEntered:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits;
- (NSDragOperation)handleDraggingUpdated:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)handleDraggingEnded:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)handleDraggingExited:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits;
- (BOOL)prepareForDragOperation:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits;
- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits;
- (void)concludeDragOperation:(id <NSDraggingInfo>)sender ray:(TRay *)ray hits:(PickingHitList *)hits;

@end
