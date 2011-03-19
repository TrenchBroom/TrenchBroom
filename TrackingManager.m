//
//  TrackingManager.m
//  TrenchBroom
//
//  Created by Kristian Duske on 18.03.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import "TrackingManager.h"
#import "MapWindowController.h"
#import "MapDocument.h"
#import "Picker.h"
#import "PickingHit.h"
#import "PickingHitList.h"
#import "SelectionManager.h"
#import "Ray3D.h"

NSString* const TrackedObjectChanged = @"TrackedObjectChanged";
NSString* const TrackedObjectKey = @"TrackedObjectKey";
NSString* const UntrackedObjectKey = @"UntrackedObjectKey";

@implementation TrackingManager

- (id)initWithWindowController:(MapWindowController *)theWindowController {
    if (theWindowController == nil)
        [NSException raise:NSInvalidArgumentException format:@"window controller must not be nil"];
    
    if (self = [self init]) {
        windowController = [theWindowController retain];
    }
    
    return self;
}

- (void)updateWithRay:(Ray3D *)theRay {
    [currentRay release];
    currentRay = [theRay retain];
    
    SelectionManager* selectionManager = [windowController selectionManager];
    Picker* picker = [[windowController document] picker];
    
    PickingHitList* hits = [picker pickObjects:currentRay include:[selectionManager selectedBrushes] exclude:nil];
    PickingHit* hit = [hits firstHitOfType:HT_EDGE | HT_VERTEX ignoreOccluders:NO];
    if (hit == nil) {
        if (currentObject != nil) {
            NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
            [userInfo setObject:currentObject forKey:UntrackedObjectKey];
            
            [currentObject release];
            currentObject = nil;
            
            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:TrackedObjectChanged object:self userInfo:userInfo];
            [userInfo release];

            NSLog(@"tracked object changed to nil");
        }
    } else {
        if ([hit object] != currentObject) {
            NSMutableDictionary* userInfo = [[NSMutableDictionary alloc] init];
            if (currentObject != nil)
                [userInfo setObject:currentObject forKey:UntrackedObjectKey];
            
            [currentObject release];
            currentObject = [[hit object] retain];
            [userInfo setObject:currentObject forKey:TrackedObjectKey]; 

            NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
            [center postNotificationName:TrackedObjectChanged object:self userInfo:userInfo];
            [userInfo release];
            
            NSLog(@"tracked object changed to %@", currentObject);
        }
    }
}

- (void)dealloc {
    [currentObject release];
    [currentRay release];
    [windowController release];
    [super dealloc];
}

@end
