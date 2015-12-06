/*
 Copyright (C) 2010-2014 Kristian Duske
 
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
 along with TrenchBroom. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TrenchBroom_Reference_h
#define TrenchBroom_Reference_h

#include "SharedPointer.h"

namespace TrenchBroom {
    template <typename T> class TypedReference;
    class UntypedReference;
    
    namespace Reference {
        class Holder {
        public:
            typedef std::tr1::shared_ptr<Holder> Ptr;
            virtual ~Holder() {}
        };

        template <typename T> TypedReference<T> swap(T& value);
        template <typename T> TypedReference<T> copy(const T& value);
        template <typename T> TypedReference<T> ref(T& value);
    }
    
    
    template <typename T>
    class TypedReference {
    private:
        friend TypedReference<T> Reference::swap<T>(T& value);
        friend TypedReference<T> Reference::copy<T>(const T& value);
        friend TypedReference<T> Reference::ref<T>(T& value);
    private:
        class TypedHolder : public Reference::Holder {
        public:
            virtual ~TypedHolder() {}
            
            virtual T& get() = 0;
            virtual const T& get() const = 0;
        };
        
        class SwapHolder : public TypedHolder {
        private:
            T m_value;
        public:
            SwapHolder(T& value) {
                using std::swap;
                swap(value, m_value);
            }
            
            T& get() { return m_value; }
            const T& get() const { return m_value; }
        };
        
        class CopyHolder : public TypedHolder {
        private:
            T m_value;
        public:
            CopyHolder(const T& value) : m_value(value) {}
            T& get() { return m_value; }
            const T& get() const { return m_value; }
        };
        
        class RefHolder : public TypedHolder {
        private:
            T& m_value;
        public:
            RefHolder(T& value) : m_value(value) {}
            T& get() { return m_value; }
            const T& get() const { return m_value; }
        };
    private:
        friend class UntypedReference;
        Reference::Holder::Ptr m_holder;
    public:
        TypedReference(Reference::Holder::Ptr holder) : m_holder(holder) {}
        TypedReference(const UntypedReference& reference);
        TypedReference& operator=(const UntypedReference& reference);
        
        T& get() { return static_cast<TypedHolder*>(m_holder.get())->get(); }
        const T& get() const { return static_cast<TypedHolder*>(m_holder.get())->get(); }
    };
    
    namespace Reference {
        template <typename T>
        TypedReference<T> swap(T& value) {
            return TypedReference<T>(Holder::Ptr(new typename TypedReference<T>::SwapHolder(value)));
        }
        
        template <typename T>
        TypedReference<T> copy(const T& value) {
            return TypedReference<T>(Holder::Ptr(new typename TypedReference<T>::CopyHolder(value)));
        }
        
        template <typename T>
        TypedReference<T> ref(T& value) {
            return TypedReference<T>(Holder::Ptr(new typename TypedReference<T>::RefHolder(value)));
        }
    }
    
    class UntypedReference {
    private:
        template <typename> friend class TypedReference;
        Reference::Holder::Ptr m_holder;
    public:
        template <typename T>
        UntypedReference(const TypedReference<T>& reference) : m_holder(reference.m_holder) {}
        
        template <typename T>
        UntypedReference& operator=(const TypedReference<T>& reference) { m_holder = reference.m_holder; return *this; }
    };
    
    template <typename T> TypedReference<T>::TypedReference(const UntypedReference& reference) { m_holder = reference.m_holder; }
    template <typename T> TypedReference<T>& TypedReference<T>::operator=(const UntypedReference& reference) { m_holder = reference.m_holder; return *this; }
}

#endif
