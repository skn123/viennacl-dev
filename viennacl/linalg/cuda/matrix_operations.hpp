#ifndef VIENNACL_LINALG_CUDA_MATRIX_OPERATIONS_HPP_
#define VIENNACL_LINALG_CUDA_MATRIX_OPERATIONS_HPP_

/* =========================================================================
   Copyright (c) 2010-2012, Institute for Microelectronics,
                            Institute for Analysis and Scientific Computing,
                            TU Wien.

                            -----------------
                  ViennaCL - The Vienna Computing Library
                            -----------------

   Project Head:    Karl Rupp                   rupp@iue.tuwien.ac.at
               
   (A list of authors and contributors can be found in the PDF manual)

   License:         MIT (X11), see file LICENSE in the base directory
============================================================================= */

/** @file  viennacl/linalg/cuda/matrix_operations.hpp
    @brief Implementations of dense matrix related operations, including matrix-vector products, using CUDA.
*/

#include "viennacl/forwards.h"
#include "viennacl/scalar.hpp"
#include "viennacl/vector.hpp"
#include "viennacl/vector_proxy.hpp"
#include "viennacl/tools/tools.hpp"
#include "viennacl/meta/enable_if.hpp"
#include "viennacl/meta/predicate.hpp"
#include "viennacl/meta/result_of.hpp"
#include "viennacl/traits/size.hpp"
#include "viennacl/traits/start.hpp"
#include "viennacl/traits/handle.hpp"
#include "viennacl/traits/stride.hpp"

#include "viennacl/linalg/cuda/common.hpp"

#include "viennacl/linalg/cuda/matrix_operations_row.hpp"
#include "viennacl/linalg/cuda/matrix_operations_col.hpp"

namespace viennacl
{
  namespace linalg
  {
    namespace cuda
    {
      //
      // Introductory note: By convention, all dimensions are already checked in the dispatcher frontend. No need to double-check again in here!
      //
      
      template <typename M1,
                typename M2, typename ScalarType1>
      typename viennacl::enable_if< viennacl::is_any_dense_nonstructured_matrix<M1>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<M2>::value
                                    && viennacl::is_any_scalar<ScalarType1>::value
                                  >::type
      am(M1 & mat1, 
         M2 const & mat2, ScalarType1 const & alpha, std::size_t len_alpha, bool reciprocal_alpha, bool flip_sign_alpha)
      {
        typedef typename viennacl::result_of::cpu_value_type<M1>::type        value_type;

        unsigned int options_alpha =   ((len_alpha > 1) ? (len_alpha << 2) : 0)
                                    + (reciprocal_alpha ?                2 : 0)
                                     + (flip_sign_alpha ?                1 : 0);

        value_type temporary_alpha;                             
        if (viennacl::is_cpu_scalar<ScalarType1>::value)
          temporary_alpha = alpha;

        if (viennacl::is_row_major<M1>::value)
        {
          am_row_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat1),
                                      static_cast<unsigned int>(viennacl::traits::start1(mat1)),           static_cast<unsigned int>(viennacl::traits::start2(mat1)),
                                      static_cast<unsigned int>(viennacl::traits::stride1(mat1)),          static_cast<unsigned int>(viennacl::traits::stride2(mat1)),
                                      static_cast<unsigned int>(viennacl::traits::size1(mat1)),            static_cast<unsigned int>(viennacl::traits::size2(mat1)),
                                      static_cast<unsigned int>(viennacl::traits::internal_size1(mat1)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat1)),
                                      
