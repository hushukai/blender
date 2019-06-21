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

#include "../node_shader_util.h"

/* **************** VORONOI ******************** */

static bNodeSocketTemplate sh_node_tex_voronoi_in[] = {
    {SOCK_VECTOR, 1, N_("Vector"), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, PROP_NONE, SOCK_HIDE_VALUE},
    {SOCK_FLOAT, 1, N_("W"), 0.0f, 0.0f, 0.0f, 0.0f, -1000.0f, 1000.0f},
    {SOCK_FLOAT, 1, N_("Scale"), 5.0f, 0.0f, 0.0f, 0.0f, -1000.0f, 1000.0f},
    {SOCK_FLOAT, 1, N_("Smoothness"), 5.0f, 0.0f, 0.0f, 0.0f, 3.5f, 75.0f},
    {SOCK_FLOAT, 1, N_("Exponent"), 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 32.0f},
    {SOCK_FLOAT, 1, N_("Jitter"), 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, PROP_FACTOR},
    {-1, 0, ""},
};

static bNodeSocketTemplate sh_node_tex_voronoi_out[] = {
    {SOCK_FLOAT,
     0,
     N_("Distance"),
     0.0f,
     0.0f,
     0.0f,
     0.0f,
     0.0f,
     1.0f,
     PROP_NONE,
     SOCK_NO_INTERNAL_LINK},
    {SOCK_RGBA,
     0,
     N_("Color"),
     0.0f,
     0.0f,
     0.0f,
     0.0f,
     0.0f,
     1.0f,
     PROP_NONE,
     SOCK_NO_INTERNAL_LINK},
    {SOCK_VECTOR,
     0,
     N_("Position"),
     0.0f,
     0.0f,
     0.0f,
     0.0f,
     0.0f,
     1.0f,
     PROP_NONE,
     SOCK_NO_INTERNAL_LINK},
    {SOCK_FLOAT, 0, N_("W"), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, PROP_NONE, SOCK_NO_INTERNAL_LINK},
    {SOCK_FLOAT,
     0,
     N_("Radius"),
     0.0f,
     0.0f,
     0.0f,
     0.0f,
     0.0f,
     1.0f,
     PROP_NONE,
     SOCK_NO_INTERNAL_LINK},
    {-1, 0, ""},
};

static void node_shader_init_tex_voronoi(bNodeTree *UNUSED(ntree), bNode *node)
{
  NodeTexVoronoi *tex = MEM_callocN(sizeof(NodeTexVoronoi), "NodeTexVoronoi");
  BKE_texture_mapping_default(&tex->base.tex_mapping, TEXMAP_TYPE_POINT);
  BKE_texture_colormapping_default(&tex->base.color_mapping);
  tex->dimensions = 3;
  tex->distance = SHD_VORONOI_EUCLIDEAN;
  tex->feature = SHD_VORONOI_F1;

  node->storage = tex;
}

static int node_shader_gpu_tex_voronoi(GPUMaterial *mat,
                                       bNode *node,
                                       bNodeExecData *UNUSED(execdata),
                                       GPUNodeStack *in,
                                       GPUNodeStack *out)
{
  if (!in[0].link) {
    in[0].link = GPU_attribute(CD_ORCO, "");
    GPU_link(mat, "generated_texco", GPU_builtin(GPU_VIEW_POSITION), in[0].link, &in[0].link);
  }
  node_shader_gpu_tex_mapping(mat, node, in, out);

  NodeTexVoronoi *tex = (NodeTexVoronoi *)node->storage;
  float metric = tex->distance;

  switch (tex->dimensions) {
    case 1:
      switch (tex->feature) {
        case SHD_VORONOI_F1:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_f1_1d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_SMOOTH_F1:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_smooth_f1_1d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_F2:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_f2_1d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_DISTANCE_TO_EDGE:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_distance_to_edge_1d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_N_SPHERE_RADIUS:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_n_sphere_radius_1d", in, out, GPU_constant(&metric));
        default:
          return 0;
      }
    case 2:
      switch (tex->feature) {
        case SHD_VORONOI_F1:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_f1_2d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_SMOOTH_F1:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_smooth_f1_2d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_F2:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_f2_2d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_DISTANCE_TO_EDGE:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_distance_to_edge_2d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_N_SPHERE_RADIUS:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_n_sphere_radius_2d", in, out, GPU_constant(&metric));
        default:
          return 0;
      }
    case 3:
      switch (tex->feature) {
        case SHD_VORONOI_F1:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_f1_3d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_SMOOTH_F1:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_smooth_f1_3d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_F2:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_f2_3d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_DISTANCE_TO_EDGE:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_distance_to_edge_3d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_N_SPHERE_RADIUS:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_n_sphere_radius_3d", in, out, GPU_constant(&metric));
        default:
          return 0;
      }
    case 4:
      switch (tex->feature) {
        case SHD_VORONOI_F1:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_f1_4d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_SMOOTH_F1:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_smooth_f1_4d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_F2:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_f2_4d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_DISTANCE_TO_EDGE:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_distance_to_edge_4d", in, out, GPU_constant(&metric));
        case SHD_VORONOI_N_SPHERE_RADIUS:
          return GPU_stack_link(
              mat, node, "node_tex_voronoi_n_sphere_radius_4d", in, out, GPU_constant(&metric));
        default:
          return 0;
      }
    default:
      return 0;
  }
}

