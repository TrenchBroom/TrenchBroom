//
//  BrushInfo.h
//  TrenchBroom
//
//  Created by Kristian Duske on 25.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>

@class MutableBrush;
@protocol Brush;

@interface BrushInfo : NSObject {
@private
    NSNumber* brushId;
    NSMutableArray* faceInfos;
}

+ (id)brushInfoFor:(id <Brush>)theBrush;

- (id)initWithBrush:(id <Brush>)theBrush;

- (void)updateBrush:(MutableBrush *)theBrush;

@end
