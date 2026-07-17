/**************************************************************************
 *									  *
 * 	 Copyright (C) 1996 Silicon Graphics, Inc.			  *
 *									  *
 *  These coded instructions, statements, and computer programs  contain  *
 *  unpublished  proprietary  information of Silicon Graphics, Inc., and  *
 *  are protected by Federal copyright law.  They  may  not be disclosed  *
 *  to  third  parties  or copied or duplicated in any form, in whole or  *
 *  in part, without the prior written consent of Silicon Graphics, Inc.  *
 *									  *
 **************************************************************************/

#include "ogtst.h"
#include "light.h"
#include <math.h>

typedef struct { float r,g,b,a ; } C4;
typedef struct { float x,y,z ; } V3;
typedef struct { float x,y,z ; } P3;

typedef struct {
   P3    p ;
} T_eye ;

typedef struct {
   V3    n ;
   P3    p1 ;
   P3    p2 ;
   P3    p3 ;
} T_object ;

typedef struct {
   C4     Cla;
   C4     Cld;
   C4     Cls;
   float  p[4];
   V3     d;
   float  ex;
   float  an;
   float  K0,K1,K2;
   int    flag ;
} T_light ;

typedef struct {
   C4     Coa ;
   C4     Cod ;
   C4     Cos ;
   C4     Coe ;
   float  Eos ;
   float  CI[3] ;
} T_material ;

typedef struct {
   C4     Csa ;
   float  lv ;
   float  ts ;
} T_model ;

static T_eye      Eye;
static T_object   Object;
static T_light    Light[8];
static T_material FrontMaterial;
static T_material BackMaterial;
static T_material DummyMaterial;
static T_model    Model;

/* lighting error tolerance table */
static int light_tolerance[3] = {
       4, 8, 16,
};

