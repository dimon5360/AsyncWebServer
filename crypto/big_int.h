/*********************************************
 *
 *
 */
#pragma once

#include <vector>

namespace bigint {
    
    class bigint_t {

    private:
        std::vector<int32_t> value_;


    public:

        bigint_t();
        bigint_t(const bigint_t& value);

        /*bigint_t operator+(bigint_t const&) const;
        bigint_t &operator+=(bigint_t const&);

        bigint_t operator-(bigint_t const&) const;
        bigint_t &operator-=(bigint_t const&);

        bigint_t operator*(bigint_t const&) const;
        bigint_t& operator*=(bigint_t const&);

        bigint_t operator*(bigint_t const&) const;
        bigint_t& operator*=(bigint_t const&);*/

    };
}