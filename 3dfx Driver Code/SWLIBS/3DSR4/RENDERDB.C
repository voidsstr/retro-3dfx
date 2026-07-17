#include <string.h>
#include <conio.h>
#include <math.h>

#define AT_DEBUGGING
#ifndef GLIDE_HARDWARE
#define GLIDE_HARDWARE
#endif
#include "atrender.h"
#include "atmath.h"

#include "pxp.h"
#include "kxp.h"
#include "dialog.h"
#include "keys.h"

#include "kview.h"
#include "database.h"
#include "joy.h"

#define CLEAR_COLOR	(clear_color[0]),(clear_color[1]),(clear_color[2])

typedef FxU32 Triangle[3];

static float		clear_color[3];
static float		translation[3];
static float		rotation[3];
static float		step[3];
AtmVector3		centroid;
float			camz;
float			xwidth;
float			camdist;
static AtmVector3	position;
static AtmVector3	target;
static AtmVector3	direction;
static AtmVector3	horizontal;
AtrXform *		transform;
AtrCamera *		camera;
static AtmVector3	up;
static float		min[3];  /* bounding box for entire environment */
static float		max[3];
static int		anim_delay = 0;
static AtrMaterial *	default_mat = NULL;
static AtrXform *	object_xform = NULL;
static AtrXform *	cam_orientation = NULL;
static char		cam_name[11];
static AtrXform *	cam_xform = NULL;
static AtrCamera *	cam_cam = NULL;
static AtmVector3	cam_position;
static AtmVector3	cam_target;
static AtmVector3	cam_upv;


#ifdef DEBUG_OUTPUT
extern FILE *		output;
#endif

static int dummy_routine(int a) {
  
  int			b = a + 3;
  int			c = 2;

  return b + c + a;
}


static void SetLCStoWCS(
  AtrXform *		xform,
  float * 		tar,
  float * 		pos,
  float *		upv) {
  
  AtmVector3		dir;
  float *		m = &xform->data[0];
  float *		d = &dir[0];
  AtmVector3		r1;
  AtmVector3		r2;

  put_glide_fp_status();
  atmVector3Sub(dir, tar, pos);
  atmVector3Normalize(dir, dir);

  atmVector3Cross(r1, upv, dir);
  atmVector3Cross(r2, dir, r1);
  put_my_fp_status();

  m[0] = r1[0]; m[1] = r1[1]; m[2] = r1[2]; m[3] = 0;
  m[4] = r2[0]; m[5] = r2[1]; m[6] = r2[2]; m[7] = 0;
  m[8] = d[0];  m[9] = d[1];  m[10] = d[2]; m[11] = 0;
  m[12] = pos[0]; m[13] = pos[1]; m[14] = pos[2]; m[15] = 1;
}


int read_camera (
  Database_3Dfx *	db,
  char *		camera_name,
  AtmVector3		upv,
  AtmVector3		dir,
  AtmVector3		tar,
  AtmVector3		pos,
  AtrCamera *		acam) {

  int			ii;

  put_my_fp_status();

  for (ii = 0; ii < db->ncameras; ii++) {
    Camera *		cam = db->cameras + ii;
    
    if (strcmp (camera_name, cam->name) == 0) {
      upv[0] = cam->up[0] * RHCSFIX_X;
      upv[1] = cam->up[1] * RHCSFIX_Y;
      upv[2] = cam->up[2] * RHCSFIX_Z;
      dir[0] = cam->dir[0] * RHCSFIX_X;
      dir[1] = cam->dir[1] * RHCSFIX_Y;
      dir[2] = cam->dir[2] * RHCSFIX_Z;
      tar[0] = cam->target[0] * RHCSFIX_X;
      tar[1] = cam->target[1] * RHCSFIX_Y;
      tar[2] = cam->target[2] * RHCSFIX_Z;
      pos[0] = cam->position[0] * RHCSFIX_X;
      pos[1] = cam->position[1] * RHCSFIX_Y;
      pos[2] = cam->position[2] * RHCSFIX_Z;
      acam->nearClip = cam->camera->nearClip;
      acam->farClip = cam->camera->farClip;
      acam->fovRadians = cam->camera->fovRadians;
      return 1;
    }	
  }
  return 0;
}