/*************************************************************
* 
*************************************************************/
/*ARGSUSED*/
unsigned int LightVertex(int flag)
{
   C4     Col;
   V3     n,Vl,Vh,Ve;
   float  d,e,Al,spot_attenuation,spot_angle;
   float  diff,diffterm;
   float  spec,specterm;
   int    i,r,g,b,a,cpck;
   T_material *mat ;

   if (Model.ts && backfacing())  {
     mat = &BackMaterial;
     ogEnvLog(4,"Use back side properties and reverse normal\n") ;

     n.x = -Object.n.x ;
     n.y = -Object.n.y ;
     n.z = -Object.n.z ;
     }
   else  {
     mat = &FrontMaterial;
     ogEnvLog(4,"Use front side properties\n") ;

     n.x = Object.n.x ;
     n.y = Object.n.y ;
     n.z = Object.n.z ;
     }

   d = sqrt(n.x*n.x + n.y*n.y + n.z*n.z) ; 
   n.x /= d ; n.y /= d ; n.z /= d ;

   Col.r = mat->Coe.r;
   Col.g = mat->Coe.g;
   Col.b = mat->Coe.b;
   Col.a = mat->Cod.a;

   Col.r += mat->Coa.r * Model.Csa.r;
   Col.g += mat->Coa.g * Model.Csa.g;
   Col.b += mat->Coa.b * Model.Csa.b;

   /* LOCAL VIEWER */
   if (Model.lv != 0.0)  {
     Ve.x = Eye.p.x - Object.p3.x ;
     Ve.y = Eye.p.y - Object.p3.y ;
     Ve.z = Eye.p.z - Object.p3.z ;
     d = sqrt(Ve.x*Ve.x + Ve.y*Ve.y + Ve.z*Ve.z) ;
     Ve.x /= d ; Ve.y /= d ; Ve.z /= d ;
     }

   for (i=0; i<8; i++) 
     if (Light[i].flag)  {

       /* INFINITE LIGHT */
       if (Light[i].p[3] == 0.0)  {
         Vl.x = Light[i].p[0];
         Vl.y = Light[i].p[1];
         Vl.z = Light[i].p[2];
         d = sqrt(Vl.x * Vl.x + Vl.y * Vl.y + Vl.z * Vl.z);
         Vl.x /= d; Vl.y /= d; Vl.z /= d;

         Al = 1.0;
         }

       /* LOCAL LIGHT */
       else  {
         Vl.x = Light[i].p[0]/Light[i].p[3] - Object.p3.x;
         Vl.y = Light[i].p[1]/Light[i].p[3] - Object.p3.y;
         Vl.z = Light[i].p[2]/Light[i].p[3] - Object.p3.z;
         d = sqrt(Vl.x * Vl.x + Vl.y * Vl.y + Vl.z * Vl.z);
         Vl.x /= d; Vl.y /= d; Vl.z /= d;

         spot_attenuation = 1.0 ;

         if (Light[i].an != 180.0)  {
           e = sqrt(Light[i].d.x * Light[i].d.x + Light[i].d.y * Light[i].d.y + Light[i].d.z * Light[i].d.z);

           spot_angle = -(Vl.x * Light[i].d.x + Vl.y * Light[i].d.y + Vl.z * Light[i].d.z)/e ;

           if (spot_angle >= cos(Light[i].an*M_PI/180.0))
             spot_attenuation = pow(spot_angle,Light[i].ex) ;
           else spot_attenuation = 0 ;

           ogEnvLog(4,"intermediate spot_angle,spot_attenuation is %f,%f\n",spot_angle,spot_attenuation) ;
           }

         Al = spot_attenuation / (Light[i].K0 + Light[i].K1*d + Light[i].K2*d*d);
         }

       /* ambient */
       Col.r += Al * mat->Coa.r * Light[i].Cla.r;
       Col.g += Al * mat->Coa.g * Light[i].Cla.g;
       Col.b += Al * mat->Coa.b * Light[i].Cla.b;

       ogEnvLog(4,"intermediate Vl is (%f,%f,%f)\n",Vl.x,Vl.y,Vl.z) ;

       diff = n.x * Vl.x + n.y * Vl.y + n.z * Vl.z;
       if (diff < 0.0)
         diff = 0.0;
       ogEnvLog(4,"intermediate diff is %f\n",diff) ;

       diffterm = Al * diff;
       ogEnvLog(4,"intermediate diffterm is %f\n",diffterm) ;

       Col.r += mat->Cod.r * Light[i].Cld.r * diffterm;
       Col.g += mat->Cod.g * Light[i].Cld.g * diffterm;
       Col.b += mat->Cod.b * Light[i].Cld.b * diffterm;

       /* LOCAL/INFINITE VIEWER */
       if (Model.lv != 0.0)  {
	  Vh.x = Vl.x + Ve.x;
	  Vh.y = Vl.y + Ve.y;
	  Vh.z = Vl.z + Ve.z;
	  }
       else  {
	  Vh.x = Vl.x;
	  Vh.y = Vl.y;
	  Vh.z = Vl.z + 1.0;
	  }

       d = sqrt(Vh.x*Vh.x + Vh.y*Vh.y + Vh.z*Vh.z);
       Vh.x /= d; Vh.y /= d; Vh.z /= d;
       ogEnvLog(4,"intermediate Vh is (%f,%f,%f)\n",Vh.x,Vh.y,Vh.z) ;

       spec = n.x * Vh.x + n.y * Vh.y + n.z * Vh.z;
       if (spec < 0.0)
         spec = 0.0;
       ogEnvLog(4,"intermediate spec is %f\n",spec) ;

       if (mat->Eos == 0.0)
         specterm = 1.0 ;
       else specterm = pow(spec,mat->Eos);

       specterm = Al * ((diff > 0.0) ? specterm : 0.0) ;
       ogEnvLog(4,"intermediate specterm is %f\n",specterm) ;

       Col.r += mat->Cos.r * Light[i].Cls.r * specterm;
       Col.g += mat->Cos.g * Light[i].Cls.g * specterm;
       Col.b += mat->Cos.b * Light[i].Cls.b * specterm;
       ogEnvLog(2,"intermediate rgba is (%f,%f,%f,%f)\n",Col.r,Col.g,Col.b,Col.a) ;
       }

   r = (float) Col.r * 255.0 ;
   g = (float) Col.g * 255.0 ;
   b = (float) Col.b * 255.0 ;
   a = (float) Col.a * 255.0 ;

   if (r > 255) r = 255 ; else if (r < 0) r = 0;
   if (g > 255) g = 255 ; else if (g < 0) g = 0;
   if (b > 255) b = 255 ; else if (b < 0) b = 0;
   if (a > 255) a = 255 ; else if (a < 0) a = 0;

   cpck = (r<<24) | (g<<16) | (b<<8) | a ;
   ogEnvLog(1,"calculated rgb is 0x%08x\n",cpck) ;

   return(cpck) ;
}

