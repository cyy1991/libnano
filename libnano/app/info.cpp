#include <fstream>
#include <iostream>
#include <nano/table.h>
#include <nano/logger.h>
#include <nano/cmdline.h>
#include <nano/solver.h>
#include <nano/version.h>

using namespace nano;

namespace
{
    template <typename tobject>
    void print(const string_t& name, const factory_t<tobject>& factory, const string_t& regex)
    {
        table_t table;
        table.header() << name << "description" << "configuration";
        table.delim();
        for (const auto& id : factory.ids(std::regex(regex)))
        {
            const auto json = factory.get(id)->config();
            const auto path = name + "_" + id + ".json";

            std::ofstream out(path);
            if (out.is_open())
            {
                out << json.dump(4) << std::endl;
            }

            table.append() << id << factory.description(id) << path;

        }
        std::cout << table;
    }
}

static int unsafe_main(int argc, const char* argv[])
{
    // parse the command line
    cmdline_t cmdline("display the registered objects");
    cmdline.add("", "lsearch0",             "regex to select the line-search initialization methods", ".+");
    cmdline.add("", "lsearchk",             "regex to select the line-search strategies", ".+");
    cmdline.add("", "solver",               "regex to select the numerical optimization methods", ".+");
    cmdline.add("", "version",              "library version");
    cmdline.add("", "git-hash",             "git commit hash");
    cmdline.add("", "system",               "system: all available information");
    cmdline.add("", "sys-logical-cpus",     "system: number of logical cpus");
    cmdline.add("", "sys-physical-cpus",    "system: number of physical cpus");
    cmdline.add("", "sys-memsize",          "system: memory size in GB");

    cmdline.process(argc, argv);

    const auto has_lsearch0 = cmdline.has("lsearch0");
    const auto has_lsearchk = cmdline.has("lsearchk");
    const auto has_solver = cmdline.has("solver");
    const auto has_system = cmdline.has("system");
    const auto has_sys_logical = cmdline.has("sys-logical-cpus");
    const auto has_sys_physical = cmdline.has("sys-physical-cpus");
    const auto has_sys_memsize = cmdline.has("sys-memsize");
    const auto has_version = cmdline.has("version");
    const auto has_git_hash = cmdline.has("git-hash");

    if (!has_lsearch0 &&
        !has_lsearchk &&
        !has_solver &&
        !has_system &&
        !has_sys_logical &&
        !has_sys_physical &&
        !has_sys_memsize &&
        !has_version &&
        !has_git_hash)
    {
        cmdline.usage();
        return EXIT_FAILURE;
    }

    // check arguments and options
    if (has_lsearch0)
    {
        print("lsearch0", lsearch0_t::all(), cmdline.get<string_t>("lsearch0"));
    }
    if (has_lsearchk)
    {
        print("lsearchk", lsearchk_t::all(), cmdline.get<string_t>("lsearchk"));
    }
    if (has_solver)
    {
        print("solver", solver_t::all(), cmdline.get<string_t>("solver"));
    }
    if (has_system || has_sys_physical)
    {
        std::cout << "physical CPUs..." << nano::physical_cpus() << std::endl;
    }
    if (has_system || has_sys_logical)
    {
        std::cout << "logical CPUs...." << nano::logical_cpus() << std::endl;
    }
    if (has_system || has_sys_memsize)
    {
        std::cout << "memsize........." << nano::memsize_gb() << "GB" << std::endl;
    }
    if (has_version)
    {
        std::cout << nano::major_version << "." << nano::minor_version << std::endl;
    }
    if (has_git_hash)
    {
        std::cout << nano::git_commit_hash << std::endl;
    }

    // OK
    return EXIT_SUCCESS;
}

int main(int argc, const char* argv[])
{
    return nano::main(unsafe_main, argc, argv);
}
