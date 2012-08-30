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

#ifndef TrenchBroom_mmapped_fstream_h
#define TrenchBroom_mmapped_fstream_h

#include <cassert>
#include <iostream>
#include <functional>

class mmapped_streambuf : public std::streambuf {
private:
    const char* const begin_;
    const char* const end_;
    const char* current_;
    
    std::streampos seekoff (std::streamoff off, std::ios_base::seekdir way, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) {
        if (way == std::ios_base::beg) {
            if (begin_ + off > end_)
                return -1;
            current_ = begin_ + off;
            return current_ - begin_;
        } else if (way == std::ios_base::cur) {
            if (current_ + off > end_)
                return -1;
            current_ += off;
            return current_ - begin_;
        } else if (way == std::ios_base::end) {
            if (end_ - off < begin_)
                return -1;
            current_ = end_ - off;
            return current_ - begin_;
        } else {
            return -1;
        }
    }
    
    std::streampos seekpos (std::streampos sp, std::ios_base::openmode which = std::ios_base::in | std::ios_base::out) {
        if (begin_ + sp > end_)
            return -1;
        current_ = begin_ + sp;
        return current_ - begin_;
    }
    
    int_type underflow() {
        if (current_ == end_)
            return traits_type::eof();
        
        return traits_type::to_int_type(*current_);
    }
    
    int_type uflow() {
        if (current_ == end_)
            return traits_type::eof();
        
        return traits_type::to_int_type(*current_++);
    }
    
    int_type pbackfail(int_type ch) {
        if (current_ == begin_ || (ch != traits_type::eof() && ch != current_[-1]))
            return traits_type::eof();
        
        return traits_type::to_int_type(*--current_);
    }
    
    std::streamsize showmanyc() {
        assert(std::less_equal<const char *>()(current_, end_));
        return end_ - current_;
    }
    
    // copy ctor and assignment not implemented;
    // copying not allowed
    mmapped_streambuf(const mmapped_streambuf &);
    mmapped_streambuf &operator= (const mmapped_streambuf &);
public:
    mmapped_streambuf(const char* begin, const char* end) :
    begin_(begin),
    end_(end),
    current_(begin_) {
        assert(std::less_equal<const char *>()(begin_, end_));
    }
};

#if defined _WIN32
#include "mmapped_fstream_win32.h"
typedef mmapped_fstream_win32 mmapped_fstream;
#elif defined __APPLE__
#include "mmapped_fstream_posix.h"
typedef mmapped_fstream_posix mmapped_fstream;
#elif defined __linux__

#endif

#endif
