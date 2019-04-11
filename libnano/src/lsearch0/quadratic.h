#pragma once

#include <nano/lsearch/lsearch0.h>

namespace nano
{
    ///
    /// \brief use quadratic interpolation of the previous line-search step lengths.
    ///     see "Numerical optimization", Nocedal & Wright, 2nd edition, p.59-60
    ///     see errata: http://users.iems.northwestern.edu/~nocedal/book/2ndprint.pdf
    ///
    class lsearch0_quadratic_t final : public lsearch0_t
    {
    public:

        lsearch0_quadratic_t() = default;

        json_t config() const final;
        void config(const json_t&) final;
        scalar_t get(const solver_state_t&) final;

    private:

        // attributes
        scalar_t    m_tro{1.01};    ///<
        scalar_t    m_prevf{0};     ///< previous function value
        scalar_t    m_prevdg{1};    ///< previous direction dot product
    };
}