/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2005 Blender Foundation.
 * All rights reserved.
 */

/** \file
 * \ingroup shdnodes
 */

#include "node_shader_util.h"

#include "NOD_math_functions.hh"
#include "NOD_socket_search_link.hh"

#include "RNA_enum_types.h"

/* **************** SCALAR MATH ******************** */

namespace blender::nodes::node_shader_math_cc {

static void sh_node_math_declare(NodeDeclarationBuilder &b)
{
  b.is_function_node();
  b.add_input<decl::Float>(N_("Value")).default_value(0.5f).min(-10000.0f).max(10000.0f);
  b.add_input<decl::Float>(N_("Value"), "Value_001")
      .default_value(0.5f)
      .min(-10000.0f)
      .max(10000.0f);
  b.add_input<decl::Float>(N_("Value"), "Value_002")
      .default_value(0.5f)
      .min(-10000.0f)
      .max(10000.0f);
  b.add_output<decl::Float>(N_("Value"));
};

class SocketSearchOp {
 public:
  std::string socket_name;
  NodeMathOperation mode = NODE_MATH_ADD;
  void operator()(LinkSearchOpParams &params)
  {
    bNode &node = params.add_node("ShaderNodeMath");
    node.custom1 = mode;
    params.update_and_connect_available_socket(node, socket_name);
  }
};

static void sh_node_math_gather_link_searches(GatherLinkSearchOpParams &params)
{
  const NodeDeclaration &declaration = *params.node_type().fixed_declaration;
  if (params.in_out() == SOCK_OUT) {
    search_link_ops_for_declarations(params, declaration.outputs());
    return;
  }

  /* Expose first Value socket. */
  if (params.node_tree().typeinfo->validate_link(
          static_cast<eNodeSocketDatatype>(params.other_socket().type), SOCK_FLOAT)) {

    const int weight = ELEM(params.other_socket().type, SOCK_FLOAT, SOCK_BOOLEAN, SOCK_INT) ? 0 :
                                                                                              -1;

    for (const EnumPropertyItem *item = rna_enum_node_math_items; item->identifier != nullptr;
         item++) {
      if (item->name != nullptr && item->identifier[0] != '\0') {
        params.add_item(
            IFACE_(item->name), SocketSearchOp{"Value", (NodeMathOperation)item->value}, weight);
      }
    }
  }
}

static const char *gpu_shader_get_name(int mode)
{
  const blender::nodes::FloatMathOperationInfo *info =
      blender::nodes::get_float_math_operation_info(mode);
  if (!info) {
    return nullptr;
  }
  if (info->shader_name.is_empty()) {
    return nullptr;
  }
  return info->shader_name.c_str();
}

static int gpu_shader_math(GPUMaterial *mat,
                           bNode *node,
                           bNodeExecData *UNUSED(execdata),
                           GPUNodeStack *in,
                           GPUNodeStack *out)
{
  const char *name = gpu_shader_get_name(node->custom1);
  if (name != nullptr) {
    int ret = GPU_stack_link(mat, node, name, in, out);

    if (ret && node->custom2 & SHD_MATH_CLAMP) {
      float min[3] = {0.0f, 0.0f, 0.0f};
      float max[3] = {1.0f, 1.0f, 1.0f};
      GPU_link(
          mat, "clamp_value", out[0].link, GPU_constant(min), GPU_constant(max), &out[0].link);
    }
    return ret;
  }

  return 0;
}

static const blender::fn::MultiFunction *get_base_multi_function(bNode &node)
{
  const int mode = node.custom1;
  const blender::fn::MultiFunction *base_fn = nullptr;

  blender::nodes::try_dispatch_float_math_fl_to_fl(
      mode, [&](auto function, const blender::nodes::FloatMathOperationInfo &info) {
        static blender::fn::CustomMF_SI_SO<float, float> fn{info.title_case_name.c_str(),
                                                            function};
        base_fn = &fn;
      });
  if (base_fn != nullptr) {
    return base_fn;
  }

  blender::nodes::try_dispatch_float_math_fl_fl_to_fl(
      mode, [&](auto function, const blender::nodes::FloatMathOperationInfo &info) {
        static blender::fn::CustomMF_SI_SI_SO<float, float, float> fn{info.title_case_name.c_str(),
                                                                      function};
        base_fn = &fn;
      });
  if (base_fn != nullptr) {
    return base_fn;
  }

  blender::nodes::try_dispatch_float_math_fl_fl_fl_to_fl(
      mode, [&](auto function, const blender::nodes::FloatMathOperationInfo &info) {
        static blender::fn::CustomMF_SI_SI_SI_SO<float, float, float, float> fn{
            info.title_case_name.c_str(), function};
        base_fn = &fn;
      });
  if (base_fn != nullptr) {
    return base_fn;
  }

  return nullptr;
}

class ClampWrapperFunction : public blender::fn::MultiFunction {
 private:
  const blender::fn::MultiFunction &fn_;

 public:
  ClampWrapperFunction(const blender::fn::MultiFunction &fn) : fn_(fn)
  {
    this->set_signature(&fn.signature());
  }

  void call(blender::IndexMask mask,
            blender::fn::MFParams params,
            blender::fn::MFContext context) const override
  {
    fn_.call(mask, params, context);

    /* Assumes the output parameter is the last one. */
    const int output_param_index = this->param_amount() - 1;
    /* This has actually been initialized in the call above. */
    blender::MutableSpan<float> results = params.uninitialized_single_output<float>(
        output_param_index);

    for (const int i : mask) {
      float &value = results[i];
      CLAMP(value, 0.0f, 1.0f);
    }
  }
};

static void sh_node_math_build_multi_function(blender::nodes::NodeMultiFunctionBuilder &builder)
{
  const blender::fn::MultiFunction *base_function = get_base_multi_function(builder.node());

  const bool clamp_output = builder.node().custom2 != 0;
  if (clamp_output) {
    builder.construct_and_set_matching_fn<ClampWrapperFunction>(*base_function);
  }
  else {
    builder.set_matching_fn(base_function);
  }
}

}  // namespace blender::nodes::node_shader_math_cc

void register_node_type_sh_math()
{
  namespace file_ns = blender::nodes::node_shader_math_cc;

  static bNodeType ntype;

  sh_fn_node_type_base(&ntype, SH_NODE_MATH, "Math", NODE_CLASS_CONVERTER, 0);
  ntype.declare = file_ns::sh_node_math_declare;
  ntype.labelfunc = node_math_label;
  node_type_gpu(&ntype, file_ns::gpu_shader_math);
  node_type_update(&ntype, node_math_update);
  ntype.build_multi_function = file_ns::sh_node_math_build_multi_function;
  ntype.gather_link_search_ops = file_ns::sh_node_math_gather_link_searches;

  nodeRegisterType(&ntype);
}
