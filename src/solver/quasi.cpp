#include "quasi.h"
#include <nano/numeric.h>

using namespace nano;

namespace
{
    template <typename tvector>
    auto SR1(const matrix_t& H, const tvector& dx, const tvector& dg)
    {
        return  H + (dx - H * dg) * (dx - H * dg).transpose() / (dx - H * dg).dot(dg);
    }

    template <typename tvector>
    void SR1(matrix_t& H, const tvector& dx, const tvector& dg, const scalar_t r)
    {
        const auto denom = (dx - H * dg).dot(dg);
        const auto apply = std::fabs(denom) >= r * dx.norm() * (dx - H * dg).norm();

        if (apply)
        {
            H = SR1(H, dx, dg);
        }
    }

    template <typename tvector>
    auto DFP(const matrix_t& H, const tvector& dx, const tvector& dg)
    {
        return  H + (dx * dx.transpose()) / dx.dot(dg) -
                (H * dg * dg.transpose() * H) / (dg.transpose() * H * dg);
    }

    template <typename tvector>
    auto BFGS(const matrix_t& H, const tvector& dx, const tvector& dg)
    {
        const auto I = matrix_t::Identity(H.rows(), H.cols());

        return  (I - dx * dg.transpose() / dx.dot(dg)) * H * (I - dg * dx.transpose() / dx.dot(dg)) +
                dx * dx.transpose() / dx.dot(dg);
    }

    template <typename tvector>
    auto HOSHINO(const matrix_t& H, const tvector& dx, const tvector& dg)
    {
        const auto phi = dx.dot(dg) / (dx.dot(dg) + dg.transpose() * H * dg);

        return  (1 - phi) * DFP(H, dx, dg) + phi * BFGS(H, dx, dg);
    }

    template <typename tvector>
    void FLETCHER(matrix_t& H, const tvector& dx, const tvector& dg)
    {
        const auto phi = dx.dot(dg) / (dx.dot(dg) - dg.transpose() * H * dg);

        if (phi < scalar_t(0))
        {
            H = DFP(H, dx, dg);
        }
        else if (phi > scalar_t(1))
        {
            H = BFGS(H, dx, dg);
        }
        else
        {
            H = SR1(H, dx, dg);
        }
    }
}

solver_quasi_t::solver_quasi_t() :
    solver_t(1e-4, 9e-1)
{
}

json_t solver_quasi_t::config() const
{
    json_t json = solver_t::config();
    json["H0"] = strcat(m_initialization, join(enum_values<initialization>()));
    return json;
}

void solver_quasi_t::config(const json_t& json)
{
    nano::from_json(json, "H0", m_initialization);
    solver_t::config(json);
}

solver_state_t solver_quasi_t::minimize(const solver_function_t& function, const lsearch_t& lsearch,
    const vector_t& x0) const
{
    auto cstate = solver_state_t{function, x0};
    auto pstate = cstate;
    log(cstate);

    // current approximation of the Hessian's inverse
    matrix_t H = matrix_t::Identity(function.size(), function.size());

    for (int i = 0; i < max_iterations(); ++ i)
    {
        // descent direction
        cstate.d = -H * cstate.g;

        // restart:
        //  - if not a descent direction
        if (!cstate.has_descent())
        {
            cstate.d = -cstate.g;
            H.setIdentity();
        }

        // line-search
        pstate = cstate;
        const auto iter_ok = lsearch.get(cstate);
        if (solver_t::done(function, cstate, iter_ok))
        {
            break;
        }

        // initialize the Hessian's inverse
        if (i == 0)
        {
            switch (m_initialization)
            {
            case initialization::scaled:
                {
                    const auto dx = cstate.x - pstate.x;
                    const auto dg = cstate.g - pstate.g;
                    H = matrix_t::Identity(H.rows(), H.cols()) * dx.dot(dg) / dg.dot(dg);
                }
                break;

            default:
                break;
            }
        }

        // update approximation of the Hessian
        update(pstate, cstate, H);
    }

    return cstate;
}

json_t solver_quasi_sr1_t::config() const
{
    json_t json = solver_t::config();
    json["r"] = strcat(m_r, "(0,1)");
    return json;
}

void solver_quasi_sr1_t::config(const json_t& json)
{
    const auto eps = epsilon0<scalar_t>();

    solver_quasi_t::config(json);
    nano::from_json_range(json, "r", m_r, eps, 1 - eps);
}

void solver_quasi_sr1_t::update(const solver_state_t& prev, const solver_state_t& curr, matrix_t& H) const
{
    ::SR1(H, curr.x - prev.x, curr.g - prev.g, m_r);
}

void solver_quasi_dfp_t::update(const solver_state_t& prev, const solver_state_t& curr, matrix_t& H) const
{
    H = ::DFP(H, curr.x - prev.x, curr.g - prev.g);
}

void solver_quasi_bfgs_t::update(const solver_state_t& prev, const solver_state_t& curr, matrix_t& H) const
{
    H = ::BFGS(H, curr.x - prev.x, curr.g - prev.g);
}

void solver_quasi_hoshino_t::update(const solver_state_t& prev, const solver_state_t& curr, matrix_t& H) const
{
    H = ::HOSHINO(H, curr.x - prev.x, curr.g - prev.g);
}

void solver_quasi_fletcher_t::update(const solver_state_t& prev, const solver_state_t& curr, matrix_t& H) const
{
    ::FLETCHER(H, curr.x - prev.x, curr.g - prev.g);
}
