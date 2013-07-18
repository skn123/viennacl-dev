#ifndef VIENNACL_GENERATOR_GENERATE_UTILS_HPP
#define VIENNACL_GENERATOR_GENERATE_UTILS_HPP

/* =========================================================================
   Copyright (c) 2010-2013, Institute for Microelectronics,
                            Institute for Analysis and Scientific Computing,
                            TU Wien.
   Portions of this software are copyright by UChicago Argonne, LLC.

                            -----------------
                  ViennaCL - The Vienna Computing Library
                            -----------------

   Project Head:    Karl Rupp                   rupp@iue.tuwien.ac.at

   (A list of authors and contributors can be found in the PDF manual)

   License:         MIT (X11), see file LICENSE in the base directory
============================================================================= */


/** @file viennacl/generator/generate_utils.hpp
    @brief Internal upper layer to the scheduler
*/

#include <set>

#include "CL/cl.h"

#include "viennacl/forwards.h"
#include "viennacl/scheduler/forwards.h"

#include "viennacl/generator/forwards.h"

namespace viennacl{

  namespace generator{

    namespace detail{

      std::string generate_value_kernel_argument(std::string const & scalartype, std::string const & name){
        return scalartype + ' ' + name + ",";
      }

      std::string generate_pointer_kernel_argument(std::string const & address_space, std::string const & scalartype, std::string const & name){
        return "__global " +  scalartype + "* " + name + ",";
      }

      const char * generate(operation_node_type type){
        // unary expression
        switch(type){
          case OPERATION_UNARY_ABS_TYPE : return "abs";
          case OPERATION_BINARY_ASSIGN_TYPE : return "=";
          case OPERATION_BINARY_ADD_TYPE : return "+";
          case OPERATION_BINARY_ACCESS : return "";
          default : throw "not implemented";
        }
      }

      class traversal_functor{
        public:
          void call_on_op(operation_node_type_family, operation_node_type type) const { }
          void call_before_expansion() const { }
          void call_after_expansion() const { }
      };

      class expression_generation_traversal : public traversal_functor{
        private:
          std::string index_string_;
          std::string & str_;
          mapping_type const & mapping_;
        public:
          expression_generation_traversal(std::string const & index, std::string & str, mapping_type const & mapping) : index_string_(index), str_(str), mapping_(mapping){ }
          void call_on_leaf(index_info const & key, statement_node const & node,  statement::container_type const * array) const { str_ += generate(index_string_,*mapping_.at(key)); }
          void call_on_op(operation_node_type_family, operation_node_type type) const { str_ += detail::generate(type); }
          void call_before_expansion() const { str_ += '('; }
          void call_after_expansion() const { str_ += ')';  }
      };


      index_info get_new_key(statement_node_type_family type_family, std::size_t current_index, std::size_t next_index, node_type node_tag){
        if(type_family==COMPOSITE_OPERATION_FAMILY)
          return std::make_pair(next_index, PARENT_TYPE);
        else
          return std::make_pair(current_index, node_tag);
      }


      template<class TraversalFunctor>
      void traverse(statement::container_type const & array, TraversalFunctor const & fun, bool deep_traversal, index_info const & key){
        std::size_t index = key.first;
        std::size_t node_tag = key.second;
        statement::value_type const & element = array[index];
        operation_node_type op_type = element.op_type_;
        operation_node_type_family op_family = element.op_family_;
        if(node_tag == PARENT_TYPE){
          if(op_family==OPERATION_UNARY_TYPE_FAMILY){
            fun.call_on_op(op_family, op_type);
            fun.call_before_expansion();
            traverse(array, fun, deep_traversal, get_new_key(element.lhs_type_family_, index, element.lhs_.node_index_, LHS_NODE_TYPE));
            fun.call_after_expansion();
          }
          if(op_family==OPERATION_BINARY_TYPE_FAMILY){
            if(op_type==OPERATION_BINARY_ACCESS){
              fun.call_on_leaf(key, element, &array);
              if(deep_traversal)
                traverse(array, fun, deep_traversal, get_new_key(element.rhs_type_family_, index, element.rhs_.node_index_, RHS_NODE_TYPE));
            }
            else{
              bool is_binary_leaf = (op_type==OPERATION_BINARY_PROD_TYPE)
                  ||(op_type==OPERATION_BINARY_INNER_PROD_TYPE);
              bool recurse = !is_binary_leaf || (is_binary_leaf && deep_traversal);
              if(is_binary_leaf)
                fun.call_on_leaf(key, element, &array);
              if(recurse){
                fun.call_before_expansion();
                traverse(array, fun, deep_traversal, get_new_key(element.lhs_type_family_, index, element.lhs_.node_index_, LHS_NODE_TYPE));
                fun.call_on_op(op_family, op_type);
                traverse(array, fun, deep_traversal, get_new_key(element.rhs_type_family_, index, element.rhs_.node_index_, RHS_NODE_TYPE));
                fun.call_after_expansion();
              }
            }
          }
        }
        else
          fun.call_on_leaf(key, element, &array);
      }

    }

  }

}
#endif
