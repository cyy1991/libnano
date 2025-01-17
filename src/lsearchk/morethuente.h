#pragma once

#include <nano/lsearch/lsearchk.h>

namespace nano
{
    ///
    /// \brief More & Thunte line-search.
    ///     see "Line Search Algorithms with Guaranteed Sufficient Decrease",
    ///     by Jorge J. More and David J. Thuente
    ///
    /// NB: this implementation ports the 'dcsrch' and the 'dcstep' Fortran routines from MINPACK-2.
    ///     see http://ftp.mcs.anl.gov/pub/MINPACK-2/csrch/
    ///
    class lsearchk_morethuente_t final : public lsearchk_t
    {
    public:

        lsearchk_morethuente_t() = default;

        json_t config() const final;
        void config(const json_t&) final;
        bool get(const solver_state_t& state0, solver_state_t& state) final;
    };
}