/*************************************************************
*  backfacing()  - 
*************************************************************/
int backfacing(void)
{
   V3 a,b,c ;

   a.x = Object.p1.x - Object.p2.x ;
   a.y = Object.p1.y - Object.p2.y ;
   a.z = Object.p1.z - Object.p2.z ;

   b.x = Object.p1.x - Object.p3.x ;
   b.y = Object.p1.y - Object.p3.y ;
   b.z = Object.p1.z - Object.p3.z ;

   c.x = a.y * b.z - b.y * a.z ; 
   c.y = a.z * b.x - b.z * a.x ; 
   c.z = a.x * b.y - b.x * a.y ; 

   return((c.z < 0.0) ? 1 : 0) ;
}

/*************************************************************
*  ogtst_lmgen()  -
*************************************************************/
void ogtst_lmgen(int mode,int index,int flag) 
{
   T_light    *lg;
   T_material *mf,*mb;
   T_model    *md;
   int targ;

   switch(mode)  {

     case OGTST_LIGHT :
        if (index < 0 || index > 7)
          ogEnvLog(OG_LINTERNALERROR,"ogtst_lmgen(%d,%d,0x%08x): index out of range\n",mode,index,flag);

        lg = &Light[index];
        index += GL_LIGHT0;

        if (flag & OGTST_LIGHT_AMBIENT) {
          lg->Cla.r = ogLibFloatRand(0.0,0.3);
          lg->Cla.g = ogLibFloatRand(0.0,0.3);
          lg->Cla.b = ogLibFloatRand(0.0,0.3);
          lg->Cla.a = ogLibFloatRand(0.0,0.3);
          glLightfv(index,GL_AMBIENT,&lg->Cla.r);
          ogEnvLog(6,"LIGHT: ambient   %f,%f,%f,%f\n",lg->Cla.r,lg->Cla.g,lg->Cla.b,lg->Cla.a);
          }

        if (flag & OGTST_LIGHT_DIFFUSE) {
          lg->Cld.r = ogLibFloatRand(0.0,0.3);
          lg->Cld.g = ogLibFloatRand(0.0,0.3);
          lg->Cld.b = ogLibFloatRand(0.0,0.3);
          lg->Cld.a = ogLibFloatRand(0.0,0.3);
          glLightfv(index,GL_DIFFUSE,&lg->Cld.r);
          ogEnvLog(6,"LIGHT: diffuse   %f,%f,%f,%f\n",lg->Cld.r,lg->Cld.g,lg->Cld.b,lg->Cld.a);
          }

        if (flag & OGTST_LIGHT_SPECULAR) {
          lg->Cls.r = ogLibFloatRand(0.0,0.3);
          lg->Cls.g = ogLibFloatRand(0.0,0.3);
          lg->Cls.b = ogLibFloatRand(0.0,0.3);
          lg->Cls.a = ogLibFloatRand(0.0,0.3);
          glLightfv(index,GL_SPECULAR,&lg->Cls.r);
          ogEnvLog(6,"LIGHT: specular  %f,%f,%f,%f\n",lg->Cls.r,lg->Cls.g,lg->Cls.b,lg->Cls.a);
          }

        if (flag & OGTST_LIGHT_POSITION) {
          lg->p[0] = ogLibFloatRand(-1024.0,1024.0);
          lg->p[1] = ogLibFloatRand(-1024.0,1024.0);
          lg->p[2] = ogLibFloatRand(-1024.0,1024.0);
          if (ogLibBitRand(1))
            lg->p[3] = ogLibFloatRand(1.0,64.0);
          else lg->p[3] = 0.0;
          glLightfv(index,GL_POSITION,&lg->p[0]);
          ogEnvLog(6,"LIGHT: position  %f,%f,%f,%f\n",lg->p[0],lg->p[1],lg->p[2],lg->p[3]);
          }

        if (flag & OGTST_LIGHT_SPOT_DIRECTION)  {
          lg->d.x = ogLibFloatRand(-1024,1024);
          lg->d.y = ogLibFloatRand(-1024,1024);
          lg->d.z = ogLibFloatRand(-1024,1024);
          glLightfv(index,GL_SPOT_DIRECTION,&lg->d.x);
          ogEnvLog(6,"LIGHT: spot dir  %f,%f,%f\n",lg->d.x,lg->d.y,lg->d.z);
          }

        if (flag & OGTST_LIGHT_SPOT_EXPONENT)  {
          lg->ex = ogLibIntRand(1,8);  
          glLightfv(index,GL_SPOT_EXPONENT,&lg->ex);
          ogEnvLog(6,"LIGHT: spot exp  %f\n",lg->ex);
          }

        if (flag & OGTST_LIGHT_SPOT_CUTOFF)  {
          if (ogLibBitRand(1))
            lg->an = ogLibFloatRand(0.0,90.0);
          else lg->an = 180.0;
          glLightfv(index,GL_SPOT_CUTOFF,&lg->an);
          ogEnvLog(6,"LIGHT: spot cut  %f\n",lg->an);
          }

        if (flag & OGTST_LIGHT_CONSTANT_ATTENUATION)  {
          lg->K0 = ogLibFloatRand(0.1,10.0) ;
          glLightfv(index,GL_CONSTANT_ATTENUATION,&lg->K0);
          ogEnvLog(6,"LIGHT: const att %f\n",lg->K0);
          }

        if (flag & OGTST_LIGHT_LINEAR_ATTENUATION)  {
          lg->K1 = ogLibFloatRand(0.1,10.0) ;
          glLightfv(index,GL_LINEAR_ATTENUATION,&lg->K1);
          ogEnvLog(6,"LIGHT: lin   att %f\n",lg->K1);
          }

        if (flag & OGTST_LIGHT_QUADRATIC_ATTENUATION)  {
          lg->K2 = ogLibFloatRand(0.2,2.0) ;
          glLightfv(index,GL_QUADRATIC_ATTENUATION,&lg->K2);
          ogEnvLog(6,"LIGHT: quad att  %f\n",lg->K2);
          }

        break ;

     case OGTST_MATERIAL:
        if (index == 0) {
          mf = &FrontMaterial;
          mb = &DummyMaterial;
          targ = GL_FRONT;
          }
        else if (index == 1) {
          mf = &DummyMaterial;
          mb = &BackMaterial;
          targ = GL_BACK;
          }
        else if (index == 2) {
          mf = &FrontMaterial;
          mb = &BackMaterial;
          targ = GL_FRONT_AND_BACK;
          }
        else ogEnvLog(OG_LINTERNALERROR,"ogtst_lmgen(%d,%d,0x%08x): invalid index\n",mode,index,flag) ; 

        if (flag & OGTST_MATERIAL_AMBIENT) {
          mf->Coa.r = mb->Coa.r = ogLibFloatRand(0.0,0.3);
          mf->Coa.g = mb->Coa.g = ogLibFloatRand(0.0,0.3);
          mf->Coa.b = mb->Coa.b = ogLibFloatRand(0.0,0.3);
          mf->Coa.a = mb->Coa.a = ogLibFloatRand(0.0,0.3);
          glMaterialfv(targ,GL_AMBIENT,&mf->Coa.r);
          ogEnvLog(6,"MAT%d: ambient   %f,%f,%f,%f\n",index,mf->Coa.r,mf->Coa.g,mf->Coa.b,mf->Coa.a);
          }

        if (flag & OGTST_MATERIAL_DIFFUSE) {
          mf->Cod.r = mb->Cod.r = ogLibFloatRand(0.2,0.6);
          mf->Cod.g = mb->Cod.g = ogLibFloatRand(0.2,0.6);
          mf->Cod.b = mb->Cod.b = ogLibFloatRand(0.2,0.6);
          mf->Cod.a = mb->Cod.a = ogLibFloatRand(0.2,0.6);
          glMaterialfv(targ,GL_DIFFUSE,&mf->Cod.r);
          ogEnvLog(6,"MAT%d: diffuse   %f,%f,%f,%f\n",index,mf->Cod.r,mf->Cod.g,mf->Cod.b,mf->Cod.a);
          }

        if (flag & OGTST_MATERIAL_SPECULAR) {
          mf->Cos.r = mb->Cos.r = ogLibFloatRand(0.2,0.9);
          mf->Cos.g = mb->Cos.g = ogLibFloatRand(0.2,0.9);
          mf->Cos.b = mb->Cos.b = ogLibFloatRand(0.2,0.9);
          mf->Cos.a = mb->Cos.a = ogLibFloatRand(0.2,0.9);
          glMaterialfv(targ,GL_SPECULAR,&mf->Cos.r);
          ogEnvLog(6,"MAT%d: specular  %f,%f,%f,%f\n",index,mf->Cos.r,mf->Cos.g,mf->Cos.b,mf->Cos.a);
          }

        if (flag & OGTST_MATERIAL_EMISSION) {
          mf->Coe.r = mb->Coe.r = ogLibFloatRand(0.2,0.4);
          mf->Coe.g = mb->Coe.g = ogLibFloatRand(0.2,0.4);
          mf->Coe.b = mb->Coe.b = ogLibFloatRand(0.2,0.4);
          mf->Coe.a = mb->Coe.a = ogLibFloatRand(0.2,0.4);
          glMaterialfv(targ,GL_EMISSION,&mf->Coe.r);
          ogEnvLog(6,"MAT%d: emission  %f,%f,%f,%f\n",index,mf->Coe.r,mf->Coe.g,mf->Coe.b,mf->Coe.a);
          }

        if (flag & OGTST_MATERIAL_SHININESS) {
          mf->Eos = mb->Eos = ogLibIntRand(1,8);
          glMaterialfv(targ,GL_SHININESS,&mf->Eos);
          ogEnvLog(6,"MAT%d: shininess %f\n",index,mf->Eos);
          }

        if (flag & OGTST_MATERIAL_AMBIENT_AND_DIFFUSE) {
          mf->Coa.r = mf->Cod.r = mb->Coa.r = mb->Cod.r = ogLibFloatRand(0.1,0.5);
          mf->Coa.g = mf->Cod.g = mb->Coa.g = mb->Cod.g = ogLibFloatRand(0.1,0.5);
          mf->Coa.b = mf->Cod.b = mb->Coa.b = mb->Cod.b = ogLibFloatRand(0.1,0.5);
          mf->Coa.a = mf->Cod.a = mb->Coa.a = mb->Cod.a = ogLibFloatRand(0.1,0.5);
          glMaterialfv(targ,GL_AMBIENT_AND_DIFFUSE,&mf->Coa.r);
          ogEnvLog(6,"MAT%d: amb/dif   %f,%f,%f,%f\n",index,mf->Coa.r,mf->Coa.g,mf->Coa.b,mf->Coa.a);
          }

        if (flag & OGTST_MATERIAL_COLOR_INDEXES) {
          mf->CI[0] = mb->CI[0] = ogLibFloatRand(0.0,15.9);
          mf->CI[1] = mb->CI[1] = ogLibFloatRand(0.0,15.9);
          mf->CI[2] = mb->CI[2] = ogLibFloatRand(0.0,15.9);
          glMaterialfv(targ,GL_COLOR_INDEXES,&mf->CI[0]);
          ogEnvLog(6,"MAT%d: indexes   %f,%f,%f\n",index,mf->CI[0],mf->CI[1],mf->CI[2]);
          }

        break ;

     case OGTST_MODEL:
        md = &Model;
     
        if (flag & OGTST_MODEL_AMBIENT) {
          md->Csa.r = ogLibFloatRand(0.0,0.3);
          md->Csa.g = ogLibFloatRand(0.0,0.3);
          md->Csa.b = ogLibFloatRand(0.0,0.3);
          md->Csa.a = ogLibFloatRand(0.0,0.3);
          glLightModelfv(GL_LIGHT_MODEL_AMBIENT,&md->Csa.r);
          ogEnvLog(6,"MODEL: ambient   %f,%f,%f,%f\n",md->Csa.r,md->Csa.g,md->Csa.b,md->Csa.a) ;
          }
     
        if (flag & OGTST_MODEL_LOCAL_VIEWER) {
          md->lv = ogLibBitRand(1);
          glLightModelf(GL_LIGHT_MODEL_LOCAL_VIEWER,md->lv);
          ogEnvLog(6,"MODEL: localvwr  %f\n",md->lv) ;
          }
     
        if (flag & OGTST_MODEL_TWO_SIDE) {
          md->ts = ogLibBitRand(1);
          glLightModelf(GL_LIGHT_MODEL_TWO_SIDE,md->ts);
          ogEnvLog(6,"MODEL: twoside   %f\n",md->ts) ;
          }
     
        break ;

     default:
        ogEnvLog(OG_LINTERNALERROR,"ogtst_lmgen(%d,%d,0x%08x): invalid mode\n",mode,index,flag) ; 
        break ;
     }
}

