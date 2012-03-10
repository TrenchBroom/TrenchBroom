/*
Copyright (C) 2010-2012 Kristian Duske

This file is part of TrenchBroom.

TrenchBroom is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

TrenchBroom is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with TrenchBroom.  If not, see <http://www.gnu.org/licenses/>.
*/

#import "Autosaver.h"
#import "MapDocument.h"
#import "MapWriter.h"

@interface Autosaver (private)

- (int)backupNoOf:(NSString *)theFilename;
- (void)saveBackup;

@end

@implementation Autosaver (private)

- (int)backupNoOf:(NSString *)theFilename {
    NSString* basename = [theFilename stringByDeletingPathExtension];
    NSUInteger lastSpaceIndex = [basename rangeOfString:@" " options:NSBackwardsSearch].location;
    if (lastSpaceIndex == NSNotFound)
        return -1;
    
    int backupNo = [[basename substringFromIndex:lastSpaceIndex + 1] intValue];
    if (backupNo == 0)
        return -1;
    
    return backupNo;
}

- (void)saveBackup {
    [timer release];
    
    if (lastAction != nil && [map isDocumentEdited] && (lastSave == nil || [lastAction compare:lastSave] == NSOrderedDescending)) {
        NSDate* now = [NSDate date];
        if ([now timeIntervalSinceDate:lastAction] < idleInterval) {
            timer = [[NSTimer scheduledTimerWithTimeInterval:idleInterval target:self selector:@selector(saveBackup) userInfo:nil repeats:NO] retain];
            return;
        }
        
        NSURL* fileUrl = [map fileURL];
        if (fileUrl != nil) {
            NSFileManager* fileManager = [NSFileManager defaultManager];
            
            NSString* mapPath = [fileUrl path];
            NSString* mapFilename = [mapPath lastPathComponent];
            NSString* mapBasename = [mapFilename stringByDeletingPathExtension];
            NSString* mapDirPath = [mapPath stringByDeletingLastPathComponent];
            NSString* autosavePath = [mapDirPath stringByAppendingPathComponent:@"autosave"];
            
            BOOL exists, isDir;
            exists = [fileManager fileExistsAtPath:autosavePath isDirectory:&isDir];
            if (exists && !isDir) {
                NSLog(@"Unable to create autosave direction at '%@' because a file exists at that location", autosavePath);
                return;
            }
            
            if (!exists && ![fileManager createDirectoryAtPath:autosavePath withIntermediateDirectories:NO attributes:nil error:NULL]) {
                NSLog(@"Unable to create autosave direction at '%@'", autosavePath);
                return;
            }
            
            NSArray* contents = [fileManager contentsOfDirectoryAtPath:autosavePath error:NULL];
            if (contents == nil) {
                NSLog(@"Unable to access autosave direction at '%@'", autosavePath);
                return;
            }
            
            NSMutableArray* backups = [[NSMutableArray alloc] init];
            int highestBackupNo = 0;
            for (NSString* content in contents) {
                if ([content hasPrefix:mapBasename]) {
                    int backupNo = [self backupNoOf:content];
                    if (backupNo != -1) {
                        highestBackupNo = maxi(highestBackupNo, backupNo);
                        [backups addObject:content];
                    }
                }
            }
            
            int backupNo = 1;
            if ([backups count] > 0) {
                [backups sortUsingComparator:^(NSString* filename1, NSString* filename2) {
                    return [filename1 compare:filename2 options:NSNumericSearch];
                }];
                
                BOOL moveFiles = NO;
                for (int i = 0; i <= ((int)[backups count]) - numberOfBackups; i++) {
                    NSString* backupPath = [autosavePath stringByAppendingPathComponent:[backups objectAtIndex:0]];
                    if (![fileManager removeItemAtPath:backupPath error:NULL]) {
                        NSLog(@"could not delete autosave file '%@'", backupPath);
                        return;
                    }
                    
                    [backups removeObjectAtIndex:0];
                    moveFiles = YES;
                }
                
                if (moveFiles) {
                    for (int i = 0; i < [backups count]; i++) {
                        NSString* backupPath = [autosavePath stringByAppendingPathComponent:[backups objectAtIndex:i]];
                        NSString* newPath = [[autosavePath stringByAppendingPathComponent:[NSString stringWithFormat:@"%@ %i", mapBasename, i + 1]] stringByAppendingPathExtension:@"map"];
                        
                        if (![fileManager moveItemAtPath:backupPath toPath:newPath error:NULL]) {
                            NSLog(@"could not move autosave file '%@' to '%@'", backupPath, newPath);
                            return;
                        }
                    }
                }
                
                backupNo =  [backups count] + 1;
            }
            
            [backups release];
            
            NSString* backupBasename = [NSString stringWithFormat:@"%@ %i", mapBasename, backupNo];
            NSString* backupPath = [[autosavePath stringByAppendingPathComponent:backupBasename] stringByAppendingPathExtension:@"map"];
            
            NSLog(@"Autosaving to file '%@'", backupPath);
            
            MapWriter* mapWriter = [[MapWriter alloc] initWithMap:map];
            [mapWriter writeToFileAtPath:backupPath];
            [mapWriter release];
            
            lastSave = [[NSDate date] retain];
        }
    }
    
    timer = [[NSTimer scheduledTimerWithTimeInterval:saveInterval target:self selector:@selector(saveBackup) userInfo:nil repeats:NO] retain];
}

@end

@implementation Autosaver

- (id)initWithMap:(MapDocument *)theMap saveInterval:(NSTimeInterval)theSaveInterval idleInterval:(NSTimeInterval)theIdleInterval numberOfBackups:(int)theNumberOfBackups {
    NSAssert(theMap != nil, @"map must not be nil");
    NSAssert(theSaveInterval > 0, @"save interval must be greater than 0");
    NSAssert(theIdleInterval >= 0, @"idle interval must be greater than or equal to 0");
    NSAssert(theNumberOfBackups > 0, @"number of backups must be greater than 0");
    
    if ((self = [self init])) {
        map = theMap;
        saveInterval = theSaveInterval;
        idleInterval = theIdleInterval;
        numberOfBackups = theNumberOfBackups;
        timer = [[NSTimer scheduledTimerWithTimeInterval:saveInterval target:self selector:@selector(saveBackup) userInfo:nil repeats:NO] retain];
    }
    
    return self;
}

- (void)dealloc {
    [self terminate];
    [lastSave release];
    [lastAction release];
    [super dealloc];
}

- (void)updateLastAction {
    [lastAction release];
    lastAction = [[NSDate date] retain];
}

- (void)terminate {
    if (timer != nil) {
        [timer invalidate];
        [timer release];
        timer = nil;
    }
}

@end
