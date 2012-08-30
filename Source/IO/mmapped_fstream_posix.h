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

#ifndef __TrenchBroom__mmapped_fstream_posix__
#define __TrenchBroom__mmapped_fstream_posix__

#include <cassert>
#include <iostream>
#include <functional>

#include <fcntl.h>
#include <sys/mman.h>

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
            return - 1;
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

class mmapped_fstream_posix : public std::istream {
private:
    int m_filedesc;
    void* m_address;
    size_t m_length;
    mmapped_streambuf* m_buf;

    mmapped_streambuf* makebuf(const char* filename, ios_base::openmode mode = ios_base::in | ios_base::out) {
        m_filedesc = -1;
        m_address = NULL;
        m_length = 0;
        m_buf = NULL;
        
        int flags = 0;
        int prot = 0;
        if ((mode & ios_base::in)) {
            if ((mode & ios_base::out))
                flags = O_RDWR;
            else
                flags = O_RDONLY;
            prot |= PROT_READ;
        }
        if ((mode & ios_base::out)) {
            flags = O_WRONLY;
            prot |= PROT_WRITE;
        }
        
        m_filedesc = open(filename, flags);
        if (m_filedesc >= 0) {
            m_length = lseek(m_filedesc, 0, SEEK_END);
            lseek(m_filedesc, 0, SEEK_SET);
            m_address = mmap(NULL, m_length, prot, MAP_FILE | MAP_PRIVATE, m_filedesc, 0);
            if (m_address == NULL) {
                close(m_filedesc);
                m_filedesc = -1;
            } else {
                char* begin = static_cast<char*>(m_address);
                char* end = begin + m_length;
                m_buf = new mmapped_streambuf(begin, end);
            }
        }
        
        return m_buf;
    }

    
    // copy ctor and assignment not implemented;
    // copying not allowed
    mmapped_fstream_posix(const mmapped_fstream_posix &);
    mmapped_fstream_posix &operator= (const mmapped_fstream_posix &);
public:
    mmapped_fstream_posix(const char* filename, ios_base::openmode mode = ios_base::in | ios_base::out) : std::istream(makebuf(filename, mode)) {
        init(m_buf);
    }
    
    ~mmapped_fstream_posix() {
        if (m_buf != NULL) {
            delete m_buf;
            m_buf = NULL;
        }
        
        if (m_address != NULL) {
            munmap(m_address, m_length);
            m_address = NULL;
        }
        
        if (m_filedesc >= 0) {
            close(m_filedesc);
            m_filedesc = -1;
        }
    }
    
    inline bool is_open() const {
        return m_address != NULL;
    }
};

#endif /* defined(__TrenchBroom__mmapped_fstream_posix__) */