/*************************************************************
*  reset_all()  -
*************************************************************/
void reset_all(void)
{
   int i ;

   GLfloat expo[] = { 0.0 };
   GLfloat emis[] = { 0.0, 0.0, 0.0, 1.0 };
   GLfloat ambi[] = { 0.2, 0.2, 0.2, 1.0 };
   GLfloat diff[] = { 0.8, 0.8, 0.8, 1.0 };
   GLfloat spec[] = { 0.0, 0.0, 0.0, 1.0 };
   GLfloat posi[] = { 0.0, 0.0, 1.0, 0.0 };
   GLfloat ligt[] = { 1.0, 1.0, 1.0, 1.0 };
   GLfloat coin[] = { 0.0, 1.0, 1.0 };
   GLfloat dire[] = { 0.0, 0.0, -1.0 };
   GLfloat cuto[] = { 180.0 };

   /* model reset */
   Model.Csa.r = ambi[0]; 
   Model.Csa.g = ambi[1]; 
   Model.Csa.b = ambi[2]; 
   Model.Csa.a = ambi[3]; 
   Model.lv = 0.0; 
   Model.ts =  0.0;
   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambi);
   glLightModelfv(GL_LIGHT_MODEL_LOCAL_VIEWER, expo);
   glLightModelfv(GL_LIGHT_MODEL_TWO_SIDE, expo);

   /* material reset */
   BackMaterial.Coa.r = FrontMaterial.Coa.r = ambi[0];
   BackMaterial.Coa.g = FrontMaterial.Coa.g = ambi[1];
   BackMaterial.Coa.b = FrontMaterial.Coa.b = ambi[2];
   BackMaterial.Coa.a = FrontMaterial.Coa.a = ambi[3];
   BackMaterial.Cod.r = FrontMaterial.Cod.r = diff[0];
   BackMaterial.Cod.g = FrontMaterial.Cod.g = diff[1];
   BackMaterial.Cod.b = FrontMaterial.Cod.b = diff[2];
   BackMaterial.Cod.a = FrontMaterial.Cod.a = diff[3];
   BackMaterial.Cos.r = FrontMaterial.Cos.r = spec[0];
   BackMaterial.Cos.g = FrontMaterial.Cos.g = spec[1];
   BackMaterial.Cos.b = FrontMaterial.Cos.b = spec[2];
   BackMaterial.Cos.a = FrontMaterial.Cos.a = spec[3];
   BackMaterial.Coe.r = FrontMaterial.Coe.r = emis[0];
   BackMaterial.Coe.g = FrontMaterial.Coe.g = emis[1];
   BackMaterial.Coe.b = FrontMaterial.Coe.b = emis[2];
   BackMaterial.Coe.a = FrontMaterial.Coe.a = emis[3];
   BackMaterial.Eos   = FrontMaterial.Eos   = expo[0];
   BackMaterial.CI[0] = FrontMaterial.CI[0] = coin[0];
   BackMaterial.CI[1] = FrontMaterial.CI[1] = coin[1];
   BackMaterial.CI[2] = FrontMaterial.CI[2] = coin[2];
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   ambi);
   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   diff);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  spec);
   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION,  emis);
   glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, expo);
   glMaterialfv(GL_FRONT_AND_BACK, GL_COLOR_INDEXES, coin);

   /* light reset */
   for (i=0; i<8; i++) {
     Light[i].Cla.r = spec[0];
     Light[i].Cla.g = spec[1];
     Light[i].Cla.b = spec[2];
     Light[i].Cla.a = spec[3];
     Light[i].Cld.r = spec[0];
     Light[i].Cld.g = spec[1];
     Light[i].Cld.b = spec[2];
     Light[i].Cld.a = spec[3];
     Light[i].Cls.r = spec[0];
     Light[i].Cls.g = spec[1];
     Light[i].Cls.b = spec[2];
     Light[i].Cls.a = spec[3];
     Light[i].p[0] = posi[0];
     Light[i].p[1] = posi[1];
     Light[i].p[2] = posi[2];
     Light[i].p[3] = posi[3];
     Light[i].d.x = dire[0];
     Light[i].d.y = dire[1];
     Light[i].d.z = dire[2];
     Light[i].ex = expo[0];
     Light[i].an = cuto[0];
     Light[i].K0 = ligt[0];
     Light[i].K1 = expo[0];
     Light[i].K2 = expo[0];
     Light[i].flag = 0;
     glLightfv(GL_LIGHT0+i, GL_AMBIENT,   spec);
     glLightfv(GL_LIGHT0+i, GL_DIFFUSE,   spec);
     glLightfv(GL_LIGHT0+i, GL_SPECULAR,  spec);
     glLightfv(GL_LIGHT0+i, GL_POSITION,  posi);
     glLightfv(GL_LIGHT0+i, GL_SPOT_DIRECTION,  dire);
     glLightfv(GL_LIGHT0+i, GL_SPOT_EXPONENT,  expo);
     glLightfv(GL_LIGHT0+i, GL_SPOT_CUTOFF,  cuto);
     glLightfv(GL_LIGHT0+i, GL_CONSTANT_ATTENUATION,  ligt);
     glLightfv(GL_LIGHT0+i, GL_LINEAR_ATTENUATION,  expo);
     glLightfv(GL_LIGHT0+i, GL_QUADRATIC_ATTENUATION,  expo);
     glDisable(GL_LIGHT0+i);
     }
   Light[0].Cld.r = ligt[0];
   Light[0].Cld.g = ligt[1];
   Light[0].Cld.b = ligt[2];
   Light[0].Cld.a = ligt[3];
   Light[0].Cls.r = ligt[0];
   Light[0].Cls.g = ligt[1];
   Light[0].Cls.b = ligt[2];
   Light[0].Cls.a = ligt[3];
   glLightfv(GL_LIGHT0, GL_DIFFUSE,   ligt);
   glLightfv(GL_LIGHT0, GL_SPECULAR,  ligt);

   ogEnvLog(1,"  RESET ALL PROPERTIES\n") ;
}

