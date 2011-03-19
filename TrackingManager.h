//
//  TrackingManager.h
//  TrenchBroom
//
//  Created by Kristian Duske on 18.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Cocoa/Cocoa.h>

extern NSString* const TrackedObjectChanged;
extern NSString* const UntrackedObjectKey;
extern NSString* const TrackedObjectKey;

@class MapWindowController;
@class Ray3D;

@interface TrackingManager : NSObject {
    @private
    id currentObject;
    Ray3D* currentRay;
    MapWindowController* windowController;
}

- (id)initWithWindowController:(MapWindowController *)theWindowController;

- (void)updateWithRay:(Ray3D *)theRay;

@end
