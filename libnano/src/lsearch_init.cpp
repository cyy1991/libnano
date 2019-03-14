#include <mutex>
#include "lsearch/init.h"

using namespace nano;

lsearch_init_factory_t& nano::get_lsearch_inits()
{
    static lsearch_init_factory_t manager;

    static std::once_flag flag;
    std::call_once(flag, [] ()
    {
        manager.add<lsearch_unit_init_t>("unit", "unit line-search step length");
        manager.add<lsearch_linear_init_t>("linear", "linearly interpolate the previous line-search step");
        manager.add<lsearch_quadratic_init_t>("quadratic", "quadratically interpolate the previous line-search step");
        manager.add<lsearch_cgdescent_init_t>("cgdescent", "the initial line-search step length described in CG-DESCENT");
    });

    return manager;
}