/*
 Copyright 2010-2019 Kristian Duske

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
 documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
 rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 persons to whom the Software is furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
 Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef KDL_SET_TEMP_H
#define KDL_SET_TEMP_H

#include <utility>
#include <type_traits>

namespace kdl {
    /**
     * RAII class that temporarily sets a value and resets it to its original value when going out of scope.
     *
     * @tparam T the type of the value to set
     */
    template <typename T>
    class set_temp {
    private:
        T& m_value;
        T m_oldValue;
    public:
        /**
         * Sets the given value to the given new value.
         *
         * @param value the value to change
         * @param newValue the new value to set temporarily
         */
        set_temp(T& value, T newValue) :
        m_value(value),
        m_oldValue(std::move(m_value)) {
            m_value = std::move(newValue);
        }

        /**
         * Constructor for setting boolean values to true, this is only enabled if T = bool.
         *
         * @tparam B the type of the value to set (must be bool)
         * @param value the value to set to true temporarily
         */
        template <typename B, typename std::enable_if_t<std::is_same_v<B, bool>>* = nullptr>
        explicit set_temp(B& value) :
        set_temp(value, true) {}

        /**
         * Resets the original value.
         */
        ~set_temp() {
            m_value = std::move(m_oldValue);
        }
    };

    template <typename B>
    set_temp(B& value) -> set_temp<bool>;

    /**
     * RAII class that sets a value when going out of scope.
     *
     * @tparam T the type of the value to set
     */
    template <typename T>
    class set_later {
    private:
        T& m_value;
        T m_newValue;
    public:
        /**
         * Creates a new instance, but doesn't change any values.
         *
         * @param value the value to change later
         * @param newValue the value to set
         */
        set_later(T& value, T newValue) :
        m_value(value),
        m_newValue(std::move(newValue)) {}

        /**
         * Sets the new value.
         */
        ~set_later() {
            m_value = std::move(m_newValue);
        }
    };

    template <typename T>
    set_later(T& value, T newValue) -> set_later<T>;

    /**
     * RAII class that increases a value and decreases it again when going out of scope.
     *
     * @tparam T the type of the value to change
     */
    template <typename T>
    class inc_temp {
    private:
        T& m_value;
    public:
        /**
         * Creates an instance and increases the given value by 1.
         *
         * @param value the value
         */
        explicit inc_temp(T& value) :
        m_value(value) {
            ++m_value;
        }

        /**
         * Decreases the value.
         */
        ~inc_temp() {
            --m_value;
        }
    };

    template <typename T>
    inc_temp(T& value) -> inc_temp<T>;

    /**
     * RAII class that decreases a value and increases it again when going out of scope.
     *
     * @tparam T the type of the value to change
     */
    template <typename T>
    class dec_temp {
    private:
        T& m_value;
    public:
        /**
         * Creates an instance and decreases the given value by 1.
         *
         * @param value the value
         */
        explicit dec_temp(T& value) :
        m_value(value) {
            --m_value;
        }

        /**
         * Increases the value.
         */
        ~dec_temp() {
            ++m_value;
        }
    };
    
    template <typename T>
    dec_temp(T& value) -> dec_temp<T>;
}

#endif //KDL_SET_TEMP_H
