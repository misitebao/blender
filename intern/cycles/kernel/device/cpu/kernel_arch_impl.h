/*
 * Copyright 2011-2013 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Templated common implementation part of all CPU kernels.
 *
 * The idea is that particular .cpp files sets needed optimization flags and
 * simply includes this file without worry of copying actual implementation over.
 */

#pragma once

// clang-format off
#include "kernel/device/cpu/compat.h"

#ifndef KERNEL_STUB
#    include "kernel/device/cpu/globals.h"
#    include "kernel/device/cpu/image.h"

#    include "kernel/integrator/integrator_path_state.h"
#    include "kernel/integrator/integrator_state.h"
#    include "kernel/integrator/integrator_state_util.h"

#    include "kernel/integrator/integrator_init_from_camera.h"
#    include "kernel/integrator/integrator_intersect_closest.h"
#    include "kernel/integrator/integrator_intersect_shadow.h"
#    include "kernel/integrator/integrator_intersect_subsurface.h"
#    include "kernel/integrator/integrator_shade_background.h"
#    include "kernel/integrator/integrator_shade_light.h"
#    include "kernel/integrator/integrator_shade_shadow.h"
#    include "kernel/integrator/integrator_shade_surface.h"
#    include "kernel/integrator/integrator_shade_volume.h"
#    include "kernel/integrator/integrator_megakernel.h"

#    include "kernel/kernel_film.h"
#    include "kernel/kernel_adaptive_sampling.h"
#    include "kernel/kernel_bake.h"

