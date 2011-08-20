//
//  MapWriter.h
//  TrenchBroom
//
//  Created by Kristian Duske on 06.03.11.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class SelectionManager;
@protocol Map;

@interface MapWriter : NSObject {
    @private
    id <Map> map;
    SelectionManager* selection;
    void* buffer;
}

- (id)initWithMap:(id <Map>)theMap;
- (id)initWithSelection:(SelectionManager *)theSelection;

- (void)writeToStream:(NSOutputStream *)theStream;
- (void)writeToFileAtPath:(NSString *)thePath;
- (void)writeToFileAtUrl:(NSURL *)theUrl;

@end
