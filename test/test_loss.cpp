#include <nano/loss.h>
#include <nano/mlearn.h>
#include <nano/random.h>
#include <utest/utest.h>
#include <nano/numeric.h>
#include <nano/function.h>

using namespace nano;

struct loss_function_t final : public function_t
{
    loss_function_t(const rloss_t& loss, const tensor_size_t xmaps) :
        function_t("loss", xmaps, convexity::no),
        m_loss(loss), m_target(xmaps, 1, 1)
    {
        m_target = class_target(xmaps, 11 % xmaps);
    }

    scalar_t vgrad(const vector_t& x, vector_t* gx = nullptr) const override
    {
        UTEST_REQUIRE_EQUAL(x.size(), m_target.size());
        const auto output = map_tensor(x.data(), m_target.dims());

        if (gx)
        {
            gx->resize(m_target.size());

            m_loss->vgrad(m_target, output, map_tensor(gx->data(), m_target.dims()));
            UTEST_REQUIRE(gx->array().isFinite().all());
        }

        const auto value = m_loss->value(m_target, output);
        UTEST_REQUIRE(std::isfinite(value));
        return value;
    }

    const rloss_t&      m_loss;
    tensor3d_t          m_target;
};

UTEST_BEGIN_MODULE(test_loss)

UTEST_CASE(config)
{
    for (const auto& loss_id : loss_t::all().ids())
    {
        const auto loss = loss_t::all().get(loss_id);

        // NB: the loss functions are non-parametric!
        auto json = loss->config();
        UTEST_CHECK(json.empty());
        UTEST_CHECK_NOTHROW(loss->config(json));
    }
}

UTEST_CASE(gradient)
{
    const tensor_size_t cmd_min_dims = 2;
    const tensor_size_t cmd_max_dims = 8;
    const size_t cmd_tests = 128;

    // evaluate the analytical gradient vs. the finite difference approximation
    for (const auto& loss_id : loss_t::all().ids())
    {
        for (tensor_size_t cmd_dims = cmd_min_dims; cmd_dims <= cmd_max_dims; ++ cmd_dims)
        {
            const auto loss = loss_t::all().get(loss_id);
            const auto function = loss_function_t(loss, cmd_dims);

            for (size_t t = 0; t < cmd_tests; ++ t)
            {
                vector_t x = vector_t::Random(cmd_dims) / 10;

                UTEST_CHECK_GREATER(function.vgrad(x), scalar_t(0));
                UTEST_CHECK_LESS(function.grad_accuracy(x), 2 * epsilon2<scalar_t>());
            }
        }
    }
}

UTEST_CASE(single_class)
{
    for (const auto& loss_id : {"s-classnll", "s-logistic", "s-exponential", "s-hinge"})
    {
        const auto loss = loss_t::all().get(loss_id);
        UTEST_REQUIRE(loss);

        const auto n_classes = 1;
        tensor3d_t target(n_classes, 1, 1);
        tensor3d_t output(n_classes, 1, 1);

        {
            target = class_target(n_classes);
            output = class_target(n_classes);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(0), epsilon0<scalar_t>());
        }
        {
            target = class_target(n_classes, 0);
            output = class_target(n_classes, 0);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(0), epsilon0<scalar_t>());
        }
        {
            target = class_target(n_classes);
            output = class_target(n_classes, 0);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
        }
        {
            target = class_target(n_classes, 0);
            output = class_target(n_classes);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
        }
    }
}

UTEST_CASE(single_label_multi_class)
{
    for (const auto& loss_id : {"s-classnll", "s-logistic", "s-exponential", "s-hinge"})
    {
        const auto loss = loss_t::all().get(loss_id);
        UTEST_REQUIRE(loss);

        const auto n_classes = 13;
        tensor3d_t target(n_classes, 1, 1);
        tensor3d_t output(n_classes, 1, 1);

        {
            target = class_target(n_classes, 11);
            output = class_target(n_classes, 11);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(0), epsilon0<scalar_t>());
        }
        {
            target = class_target(n_classes, 11);
            output = class_target(n_classes, 12);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
        }
        {
            target = class_target(n_classes, 11);
            output = class_target(n_classes, 11);
            output.vector()(7) = pos_target() + 1;

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
        }
        {
            target = class_target(n_classes, 11);
            output = class_target(n_classes);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
        }
    }
}

UTEST_CASE(multi_label_multi_class)
{
    for (const auto& loss_id : {"m-logistic", "m-exponential", "m-hinge"})
    {
        const auto loss = loss_t::all().get(loss_id);
        UTEST_REQUIRE(loss);

        const auto n_classes = 13;
        tensor3d_t target(n_classes, 1, 1);
        tensor3d_t output(n_classes, 1, 1);

        {
            target = class_target(n_classes, 7, 9);
            output = class_target(n_classes, 7, 9);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(0), epsilon0<scalar_t>());
        }
        {
            target = class_target(n_classes, 7, 9);
            output = class_target(n_classes);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(2), epsilon0<scalar_t>());
        }
        {
            target = class_target(n_classes, 7, 9);
            output = class_target(n_classes, 5);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(3), epsilon0<scalar_t>());
        }
        {
            target = class_target(n_classes, 7, 9);
            output = class_target(n_classes, 7);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
        }
        {
            target = class_target(n_classes, 7, 9);
            output = class_target(n_classes, 5, 9);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(2), epsilon0<scalar_t>());
        }
        {
            target = class_target(n_classes, 7, 9);
            output = class_target(n_classes, 7, 9, 11);

            const auto error = loss->error(target, output);
            UTEST_CHECK_CLOSE(error, scalar_t(1), epsilon0<scalar_t>());
        }
    }
}

UTEST_CASE(regression)
{
    for (const auto& loss_id : {"absolute", "squared", "cauchy"})
    {
        const auto loss = loss_t::all().get(loss_id);
        UTEST_REQUIRE(loss);

        tensor3d_t target(4, 1, 1);
        target.random();

        tensor3d_t output = target;

        const auto error = loss->error(target, output);
        UTEST_CHECK_LESS(error, epsilon0<scalar_t>());
    }
}

UTEST_END_MODULE()
