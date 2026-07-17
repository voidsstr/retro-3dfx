/*
 * kview.c
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "pxp.h"
#include "kxp.h"
#include "kview.h"
#include "database.h"

#include "mymalloc.h"

#define 	BUFFER_MAX		(4096)



Database_3Dfx *		kxp_database = NULL;

ItemData		temp_item;
ViewportInfo		temp_vpinfo;


int do_kview(
  char * 		name,
  int			resolution_mode,
  int			texture_mode,
  int			camera_mode,
  int			restrict_mode) {
  int			status;
  static char 		buffer[BUFFER_MAX];
  int			screen_width;
  int			screen_height;
  static char		home[512];
  int			next_mode = USR_END;
  ItemData *		item = &temp_item;
  Name3ds *		selection_list = NULL;
  int			nselections = 0;
  int			ii = 0;
  ViewportInfo *	vpinfo = &temp_vpinfo;
  static int		init_atr = 0;
  AtrVoodooCaps		caps;
  GrHwConfiguration	gr_caps;

  gfx_get_paths(GFX_HOME_PATH, 0, home, buffer);
  sprintf(buffer, "%s\\3dfx\\temp.3ds", home);

  do_3ds_save (buffer, status);

  grSstQueryHardware (&gr_caps);

  if (resolution_mode == RESMODE_800_600 &&
      gr_caps.SSTs[0].sstBoard.VoodooConfig.fbRam >= 4) {
    screen_width = 800;
    screen_height = 600;
  } else {
    screen_width = 640;
    screen_height = 480;
  }
  if (!init_atr) {
    init_atr = 1;
    atrInit(screen_width, screen_height);
    get_glide_fp_status();
    put_my_fp_status();
  }
    
  atrQueryVoodooCaps (&caps);

  if (restrict_mode == RESTRICT_SELECTED) {
    int			nobjects;

    pxp_get_item_count (nobjects);
    
    selection_list = (Name3ds *) malloc(sizeof(Name3ds) * nobjects);
    nselections = 0;
  
    for (ii = 0; ii < nobjects; ii++) {
      pxp_get_item (ii, item, status);
      if (!status)
	return USR_END;

      if (item->type == PXPMESH) {
	ItemMesh *		m = &item->item.m;
      
	if (m->flags & MESH_SELECTED) {
	  strcpy (selection_list[nselections], item->name);
	  nselections++;
	}
      }
    }
    if (nselections == 0) {
      free (selection_list);
      selection_list = NULL;
    }
  }
  else {
    nselections = 0;
    selection_list = NULL;
  }

  if (kxp_database == NULL) {
    kxp_database = alloc_db();
    kxp_database->screen_width = screen_width;
    kxp_database->screen_height = screen_height;
    kxp_database->num_tmus = caps.numTex;
    kxp_database->max_tmu_size = caps.tfxMem;
    
    switch (texture_mode) {
    case TEXMODE_NONE:
      kxp_database->texture_mode = MIPMAP_NONE;
      break;

    case TEXMODE_POINT:
      kxp_database->texture_mode = MIPMAP_POINT;
      break;

    case TEXMODE_BI:
      kxp_database->texture_mode = MIPMAP_POINT;
      break;

    case TEXMODE_TRI:
      kxp_database->texture_mode = MIPMAP_POINT;
      break;
    }
    
    if (!read_db (kxp_database, selection_list, nselections))
      return USR_END;
  }

  gfx_viewports(vpinfo);
  next_mode = render_db (kxp_database, name, camera_mode, vpinfo);

  if (selection_list)
    free (selection_list);
  
  return next_mode;
}
  