void set_camera(
  Database_3Dfx *	db,		
  AtrCamera * 		cam,
  ViewportInfo * 	vp,
  AtmVector3		upv,
  AtmVector3		dir,
  AtmVector3		horiz,
  AtmVector3		tar,
  AtmVector3		pos) {
  
  Viewport *		view = &vp->port[vp->active];
  float			distance;
  float			camera_aspect;
  float			screen_aspect;

  put_my_fp_status();

  camera_aspect = (float)view->width / view->height;
  screen_aspect = (float)db->screen_width / db->screen_height;

  if (view->view != VP_CAMERA) {
    switch (view->view) {
    case VP_TOP:
      upv[0] = 0; upv[1] = 1; upv[2] = 0;
      dir[0] = 0; dir[1] = 0; dir[2] = -1;
      break;

    case VP_BOTTOM:
      upv[0] = 0; upv[1] = 1; upv[2] = 0;
      dir[0] = 0; dir[1] = 0; dir[2] = 1;
      break;

    case VP_LEFT:
      upv[0] = 0; upv[1] = 0; upv[2] = 1;
      dir[0] = 1; dir[1] = 0; dir[2] = 0;
      break;

    case VP_RIGHT:
      upv[0] = 0; upv[1] = 0; upv[2] = 1;
      dir[0] = -1; dir[1] = 0; dir[2] = 0;
      break;

    case VP_FRONT:
      upv[0] = 0; upv[1] = 0; upv[2] = 1;
      dir[0] = 0; dir[1] = 1; dir[2] = 0;
      break;

    case VP_BACK:    
      upv[0] = 0; upv[1] = 0; upv[2] = 1;
      dir[0] = 0; dir[1] = -1; dir[2] = 0;
      break;

    case VP_SPECIAL: {
	float		v = -view->vangle * D2RADIANS;
	float		h = view->hangle * D2RADIANS;
	
	upv[0] = 0; upv[1] = 0; upv[2] = 1;
	dir[0] = -sin (h);
	dir[1] = cos (v) * cos (h);
	dir[2] = cos (h) * sin (v);
	break;
    }
    
    }

    dir[0] *= RHCSFIX_X;
    dir[1] *= RHCSFIX_Y;
    dir[2] *= RHCSFIX_Z;

    upv[0] *= RHCSFIX_X;
    upv[1] *= RHCSFIX_Y;
    upv[2] *= RHCSFIX_Z;

    tar[0] = RHCSFIX_X * view->cx;
    tar[1] = RHCSFIX_Y * view->cy;
    tar[2] = RHCSFIX_Z * view->cz;

#define VIEWPORT_EYE_ANGLE	(30.0)

    if (camera_aspect < screen_aspect) {
      distance = (((float) view->width * screen_aspect / camera_aspect)
		  /(2.0 * view->zoom)) /
	tan (D2RADIANS * VIEWPORT_EYE_ANGLE /2.0);
    }
    else {
      distance = (view->width/(2.0 * view->zoom)) /
	tan (D2RADIANS * VIEWPORT_EYE_ANGLE /2.0);
    }
  
    pos[0] = tar[0] + -dir[0]*distance;
    pos[1] = tar[1] + -dir[1]*distance;
    pos[2] = tar[2] + -dir[2]*distance;

    cam->nearClip = 0.01 * distance;
    cam->farClip = 20 * distance;
    cam->fovRadians = D2RADIANS * VIEWPORT_EYE_ANGLE;
    cam_name[0] = '\0';
  }
  else {
    read_camera (db, view->camera, upv, dir, tar, pos, cam);
    strcpy (cam_name, view->camera);
  }

  if (camera_aspect >= screen_aspect) {    
    cam->aspectRatio = ((float) view->width) /
      ((float) view->height * camera_aspect / screen_aspect);
  }
  else {
    cam->aspectRatio = ((float) view->width * screen_aspect / camera_aspect) / 
      ((float) view->height);
  }

  put_glide_fp_status();
  atmVector3Cross (horiz, upv, dir);
  atmVector3Normalize (horiz, horiz);
  
  SetLCStoWCS(&cam->lcsToWCS, tar, pos, upv);
}


void render_object(Object * o) {
  
  int		jj;

  if (o->hidden)
    return;
  
  if (o->xform)
    atrPushXform (o->xform);
    
  for (jj = 0; jj < o->nmeshes; jj++) {
    if (o->material[jj]) {
      atrPushMaterial (o->material[jj]->material);
      atrRenderTriSet (o->mesh + jj);
      atrPopMaterial (1);
    }
    else {
      atrPushMaterial (default_mat);
      atrRenderTriSet (o->mesh + jj);
      atrPopMaterial (1);
    }
  }
  
  if (o->xform)
    atrPopXform (1);
}


void render_all_objects(Database_3Dfx * db) {
  
  int			ii;
  int			jj;
  static AtrXform *	tmp = NULL;

  if (tmp == NULL)
    tmp = atrXformAllocate (1);

  for (ii = 0; ii < db->nobjects; ii++) {
    Object *		o = db->objects + ii;

    if (o->hidden)
      continue;

    if (object_xform) {
      if (o->xform) {
	atmMatrix4x4Cat (tmp->data,
			 o->xform->data,
			 object_xform->data);
      
	atrPushXform (tmp);
      }
      else {
	atrPushXform (object_xform);
      }
#ifdef DEBUG_OUTPUT
      fprintf (output, "xform for render object %d START\n", jj);
      fflush (output);
#endif      
    }
    else {
      if (o->xform)
	atrPushXform (o->xform);
    }
    
    for (jj = 0; jj < o->nmeshes; jj++) {
#ifdef DEBUG_OUTPUT
      fprintf (output, "render object %d START\n", jj);
      fflush (output);
#endif      
      if (o->material[jj]) {
	atrPushMaterial (o->material[jj]->material);
	atrRenderTriSet (o->mesh + jj);
	atrPopMaterial (1);
      }
      else {
	atrPushMaterial (default_mat);
	atrRenderTriSet (o->mesh + jj);
	atrPopMaterial (1);
      }
#ifdef DEBUG_OUTPUT
      fprintf (output, "render object %d END\n", jj);
      fflush (output);
#endif      
    }

    if (o->xform || object_xform) {
#ifdef DEBUG_OUTPUT
      fprintf (output, "xform for render object %d END\n", jj);
#endif            
      atrPopXform (1);
    }
  }
}  


