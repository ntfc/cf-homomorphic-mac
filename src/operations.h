#pragma once
#include "tag.h"

extern int use_fft;

//! @brief Perform the addition of two CF tags.
void tag_addition(cf_tag_t *, const cf_tag_t, const cf_tag_t, const cf_mpi_t);

void tag_product(cf_tag_t *, const cf_tag_t, const cf_tag_t, const cf_mpi_t);

//! @brief Perform the \f$\mathcal{O}(n^2)\f$ multiplication of two CF tags.
void tag_mul_naive(cf_tag_t *, const cf_tag_t, const cf_tag_t, const cf_mpi_t);

//! @brief Perform the faster FFT multiplication of two tags.
void tag_mul_fft(cf_tag_t *, const cf_tag_t, const cf_tag_t, const cf_mpi_t);

//! @brief Perform a constant multiplication of a CF tag.
void tag_constant_product(cf_tag_t *, const cf_tag_t, const cf_mpi_t, const cf_mpi_t);
