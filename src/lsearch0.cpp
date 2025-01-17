#include <mutex>
#include "lsearch0/linear.h"
#include "lsearch0/constant.h"
#include "lsearch0/cgdescent.h"
#include "lsearch0/quadratic.h"

using namespace nano;

lsearch0_factory_t& lsearch0_t::all()
{
    static lsearch0_factory_t manager;

    static std::once_flag flag;
    std::call_once(flag, [] ()
    {
        manager.add<lsearch0_linear_t>("linear", "linearly interpolate the previous line-search step length");
        manager.add<lsearch0_constant_t>("constant", "constant line-search step length");
        manager.add<lsearch0_quadratic_t>("quadratic", "quadratically interpolate the previous line-search step length");
        manager.add<lsearch0_cgdescent_t>("cgdescent", "the initial line-search step length described in CG-DESCENT");
    });

    return manager;
}