#define set_min(m, a) ((m) = ((m) < (a) ? (m) : (a)))
#define set_max(m, a) ((m) = ((m) > (a) ? (m) : (a)))


void find_db_bbox (
  Database_3Dfx *	db,
  float *		min,
  float * 		max) {
  
  Object *		o = db->objects;
  int			ii;

  put_my_fp_status ();
  min[0] = o->min[0];
  min[1] = o->min[1];
  min[2] = o->min[2];

  max[0] = o->max[0];
  max[1] = o->max[1];
  max[2] = o->max[2];

  for (ii = 1; ii < db->nobjects; ii++) {
    o = db->objects + ii;
    set_min(min[0], o->min[0]);
    set_min(min[1], o->min[1]);
    set_min(min[2], o->min[2]);

    set_max(max[0], o->max[0]);
    set_max(max[1], o->max[1]);
    set_max(max[2], o->max[2]);
  }
}


float find_bbox_greatest_extent(
  float		min[3],
  float		max[3]) {

  float		dx;
  float		dy;
  float		dz;

  put_my_fp_status ();

  dx = max[0] - min[0];
  dy = max[1] - min[1];
  dz = max[2] - min[2];

  if (dx > dy) {
    if (dx > dz)
      return dx;
    else
      return dz;
  }
  else if (dy > dz)
    return dy;
  else
    return dz;
}
      



int find_root_nodes (
  int *			root_nodes,
  int			count) {
  
  int			ii;
  int			root_count = 0;

  put_my_fp_status ();
  for (ii = 0; ii < count; ii++) {
    int			parent = 0;
    
    kxp_get_parent (ii, parent);
    if (parent == -1) {
      root_nodes[root_count++] = ii;
    }
  }
  return root_count;
}


int node_needs_xform (
  int				frame,		      
  int				node_num) {

  int				key;
  int				status;

  put_my_fp_status ();
  kxp_find_prev_key (node_num, KT_POSITION, frame, key, status);
  if (status > 0)
    return 1;

  kxp_find_prev_key (node_num, KT_SCALE, frame, key, status);
  if (status > 0)
    return 1;

  kxp_find_prev_key (node_num, KT_ROTATE, frame, key, status);
  if (status > 0)
    return 1;

  return 0;
}


void xpose_matrix (
  float *			m) {

  int				ii;
  int				jj;
  static float			tmp[16];

  put_my_fp_status ();
  for (jj = 0; jj < 4; jj++) {
    for (ii = 0; ii < 4; ii++) {
      tmp[ii*4 + jj] = m[jj*4 + ii];
    }
  }

  for (ii = 0; ii < 16; ii++) {
    m[ii] = tmp[ii];
  }
}


