//
//  RotateCursor.h
//  TrenchBroom
//
//  Created by Kristian Duske on 21.06.11.
//  Copyright 2011 TU Berlin. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "Cursor.h"
#import "Math.h"

@interface RotateCursor : NSObject <Cursor> {
    TVector3f* quads;
    int quadCount;
}

@end