#if 0
#    include "kernel/kernel_color.h"
#    include "kernel/kernel_path.h"
#    include "kernel/kernel_path_branched.h"
#    include "kernel/kernel_bake.h"
#endif
#else
#  define STUB_ASSERT(arch, name) \
    assert(!(#name " kernel stub for architecture " #arch " was called!"))
#endif   /* KERNEL_STUB */
// clang-format on

CCL_NAMESPACE_BEGIN

/* Integrator. */

#ifdef KERNEL_STUB
#  define KERNEL_INVOKE(name, ...) (STUB_ASSERT(KERNEL_ARCH, name), 0)
#else
#  define KERNEL_INVOKE(name, ...) integrator_##name(__VA_ARGS__)
#endif

#define DEFINE_INTEGRATOR_KERNEL(name) \
  void KERNEL_FUNCTION_FULL_NAME(integrator_##name)(const KernelGlobals *kg, \
                                                    IntegratorState *state) \
  { \
    KERNEL_INVOKE(name, kg, state); \
  }

#define DEFINE_INTEGRATOR_SHADE_KERNEL(name) \
  void KERNEL_FUNCTION_FULL_NAME(integrator_##name)( \
      const KernelGlobals *kg, IntegratorState *state, ccl_global float *render_buffer) \
  { \
    KERNEL_INVOKE(name, kg, state, render_buffer); \
  }

/* TODO: Either use something like get_work_pixel(), or simplify tile which is passed here, so
 * that it does not contain unused fields. */
#define DEFINE_INTEGRATOR_INIT_KERNEL(name) \
  bool KERNEL_FUNCTION_FULL_NAME(integrator_##name)(const KernelGlobals *kg, \
                                                    IntegratorState *state, \
                                                    KernelWorkTile *tile, \
                                                    ccl_global float *render_buffer) \
  { \
    return KERNEL_INVOKE( \
        name, kg, state, tile, render_buffer, tile->x, tile->y, tile->start_sample); \
  }

DEFINE_INTEGRATOR_INIT_KERNEL(init_from_camera)
DEFINE_INTEGRATOR_KERNEL(intersect_closest)
DEFINE_INTEGRATOR_KERNEL(intersect_shadow)
DEFINE_INTEGRATOR_KERNEL(intersect_subsurface)
DEFINE_INTEGRATOR_SHADE_KERNEL(shade_background)
DEFINE_INTEGRATOR_SHADE_KERNEL(shade_light)
DEFINE_INTEGRATOR_SHADE_KERNEL(shade_shadow)
DEFINE_INTEGRATOR_SHADE_KERNEL(shade_surface)
DEFINE_INTEGRATOR_SHADE_KERNEL(shade_volume)
DEFINE_INTEGRATOR_SHADE_KERNEL(megakernel)

/* Film. */

void KERNEL_FUNCTION_FULL_NAME(convert_to_half_float)(const KernelGlobals *kg,
                                                      uchar4 *rgba,
                                                      float *render_buffer,
                                                      float sample_scale,
                                                      int x,
                                                      int y,
                                                      int offset,
                                                      int stride)
{
#ifdef KERNEL_STUB
  STUB_ASSERT(KERNEL_ARCH, convert_to_half_float);
#else
  kernel_film_convert_to_half_float(kg, rgba, render_buffer, sample_scale, x, y, offset, stride);
#endif /* KERNEL_STUB */
}

/* Shader evaluation. */

void KERNEL_FUNCTION_FULL_NAME(shader_eval_displace)(const KernelGlobals *kg,
                                                     const KernelShaderEvalInput *input,
                                                     float4 *output,
                                                     const int offset)
{
#ifdef KERNEL_STUB
  STUB_ASSERT(KERNEL_ARCH, shader_eval_displace);
#else
  kernel_displace_evaluate(kg, input, output, offset);
#endif
}

void KERNEL_FUNCTION_FULL_NAME(shader_eval_background)(const KernelGlobals *kg,
                                                       const KernelShaderEvalInput *input,
                                                       float4 *output,
                                                       const int offset)
{
#ifdef KERNEL_STUB
  STUB_ASSERT(KERNEL_ARCH, shader_eval_background);
#else
  kernel_background_evaluate(kg, input, output, offset);
#endif
}

/* Adaptive sampling. */

bool KERNEL_FUNCTION_FULL_NAME(adaptive_sampling_convergence_check)(
    const KernelGlobals *kg,
    ccl_global float *render_buffer,
    int x,
    int y,
    float threshold,
    int offset,
    int stride)
{
#ifdef KERNEL_STUB
  STUB_ASSERT(KERNEL_ARCH, adaptive_sampling_convergence_check);
  return false;
#else
  return kernel_adaptive_sampling_convergence_check(
      kg, render_buffer, x, y, threshold, offset, stride);
#endif
}

void KERNEL_FUNCTION_FULL_NAME(adaptive_sampling_filter_x)(const KernelGlobals *kg,
                                                           ccl_global float *render_buffer,
                                                           int y,
                                                           int start_x,
                                                           int width,
                                                           int offset,
                                                           int stride)
{
#ifdef KERNEL_STUB
  STUB_ASSERT(KERNEL_ARCH, adaptive_sampling_filter_x);
#else
  kernel_adaptive_sampling_filter_x(kg, render_buffer, y, start_x, width, offset, stride);
#endif
}

void KERNEL_FUNCTION_FULL_NAME(adaptive_sampling_filter_y)(const KernelGlobals *kg,
                                                           ccl_global float *render_buffer,
                                                           int x,
                                                           int start_y,
                                                           int height,
                                                           int offset,
                                                           int stride)
{
#ifdef KERNEL_STUB
  STUB_ASSERT(KERNEL_ARCH, adaptive_sampling_filter_y);
#else
  kernel_adaptive_sampling_filter_y(kg, render_buffer, x, start_y, height, offset, stride);
#endif
}

/* Bake. */
/* TODO(sergey): Needs to be re-implemented. */

void KERNEL_FUNCTION_FULL_NAME(bake)(
    const KernelGlobals *kg, float *buffer, int sample, int x, int y, int offset, int stride)
{
#if 0
#  ifdef KERNEL_STUB
  STUB_ASSERT(KERNEL_ARCH, bake);
#  else
#    ifdef __BAKING__
  kernel_bake_evaluate(kg, buffer, sample, x, y, offset, stride);
#    endif
#  endif /* KERNEL_STUB */
#endif
}

#undef KERNEL_INVOKE
#undef DEFINE_INTEGRATOR_KERNEL
#undef DEFINE_INTEGRATOR_SHADE_KERNEL
#undef DEFINE_INTEGRATOR_INIT_KERNEL

#undef KERNEL_STUB
#undef STUB_ASSERT
#undef KERNEL_ARCH

CCL_NAMESPACE_END