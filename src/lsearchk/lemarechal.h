#pragma once

#include <nano/lsearch/lsearchk.h>

namespace nano
{
    ///
    /// \brief the line-search algorithm described here:
    ///     see "A view of line-searches", C. Lemarechal
    ///
    class lsearchk_lemarechal_t final : public lsearchk_t
    {
    public:

        lsearchk_lemarechal_t() = default;

        json_t config() const final;
        void config(const json_t&) final;
        bool get(const solver_state_t& state0, solver_state_t& state) final;

    private:

        // attributes
        interpolation   m_interpolation{interpolation::cubic};  ///<
        scalar_t        m_tau1{9};                              ///< extrapolation factor
    };
}
