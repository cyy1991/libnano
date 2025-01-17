#include <sstream>
#include <utest/utest.h>
#include <nano/tensor/index.h>

using namespace nano;

template <size_t trank>
std::string dims2str(const tensor_dims_t<trank>& dims)
{
    std::stringstream str;
    str << dims;
    return str.str();
}

UTEST_BEGIN_MODULE(test_tensor_index)

UTEST_CASE(dims1d)
{
    const auto dims1 = nano::make_dims(3);
    const auto dims2 = nano::make_dims(3);
    const auto dims3 = nano::make_dims(1);

    UTEST_CHECK_EQUAL(dims1, dims1);
    UTEST_CHECK_EQUAL(dims1, dims2);
    UTEST_CHECK_NOT_EQUAL(dims2, dims3);

    UTEST_CHECK_EQUAL(dims2str(dims1), "3");
    UTEST_CHECK_EQUAL(dims2str(dims2), "3");
    UTEST_CHECK_EQUAL(dims2str(dims3), "1");
}

UTEST_CASE(dims2d)
{
    const auto dims1 = nano::make_dims(3, 7);
    const auto dims2 = nano::make_dims(7, 5);
    const auto dims3 = nano::make_dims(3, 7);

    UTEST_CHECK_EQUAL(dims1, dims1);
    UTEST_CHECK_EQUAL(dims1, dims3);
    UTEST_CHECK_NOT_EQUAL(dims1, dims2);

    UTEST_CHECK_EQUAL(dims2str(dims1), "3x7");
    UTEST_CHECK_EQUAL(dims2str(dims2), "7x5");
    UTEST_CHECK_EQUAL(dims2str(dims3), "3x7");
}

UTEST_CASE(dims3d)
{
    const auto dims1 = nano::make_dims(3, 7, 5);
    const auto dims2 = nano::make_dims(7, 5, 3);
    const auto dims3 = nano::make_dims(1, 1, 1);

    UTEST_CHECK_EQUAL(dims1, dims1);
    UTEST_CHECK_NOT_EQUAL(dims2, dims1);
    UTEST_CHECK_NOT_EQUAL(dims2, dims3);

    UTEST_CHECK_EQUAL(dims2str(dims1), "3x7x5");
    UTEST_CHECK_EQUAL(dims2str(dims2), "7x5x3");
    UTEST_CHECK_EQUAL(dims2str(dims3), "1x1x1");
}

UTEST_CASE(index1d)
{
    const auto dims = nano::make_dims(7);

    UTEST_CHECK_EQUAL(std::get<0>(dims), 7);
    UTEST_CHECK_EQUAL(nano::size(dims), 7);

    UTEST_CHECK_EQUAL(nano::index(dims, 0), 0);
    UTEST_CHECK_EQUAL(nano::index(dims, 1), 1);
    UTEST_CHECK_EQUAL(nano::index(dims, 6), 6);

    UTEST_CHECK_EQUAL(nano::index0(dims), nano::index(dims, 0));
    UTEST_CHECK_EQUAL(nano::index0(dims, 6), nano::index(dims, 6));

    UTEST_CHECK_EQUAL(nano::dims0(dims), nano::make_dims(7));
}

UTEST_CASE(index2d)
{
    const auto dims = nano::make_dims(7, 5);

    UTEST_CHECK_EQUAL(dims, nano::cat_dims(7, nano::make_dims(5)));

    UTEST_CHECK_EQUAL(std::get<0>(dims), 7);
    UTEST_CHECK_EQUAL(std::get<1>(dims), 5);
    UTEST_CHECK_EQUAL(nano::size(dims), 35);

    UTEST_CHECK_EQUAL(nano::index(dims, 0, 1), 1);
    UTEST_CHECK_EQUAL(nano::index(dims, 0, 4), 4);
    UTEST_CHECK_EQUAL(nano::index(dims, 1, 0), 5);
    UTEST_CHECK_EQUAL(nano::index(dims, 3, 2), 17);
    UTEST_CHECK_EQUAL(nano::index(dims, 6, 4), 34);

    UTEST_CHECK_EQUAL(nano::index0(dims), nano::index(dims, 0, 0));
    UTEST_CHECK_EQUAL(nano::index0(dims, 3), nano::index(dims, 3, 0));
    UTEST_CHECK_EQUAL(nano::index0(dims, 3, 1), nano::index(dims, 3, 1));

    UTEST_CHECK_EQUAL(nano::dims0(dims), nano::make_dims(7, 5));
    UTEST_CHECK_EQUAL(nano::dims0(dims, 3), nano::make_dims(5));
}

UTEST_CASE(index3d)
{
    const auto dims = nano::make_dims(3, 7, 5);

    UTEST_CHECK_EQUAL(dims, nano::cat_dims(3, nano::make_dims(7, 5)));

    UTEST_CHECK_EQUAL(std::get<0>(dims), 3);
    UTEST_CHECK_EQUAL(std::get<1>(dims), 7);
    UTEST_CHECK_EQUAL(std::get<2>(dims), 5);
    UTEST_CHECK_EQUAL(nano::size(dims), 105);

    UTEST_CHECK_EQUAL(nano::index(dims, 0, 0, 1), 1);
    UTEST_CHECK_EQUAL(nano::index(dims, 0, 0, 4), 4);
    UTEST_CHECK_EQUAL(nano::index(dims, 0, 1, 0), 5);
    UTEST_CHECK_EQUAL(nano::index(dims, 0, 2, 1), 11);
    UTEST_CHECK_EQUAL(nano::index(dims, 1, 2, 1), 46);
    UTEST_CHECK_EQUAL(nano::index(dims, 1, 0, 3), 38);
    UTEST_CHECK_EQUAL(nano::index(dims, 2, 4, 1), 91);
    UTEST_CHECK_EQUAL(nano::index(dims, 2, 6, 4), 104);

    UTEST_CHECK_EQUAL(nano::index0(dims), nano::index(dims, 0, 0, 0));
    UTEST_CHECK_EQUAL(nano::index0(dims, 2), nano::index(dims, 2, 0, 0));
    UTEST_CHECK_EQUAL(nano::index0(dims, 2, 4), nano::index(dims, 2, 4, 0));
    UTEST_CHECK_EQUAL(nano::index0(dims, 2, 4, 3), nano::index(dims, 2, 4, 3));

    UTEST_CHECK_EQUAL(nano::dims0(dims), nano::make_dims(3, 7, 5));
    UTEST_CHECK_EQUAL(nano::dims0(dims, 2), nano::make_dims(7, 5));
    UTEST_CHECK_EQUAL(nano::dims0(dims, 2, 4), nano::make_dims(5));
}

UTEST_END_MODULE()