void process_node (
  Database_3Dfx *		db,		   
  int				frame,		   
  int				node_num,
  AtrXform *			in_axform) {

  int				ii;
  int				nchildren;
  char				name[22];
  int				type;
  Object *			o = NULL;
  AtrXform *			out_axform = NULL;
  static AtrXform *		tmp1 = NULL;
  static AtrXform *		tmp2 = NULL;
  static AtrXform *		tmp3 = NULL;
  static AtmVector3		position;
  static AtmVector3		target;
  static int			cam_init = 0;

  if (!cam_init) {
    put_my_fp_status ();
    position[0] = cam_position[0];
    position[1] = cam_position[1];
    position[2] = cam_position[2];
    target[0] = cam_target[0];
    target[1] = cam_target[1];
    target[2] = cam_target[2];
    cam_init = 1;
    put_glide_fp_status ();
  }
  
  if (tmp1 == NULL) {
    put_glide_fp_status ();
    tmp1 = atrXformAllocate (1);
    atrXformSetIdentity (tmp1);
    tmp1->data[0] = tmp1->data[5] = 1.0;
    tmp1->data[10] = -1.0;
    tmp2 = atrXformAllocate (1);
    tmp3 = atrXformAllocate (1);
  }

  put_my_fp_status ();
  kxp_get_node_name (node_num, name, type);
  put_glide_fp_status ();

  if (type == DUMMY_NODE) {
    float			xform[12];
    AtrXform *			axform = atrXformAllocate (1);

    put_my_fp_status ();
    if (node_needs_xform (frame, node_num)) {
      kxp_get_xform (node_num, frame, xform);
      xform_to_axform (xform, axform);

      put_glide_fp_status ();
      atmMatrix4x4Cat (tmp2->data,
		       axform->data,
		       tmp1->data);
      
      atmMatrix4x4Cat (axform->data,
		       tmp1->data,
		       tmp2->data);
      out_axform = axform;
    }
    else {
      out_axform = in_axform;
    }
  }
  else if (type == CAMERA_NODE) {
    put_my_fp_status ();
    if (node_needs_xform (frame, node_num)) {
      float			xform[12];

      kxp_get_xform (node_num, frame, xform);
      xform_to_axform (xform, tmp3);
      put_glide_fp_status ();
      atmMatrix4x4Cat (tmp2->data,
		       tmp3->data,
		       tmp1->data);
      
      atmMatrix4x4Cat (tmp3->data,
		       tmp1->data,
		       tmp2->data);

      if (strcmp (name, cam_name) == 0) {
	position[0] = position[1] = position[2] = 0.0;
	
	atmVectorMatrix4x4Mult (position, position, tmp3->data);
	SetLCStoWCS (cam_xform, target, position, cam_upv);
	put_glide_fp_status ();
      }

      out_axform = tmp3;
    }
    else {
      out_axform = in_axform;
    }
  }
  else if (type == TARGET_NODE) {
    put_my_fp_status ();
    if (node_needs_xform (frame, node_num)) {
      float			xform[12];
      char *			real_name = &name[0];

      kxp_get_xform (node_num, frame, xform);
      xform_to_axform (xform, tmp3);
      put_glide_fp_status ();
      atmMatrix4x4Cat (tmp2->data,
		       tmp3->data,
		       tmp1->data);
      
      atmMatrix4x4Cat (tmp3->data,
		       tmp1->data,
		       tmp2->data);

      while (*real_name != '.' && (real_name - name < 11)) {
	real_name++;
      }
      if (*real_name != '.') {
	error_printf (__FILE__, __LINE__, "Could not parse target name!\n");
	return;
      }
      else {
	*real_name = '\0';
      }
      
      if (strcmp (name, cam_name) == 0) {
	target[0] = target[1] = target[2] = 0.0;
	
	atmVectorMatrix4x4Mult (target, target, tmp3->data);
	SetLCStoWCS (cam_xform, target, position, cam_upv);
	put_glide_fp_status ();
      }

      out_axform = tmp3;
    }
    else {
      out_axform = in_axform;
    }
  }
  else if (type == OBJECT_NODE) {
    KXPInterp		interp;
    int			key_index = -1;
    int			status;
    
    o = find_object_by_name (db, &name[0]);

    if (o == NULL)
      error_printf (__FILE__, __LINE__, "could not find object %s!\n",
		    name);

    kxp_find_key_at_time (node_num, KT_HIDE, frame, key_index, status);
    if (status >= 0) {
      kxp_interpolate_key (node_num, KT_HIDE, frame, &interp, status);
      if (status >= 0) {
	if (interp.u.i > 0)
	  o->hidden = 1;
	else
	  o->hidden = 0;
      }
    }

    put_my_fp_status ();
    if (node_needs_xform (frame, node_num)) {
      float			xform[12];
      AtrXform *		axform = NULL;

      kxp_get_xform (node_num, frame, xform);
      if (o->xform == NULL) {
	put_glide_fp_status ();
	axform = atrXformAllocate (1);
	put_my_fp_status ();
	xform_to_axform (xform, axform);
	o->xform = axform;
      }
      else {
	xform_to_axform (xform, o->xform);
      }
      put_glide_fp_status ();
    }
    else {
      if (o->xform == NULL) {
	put_glide_fp_status ();
	o->xform = atrXformAllocate (1);
      }
      else {
	put_glide_fp_status ();
      }
      
      atrXformAssign (o->xform, in_axform);
    }

    if (o->locmatrix != NULL) {
      atmMatrix4x4Cat (o->xform->data,
		       o->locmatrix->data,
		       o->xform->data);
    }

    atmMatrix4x4Cat (tmp2->data,
		     o->xform->data,
		     tmp1->data);
      
    atmMatrix4x4Cat (o->xform->data,
		     tmp1->data,
		     tmp2->data);

    out_axform = o->xform;
  }

  put_my_fp_status ();
  kxp_get_num_children (node_num, nchildren);

  for (ii = 0; ii < nchildren; ii++) {
    int				child_node;

    kxp_get_child (node_num, ii, child_node);
    process_node (db, frame, child_node, out_axform);
  }
  put_glide_fp_status();
}    


void render_animation (
  Database_3Dfx *		db,
  int				frame) {

  int				nn;
  
  for (nn = 0; nn < db->nroots; nn++) {
    process_node (db, frame, db->root_nodes[nn], NULL);
  }
  atrClearCanvas (CLEAR_COLOR, ATR_WBUFFER_CLEAR);
  render_all_objects (db);
  for (nn = 0; nn < anim_delay; nn++) {
  }
}


