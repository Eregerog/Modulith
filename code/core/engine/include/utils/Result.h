/*
 * @brief
 * @author Daniel Götz
 */

# pragma once

#include "Core.h"

namespace modulith {

    /**
     * A class that represents an error whose error message can be displayed to the user
     */
    class ENGINE_API Error {

    public:
        /**
         * Creates an error from a string, which is the error message / reason for the error
         */
        explicit Error(std::string reason) : _reason(std::move(reason)) {}

        /**
         * @return Returns the error's error message / reason
         */
        [[nodiscard]] const std::string& Reason() const { return _reason; }
    private:
        std::string _reason;
    };

    /**
     * An immutable object that either represents a result of a template-typed value or an error
     * @tparam T The type of the result value
     */
    template<class T>
    class ENGINE_API Result {
    public:
        /**
         * Creates a result from the given value
         * @param value
         */
        Result(T value) {
            _underlying = value;
        }

        /**
         * Creates a result from the given error
         */
        Result(Error error) {
            _underlying = error;
        }

        /**
         * @return Returns true if the result's underlying value is not an error
         */
        [[nodiscard]] bool HasResult() const { return std::holds_alternative<T>(_underlying); }

        /**
         * @return Returns the result's underlying value if it is not an error. Throws otherwise.
         * @remark {@link HasResult()} can be used to check if this object has a result value
         */
        const T& GetResult() const { return std::get<T>(_underlying); }

        /**
         * @see GetResult()
         */
        const T* operator->() const { return &std::get<T>(_underlying); }

        /**
         * @return Returns true if the result's underlying value is an error
         */
        [[nodiscard]] bool HasError() const { return std::holds_alternative<Error>(_underlying); }

        /**
         * @return Returns the result's underlying error if it has one. Throws otherwise.
         * @remark {@link HasError()} can be used to check if this object has an error
         */
        [[nodiscard]] const Error& GetError() const { return std::get<Error>(_underlying); }

        /**
         * @return An optional. If this object has a value, the optional will be returned with a value. Otherwise, the optional will be {@link std::nullopt}
         */
        std::optional<T> AsOptional() const { if (HasResult()) return std::get<T>(_underlying); else return std::nullopt; };

        /**
         * This object can be used as a boolean to check if its underlying value is a result
         * @see HasResult()
         */
        constexpr explicit operator bool() const { return HasResult(); }
    private:
        std::variant<T, Error> _underlying;
    };
}