static void node_shader_update_tex_voronoi(bNodeTree *UNUSED(ntree), bNode *node)
{
  NodeTexVoronoi *tex = (NodeTexVoronoi *)node->storage;

  bNodeSocket *inVecSock = BLI_findlink(&node->inputs, 0);
  bNodeSocket *inWSock = BLI_findlink(&node->inputs, 1);
  bNodeSocket *inSmoothnessSock = BLI_findlink(&node->inputs, 3);
  bNodeSocket *inExponentSock = BLI_findlink(&node->inputs, 4);

  bNodeSocket *outDistanceSock = BLI_findlink(&node->outputs, 0);
  bNodeSocket *outColorSock = BLI_findlink(&node->outputs, 1);
  bNodeSocket *outPositionSock = BLI_findlink(&node->outputs, 2);
  bNodeSocket *outWSock = BLI_findlink(&node->outputs, 3);
  bNodeSocket *outRadiusSock = BLI_findlink(&node->outputs, 4);

  if (tex->dimensions == 1 || tex->dimensions == 4) {
    inWSock->flag &= ~SOCK_UNAVAIL;
  }
  else {
    inWSock->flag |= SOCK_UNAVAIL;
  }

  if (tex->dimensions == 1) {
    inVecSock->flag |= SOCK_UNAVAIL;
  }
  else {
    inVecSock->flag &= ~SOCK_UNAVAIL;
  }

  if (tex->distance == SHD_VORONOI_MINKOWSKI) {
    inExponentSock->flag &= ~SOCK_UNAVAIL;
  }
  else {
    inExponentSock->flag |= SOCK_UNAVAIL;
  }

  if (tex->feature == SHD_VORONOI_SMOOTH_F1) {
    inSmoothnessSock->flag &= ~SOCK_UNAVAIL;
  }
  else {
    inSmoothnessSock->flag |= SOCK_UNAVAIL;
  }

  if (tex->feature == SHD_VORONOI_N_SPHERE_RADIUS) {
    outDistanceSock->flag |= SOCK_UNAVAIL;
  }
  else {
    outDistanceSock->flag &= ~SOCK_UNAVAIL;
  }

  if (tex->feature != SHD_VORONOI_DISTANCE_TO_EDGE &&
      tex->feature != SHD_VORONOI_N_SPHERE_RADIUS) {
    outColorSock->flag &= ~SOCK_UNAVAIL;
  }
  else {
    outColorSock->flag |= SOCK_UNAVAIL;
  }

  if (tex->feature != SHD_VORONOI_DISTANCE_TO_EDGE &&
      tex->feature != SHD_VORONOI_N_SPHERE_RADIUS && tex->dimensions != 1) {
    outPositionSock->flag &= ~SOCK_UNAVAIL;
  }
  else {
    outPositionSock->flag |= SOCK_UNAVAIL;
  }

  if (tex->feature != SHD_VORONOI_DISTANCE_TO_EDGE &&
      tex->feature != SHD_VORONOI_N_SPHERE_RADIUS &&
      (tex->dimensions == 1 || tex->dimensions == 4)) {
    outWSock->flag &= ~SOCK_UNAVAIL;
  }
  else {
    outWSock->flag |= SOCK_UNAVAIL;
  }

  if (tex->feature == SHD_VORONOI_N_SPHERE_RADIUS) {
    outRadiusSock->flag &= ~SOCK_UNAVAIL;
  }
  else {
    outRadiusSock->flag |= SOCK_UNAVAIL;
  }
}

void register_node_type_sh_tex_voronoi(void)
{
  static bNodeType ntype;

  sh_node_type_base(&ntype, SH_NODE_TEX_VORONOI, "Voronoi Texture", NODE_CLASS_TEXTURE, 0);
  node_type_socket_templates(&ntype, sh_node_tex_voronoi_in, sh_node_tex_voronoi_out);
  node_type_init(&ntype, node_shader_init_tex_voronoi);
  node_type_storage(
      &ntype, "NodeTexVoronoi", node_free_standard_storage, node_copy_standard_storage);
  node_type_gpu(&ntype, node_shader_gpu_tex_voronoi);
  node_type_update(&ntype, node_shader_update_tex_voronoi);

  nodeRegisterType(&ntype);
}
