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

#import "MacFileManager.h"
#include "NSString+StdStringAdditions.h"

#import "Utilities/Utils.h"

#import <cassert>

namespace TrenchBroom {
    namespace IO {
        bool MacFileManager::isDirectory(const std::string& path) {
            NSFileManager* fileManager = [NSFileManager defaultManager];
            
            BOOL directory = false;
            BOOL exists = [fileManager fileExistsAtPath:[NSString stringWithStdString:path] isDirectory:&directory];
            
            return exists && directory;
        }
        
        bool MacFileManager::exists(const std::string& path) {
            NSFileManager* fileManager = [NSFileManager defaultManager];
            return [fileManager fileExistsAtPath:[NSString stringWithStdString:path]];
        }
        
        bool MacFileManager::makeDirectory(const std::string& path) {
            NSString* objcPath = [NSString stringWithStdString:path];
            NSFileManager* fileManager = [NSFileManager defaultManager];

            if ([fileManager fileExistsAtPath:objcPath])
                return false;
            
            return [fileManager createDirectoryAtPath:objcPath withIntermediateDirectories:YES attributes:nil error:NULL];
        }

        bool MacFileManager::deleteFile(const std::string& path) {
            NSFileManager* fileManager = [NSFileManager defaultManager];
            NSString* objcPath = [NSString stringWithStdString:path];
            
            BOOL directory = false;
            BOOL exists = [fileManager fileExistsAtPath:objcPath isDirectory:&directory];
            if (!exists || directory)
                return false;
            
            return [fileManager removeItemAtPath:objcPath error:NULL];
        }

        bool MacFileManager::moveFile(const std::string& sourcePath, const std::string& destPath, bool overwrite) {
            NSFileManager* fileManager = [NSFileManager defaultManager];
            NSString* objcSourcePath = [NSString stringWithStdString:sourcePath];
            NSString* objcDestPath = [NSString stringWithStdString:destPath];

            BOOL directory = false;
            BOOL exists = [fileManager fileExistsAtPath:objcSourcePath isDirectory:&directory];
            if (!exists || directory) {
                std::stringstream msg;
                msg << "Cannot move file at location " << sourcePath << " because it does not exist or is a directory";
                throw FileManagerException(msg);
            }
            
            exists = [fileManager fileExistsAtPath:objcDestPath isDirectory:&directory];
            if (exists) {
                if (!overwrite || directory)
                    return false;
                else if (![fileManager removeItemAtPath:objcDestPath error:NULL])
                    return false;
            }
            
            return [fileManager moveItemAtPath:objcSourcePath toPath:objcDestPath error:NULL];
        }

        std::vector<std::string> MacFileManager::directoryContents(const std::string& path, std::string extension) {
            std::vector<std::string> result;
            
            NSFileManager* fileManager = [NSFileManager defaultManager];
            NSArray* entries = [fileManager contentsOfDirectoryAtPath:[NSString stringWithStdString:path] error:NULL];
            
            if (entries != nil) {
                std::string extensionLower = toLower(extension);
                for (NSString* entry in entries) {
                    std::string entryName = [entry stdString];
                    if (extension.empty() || toLower(pathExtension(entryName)) == extensionLower)
                        result.push_back(pathComponents(entryName).back());
                }
            }
            
            return result;
        }
    }
}