/*************************************************************
*  dumpster()  -
*************************************************************/
/*ARGSUSED*/
void dumpster(unsigned int retcol,int pass,int flag)
{
   int x,y,tol ;
   int r,g,b,a ;
   int expected_r,expected_g,expected_b,expected_a ;
   unsigned int buffer ;

   glBegin(GL_TRIANGLES) ;
   glNormal3fv(&Object.n.x) ;
   glVertex3fv(&Object.p1.x) ;
   glVertex3fv(&Object.p2.x) ;
   glVertex3fv(&Object.p3.x) ;
   glEnd() ;

   x = Object.p1.x ;
   y = Object.p1.y ;

   ogLibReadPixels(x,y,x,y,&buffer);
   ogEnvLog(2,"   readback returned 0x%08x at (%d, %d)\n", buffer, x, y) ;

   a = (buffer      ) & 0xFF;
   b = (buffer >>  8) & 0xFF;
   g = (buffer >> 16) & 0xFF;
   r = (buffer >> 24) & 0xFF;

   tol = light_tolerance[pass];

   expected_a = (retcol      ) & 0xFF ;
   expected_b = (retcol >>  8) & 0xFF ;
   expected_g = (retcol >> 16) & 0xFF ;
   expected_r = (retcol >> 24) & 0xFF ;

   if (ABS(r - expected_r) > tol || ABS(g - expected_g) > tol ||
       ABS(b - expected_b) > tol || ABS(a - expected_a) > tol)
     ogEnvLog (OG_LFAIL,"color at (%d,%d) is (0x%x,0x%x,0x%x,0x%x) but should be (0x%x,0x%x,0x%x,0x%x)\n",
                  x,y,r,g,b,a,expected_r,expected_g,expected_b,expected_a);
}