void render (
  Database_3Dfx *		db,
  int				animate) {

  static int			frame = -1;

  put_glide_fp_status ();
  atrSelectCamera (cam_cam);
  atrClearCanvas (CLEAR_COLOR, ATR_WBUFFER_CLEAR);
  if (animate) {
    if (frame == -1) {
      put_my_fp_status ();
      kxp_get_cur_frame (frame);
      put_glide_fp_status ();
    }
    else if (frame < db->end) {
      frame++;
    }
    else {
      frame = db->start;
    }

    render_animation (db, frame);
  }
  else {
    render_all_objects (db);
  }
  atrSwapBuffer (1);
}


extern int render_db (
  Database_3Dfx * 	db,
  char *		center_name,
  int			camera_mode,
  ViewportInfo *	vpinfo) {
  
  int 			screen_width = db->screen_width;
  int 			screen_height = db->screen_height;
  AtrCanvas *		canvas;
  AtrXform *		rotate;
  AtrLight * 		light;
  AtrEnv *		env;
  AtrMaterial *		mat;
  Object *		center_object;
  int			ii;
  float			jx;
  float			jy;
  static int		jb[4];
  int			c;
  int			active_vp = vpinfo->active;
  AtrXform *		cam_start;
  int			first_time = 1;
  int			quit = 0;
  float			anglex = 0;
  float			angley = 0;
  float			xlate_dir = 0;
  float			xlate_horiz = 0;
  float			xlate_up = 0;
  AtmVector3		rot_origin;
  int			animate;
  int			headlight;

  put_glide_fp_status ();
  canvas = atrCanvasAllocate(1);
  rotate = atrXformAllocate(1);
  light = atrLightAllocate(3);
  env = atrEnvAllocate(1);
  mat = atrMaterialAllocate(1);
  cam_start = atrXformAllocate (1);
  put_my_fp_status ();

  clear_color[0] = clear_color[1] = clear_color[2] = 1.0;

  {
    int				nitems;

    pxp_get_item_count (nitems);

    for (ii = 0; ii < nitems; ii++) {
      ItemData 			item;
      int			status;
      Object *			o = NULL;

      pxp_get_item (ii, &item, status);
      if (!status) {
	return USR_END;
      }

      if (item.type != PXPMESH)
	continue;

      o = find_object_by_name (db, item.name);

      if (o != NULL) {
	o->ipas_index = ii;
	o->hidden = item.item.m.flags & MESH_HIDDEN;
      }
    }
  }

#ifdef KXP  
  kxp_get_node_count (db->nnodes);
  kxp_num_frames (db->nframes);
  kxp_get_segment (db->start, db->end);
  
  db->root_nodes = (int *) malloc (sizeof(int) * db->nnodes);
  db->nroots = find_root_nodes (db->root_nodes, db->nnodes);
#endif
  
  put_glide_fp_status();  

  transform = atrXformAllocate(10);
  cam_cam = camera = atrCameraAllocate(1);

  if (vpinfo->viewports == 0)
    goto quit;

  env->flags = ATR_HSR_WBUFFER;
  atrPushEnv (env);
  put_my_fp_status();

  mat->emissive.r = mat->emissive.g = mat->emissive.b = 0.0;
  mat->diffuse.r = mat->diffuse.g = mat->diffuse.b = 1.0;
  mat->specular.r = mat->specular.g = mat->specular.b = 1.0;
  /*  mat->transparency = 0.0; */
  mat->isTwoSided = 1;

  put_glide_fp_status ();
  atrMaterialSetup (mat, ATR_MAT_GSHADE);
  put_my_fp_status ();
  mat->texture[0] = mat->texture[1] = NULL;
  default_mat = mat;
  
  canvas->xMin = 0;
  canvas->yMin = 0;
  canvas->xMax = screen_width-1;
  canvas->yMax = screen_height-1;
  put_glide_fp_status ();
  atrSelectCanvas (canvas);
  put_my_fp_status();

  if (center_name != NULL) {
    center_object = find_object_by_name (db, center_name);
    min[0] = center_object->min[0];    
    min[1] = center_object->min[1];
    min[2] = center_object->min[2];

    max[0] = center_object->max[0];
    max[1] = center_object->max[1];
    max[2] = center_object->max[2];
  }
  else {
    find_db_bbox (db, min, max);
  }
  
  centroid[0] = min[0] + (xwidth = 0.5 * (max[0] - min[0]));
  centroid[1] = min[1] + 0.5 * (max[1] - min[1]);
  centroid[2] = min[2] + 0.5 * (max[2] - min[2]);

  camz = min[2] - (camdist = xwidth/tan((double)3.14159/180.0 * 20));

  put_glide_fp_status ();
  atmVector3Set (rot_origin, 0, 0, 0);
  put_my_fp_status ();
  
reset:

  put_my_fp_status();
  
  anglex = 0;
  angley = 0;
  xlate_dir = 0;
  xlate_horiz = 0;
  xlate_up = 0;

  if (vpinfo != NULL) {
    set_camera(db, camera, vpinfo,
	       up, direction, horizontal, target, position);
    put_my_fp_status ();
    camera->nearClip = 0.1;
    camera->farClip = 100 * find_bbox_greatest_extent (min,max);    
  }
  else {
    camera->aspectRatio = ((float) screen_width)/screen_height;
    camera->fovRadians = D2RADIANS * VIEWPORT_EYE_ANGLE;
    camera->nearClip = 0.1 * camdist;
    camera->farClip = 10 * (camdist + max[2] - min[2]);
    put_glide_fp_status ();
    atmVector3Set (position, centroid[0], centroid[1], camz);
    atmVector3Set (target, centroid[0], centroid[1], min[2]);
    atmVector3Set (direction, 0, 0, 1);
    atmVector3Set (up, 0, 1, 0);
    atmVector3Cross (horizontal, up, direction);
    atmVector3Normalize (horizontal, horizontal);
    SetLCStoWCS (&camera->lcsToWCS, target, position, up);
    put_glide_fp_status();
  }

  cam_orientation = atrXformAllocate (1);
  cam_xform = &camera->lcsToWCS;
  atrXformAssign (cam_orientation, &camera->lcsToWCS);
  put_my_fp_status ();
  cam_position[0] = position[0];
  cam_position[1] = position[1];
  cam_position[2] = position[2];

  cam_target[0] = target[0];
  cam_target[1] = target[1];
  cam_target[2] = target[2];

  cam_upv[0] = up[0];
  cam_upv[1] = up[1];
  cam_upv[2] = up[2];    

  put_glide_fp_status();
  
  atrSelectCamera (camera);
#ifdef DEBUG
  atrCameraPrint (camera, stdout, 2);
#endif  

  if (first_time) {
    static int			no_lights = 1;
    
    first_time = 0;
    for (ii = 0; ii < db->nlights; ii++) {
      Light *			light =  db->lights + ii;
      
      if (!light->off) {
	atrAddLight (light->light, ii);
	if (light->slight) {
	  atrAddLight (light->slight, ii + db->nlights);
	}
	no_lights = 0;
      }
    }


    /* $AMBIENT$ */
    put_my_fp_status ();
    light->color = db->ambient;
    put_glide_fp_status ();
    light->flags = ATR_LIGHT_AMBIENT;
    atrAddLight (light, -3);
    
#if 1
    /* put an infinite light in the direction of the camera */
    if (1 /*no_lights*/) {
      AtrLight *		l = light + 1;
      
      atrXformAssign (&l->lcsToWCS, &camera->lcsToWCS);
      l->color.r = l->color.g = l->color.b = 0.5;
      l->flags = ATR_LIGHT_DIRECTED;
      atrAddLight (l, -1);
      headlight = ~0;
    }
#endif    
  }

  c = ' ';
  put_my_fp_status ();
  translation[0] = translation[1] = translation[2] = 0.0f;
  rotation[0] = rotation[1] = rotation[2] = 0.0f;
  step[0] = (max[0] - min[0]) / 3.2f;
  step[1] = (max[1] - min[1]) / 3.2f;
  step[2] = (max[2] - min[2]) / 3.2f;

  /* multiply out any translation component so we can cat all of our
   * rotations first.
   */
  put_glide_fp_status ();
  atrXformAssign (cam_start, &camera->lcsToWCS);
  atrXformSetTranslation (transform, -position[0], -position[1], -position[2]);
  atmMatrix4x4Cat ((transform + 1)->data, cam_start->data, transform->data);
  atrXformAssign (cam_start, transform + 1);

  jy = jx = 0.0;
  jb[0] = jb[1] = jb[2] = jb[3] = 0;
  joystick_set_deadspot(0.1);

#define HAT_STEP		(0.65f)
#define ROT_STEP		(6.0f)

#ifdef KXP
  animate = camera_mode & CAMERA_ANIMATE;
#else
  animate = 0;
#endif  

  do {
    int				key_hit = 0;


    if (camera_mode & CAMERA_FLIP) {
      AtrXform *		xlate = transform;
      AtrXform *		rot = transform + 1;
      AtrXform *		tmp = transform + 2;      
      AtrXform *		tmp2 = transform + 3;
      AtrXform *		tmp3 = transform + 4;
      AtrXform *		tmp4 = transform + 5;

      if (object_xform == NULL) {
	object_xform = atrXformAllocate (1);
      }
  
      put_my_fp_status();
      if (jb[0] && !jb[1] && !jb[2] && !jb[3]) {
	xlate_dir -= jy*step[2];
      }
      else if (jb[1] && !jb[0] && !jb[2] && !jb[3]) {
	xlate_up += jy * step[2];
	xlate_horiz += jx*step[2];
      }
      else {
	angley += jy * ROT_STEP;
	anglex += (-jx) * ROT_STEP;
      }

      /* check hat */
      if (jb[0] && jb[1] && jb[2] && jb[3]) {
	xlate_up += HAT_STEP * step[2];
      }
      else if (jb[0] && jb[1] && jb[2] && !jb[3]) {
	xlate_up -= HAT_STEP * step[2];
      }
      else if (jb[0] && jb[1] && !jb[2] && !jb[3]) {
	xlate_horiz -= HAT_STEP * step[2];
      }
      else if (jb[0] && jb[1] && !jb[2] && jb[3]) {
	xlate_horiz += HAT_STEP * step[2];
      }
      
      
      put_glide_fp_status();
      atrXformSetIdentity (object_xform);

      atrXformSetIdentity (tmp);
      atrXformSetTranslation (tmp,
			      xlate_dir*direction[0] +
			      xlate_horiz*horizontal[0] +
			      xlate_up * up[0],
			      xlate_dir*direction[1] +
			      xlate_horiz*horizontal[1] +
			      xlate_up * up[1],
			      xlate_dir*direction[2] +
			      xlate_horiz*horizontal[2] +
      			      xlate_up * up[2]);
    
      put_my_fp_status();
      TransformSetRotationAboutAxis (xlate, horizontal, angley);
      TransformSetRotationAboutAxis (tmp2, up, anglex);
      put_glide_fp_status();
      atmMatrix4x4Cat (rot->data, xlate->data, tmp2->data);

      atrXformSetIdentity (xlate);
      atrXformSetTranslation (xlate,
			      -centroid[0], -centroid[1], -centroid[2]);
      atrXformSetIdentity (tmp2);      
      atrXformSetTranslation (tmp2,
			      centroid[0], centroid[1], centroid[2]);
      
      atmMatrix4x4Cat (tmp4->data, xlate->data, rot->data);      
      atmMatrix4x4Cat (tmp3->data, tmp4->data, tmp2->data);
      atmMatrix4x4Cat (object_xform->data, tmp3->data, tmp->data);

      
      atrSelectCamera (camera);
      render (db, animate);
    }
    else if (camera_mode & CAMERA_ROTATE) {
      AtrXform *		rot_up = transform;
      AtrXform *		rot_horiz = transform + 1;
      AtrXform *		xlate = transform + 2;
      AtrXform *		tmp = transform + 3;
      AtrXform * 		tmp2 = transform + 4;

      /* use user entered origin for rotates here? */

      put_my_fp_status();
      if (jb[0] && !jb[1] && !jb[2] && !jb[3]) {
	xlate_dir += jy * step[2];
      }
      else if (jb[1] && !jb[0] && !jb[2] && !jb[3]) {
	xlate_up += jy * step[2];	
	xlate_horiz += jx * step[2];
      }
      else if (!jb[0] && !jb[1] && !jb[2] && !jb[3]) {
	angley += jy * ROT_STEP;
	anglex += (-jx) * ROT_STEP;
      }

      /* check hat */
      if (jb[0] && jb[1] && jb[2] && jb[3]) {
	xlate_up += HAT_STEP * step[2];
      }
      else if (jb[0] && jb[1] && jb[2] && !jb[3]) {
	xlate_up -= HAT_STEP * step[2];
      }
      else if (jb[0] && jb[1] && !jb[2] && !jb[3]) {
	xlate_horiz -= HAT_STEP * step[2];
      }
      else if (jb[0] && jb[1] && !jb[2] && jb[3]) {
	xlate_horiz += HAT_STEP * step[2];
      }
      
      put_glide_fp_status();
      atrXformSetIdentity (xlate);
      atrXformSetTranslation (xlate,
			      position[0] + 
			      xlate_dir*direction[0] +
			      xlate_horiz*horizontal[0] +
			      xlate_up * up[0],
			      position[1] +
			      xlate_dir*direction[1] +
			      xlate_horiz*horizontal[1] +
			      xlate_up * up[1],
			      position[2] +
			      xlate_dir*direction[2] +
			      xlate_horiz*horizontal[2] +
      			      xlate_up * up[2]);

      put_my_fp_status();
      TransformSetRotationAboutAxis (rot_horiz, horizontal, angley);
      TransformSetRotationAboutAxis (rot_up, up, anglex);
      
      put_glide_fp_status();
      atmMatrix4x4Cat (tmp2->data, rot_horiz->data, rot_up->data);
      atmMatrix4x4Cat (tmp->data, cam_start->data, tmp2->data);
      atmMatrix4x4Cat (camera->lcsToWCS.data, tmp->data, xlate->data);

      atrSelectCamera (camera);      
      render (db, animate);
    }
    else {
      if (jb[0] && !jb[1] && !jb[2] && !jb[3]) {
	/* button 1 down, go into translate mode */

	AtmVector3		delta;
      

	atmVector3Scale (delta, direction, jy * step[2]);
	atmVector3Add (position, position, delta);
	atmVector3Add (target, target, delta);
      }
      else if (jb[1] && !jb[0] && !jb[2] && !jb[3]) {
	/* button 2 down, do other translate */
      
	AtmVector3		delta;
      
	atmVector3Scale (delta, up, jy * step[1]);
	atmVector3Add (position, position, delta);
	atmVector3Add (target, target, delta);

	atmVector3Scale (delta, horizontal, jx * step[0]);
	atmVector3Add (position, position, delta); 
	atmVector3Add (target, target, delta);
      }
      else {
	/* no buttons down, do change direction mode */

	AtrXform *		tmp = transform + 1;

	put_my_fp_status ();
	TransformSetRotationAboutAxis (tmp, horizontal, jy * ROT_STEP);
	put_glide_fp_status();
	atmVectorMatrix4x4Mult (direction, direction, tmp->data);

	put_my_fp_status ();
	TransformSetRotationAboutAxis (tmp, up, (jx) * ROT_STEP);
	atmVectorMatrix4x4Mult (direction, direction, tmp->data);
	put_glide_fp_status();

	atmVector3Scale (target, direction, camdist);
	atmVector3Add (target, target, position);
      }

      /* check hat */
      if (jb[0] && jb[1] && jb[2] && jb[3]) {
	AtmVector3		delta;
      
	atmVector3Scale (delta, up, HAT_STEP * step[1]);
	atmVector3Add (position, position, delta);
	atmVector3Add (target, target, delta);
      }
      else if (jb[0] && jb[1] && jb[2] && !jb[3]) {
	AtmVector3		delta;
      
	atmVector3Scale (delta, up, -HAT_STEP * step[1]);
	atmVector3Add (position, position, delta);
	atmVector3Add (target, target, delta);
      }
      else if (jb[0] && jb[1] && !jb[2] && !jb[3]) {
	AtmVector3		delta;
      
	atmVector3Scale (delta, horizontal, -HAT_STEP * step[0]);
	atmVector3Add (position, position, delta); 
	atmVector3Add (target, target, delta);
      }
      else if (jb[0] && jb[1] && !jb[2] && jb[3]) {
	AtmVector3		delta;

	atmVector3Scale (delta, horizontal, HAT_STEP * step[0]);
	atmVector3Add (position, position, delta); 
	atmVector3Add (target, target, delta);
      }
      
      SetLCStoWCS (&camera->lcsToWCS, target, position, up);
      put_glide_fp_status();
      atrSelectCamera (camera);
      
      render (db, animate);
    }
    
    joystick_read (&jx, &jy, jb, jb+1, jb+2, jb+3);    

    gfx_key_hit (key_hit);

    if (key_hit) {
      int			key_code;
      int			key_state;

      gfx_get_key (key_code);
      gfx_kstate (key_state);

      jb[0] = jb[1] = jb[2] = jb[3] = 0;

      if (key_state & 0x3) {
	jb[0] = 1;
      }

      switch (key_code) {
      case LC_S:
	anim_delay += 10000;
	break;

      case LC_F:
	anim_delay -= 10000;
	if (anim_delay < 0)
	  anim_delay = 0;
	break;

      case LC_N:
	anim_delay = 0;
	break;
	
      case NUM_0:
	vpinfo->active = active_vp;
	goto reset;
	break;

      case NUM_1:
	vpinfo->active = 0;
	goto reset;
	break;

      case NUM_2:
	if (vpinfo->viewports > 1)
	  vpinfo->active = 1;
	goto reset;
	break;

      case NUM_3:
	if (vpinfo->viewports > 2)
	  vpinfo->active = 2;
	goto reset;
	break;

      case NUM_4:
	if (vpinfo->viewports > 3)
	  vpinfo->active = 3;
	goto reset;
	break;

      case UPARROW:
	jy = 0.5;
	jx = 0;
	break;

      case DOWNARROW:
	jy = -0.5;
	jx = 0;
	break;

      case LEFTARROW:
	jx = -0.5;
	jy = 0;
	break;

      case RIGHTARROW:
	jx = 0.5;
	jy = 0;
	break;

      case ESC:
	quit = 1;
	break;

      case ALT_B:
	if (clear_color[0] != 0.0)
	  clear_color[0] = clear_color[1] = clear_color[2] = 0.0;
	else
	  clear_color[0] = clear_color[1] = clear_color[2] = 1.0;
	break;

#ifdef KXP	
      case ALT_A:
	animate = ~animate;
	break;
#endif
	
      case ALT_L:
	headlight = ~headlight;
	if (!headlight) {
	  atrRemoveLight (-1);
	}
	else {
	  AtrLight *		l = light + 1;
      
	  atrXformAssign (&l->lcsToWCS, &camera->lcsToWCS);
	  l->color.r = l->color.g = l->color.b = 0.5;
	  l->flags = ATR_LIGHT_DIRECTED;
	  atrAddLight (l, -1);
	}
	break;
	  

      case LC_R:
      case UC_R:
	goto reset;
	break;
      }
    }
  } while (!quit);

quit:
  atrCanvasDeallocate(canvas);
  atrCameraDeallocate(camera);
  atrXformDeallocate(cam_start);
  if (object_xform)
    atrXformDeallocate(object_xform);
  atrXformDeallocate(transform);
  atrXformDeallocate(rotate);
  atrLightDeallocate(light);
  atrEnvDeallocate(env);
  atrMaterialDeallocate(mat);

  return USR_END;
}
