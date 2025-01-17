#pragma once

#include <nano/dataset/tabular.h>

namespace nano
{
    ///
    /// \brief Adult dataset: http://archive.ics.uci.edu/ml/datasets/Adult
    ///
    class adult_dataset_t final : public tabular_dataset_t
    {
    public:

        adult_dataset_t();

        json_t config() const override;
        void config(const json_t&) override;
        void split(const tensor_size_t samples, split_t&) const override;

    private:

        // attributes
        string_t        m_dir;              ///< directory where to load the data from
        size_t          m_folds{10};        ///<
        tensor_size_t   m_train_per{80};    ///< percentage of training samples (excluding the test samples)
    };
}
