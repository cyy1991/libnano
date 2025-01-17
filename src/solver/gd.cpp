#include "gd.h"

using namespace nano;

solver_gd_t::solver_gd_t() :
    solver_t(1e-1, 9e-1)
{
}

json_t solver_gd_t::config() const
{
    json_t json = solver_t::config();
    return json;
}

void solver_gd_t::config(const json_t& json)
{
    solver_t::config(json);
}

solver_state_t solver_gd_t::minimize(const solver_function_t& function, const lsearch_t& lsearch,
    const vector_t& x0) const
{
    auto cstate = solver_state_t{function, x0};
    log(cstate);

    for (int i = 0; i < max_iterations(); ++ i)
    {
        // descent direction
        cstate.d = -cstate.g;

        // line-search
        const auto iter_ok = lsearch.get(cstate);
        if (solver_t::done(function, cstate, iter_ok))
        {
            break;
        }
    }

    return cstate;
}
