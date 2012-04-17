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

#ifndef TrenchBroom_substream_h
#define TrenchBroom_substream_h

#include <streambuf>

using namespace std;

namespace TrenchBroom {
    namespace IO {
        class substreambuf : public streambuf {
        public:
            substreambuf(streambuf *sbuf, int pos, int len) 
            : m_sbuf(sbuf), m_pos(pos), m_len(len), m_read(0) {
                m_sbuf->pubseekpos(pos);
                setbuf(NULL,0);
            }
            
        protected:
            int underflow() {
                if (m_read >= m_len)
                    return traits_type::eof();
                m_read += 1;
                return m_sbuf->sgetc();
            }
            
            int uflow() {
                if (m_read >= m_len)
                    return traits_type::eof();
                m_read += 1;
                return m_sbuf->sbumpc();
            }
            
            streampos seekoff (streamoff off, ios_base::seekdir way, ios_base::openmode which = ios_base::in | ios_base::out) {
                if (way == ios_base::beg) {
                    m_read = off;
                    off += m_pos;
                } else if (way == ios_base::cur) {
                    m_read += off;
                    off = m_read;
                    off += m_pos;
                } else if (way == ios_base::end) {
                    off += m_pos;
                    off += m_len;
                    m_read = 0;
                }
                
                return m_sbuf->pubseekpos(off,which)-m_pos;
            }
            
            streampos seekpos (streampos sp, ios_base::openmode which = ios_base::in | ios_base::out) {
                sp += m_pos;
                if (sp - m_pos > m_len)
                    return -1;
                
                return m_sbuf->pubseekpos(sp,which)-m_pos;
            }
            
        private:
            streambuf *m_sbuf;
            streampos m_pos;
            streamsize m_len;
            streampos m_read;
        };
        
        class isubstream : public istream {
        public:
            isubstream(streambuf* sbuf) : istream(sbuf), m_sbuf(sbuf) { this->init(sbuf); };
            ~isubstream() { delete m_sbuf; }
        private:
            streambuf* m_sbuf;
        };
    }
}
#endif
