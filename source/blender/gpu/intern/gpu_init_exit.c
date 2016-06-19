/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
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
 * The Original Code is Copyright (C) 2013 Blender Foundation.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Jason Wilkins
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file source/blender/gpu/intern/gpu_init_exit.c
 *  \ingroup gpu
 */

#include "BLI_sys_types.h"
#include "GPU_init_exit.h"  /* interface */
#include "GPU_immediate.h"
#include "GPU_aspect.h"

#include "BKE_global.h"

#include "intern/gpu_codegen.h"
#include "intern/gpu_private.h"

/**
 * although the order of initialization and shutdown should not matter
 * (except for the extensions), I chose alphabetical and reverse alphabetical order
 */

static bool initialized = false;
static GPUImmediate *gpu_immediate;
static GPUindex *gpu_index;


void GPU_init(void)
{
	/* can't avoid calling this multiple times, see wm_window_ghostwindow_add */
	if (initialized)
		return;

	initialized = true;

	gpu_extensions_init(); /* must come first */
	gpu_aspect_init();
	gpu_basic_init();
	gpu_blender_aspect_init();
	gpu_clipping_init();
	gpu_codegen_init();
	gpu_common_init();
	gpu_font_init();
	gpu_immediate_init();
	gpu_lighting_init();
	gpu_matrix_init();
	gpu_pixels_init();
	gpu_raster_init();
//	gpu_select_init();
	gpu_sprite_init();
	gpu_state_latch_init();

	gpu_immediate = gpuNewImmediate();
	gpuImmediateMakeCurrent(gpu_immediate);
	gpuImmediateMaxVertexCount(500000); // XXX jwilkins: temporary!

	gpu_index = gpuNewIndex();
	gpuImmediateIndex(gpu_index);
	gpuImmediateMaxIndexCount(50000, GL_UNSIGNED_SHORT); // XXX jwilkins: temporary!

	GPU_aspect_begin(GPU_ASPECT_BASIC, NULL);

	if (G.debug & G_DEBUG_GPU)
		gpu_debug_init();
}



void GPU_exit(void)
{
	BLI_assert(initialized);

	GPU_aspect_end();

	gpuDeleteIndex(gpu_index);
	gpuImmediateIndex(NULL);

	gpuImmediateMakeCurrent(NULL);
	gpuDeleteImmediate(gpu_immediate);

	gpu_state_latch_exit();
	gpu_sprite_exit();
	gpu_select_exit();
	gpu_raster_exit();
	gpu_pixels_exit();
	gpu_matrix_exit();
	gpu_lighting_exit();
	gpu_immediate_exit();
	gpu_font_exit();
	gpu_common_exit();

	if (G.debug & G_DEBUG_GPU)
		gpu_debug_exit();

	gpu_codegen_exit();
	gpu_clipping_exit();
	gpu_blender_aspect_exit();
	gpu_basic_exit();
	gpu_aspect_exit();

	gpu_extensions_exit(); /* must come last */

	initialized = false;
}
