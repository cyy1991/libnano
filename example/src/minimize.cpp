#include <iomanip>
#include <iostream>
#include <nano/solver.h>

class objective_t final : public nano::function_t
{
public:

    objective_t(const int size) :
        nano::function_t("objective's name", size, nano::convexity::yes),
        m_b(nano::vector_t::Random(size))
    {
    }

    nano::scalar_t vgrad(const nano::vector_t& x, nano::vector_t* gx) const override
    {
        assert(size() == x.size());
        assert(size() == m_b.size());

        const auto dx = 1 + (x - m_b).dot(x - m_b) / 2;

        if (gx)
        {
            *gx = (x - m_b) / dx;
        }

        return std::log(dx);
    }

    const auto& b() const { return m_b; }

private:

    // attributes
    nano::vector_t  m_b;
};

int main(const int, char* argv[])
{
    // construct an objective function
    const auto objective = objective_t{13};

    // check the objective function's gradient using central finite-difference
    const auto trials = 10;
    for (auto trial = 0; trial < 10; ++ trial)
    {
        const auto x0 = nano::vector_t::Random(objective.size());

        std::cout << std::fixed << std::setprecision(12)
            << "check_grad[" << (trial + 1) << "/" << trials
            << "]: dg=" << objective.grad_accuracy(x0) << std::endl;
    }
    std::cout << std::endl;

    // construct a solver_t object to minimize the objective function
    const auto solver = nano::solver_t::all().get("lbfgs");
    // NB: this may be not needed as the default configuration will minimize the objective as well!
    const auto history = 6;
    const auto epsilon = 1e-6;
    const auto max_iterations = 100;
    solver->config(nano::to_json(
        "c1", 1e-4,
        "c2", 9e-1,
        "history", history,
        "eps", epsilon,
        "maxit", max_iterations));

    // minimize starting from various random points
    for (auto trial = 0; trial < trials; ++ trial)
    {
        const auto x0 = nano::vector_t::Random(objective.size());

        std::cout << std::fixed << std::setprecision(12)
            << "minimize[" << (trial + 1) << "/" << trials
            << "]: f0=" << objective.vgrad(x0, nullptr)
            << "...\n";

        // log the optimization steps
        solver->logger([&] (const nano::solver_state_t& state)
        {
            std::cout
                << "\tdescent: i=" << state.m_iterations
                << ",f=" << state.f << ",g=" << state.convergence_criterion()
                << "[" << to_string(state.m_status) << "]" << ",calls=" << state.m_fcalls << "/" << state.m_gcalls
                << ".\n";
            return true;
        });

        // log the line-search steps
        solver->lsearch0_logger([&] (const nano::solver_state_t& state0, const nano::scalar_t t0)
        {
            std::cout
                << "\t\tlsearch(0): t=" << state0.t << ",f=" << state0.f << ",g=" << state0.convergence_criterion()
                << ",t0=" << t0 <<".\n";
        });

        solver->lsearchk_logger([&] (const nano::solver_state_t& state0, const nano::solver_state_t& state)
        {
            std::cout
                << "\t\tlsearch(t): t=" << state.t << ",f=" << state.f << ",g=" << state.convergence_criterion()
                << ",armijo=" << state.has_armijo(state0, solver->c1())
                << ",wolfe=" << state.has_wolfe(state0, solver->c2())
                << ",swolfe=" << state.has_strong_wolfe(state0, solver->c2()) << ".\n";
        });

        const auto state = solver->minimize(objective, x0);

        std::cout << std::fixed << std::setprecision(12)
            << "minimize[" << (trial + 1) << "/" << trials
            << "]: f0=" << objective.vgrad(x0, nullptr)
            << ", f=" << state.f
            << ", g=" << state.convergence_criterion()
            << ", x-x*=" << (state.x - objective.b()).lpNorm<Eigen::Infinity>()
            << ", iters=" << state.m_iterations
            << ", fcalls=" << state.m_fcalls
            << ", gcalls=" << state.m_gcalls
            << ", status=" << nano::to_string(state.m_status)
            << "\n" << std::endl;
    }

    return EXIT_SUCCESS;
}
