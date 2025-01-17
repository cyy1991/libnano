#pragma once

#include <nano/arch.h>
#include <nano/json.h>
#include <nano/factory.h>
#include <nano/lsearch/step.h>
#include <nano/solver/state.h>

namespace nano
{
    class lsearch0_t;
    using lsearch0_factory_t = factory_t<lsearch0_t>;
    using rlsearch0_t = lsearch0_factory_t::trobject;

    ///
    /// \brief estimate the initial step length of the line-search procedure.
    ///     see "Numerical optimization", Nocedal & Wright, 2nd edition, p.59
    ///     see "Practical methods of optimization", Fletcher, chapter 2
    ///
    class NANO_PUBLIC lsearch0_t : public json_configurable_t
    {
    public:

        ///
        /// logging operator: op(solver_state, proposed_line_search_step_length_length).
        ///
        using logger_t = std::function<void(const solver_state_t&, const scalar_t)>;

        ///
        /// \brief constructor
        ///
        lsearch0_t() = default;

        ///
        /// \brief returns the available implementations
        ///
        static lsearch0_factory_t& all();

        ///
        /// \brief returns the initial step length given the current state
        /// NB: may keep track of the previous states
        ///
        virtual scalar_t get(const solver_state_t& state) = 0;

        ///
        /// \brief change parameters
        ///
        void logger(const logger_t& logger) { m_logger = logger; }
        void epsilon(const scalar_t epsilon) { m_epsilon = epsilon; }

        ///
        /// \brief access functions
        ///
        auto epsilon() const { return m_epsilon; }

    protected:

        ///
        /// \brief log the current line-search trial length (if the logger is provided)
        ///
        void log(const solver_state_t& state, const scalar_t t) const
        {
            if (m_logger)
            {
                m_logger(state, t);
            }
        }

    private:

        // attributes
        logger_t    m_logger;           ///<
        scalar_t    m_epsilon{0};       ///< tolerance of the convergence criterion
    };
}