/*************************************************************
* 
*************************************************************/
void eye_init(void)
{
   Eye.p.x = 0.0 ;
   Eye.p.x = 0.0 ;
   Eye.p.z = 0.0 ;

   ogEnvLog(3,"EYE position %f,%f,%f\n",Eye.p.x,Eye.p.y,Eye.p.z) ;
}

/*************************************************************
*  object_init()  - 
*************************************************************/
/*ARGSUSED*/
void object_init(int flag)
{
   Object.p1.x = ogLibIntRand(3,636);
   Object.p1.y = ogLibIntRand(3,508);
   Object.p1.z = ogLibFloatRand(-1023.0,1023.0);

   Object.n.x = ogLibFloatRand(-64.0,64.0) ;
   Object.n.y = ogLibFloatRand(-64.0,64.0) ;
   Object.n.z = ogLibFloatRand(-64.0,64.0) ;

   Object.p2.z = Object.p1.z ;
   Object.p3.z = Object.p1.z ;

   if (ogLibBitRand(1))  {
     Object.p2.x = Object.p1.x + 3 ;
     Object.p2.y = Object.p1.y     ;
     Object.p3.x = Object.p1.x + 3 ;
     Object.p3.y = Object.p1.y + 3 ;
     }
   else  {
     Object.p3.x = Object.p1.x + 3 ;
     Object.p3.y = Object.p1.y     ;
     Object.p2.x = Object.p1.x + 3 ;
     Object.p2.y = Object.p1.y + 3 ;
     }

   ogEnvLog(3,"OBJECT\n") ;
   ogEnvLog(3,"normal   %f,%f,%f\n",Object.n.x,Object.n.y,Object.n.z) ;
   ogEnvLog(3,"position %f,%f,%f\n",Object.p1.x,Object.p1.y,Object.p1.z) ;
   ogEnvLog(3,"position %f,%f,%f\n",Object.p2.x,Object.p2.y,Object.p2.z) ;
   ogEnvLog(3,"position %f,%f,%f\n",Object.p3.x,Object.p3.y,Object.p3.z) ;
}

/*************************************************************
*  enable()  - 
*************************************************************/
void enable(int index)
{
   if (index < 0 || index > 7)
     ogEnvLog(OG_LINTERNALERROR,"enable(%d): invalid index\n",index) ;

   Light[index].flag = 1;
   glEnable(GL_LIGHT0+index);
   ogEnvLog(2,"glEnable(GL_LIGHT%d)\n",index);
}

/*************************************************************
*  disable()  - 
*************************************************************/
void disable(int index)
{
   if (index < 0 || index > 7)
     ogEnvLog(OG_LINTERNALERROR,"disable(%d): invalid index\n",index) ;

   Light[index].flag = 0;
   glDisable(GL_LIGHT0+index);
   ogEnvLog(2,"glDisable(GL_LIGHT%d)\n",index);
}

