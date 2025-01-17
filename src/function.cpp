#include <nano/function.h>
#include <nano/function/trid.h>
#include <nano/function/qing.h>
#include <nano/function/cauchy.h>
#include <nano/function/sphere.h>
#include <nano/function/powell.h>
#include <nano/function/sargan.h>
#include <nano/function/zakharov.h>
#include <nano/function/quadratic.h>
#include <nano/function/rosenbrock.h>
#include <nano/function/exponential.h>
#include <nano/function/dixon_price.h>
#include <nano/function/axis_ellipsoid.h>
#include <nano/function/chung_reynolds.h>
#include <nano/function/styblinski_tang.h>
#include <nano/function/rotated_ellipsoid.h>
#include <nano/function/schumer_steiglitz.h>
#include <nano/function/geometric_optimization.h>

using namespace nano;

function_t::function_t(const char* name, const tensor_size_t size, const convexity convex) :
    m_name(name),
    m_size(size),
    m_convex(convex)
{
}

function_t::function_t(string_t name, const tensor_size_t size, const convexity convex) :
    m_name(std::move(name)),
    m_size(size),
    m_convex(convex)
{
}

scalar_t function_t::grad_accuracy(const vector_t& x) const
{
    assert(x.size() == size());

    const auto n = size();

    vector_t gx(n);
    vector_t gx_approx(n);
    vector_t xp = x, xn = x;

    // analytical gradient
    const auto fx = vgrad(x, &gx);
    assert(gx.size() == size());

    // finite-difference approximated gradient
    //      see "Numerical optimization", Nocedal & Wright, 2nd edition, p.197
    const auto finite_difference = [&] (const scalar_t dx)
    {
        for (auto i = 0; i < n; i ++)
        {
            xp = x; xp(i) += dx * (1 + std::fabs(x(i)));
            xn = x; xn(i) -= dx * (1 + std::fabs(x(i)));

            const auto dfi = vgrad(xp, nullptr) - vgrad(xn, nullptr);
            const auto dxi = xp(i) - xn(i);
            gx_approx(i) = dfi / dxi;

            assert(std::isfinite(gx(i)));
            assert(std::isfinite(gx_approx(i)));
        }

        return (gx - gx_approx).lpNorm<Eigen::Infinity>() / (1 + std::fabs(fx));
    };

    return finite_difference(epsilon2<scalar_t>());
}

bool function_t::is_convex(const vector_t& x1, const vector_t& x2, const int steps) const
{
    assert(steps > 2);
    assert(x1.size() == size());
    assert(x2.size() == size());

    const auto f1 = vgrad(x1, nullptr);
    assert(std::isfinite(f1));

    const auto f2 = vgrad(x2, nullptr);
    assert(std::isfinite(f2));

    for (int step = 1; step < steps; step ++)
    {
        const auto t1 = scalar_t(step) / scalar_t(steps);
        const auto t2 = scalar_t(1) - t1;
        const auto ftc = t1 * f1 + t2 * f2;

        const auto ft = vgrad(t1 * x1 + t2 * x2, nullptr);
        if (std::isfinite(ft) && ft > ftc + epsilon0<scalar_t>())
        {
            return false;
        }
    }

    return true;
}

string_t function_t::name() const
{
    return string_t(m_name) + "[" + std::to_string(size()) + "D]";
}

static void append(rfunction_t&& func, const std::regex& regex, rfunctions_t& funcs)
{
    if (std::regex_match(func->name(), regex))
    {
        funcs.push_back(std::move(func));
    }
}

rfunctions_t nano::get_functions(const tensor_size_t min_size, const tensor_size_t max_size, const std::regex& regex)
{
    assert(min_size >= 1);
    assert(min_size <= max_size);

    rfunctions_t funcs;
    for (tensor_size_t dims = min_size; dims <= max_size; )
    {
        append(std::make_unique<function_trid_t>(dims), regex, funcs);
        append(std::make_unique<function_qing_t>(dims), regex, funcs);
        append(std::make_unique<function_cauchy_t>(dims), regex, funcs);
        append(std::make_unique<function_sargan_t>(dims), regex, funcs);
        if (dims % 4 == 0)
        {
            append(std::make_unique<function_powell_t>(dims), regex, funcs);
        }
        append(std::make_unique<function_sphere_t>(dims), regex, funcs);
        append(std::make_unique<function_zakharov_t>(dims), regex, funcs);
        append(std::make_unique<function_quadratic_t>(dims), regex, funcs);
        if (dims > 1)
        {
            append(std::make_unique<function_rosenbrock_t>(dims), regex, funcs);
        }
        append(std::make_unique<function_exponential_t>(dims), regex, funcs);
        append(std::make_unique<function_dixon_price_t>(dims), regex, funcs);
        append(std::make_unique<function_chung_reynolds_t>(dims), regex, funcs);
        append(std::make_unique<function_axis_ellipsoid_t>(dims), regex, funcs);
        append(std::make_unique<function_styblinski_tang_t>(dims), regex, funcs);
        append(std::make_unique<function_schumer_steiglitz_t>(dims), regex, funcs);
        append(std::make_unique<function_rotated_ellipsoid_t>(dims), regex, funcs);
        append(std::make_unique<function_geometric_optimization_t>(dims), regex, funcs);

        if (dims < 4)
        {
            ++ dims;
        }
        else
        {
            dims *= 2;
        }
    }

    return funcs;
}

rfunctions_t nano::get_convex_functions(const tensor_size_t min_size, const tensor_size_t max_size, const std::regex& regex)
{
    auto funcs = get_functions(min_size, max_size, regex);

    funcs.erase(
        std::remove_if(funcs.begin(), funcs.end(), [] (const auto& func) { return !func->is_convex(); }),
        funcs.end());

    return funcs;
}