                                      detail::cuda_arg<value_type>(detail::arg_reference(alpha, temporary_alpha)),
                                      options_alpha,
                                      detail::cuda_arg<value_type>(mat2),
                                      static_cast<unsigned int>(viennacl::traits::start1(mat2)),           static_cast<unsigned int>(viennacl::traits::start2(mat2)),
                                      static_cast<unsigned int>(viennacl::traits::stride1(mat2)),          static_cast<unsigned int>(viennacl::traits::stride2(mat2)),
                                      static_cast<unsigned int>(viennacl::traits::internal_size1(mat2)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat2))
                                    );
          VIENNACL_CUDA_LAST_ERROR_CHECK("am_row_kernel");
        }
        else
        {
          am_col_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat1),
                                      static_cast<unsigned int>(viennacl::traits::start1(mat1)),           static_cast<unsigned int>(viennacl::traits::start2(mat1)),
                                      static_cast<unsigned int>(viennacl::traits::stride1(mat1)),          static_cast<unsigned int>(viennacl::traits::stride2(mat1)),
                                      static_cast<unsigned int>(viennacl::traits::size1(mat1)),            static_cast<unsigned int>(viennacl::traits::size2(mat1)),
                                      static_cast<unsigned int>(viennacl::traits::internal_size1(mat1)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat1)),
                                      
                                      detail::cuda_arg<value_type>(detail::arg_reference(alpha, temporary_alpha)),
                                      options_alpha,
                                      detail::cuda_arg<value_type>(mat2),
                                      static_cast<unsigned int>(viennacl::traits::start1(mat2)),           static_cast<unsigned int>(viennacl::traits::start2(mat2)),
                                      static_cast<unsigned int>(viennacl::traits::stride1(mat2)),          static_cast<unsigned int>(viennacl::traits::stride2(mat2)),
                                      static_cast<unsigned int>(viennacl::traits::internal_size1(mat2)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat2))
                                    );
          VIENNACL_CUDA_LAST_ERROR_CHECK("am_col_kernel");
        }
      }
      
      
      template <typename M1,
                typename M2, typename ScalarType1,
                typename M3, typename ScalarType2>
      typename viennacl::enable_if< viennacl::is_any_dense_nonstructured_matrix<M1>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<M2>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<M3>::value
                                    && viennacl::is_any_scalar<ScalarType1>::value
                                    && viennacl::is_any_scalar<ScalarType2>::value
                                  >::type
      ambm(M1 & mat1, 
          M2 const & mat2, ScalarType1 const & alpha, std::size_t len_alpha, bool reciprocal_alpha, bool flip_sign_alpha,
          M3 const & mat3, ScalarType2 const & beta,  std::size_t len_beta,  bool reciprocal_beta,  bool flip_sign_beta) 
      {
        typedef typename viennacl::result_of::cpu_value_type<M1>::type        value_type;
          
        unsigned int options_alpha =   ((len_alpha > 1) ? (len_alpha << 2) : 0)
                                    + (reciprocal_alpha ?                2 : 0)
                                    +  (flip_sign_alpha ?                1 : 0);
                                
        value_type temporary_alpha;                             
        if (viennacl::is_cpu_scalar<ScalarType1>::value)
          temporary_alpha = alpha;

                                
        unsigned int options_beta =    ((len_beta > 1) ? (len_beta << 2) : 0)
                                    + (reciprocal_beta ?               2 : 0)
                                    +  (flip_sign_beta ?               1 : 0);

        value_type temporary_beta;
        if (viennacl::is_cpu_scalar<ScalarType2>::value)
          temporary_beta = beta;
                                
        
        if (viennacl::is_row_major<M1>::value)
        {
          ambm_row_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat1),
                                        static_cast<unsigned int>(viennacl::traits::start1(mat1)),           static_cast<unsigned int>(viennacl::traits::start2(mat1)),
                                        static_cast<unsigned int>(viennacl::traits::stride1(mat1)),          static_cast<unsigned int>(viennacl::traits::stride2(mat1)),
                                        static_cast<unsigned int>(viennacl::traits::size1(mat1)),            static_cast<unsigned int>(viennacl::traits::size2(mat1)),
                                        static_cast<unsigned int>(viennacl::traits::internal_size1(mat1)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat1)),
                                        
                                        detail::cuda_arg<value_type>(detail::arg_reference(alpha, temporary_alpha)),
                                        options_alpha,
                                        detail::cuda_arg<value_type>(mat2),
                                        static_cast<unsigned int>(viennacl::traits::start1(mat2)),           static_cast<unsigned int>(viennacl::traits::start2(mat2)),
                                        static_cast<unsigned int>(viennacl::traits::stride1(mat2)),          static_cast<unsigned int>(viennacl::traits::stride2(mat2)),
                                        static_cast<unsigned int>(viennacl::traits::internal_size1(mat2)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat2)),
                                        
                                        detail::cuda_arg<value_type>(detail::arg_reference(beta, temporary_beta)),
                                        options_beta,
                                        detail::cuda_arg<value_type>(mat3),
                                        static_cast<unsigned int>(viennacl::traits::start1(mat3)),           static_cast<unsigned int>(viennacl::traits::start2(mat3)),
                                        static_cast<unsigned int>(viennacl::traits::stride1(mat3)),          static_cast<unsigned int>(viennacl::traits::stride2(mat3)),
                                        static_cast<unsigned int>(viennacl::traits::internal_size1(mat3)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat3))
                                      );
          VIENNACL_CUDA_LAST_ERROR_CHECK("ambm_row_kernel");
        }
        else
        {
          ambm_col_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat1),
                                        static_cast<unsigned int>(viennacl::traits::start1(mat1)),           static_cast<unsigned int>(viennacl::traits::start2(mat1)),
                                        static_cast<unsigned int>(viennacl::traits::stride1(mat1)),          static_cast<unsigned int>(viennacl::traits::stride2(mat1)),
                                        static_cast<unsigned int>(viennacl::traits::size1(mat1)),            static_cast<unsigned int>(viennacl::traits::size2(mat1)),
                                        static_cast<unsigned int>(viennacl::traits::internal_size1(mat1)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat1)),
                                        
                                        detail::cuda_arg<value_type>(detail::arg_reference(alpha, temporary_alpha)),
                                        options_alpha,
                                        detail::cuda_arg<value_type>(mat2),
                                        static_cast<unsigned int>(viennacl::traits::start1(mat2)),           static_cast<unsigned int>(viennacl::traits::start2(mat2)),
                                        static_cast<unsigned int>(viennacl::traits::stride1(mat2)),          static_cast<unsigned int>(viennacl::traits::stride2(mat2)),
                                        static_cast<unsigned int>(viennacl::traits::internal_size1(mat2)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat2)),
                                        
                                        detail::cuda_arg<value_type>(detail::arg_reference(beta, temporary_beta)),
                                        options_beta,
                                        detail::cuda_arg<value_type>(mat3),
                                        static_cast<unsigned int>(viennacl::traits::start1(mat3)),           static_cast<unsigned int>(viennacl::traits::start2(mat3)),
                                        static_cast<unsigned int>(viennacl::traits::stride1(mat3)),          static_cast<unsigned int>(viennacl::traits::stride2(mat3)),
                                        static_cast<unsigned int>(viennacl::traits::internal_size1(mat3)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat3))
                                      );
          VIENNACL_CUDA_LAST_ERROR_CHECK("ambm_col_kernel");
        }
        
      }
      
      
      template <typename M1,
                typename M2, typename ScalarType1,
                typename M3, typename ScalarType2>
      typename viennacl::enable_if< viennacl::is_any_dense_nonstructured_matrix<M1>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<M2>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<M3>::value
                                    && viennacl::is_any_scalar<ScalarType1>::value
                                    && viennacl::is_any_scalar<ScalarType2>::value
                                  >::type
      ambm_m(M1 & mat1,
             M2 const & mat2, ScalarType1 const & alpha, std::size_t len_alpha, bool reciprocal_alpha, bool flip_sign_alpha,
             M3 const & mat3, ScalarType2 const & beta,  std::size_t len_beta,  bool reciprocal_beta,  bool flip_sign_beta) 
      {
        typedef typename viennacl::result_of::cpu_value_type<M1>::type        value_type;
        
        unsigned int options_alpha =   ((len_alpha > 1) ? (len_alpha << 2) : 0)
                                    + (reciprocal_alpha ?                2 : 0)
                                    +  (flip_sign_alpha ?                1 : 0);
                                
        value_type temporary_alpha;                             
        if (viennacl::is_cpu_scalar<ScalarType1>::value)
          temporary_alpha = alpha;

                                
        unsigned int options_beta =    ((len_beta > 1) ? (len_beta << 2) : 0)
                                    + (reciprocal_beta ?               2 : 0)
                                    +  (flip_sign_beta ?               1 : 0);

        value_type temporary_beta;
        if (viennacl::is_cpu_scalar<ScalarType2>::value)
          temporary_beta = beta;
                                
        
        if (viennacl::is_row_major<M1>::value)
        {
          ambm_m_row_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat1),
                                          static_cast<unsigned int>(viennacl::traits::start1(mat1)),           static_cast<unsigned int>(viennacl::traits::start2(mat1)),
                                          static_cast<unsigned int>(viennacl::traits::stride1(mat1)),          static_cast<unsigned int>(viennacl::traits::stride2(mat1)),
                                          static_cast<unsigned int>(viennacl::traits::size1(mat1)),            static_cast<unsigned int>(viennacl::traits::size2(mat1)),
                                          static_cast<unsigned int>(viennacl::traits::internal_size1(mat1)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat1)),
                                          
                                          detail::cuda_arg<value_type>(detail::arg_reference(alpha, temporary_alpha)),
                                          options_alpha,
                                          detail::cuda_arg<value_type>(mat2),
                                          static_cast<unsigned int>(viennacl::traits::start1(mat2)),           static_cast<unsigned int>(viennacl::traits::start2(mat2)),
                                          static_cast<unsigned int>(viennacl::traits::stride1(mat2)),          static_cast<unsigned int>(viennacl::traits::stride2(mat2)),
                                          static_cast<unsigned int>(viennacl::traits::internal_size1(mat2)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat2)),
                                          
                                          detail::cuda_arg<value_type>(detail::arg_reference(beta, temporary_beta)),
                                          options_beta,
                                          detail::cuda_arg<value_type>(mat3),
                                          static_cast<unsigned int>(viennacl::traits::start1(mat3)),           static_cast<unsigned int>(viennacl::traits::start2(mat3)),
                                          static_cast<unsigned int>(viennacl::traits::stride1(mat3)),          static_cast<unsigned int>(viennacl::traits::stride2(mat3)),
                                          static_cast<unsigned int>(viennacl::traits::internal_size1(mat3)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat3))
                                        );
          VIENNACL_CUDA_LAST_ERROR_CHECK("ambm_m_row_kernel");
        }
        else
        {
          ambm_m_col_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat1),
                                          static_cast<unsigned int>(viennacl::traits::start1(mat1)),           static_cast<unsigned int>(viennacl::traits::start2(mat1)),
                                          static_cast<unsigned int>(viennacl::traits::stride1(mat1)),          static_cast<unsigned int>(viennacl::traits::stride2(mat1)),
                                          static_cast<unsigned int>(viennacl::traits::size1(mat1)),            static_cast<unsigned int>(viennacl::traits::size2(mat1)),
                                          static_cast<unsigned int>(viennacl::traits::internal_size1(mat1)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat1)),
                                          
                                          detail::cuda_arg<value_type>(detail::arg_reference(alpha, temporary_alpha)),
                                          options_alpha,
                                          detail::cuda_arg<value_type>(mat2),
                                          static_cast<unsigned int>(viennacl::traits::start1(mat2)),           static_cast<unsigned int>(viennacl::traits::start2(mat2)),
                                          static_cast<unsigned int>(viennacl::traits::stride1(mat2)),          static_cast<unsigned int>(viennacl::traits::stride2(mat2)),
                                          static_cast<unsigned int>(viennacl::traits::internal_size1(mat2)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat2)),
                                          
                                          detail::cuda_arg<value_type>(detail::arg_reference(beta, temporary_beta)),
                                          options_beta,
                                          detail::cuda_arg<value_type>(mat3),
                                          static_cast<unsigned int>(viennacl::traits::start1(mat3)),           static_cast<unsigned int>(viennacl::traits::start2(mat3)),
                                          static_cast<unsigned int>(viennacl::traits::stride1(mat3)),          static_cast<unsigned int>(viennacl::traits::stride2(mat3)),
                                          static_cast<unsigned int>(viennacl::traits::internal_size1(mat3)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat3))
                                        );
          VIENNACL_CUDA_LAST_ERROR_CHECK("ambm_m_col_kernel");
        }
        
      }



      //
      /////////////////////////   matrix-vector products /////////////////////////////////
      //

      // A * x
      
      /** @brief Carries out matrix-vector multiplication
      *
      * Implementation of the convenience expression result = prod(mat, vec);
      *
      * @param mat    The matrix
      * @param vec    The vector
      * @param result The result vector
      */
      template <typename MatrixType, typename VectorType1, typename VectorType2>
      typename viennacl::enable_if<   viennacl::is_any_dense_nonstructured_matrix<MatrixType>::value 
                                    && viennacl::is_any_dense_nonstructured_vector<VectorType1>::value 
                                    && viennacl::is_any_dense_nonstructured_vector<VectorType2>::value >::type
      prod_impl(const MatrixType & mat, 
                const VectorType1 & vec, 
                      VectorType2 & result)
      {
        typedef typename viennacl::result_of::cpu_value_type<VectorType1>::type        value_type;

        assert(viennacl::traits::handle(vec) != viennacl::traits::handle(result) && bool("No direct inplace matrix-vector product possible. Introduce a temporary!"));

        if (viennacl::is_row_major<MatrixType>::value)
        {
          vec_mul_row_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat),
                                           static_cast<unsigned int>(viennacl::traits::start1(mat)),         static_cast<unsigned int>(viennacl::traits::start2(mat)), 
                                           static_cast<unsigned int>(viennacl::traits::stride1(mat)),        static_cast<unsigned int>(viennacl::traits::stride2(mat)),
                                           static_cast<unsigned int>(viennacl::traits::size1(mat)),          static_cast<unsigned int>(viennacl::traits::size2(mat)),
                                           static_cast<unsigned int>(viennacl::traits::internal_size1(mat)), static_cast<unsigned int>(viennacl::traits::internal_size2(mat)),
                                          
                                           detail::cuda_arg<value_type>(vec),
                                           static_cast<unsigned int>(viennacl::traits::start(vec)),
                                           static_cast<unsigned int>(viennacl::traits::stride(vec)),
                                           static_cast<unsigned int>(viennacl::traits::size(vec)), 
                                          
                                           detail::cuda_arg<value_type>(result),
                                           static_cast<unsigned int>(viennacl::traits::start(result)),
                                           static_cast<unsigned int>(viennacl::traits::stride(result)),
                                           static_cast<unsigned int>(viennacl::traits::size(result))
                                          );
          VIENNACL_CUDA_LAST_ERROR_CHECK("vec_mul_row_kernel");
        }
        else
        {
          vec_mul_col_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat),
                                           static_cast<unsigned int>(viennacl::traits::start1(mat)),         static_cast<unsigned int>(viennacl::traits::start2(mat)), 
                                           static_cast<unsigned int>(viennacl::traits::stride1(mat)),        static_cast<unsigned int>(viennacl::traits::stride2(mat)),
                                           static_cast<unsigned int>(viennacl::traits::size1(mat)),          static_cast<unsigned int>(viennacl::traits::size2(mat)),
                                           static_cast<unsigned int>(viennacl::traits::internal_size1(mat)), static_cast<unsigned int>(viennacl::traits::internal_size2(mat)),
                                          
                                           detail::cuda_arg<value_type>(vec),
                                           static_cast<unsigned int>(viennacl::traits::start(vec)),
                                           static_cast<unsigned int>(viennacl::traits::stride(vec)),
                                           static_cast<unsigned int>(viennacl::traits::size(vec)), 
                                          
                                           detail::cuda_arg<value_type>(result),
                                           static_cast<unsigned int>(viennacl::traits::start(result)),
                                           static_cast<unsigned int>(viennacl::traits::stride(result)),
                                           static_cast<unsigned int>(viennacl::traits::size(result))
                                          );
          VIENNACL_CUDA_LAST_ERROR_CHECK("vec_mul_col_kernel");
        }
      }


      // trans(A) * x
      
      /** @brief Carries out matrix-vector multiplication with a transposed matrix
      *
      * Implementation of the convenience expression result = trans(mat) * vec;
      *
      * @param mat_trans  The transposed matrix proxy
      * @param vec        The vector
      * @param result     The result vector
      */
      template <typename M1, typename V1, typename V2>
      typename viennacl::enable_if<    viennacl::is_any_dense_nonstructured_matrix<M1>::value 
                                    && viennacl::is_any_dense_nonstructured_vector<V1>::value 
                                    && viennacl::is_any_dense_nonstructured_vector<V2>::value 
                                  >::type
      prod_impl(const viennacl::matrix_expression< const M1,
                                                   const M1,
                                                   op_trans> & mat_trans,
                const V1 & vec, 
                      V2 & result)
      {
        assert( (viennacl::traits::size1(mat_trans) == viennacl::traits::size(result)) && bool("Size check failed for transposed matrix-vector product: size1(A^T) == size(result)"));
        assert( (viennacl::traits::size2(mat_trans) == viennacl::traits::size(vec)) && bool("Size check failed for transposed matrix-vector product: size2(A^T) == size(x)"));  //remember: mat is transposed!
        
        typedef typename viennacl::result_of::cpu_value_type<V1>::type    value_type;

        
        // Inplace matrix-vector products like x = prod(A, x) are currently illegal: Introduce a temporary like y = prod(A, x); x = y; instead
        assert(vec.handle() != result.handle() && bool("No direct inplace transposed matrix-vector product possible. Introduce a temporary!"));
        
        if (viennacl::is_row_major<M1>::value)
        {
          trans_vec_mul_row_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat_trans.lhs()),
                                                 static_cast<unsigned int>(viennacl::traits::start1(mat_trans.lhs())),         static_cast<unsigned int>(viennacl::traits::start2(mat_trans.lhs())), 
                                                 static_cast<unsigned int>(viennacl::traits::stride1(mat_trans.lhs())),        static_cast<unsigned int>(viennacl::traits::stride2(mat_trans.lhs())),
                                                 static_cast<unsigned int>(viennacl::traits::size1(mat_trans.lhs())),          static_cast<unsigned int>(viennacl::traits::size2(mat_trans.lhs())),
                                                 static_cast<unsigned int>(viennacl::traits::internal_size1(mat_trans.lhs())), static_cast<unsigned int>(viennacl::traits::internal_size2(mat_trans.lhs())),
                                                
                                                 detail::cuda_arg<value_type>(vec),
                                                 static_cast<unsigned int>(viennacl::traits::start(vec)),
                                                 static_cast<unsigned int>(viennacl::traits::stride(vec)),
                                                 static_cast<unsigned int>(viennacl::traits::size(vec)), 
                                                
                                                 detail::cuda_arg<value_type>(result),
                                                 static_cast<unsigned int>(viennacl::traits::start(result)),
                                                 static_cast<unsigned int>(viennacl::traits::stride(result)),
                                                 static_cast<unsigned int>(viennacl::traits::size(result))
                                                );
          VIENNACL_CUDA_LAST_ERROR_CHECK("trans_vec_mul_row_kernel");
        }
        else
        {
          trans_vec_mul_col_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat_trans.lhs()),
                                                 static_cast<unsigned int>(viennacl::traits::start1(mat_trans.lhs())),         static_cast<unsigned int>(viennacl::traits::start2(mat_trans.lhs())), 
                                                 static_cast<unsigned int>(viennacl::traits::stride1(mat_trans.lhs())),        static_cast<unsigned int>(viennacl::traits::stride2(mat_trans.lhs())),
                                                 static_cast<unsigned int>(viennacl::traits::size1(mat_trans.lhs())),          static_cast<unsigned int>(viennacl::traits::size2(mat_trans.lhs())),
                                                 static_cast<unsigned int>(viennacl::traits::internal_size1(mat_trans.lhs())), static_cast<unsigned int>(viennacl::traits::internal_size2(mat_trans.lhs())),
                                                
                                                 detail::cuda_arg<value_type>(vec),
                                                 static_cast<unsigned int>(viennacl::traits::start(vec)),
                                                 static_cast<unsigned int>(viennacl::traits::stride(vec)),
                                                 static_cast<unsigned int>(viennacl::traits::size(vec)), 
                                                
                                                 detail::cuda_arg<value_type>(result),
                                                 static_cast<unsigned int>(viennacl::traits::start(result)),
                                                 static_cast<unsigned int>(viennacl::traits::stride(result)),
                                                 static_cast<unsigned int>(viennacl::traits::size(result))
                                                );
          VIENNACL_CUDA_LAST_ERROR_CHECK("trans_vec_mul_col_kernel");
        }
      }


      //
      /////////////////////////   matrix-matrix products /////////////////////////////////
      //
      
      namespace detail
      {
        // C = A * B and possibly transposed variants
        template <typename T1, typename T2, typename T3, typename ScalarType >
        void prod_slow_kernel(const T1 & A, 
                              const T2 & B, 
                              T3 & C,
                              ScalarType alpha,
                              ScalarType beta,
                              std::string kernel_name)
        {
          typedef typename viennacl::result_of::cpu_value_type< typename T1::value_type >::type   cpu_value_type;
          
          cpu_value_type cl_alpha = static_cast<cpu_value_type>(alpha);
          cpu_value_type cl_beta  = static_cast<cpu_value_type>(beta);
          
          /*viennacl::ocl::enqueue(k(cl_alpha,
                                  viennacl::traits::opencl_handle(A), 
                                  cl_uint(viennacl::traits::start1(A)),           cl_uint(viennacl::traits::start2(A)), 
                                  cl_uint(viennacl::traits::stride1(A)),          cl_uint(viennacl::traits::stride2(A)),
                                  cl_uint(viennacl::traits::size1(A)),            cl_uint(viennacl::traits::size2(A)),
                                  cl_uint(viennacl::traits::internal_size1(A)),   cl_uint(viennacl::traits::internal_size2(A)),
                                   
                                  viennacl::traits::opencl_handle(B), 
                                  cl_uint(viennacl::traits::start1(B)),           cl_uint(viennacl::traits::start2(B)), 
                                  cl_uint(viennacl::traits::stride1(B)),          cl_uint(viennacl::traits::stride2(B)),
                                  cl_uint(viennacl::traits::size1(B)),            cl_uint(viennacl::traits::size2(B)),
                                  cl_uint(viennacl::traits::internal_size1(B)),   cl_uint(viennacl::traits::internal_size2(B)),
                                   
                                  cl_beta,
                                  viennacl::traits::opencl_handle(C), 
                                  cl_uint(viennacl::traits::start1(C)),           cl_uint(viennacl::traits::start2(C)), 
                                  cl_uint(viennacl::traits::stride1(C)),          cl_uint(viennacl::traits::stride2(C)),
                                  cl_uint(viennacl::traits::size1(C)),            cl_uint(viennacl::traits::size2(C)),
                                  cl_uint(viennacl::traits::internal_size1(C)),   cl_uint(viennacl::traits::internal_size2(C))
                                  )
                                );        */
          throw "not implemented yet";
        }
        
        // C = A * B, using fast kernel
        template <typename T1, typename T2, typename T3, typename ScalarType >
        void prod_fast_kernel(const T1 & A, 
                              const T2 & B, 
                              T3 & C,
                              ScalarType alpha,
                              ScalarType beta,
                              std::string kernel_name)
        {
          typedef typename viennacl::result_of::cpu_value_type< typename T1::value_type >::type   cpu_value_type;
          
          cpu_value_type cl_alpha = static_cast<cpu_value_type>(alpha);
          cpu_value_type cl_beta  = static_cast<cpu_value_type>(beta);
          
          /*viennacl::ocl::enqueue(k(cl_alpha,
                                  viennacl::traits::opencl_handle(A), 
                                  cl_uint(viennacl::traits::start1(A)),           cl_uint(viennacl::traits::start2(A)), 
                                  cl_uint(viennacl::traits::stride1(A)),          cl_uint(viennacl::traits::stride2(A)),
                                  cl_uint(viennacl::traits::size1(A)),            cl_uint(viennacl::traits::size2(A)),
                                  cl_uint(viennacl::traits::internal_size1(A)),   cl_uint(viennacl::traits::internal_size2(A)),
                                   
                                  viennacl::traits::opencl_handle(B), 
                                  cl_uint(viennacl::traits::start1(B)),           cl_uint(viennacl::traits::start2(B)), 
                                  cl_uint(viennacl::traits::stride1(B)),          cl_uint(viennacl::traits::stride2(B)),
                                  cl_uint(viennacl::traits::size1(B)),            cl_uint(viennacl::traits::size2(B)),
                                  cl_uint(viennacl::traits::internal_size1(B)),   cl_uint(viennacl::traits::internal_size2(B)),
                                   
                                  cl_beta,
                                  viennacl::traits::opencl_handle(C), 
                                  cl_uint(viennacl::traits::start1(C)),           cl_uint(viennacl::traits::start2(C)), 
                                  cl_uint(viennacl::traits::stride1(C)),          cl_uint(viennacl::traits::stride2(C)),
                                  cl_uint(viennacl::traits::size1(C)),            cl_uint(viennacl::traits::size2(C)),
                                  cl_uint(viennacl::traits::internal_size1(C)),   cl_uint(viennacl::traits::internal_size2(C))
                                  )
                                );*/
          
          throw "not implemented yet";
        }
        
        template <typename T1, typename T2, typename T3, typename ScalarType >
        void prod(const T1 & A, 
                  const T2 & B, 
                  T3 & C,
                  ScalarType alpha,
                  ScalarType beta,
                  std::string fast_kernel_name,
                  std::string slow_kernel_name)
        {
          if (   (viennacl::traits::size1(A) < 64)
              || (viennacl::traits::size2(A) < 64)
              || (viennacl::traits::size1(B) < 64) )   //there is most likely not enough to compute, rendering kernel launch overhead considerable
          {
            prod_slow_kernel(A, B, C, alpha, beta, slow_kernel_name);
          }
          else if (   (viennacl::traits::size1(A) % 64 == 0)
                  && (viennacl::traits::size2(A) % 64 == 0)
                  && (viennacl::traits::size1(B) % 64 == 0) )   // allows the use of the fast kernel only
          {
            prod_fast_kernel(A, B, C, alpha, beta, fast_kernel_name);
            //prod_slow_kernel(A, B, C, slow_kernel_name);
          }
          else //TODO: use four kernels
          {
            prod_slow_kernel(A, B, C, alpha, beta, slow_kernel_name);
          }
          
        }
      } // namespace detail


      /** @brief Carries out matrix-matrix multiplication
      *
      * Implementation of C = prod(A, B);
      *
      */
      template <typename T1, typename T2, typename T3, typename ScalarType >
      typename viennacl::enable_if<    viennacl::is_any_dense_nonstructured_matrix<T1>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<T2>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<T3>::value
                                  >::type
      prod_impl(const T1 & A, 
                const T2 & B, 
                      T3 & C,
                ScalarType alpha,
                ScalarType beta)
      {
        assert( (viennacl::traits::size1(A) == viennacl::traits::size1(C)) && bool("Size mismatch in C = prod(A, B): size1(A) != size1(C)"));
        assert( (viennacl::traits::size2(A) == viennacl::traits::size1(B)) && bool("Size mismatch in C = prod(A, B): size2(A) != size1(B)"));
        assert( (viennacl::traits::size2(B) == viennacl::traits::size2(C)) && bool("Size mismatch in C = prod(A, B): size2(B) != size2(C)"));
        
        // Inplace matrix-vector products like B = prod(A, B) are currently illegal: Introduce a temporary like C = prod(A, B); B = C; instead
        assert(  (viennacl::traits::handle(C) != viennacl::traits::handle(A))
              && (viennacl::traits::handle(C) != viennacl::traits::handle(B))
              && bool("No direct inplace matrix-matrix product possible. Introduce a temporary!"));

        
        detail::prod(A, B, C, alpha, beta, "prod16_AA", "prod_AA");
      }



      /** @brief Carries out matrix-matrix multiplication
      *
      * Implementation of C = prod(trans(A), B);
      *
      */
      template <typename T1, typename T2, typename T3, typename ScalarType >
      typename viennacl::enable_if<    viennacl::is_any_dense_nonstructured_matrix<T1>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<T2>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<T3>::value
                                  >::type
      prod_impl(const viennacl::matrix_expression< const T1,
                                                  const T1,
                                                  op_trans> & A, 
                const T2 & B, 
                      T3 & C,
                ScalarType alpha,
                ScalarType beta)
      {
        //std::cout << "size2(A): " << viennacl::traits::size2(A.lhs()) << std::endl;
        //std::cout << "size1(C): " << viennacl::traits::size1(C) << std::endl;
        assert( (viennacl::traits::size2(A.lhs()) == viennacl::traits::size1(C)) && bool("Size mismatch in C = prod(trans(A), B): size2(A) != size1(C)"));
        assert( (viennacl::traits::size1(A.lhs()) == viennacl::traits::size1(B)) && bool("Size mismatch in C = prod(trans(A), B): size1(A) != size1(B)"));
        assert( (viennacl::traits::size2(B)       == viennacl::traits::size2(C)) && bool("Size mismatch in C = prod(trans(A), B): size2(B) != size2(C)"));
        
        // Inplace matrix-vector products like B = prod(A, B) are currently illegal: Introduce a temporary like C = prod(A, B); B = C; instead
        assert(  (viennacl::traits::handle(C) != viennacl::traits::handle(A.lhs()))
              && (viennacl::traits::handle(C) != viennacl::traits::handle(B))
              && bool("No direct inplace matrix-matrix product possible. Introduce a temporary!"));
        
        detail::prod(A.lhs(), B, C, alpha, beta, "prod16_TA", "prod_TA");
      }




      /** @brief Carries out matrix-matrix multiplication
      *
      * Implementation of C = prod(A, trans(B));
      *
      */
      template <typename T1, typename T2, typename T3, typename ScalarType >
      typename viennacl::enable_if<    viennacl::is_any_dense_nonstructured_matrix<T1>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<T2>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<T3>::value
                                  >::type
      prod_impl(const T1 & A, 
                const viennacl::matrix_expression< const T2,
                                                  const T2,
                                                  op_trans> & B,
                      T3 & C,
                ScalarType alpha,
                ScalarType beta)
      {
        assert( (viennacl::traits::size1(A)       == viennacl::traits::size1(C))       && bool("Size mismatch in C = prod(A, trans(B)): size1(A) != size1(C)"));
        assert( (viennacl::traits::size2(A)       == viennacl::traits::size2(B.lhs())) && bool("Size mismatch in C = prod(A, trans(B)): size2(A) != size2(B)"));
        assert( (viennacl::traits::size1(B.lhs()) == viennacl::traits::size2(C))       && bool("Size mismatch in C = prod(A, trans(B)): size1(B) != size2(C)"));
        
        // Inplace matrix-vector products like B = prod(A, B) are currently illegal: Introduce a temporary like C = prod(A, B); B = C; instead
        assert(  (viennacl::traits::handle(C) != viennacl::traits::handle(A))
              && (viennacl::traits::handle(C) != viennacl::traits::handle(B.lhs()))
              && bool("No direct inplace matrix-matrix product possible. Introduce a temporary!"));
        
        detail::prod(A, B.lhs(), C, alpha, beta, "prod16_AT", "prod_AT");
      }



      /** @brief Carries out matrix-matrix multiplication
      *
      * Implementation of C = prod(trans(A), trans(B));
      *
      */
      template <typename T1, typename T2, typename T3, typename ScalarType >
      typename viennacl::enable_if<    viennacl::is_any_dense_nonstructured_matrix<T1>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<T2>::value
                                    && viennacl::is_any_dense_nonstructured_matrix<T3>::value
                                  >::type
      prod_impl(const viennacl::matrix_expression< const T1, const T1, op_trans> & A,
                const viennacl::matrix_expression< const T2, const T2, op_trans> & B,
                T3 & C,
                ScalarType alpha,
                ScalarType beta)
      {
        assert(viennacl::traits::size2(A.lhs()) == viennacl::traits::size1(C)       && bool("Size mismatch in C = prod(trans(A), trans(B)): size2(A) != size1(C)"));
        assert(viennacl::traits::size1(A.lhs()) == viennacl::traits::size2(B.lhs()) && bool("Size mismatch in C = prod(trans(A), trans(B)): size1(A) != size2(B)"));
        assert(viennacl::traits::size1(B.lhs()) == viennacl::traits::size2(C)       && bool("Size mismatch in C = prod(trans(A), trans(B)): size1(B) != size2(C)"));
        
        // Inplace matrix-vector products like B = prod(A, B) are currently illegal: Introduce a temporary like C = prod(A, B); B = C; instead
        assert(  (viennacl::traits::handle(C) != viennacl::traits::handle(A.lhs()))
              && (viennacl::traits::handle(C) != viennacl::traits::handle(B.lhs()))
              && bool("No direct inplace matrix-matrix product possible. Introduce a temporary!"));
        
        detail::prod(A.lhs(), B.lhs(), C, alpha, beta, "prod16_TT", "prod_TT");
      }




      //
      /////////////////////////   miscellaneous operations /////////////////////////////////
      //

      
      /** @brief The implementation of the operation mat += alpha * vec1 * vec2^T, i.e. a scaled rank 1 update
      *
      * Implementation of the convenience expression result += alpha * outer_prod(vec1, vec2);
      *
      * @param mat1    The matrix to be updated
      * @param val     The scaling factor
      * @param vec1    The first vector
      * @param vec2    The second vector
      */
      template <typename M1, typename S1, typename V1, typename V2>
      typename viennacl::enable_if<    viennacl::is_any_dense_nonstructured_matrix<M1>::value
                                    && viennacl::is_any_scalar<S1>::value
                                    && viennacl::is_any_dense_nonstructured_vector<V1>::value
                                    && viennacl::is_any_dense_nonstructured_vector<V2>::value
                                  >::type
      scaled_rank_1_update(M1 & mat1,
                    S1 const & alpha, std::size_t len_alpha, bool reciprocal_alpha, bool flip_sign_alpha,
                    const V1 & vec1, 
                    const V2 & vec2)
      {
        assert( (viennacl::traits::size1(mat1) == viennacl::traits::size(vec1)) && bool("Size mismatch in scaled_rank_1_update: size1(A) != size(v1)"));
        assert( (viennacl::traits::size2(mat1) == viennacl::traits::size(vec2)) && bool("Size mismatch in scaled_rank_1_update: size2(A) != size(v2)"));

        typedef typename viennacl::result_of::cpu_value_type<V1>::type        value_type;
        
        unsigned int options_alpha =   ((len_alpha > 1) ? (len_alpha << 2) : 0)
                                    + (reciprocal_alpha ?                2 : 0)
                                     + (flip_sign_alpha ?                1 : 0);
        
        value_type temporary_alpha;                             
        if (viennacl::is_cpu_scalar<S1>::value)
          temporary_alpha = alpha;

        if (viennacl::is_row_major<M1>::value)
        {
          scaled_rank1_update_row_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat1), 
                                                       static_cast<unsigned int>(viennacl::traits::start1(mat1)),           static_cast<unsigned int>(viennacl::traits::start2(mat1)), 
                                                       static_cast<unsigned int>(viennacl::traits::stride1(mat1)),          static_cast<unsigned int>(viennacl::traits::stride2(mat1)),
                                                       static_cast<unsigned int>(viennacl::traits::size1(mat1)),            static_cast<unsigned int>(viennacl::traits::size2(mat1)),
                                                       static_cast<unsigned int>(viennacl::traits::internal_size1(mat1)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat1)),
                                                      
                                                       detail::cuda_arg<value_type>(detail::arg_reference(alpha, temporary_alpha)),
                                                       options_alpha,
                                                      
                                                       detail::cuda_arg<value_type>(vec1),
                                                       static_cast<unsigned int>(viennacl::traits::start(vec1)),
                                                       static_cast<unsigned int>(viennacl::traits::stride(vec1)),
                                                       static_cast<unsigned int>(viennacl::traits::size(vec1)),
                                                      
                                                       detail::cuda_arg<value_type>(vec2),
                                                       static_cast<unsigned int>(viennacl::traits::start(vec2)),
                                                       static_cast<unsigned int>(viennacl::traits::stride(vec2)),
                                                       static_cast<unsigned int>(viennacl::traits::size(vec2))
                                                     );
          VIENNACL_CUDA_LAST_ERROR_CHECK("scaled_rank1_update_row_kernel");
        }
        else
        {
          scaled_rank1_update_col_kernel<<<128, 128>>>(detail::cuda_arg<value_type>(mat1), 
                                                       static_cast<unsigned int>(viennacl::traits::start1(mat1)),           static_cast<unsigned int>(viennacl::traits::start2(mat1)), 
                                                       static_cast<unsigned int>(viennacl::traits::stride1(mat1)),          static_cast<unsigned int>(viennacl::traits::stride2(mat1)),
                                                       static_cast<unsigned int>(viennacl::traits::size1(mat1)),            static_cast<unsigned int>(viennacl::traits::size2(mat1)),
                                                       static_cast<unsigned int>(viennacl::traits::internal_size1(mat1)),   static_cast<unsigned int>(viennacl::traits::internal_size2(mat1)),
                                                      
                                                       detail::cuda_arg<value_type>(detail::arg_reference(alpha, temporary_alpha)),
                                                       options_alpha,
                                                      
                                                       detail::cuda_arg<value_type>(vec1),
                                                       static_cast<unsigned int>(viennacl::traits::start(vec1)),
                                                       static_cast<unsigned int>(viennacl::traits::stride(vec1)),
                                                       static_cast<unsigned int>(viennacl::traits::size(vec1)),
                                                      
                                                       detail::cuda_arg<value_type>(vec2),
                                                       static_cast<unsigned int>(viennacl::traits::start(vec2)),
                                                       static_cast<unsigned int>(viennacl::traits::stride(vec2)),
                                                       static_cast<unsigned int>(viennacl::traits::size(vec2))
                                                      );
          VIENNACL_CUDA_LAST_ERROR_CHECK("scaled_rank1_update_col_kernel");
        }
      }

    } // namespace opencl
  } //namespace linalg
} //namespace viennacl


#endif