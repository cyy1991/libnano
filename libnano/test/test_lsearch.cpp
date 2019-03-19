#include <iomanip>
#include <utest/utest.h>
#include <nano/solver.h>
#include <nano/numeric.h>

using namespace nano;

static auto get_lsearch(const string_t& id, const scalar_t c1 = 1e-4, const scalar_t c2 = 9e-1)
{
    auto lsearch = lsearch_strategy_t::all().get(id);
    UTEST_REQUIRE(lsearch);
    lsearch->c1(c1);
    lsearch->c2(c2);
    lsearch->max_iterations(100);
    return lsearch;
}

enum class lsearch_type
{
    backtrack = 0,
    lemarechal,
    morethuente,
    nocedalwright,
    cgdescent
};

static void test(
    const rlsearch_strategy_t& lsearch, const string_t& lsearch_id, const function_t& function,
    const lsearch_type type, const vector_t& x0, const scalar_t t0)
{
    std::stringstream stream;
    const auto old_n_failures = n_failures.load();

    auto state0 = solver_state_t{function, x0};
    state0.d = -state0.g;
    const auto epsilon = 1e-6;// todo: get the updated value of epsilon for CGDESCENT!!!

    stream
        << std::fixed << std::setprecision(16) << function.name() << " " << lsearch_id
        << ": x0=[" << state0.x.transpose() << "],t0=" << t0 << ",f0=" << state0.f << "\n";

    // log the line-search trials
    lsearch->logger([&] (const solver_state_t& state)
    {
        stream
            << "\tt=" << state.t << ",f=" << state.f << ",g=" << state.convergence_criterion()
            << ",armijo=" << state.has_armijo(state0, lsearch->c1())
            << ",wolfe=" << state.has_wolfe(state0, lsearch->c2())
            << ",swolfe=" << state.has_strong_wolfe(state0, lsearch->c2())
            << ",awolfe=" << state.has_approx_wolfe(state0, lsearch->c1(), lsearch->c2()) << ".\n";
    });

    // check the Armijo and the Wolfe-like conditions are valid after line-search
    auto state = state0;
    UTEST_CHECK(lsearch->get(state, t0));
    UTEST_CHECK(state);
    UTEST_CHECK(state0);

    switch (type)
    {
    case lsearch_type::backtrack:
        UTEST_CHECK(state.has_armijo(state0, lsearch->c1()));
        break;

    case lsearch_type::lemarechal:
        UTEST_CHECK(state.has_armijo(state0, lsearch->c1()));
        UTEST_CHECK(state.has_wolfe(state0, lsearch->c2()));
        break;

    case lsearch_type::morethuente:
        UTEST_CHECK(state.has_armijo(state0, lsearch->c1()));
        UTEST_CHECK(state.has_strong_wolfe(state0, lsearch->c2()));
        break;

    case lsearch_type::nocedalwright:
        UTEST_CHECK(state.has_armijo(state0, lsearch->c1()));
        UTEST_CHECK(state.has_strong_wolfe(state0, lsearch->c2()));
        break;

    case lsearch_type::cgdescent:
        UTEST_CHECK(
            (state.has_armijo(state0, lsearch->c1()) && state.has_wolfe(state0, lsearch->c1())) ||
            (state.has_approx_armijo(state0, epsilon) && state.has_approx_wolfe(state0, lsearch->c1(), lsearch->c2())));
        break;

    default:
        break;
    }

    if (old_n_failures != n_failures.load())
    {
        std::cout << stream.str();
    }
}

static void test(
    const rlsearch_strategy_t& lsearch, const string_t& lsearch_id, const function_t& function, const lsearch_type type)
{
    for (auto i = 0; i < 20; ++ i)
    {
        const scalar_t stpmin = lsearch_strategy_t::stpmin();
        const scalar_t stpmax = lsearch_strategy_t::stpmax();
        const scalar_t t0 = nano::clamp(std::pow(3.0, 2.0 - i / 5.0), stpmin, stpmax);
        const vector_t x0 = vector_t::Random(function.size());

        test(lsearch, lsearch_id, function, type, x0, t0);
    }
}

UTEST_BEGIN_MODULE(test_lsearch)

UTEST_CASE(backtrack)
{
    const auto lsearch_id = "backtrack";
    const auto lsearch = get_lsearch(lsearch_id);

    for (const auto& function : get_functions(1, 16, std::regex(".+")))
    {
        test(lsearch, lsearch_id, *function, lsearch_type::backtrack);
    }
}

UTEST_CASE(lemarechal)
{
    const auto lsearch_id = "lemarechal";
    const auto lsearch = get_lsearch(lsearch_id);

    for (const auto& function : get_functions(1, 16, std::regex(".+")))
    {
        test(lsearch, lsearch_id, *function, lsearch_type::lemarechal);
    }
}

UTEST_CASE(morethuente)
{
    const auto lsearch_id = "morethuente";
    const auto lsearch = get_lsearch(lsearch_id);

    for (const auto& function : get_functions(1, 16, std::regex(".+")))
    {
        test(lsearch, lsearch_id, *function, lsearch_type::morethuente);
    }
}

UTEST_CASE(nocedalwright)
{
    const auto lsearch_id = "nocedalwright";
    const auto lsearch = get_lsearch(lsearch_id);

    for (const auto& function : get_functions(1, 16, std::regex(".+")))
    {
        test(lsearch, lsearch_id, *function, lsearch_type::nocedalwright);
    }
}

UTEST_CASE(cgdescent)
{
    const auto lsearch_id = "cgdescent";
    const auto lsearch = get_lsearch(lsearch_id);

    for (const auto& function : get_functions(1, 16, std::regex(".+")))
    {
        test(lsearch, lsearch_id, *function, lsearch_type::cgdescent);
    }
}

UTEST_END_MODULE()
