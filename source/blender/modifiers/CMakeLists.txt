# ***** BEGIN GPL LICENSE BLOCK *****
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# The Original Code is Copyright (C) 2006, Blender Foundation
# All rights reserved.
# ***** END GPL LICENSE BLOCK *****

set(INC
  .
  intern
  ../blenfont
  ../blenkernel
  ../blenlib
  ../blenloader
  ../blentranslation
  ../bmesh
  ../depsgraph
  ../editors/include
  ../functions
  ../geometry
  ../makesdna
  ../makesrna
  ../nodes
  ../render
  ../windowmanager
  ../../../intern/eigen
  ../../../intern/guardedalloc

  # dna_type_offsets.h in BLO_read_write.h
  ${CMAKE_BINARY_DIR}/source/blender/makesdna/intern
)

set(INC_SYS
  ${ZLIB_INCLUDE_DIRS}
)

set(SRC
  intern/MOD_armature.c
  intern/MOD_array.c
  intern/MOD_bevel.c
  intern/MOD_boolean.cc
  intern/MOD_build.c
  intern/MOD_cast.c
  intern/MOD_cloth.c
  intern/MOD_collision.c
  intern/MOD_correctivesmooth.c
  intern/MOD_curve.c
  intern/MOD_datatransfer.c
  intern/MOD_decimate.c
  intern/MOD_displace.c
  intern/MOD_dynamicpaint.c
  intern/MOD_edgesplit.c
  intern/MOD_explode.c
  intern/MOD_fluid.c
  intern/MOD_hook.c
  intern/MOD_laplaciandeform.c
  intern/MOD_laplaciansmooth.c
  intern/MOD_lattice.c
  intern/MOD_mask.cc
  intern/MOD_mesh_to_volume.cc
  intern/MOD_meshcache.c
  intern/MOD_meshcache_mdd.c
  intern/MOD_meshcache_pc2.c
  intern/MOD_meshcache_util.c
  intern/MOD_meshdeform.c
  intern/MOD_meshsequencecache.c
  intern/MOD_mirror.c
  intern/MOD_multires.c
  intern/MOD_nodes.cc
  intern/MOD_nodes_evaluator.cc
  intern/MOD_none.c
  intern/MOD_normal_edit.c
  intern/MOD_ocean.c
  intern/MOD_particleinstance.c
  intern/MOD_particlesystem.c
  intern/MOD_remesh.c
  intern/MOD_screw.c
  intern/MOD_shapekey.c
  intern/MOD_shrinkwrap.c
  intern/MOD_simpledeform.c
  intern/MOD_skin.c
  intern/MOD_smooth.c
  intern/MOD_softbody.c
  intern/MOD_solidify.c
  intern/MOD_solidify_extrude.c
  intern/MOD_solidify_nonmanifold.c
  intern/MOD_subsurf.c
  intern/MOD_surface.c
  intern/MOD_surfacedeform.c
  intern/MOD_triangulate.c
  intern/MOD_ui_common.c
  intern/MOD_util.c
  intern/MOD_uvproject.c
  intern/MOD_uvwarp.c
  intern/MOD_volume_displace.cc
  intern/MOD_volume_to_mesh.cc
  intern/MOD_warp.c
  intern/MOD_wave.c
  intern/MOD_weighted_normal.c
  intern/MOD_weightvg_util.c
  intern/MOD_weightvgedit.c
  intern/MOD_weightvgmix.c
  intern/MOD_weightvgproximity.c
  intern/MOD_weld.cc
  intern/MOD_wireframe.c

  MOD_modifiertypes.h
  MOD_nodes.h
  intern/MOD_meshcache_util.h
  intern/MOD_nodes_evaluator.hh
  intern/MOD_solidify_util.h
  intern/MOD_ui_common.h
  intern/MOD_util.h
  intern/MOD_weightvg_util.h
)

set(LIB
  bf_blenkernel
  bf_blenlib
)

if(WITH_ALEMBIC)
  add_definitions(-DWITH_ALEMBIC)
  list(APPEND INC
    ../io/alembic
  )
  list(APPEND LIB
    bf_alembic
  )
endif()

if(WITH_USD)
  add_definitions(-DWITH_USD)
  list(APPEND INC
    ../io/usd
  )
  list(APPEND LIB
    bf_usd
  )
endif()

if(WITH_MOD_REMESH)
  list(APPEND INC
    ../../../intern/dualcon
  )
  list(APPEND LIB
    bf_intern_dualcon
  )
  add_definitions(-DWITH_MOD_REMESH)
endif()

if(WITH_MOD_FLUID)
  add_definitions(-DWITH_FLUID)
endif()

if(WITH_MOD_OCEANSIM)
  add_definitions(-DWITH_OCEANSIM)
endif()

if(WITH_OPENSUBDIV)
  add_definitions(-DWITH_OPENSUBDIV)
endif()

if(WITH_BULLET)
  if(NOT WITH_SYSTEM_BULLET)
    list(APPEND LIB
      extern_bullet
    )
  endif()
  add_definitions(-DWITH_BULLET)
endif()

if(WITH_INTERNATIONAL)
  add_definitions(-DWITH_INTERNATIONAL)
endif()

# To disable adaptive subdivision test in subsurf UI without cycles
if(WITH_CYCLES)
  add_definitions(-DWITH_CYCLES)
endif()

if(WITH_GMP)
  add_definitions(-DWITH_GMP)

  list(APPEND INC_SYS
    ${GMP_INCLUDE_DIRS}
  )

  list(APPEND LIB
    ${GMP_LIBRARIES}
  )
endif()

if(WITH_TBB)
  add_definitions(-DWITH_TBB)
  if(WIN32)
    # TBB includes Windows.h which will define min/max macros
    # that will collide with the stl versions.
    add_definitions(-DNOMINMAX)
  endif()
  list(APPEND INC_SYS
    ${TBB_INCLUDE_DIRS}
  )

  list(APPEND LIB
    ${TBB_LIBRARIES}
  )
endif()

if(WITH_OPENVDB)
  list(APPEND INC
    ../../../intern/openvdb
  )
  list(APPEND INC_SYS
    ${OPENVDB_INCLUDE_DIRS}
  )
  list(APPEND LIB
    bf_intern_openvdb
    ${OPENVDB_LIBRARIES}
  )
  add_definitions(-DWITH_OPENVDB ${OPENVDB_DEFINITIONS})
endif()

if(WITH_EXPERIMENTAL_FEATURES)
  add_definitions(-DWITH_SIMULATION_DATABLOCK)
  add_definitions(-DWITH_POINT_CLOUD)
  add_definitions(-DWITH_HAIR_NODES)
endif()

# So we can have special tricks in modifier system.

blender_add_lib(bf_modifiers "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")

# Some modifiers include BLO_read_write.h, which includes dna_type_offsets.h
# which is generated by bf_dna. Need to ensure compilaiton order here.
# Also needed so we can use dna_type_offsets.h for defaults initialization.
add_dependencies(bf_modifiers bf_dna)
