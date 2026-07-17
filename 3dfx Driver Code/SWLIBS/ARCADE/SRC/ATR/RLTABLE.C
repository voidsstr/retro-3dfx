/*
** Copyright (c) 1996, 3Dfx Interactive, Inc.
** All Rights Reserved.
**
** This is UNPUBLISHED PROPRIETARY SOURCE CODE of 3Dfx Interactive, Inc.;
** the contents of this file may not be disclosed to third parties, copied or
** duplicated in any form, in whole or in part, without the prior written
** permission of 3Dfx Interactive, Inc.
**
** RESTRICTED RIGHTS LEGEND:
** Use, duplication or disclosure by the Government is subject to restrictions
** as set forth in subdivision (c)(1)(ii) of the Rights in Technical Data
** and Computer Software clause at DFARS 252.227-7013, and/or in similar or
** successor clauses in the FAR, DOD or NASA FAR Supplement. Unpublished  -
** rights reserved under the Copyright Laws of the United States.
**
**
** $Revision: 4$ 
** $Date: 10/11/00 7:34:10 PM$ 
**
*/

#include "atrender.h"
#include "fxatr.h"

/*----------------------------------------------------------
  FUNCTION DATA DECLARATION SECTIONS
  ----------------------------------------------------------*/

#define DATA_AMBIENT
#define DATA_DIRECTED
#define DATA_POSITIONAL
#define DATA_ATTENUATED                                              \
    float quadFact  = light->light.attenuation[ATR_LIGHT_QUADRATIC]; \
    float linFact   = light->light.attenuation[ATR_LIGHT_LINEAR];    \
    float constFact = light->light.attenuation[ATR_LIGHT_CONSTANT];
#define DATA_PROJECTED

/*----------------------------------------------------------
  FUNCTION INIT SECTIONS
  ----------------------------------------------------------*/
#define FINIT_INIT                                          \
    dest->r = _atrRenderCache->matBaseColor.r;              \
    dest->g = _atrRenderCache->matBaseColor.g;              \
    dest->b = _atrRenderCache->matBaseColor.b;              

#define FINIT_ACCUM    

/*----------------------------------------------------------
  FUNCTION CLAMP SECTIONS
  ----------------------------------------------------------*/
#define FCLAMP_NOCLAMP

#define FCLAMP_CLAMP                                        \
  if ( FLOAT_BITS(dest->r) > 0x437F0000 ) dest->r = 255.0f; \
  if ( FLOAT_BITS(dest->g) > 0x437F0000 ) dest->g = 255.0f; \
  if ( FLOAT_BITS(dest->b) > 0x437F0000 ) dest->b = 255.0f; 

/*----------------------------------------------------------
  FUNCTION LIGHT SECTIONS
  ----------------------------------------------------------*/
#define FLIGHT_AMBIENT_DIFFUSE_SINGLE
#define FLIGHT_AMBIENT_DIFFUSE_DOUBLE
#define FLIGHT_AMBIENT_SPEC_SINGLE
#define FLIGHT_AMBIENT_SPEC_DOUBLE
#define FLIGHT_AMBIENT_DIFFSPEC_SINGLE
#define FLIGHT_AMBIENT_DIFFSPEC_DOUBLE

#define FLIGHT_DIRECTED_DIFFUSE_SINGLE              \
    {                                               \
        float factor = light->vf[0] * src->i +      \
                       light->vf[1] * src->j +      \
                       light->vf[2] * src->k;       \
        if ( factor > 0.0f ) {                      \
            dest->r += light->ld.r * factor;        \
            dest->g += light->ld.g * factor;        \
            dest->b += light->ld.b * factor;        \
        }                                           \
    }

#define FLIGHT_DIRECTED_SPEC_SINGLE                         \
    {                                                       \
        float factor = light->vf[0] * src->i +              \
                       light->vf[1] * src->j +              \
                       light->vf[2] * src->k;               \
        if ( factor > 0.0f ) {                              \
            /*Light To Vertex Assumed to Be VF */           \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                light->vf[0];                               \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                light->vf[1];                               \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                light->vf[2];                               \
            float ooHaDist;                                 \
            ooHaDist = atmOOSqrt( sumX * sumX +             \
                                  sumY * sumY +             \
                                  sumZ * sumZ );            \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            factor = factor /                               \
                     (                                      \
                        _atrRenderMaterial->specExponent -  \
                        factor *                            \
                        _atrRenderMaterial->specExponent +  \
                        factor                              \
                     );                                     \
            if ( factor > 0.0f ) {                          \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }


#define FLIGHT_DIRECTED_DIFFSPEC_SINGLE                     \
    {                                                       \
        float factor = light->vf[0] * src->i +              \
                       light->vf[1] * src->j +              \
                       light->vf[2] * src->k;               \
        if ( factor > 0.0f ) {                              \
            /*Light To Vertex Assumed to Be VF */           \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                light->vf[0];                               \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                light->vf[1];                               \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                light->vf[2];                               \
            float ooHaDist;                                 \
            dest->r += light->ld.r * factor;                \
            dest->g += light->ld.g * factor;                \
            dest->b += light->ld.b * factor;                \
            ooHaDist = atmOOSqrt( sumX * sumX +             \
                                  sumY * sumY +             \
                                  sumZ * sumZ );            \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            factor = factor /                               \
                     (                                      \
                        _atrRenderMaterial->specExponent -  \
                        factor *                            \
                        _atrRenderMaterial->specExponent +  \
                        factor                              \
                     );                                     \
            if ( factor > 0.0f ) {                          \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }

#define FLIGHT_POSITIONAL_DIFFUSE_SINGLE                    \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float ooDist =                                  \
               atmOOSqrt( lightToVertexX * lightToVertexX + \
                          lightToVertexY * lightToVertexY + \
                          lightToVertexZ * lightToVertexZ );\
            factor *= ooDist;                               \
            dest->r += light->ld.r * factor;                \
            dest->g += light->ld.g * factor;                \
            dest->b += light->ld.b * factor;                \
        }                                                   \
    }

#define FLIGHT_POSITIONAL_SPEC_SINGLE                       \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                lightToVertexX;                             \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                lightToVertexY;                             \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                lightToVertexZ;                             \
            float ooHaDist = atmOOSqrt( sumX * sumX +       \
                                        sumY * sumY +       \
                                        sumZ * sumZ );      \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            factor = factor /                               \
                     (                                      \
                        _atrRenderMaterial->specExponent -  \
                        factor *                            \
                        _atrRenderMaterial->specExponent +  \
                        factor                              \
                     );                                     \
            if ( factor > 0.0f ) {                          \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }


#define FLIGHT_POSITIONAL_DIFFSPEC_SINGLE                   \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                lightToVertexX;                             \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                lightToVertexY;                             \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                lightToVertexZ;                             \
            float ooHaDist = atmOOSqrt( sumX * sumX +       \
                                        sumY * sumY +       \
                                        sumZ * sumZ );      \
            float ooDist =                                  \
               atmOOSqrt( lightToVertexX * lightToVertexX + \
                          lightToVertexY * lightToVertexY + \
                          lightToVertexZ * lightToVertexZ );\
            factor *= ooDist;                               \
            dest->r += light->ld.r * factor;                \
            dest->g += light->ld.g * factor;                \
            dest->b += light->ld.b * factor;                \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            factor = factor /                               \
                     (                                      \
                        _atrRenderMaterial->specExponent -  \
                        factor *                            \
                        _atrRenderMaterial->specExponent +  \
                        factor                              \
                     );                                     \
            if ( factor > 0.0f ) {                          \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }

#define FLIGHT_ATTENUATED_DIFFUSE_SINGLE                    \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float dist = 1.0f /                             \
             (atmOOSqrt( lightToVertexX * lightToVertexX +  \
                         lightToVertexY * lightToVertexY +  \
                         lightToVertexZ * lightToVertexZ )  \
              +0.001f);                                     \
            factor = factor /                               \
                     ( dist * ( dist * ( dist * quadFact +  \
                       linFact ) + constFact ));            \
            dest->r += light->ld.r * factor;                \
            dest->g += light->ld.g * factor;                \
            dest->b += light->ld.b * factor;                \
        }                                                   \
    }


#define FLIGHT_ATTENUATED_SPEC_SINGLE                       \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                lightToVertexX;                             \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                lightToVertexY;                             \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                lightToVertexZ;                             \
            float ooHaDist = atmOOSqrt( sumX * sumX +       \
                                        sumY * sumY +       \
                                        sumZ * sumZ );      \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            if ( factor > 0.0f ) {                          \
                float dist = 1.0f /                             \
                 (atmOOSqrt( lightToVertexX * lightToVertexX +  \
                             lightToVertexY * lightToVertexY +  \
                             lightToVertexZ * lightToVertexZ )  \
                  +.001f );                                     \
                float dfactor = dist * ( dist * ( dist *        \
                                quadFact + linFact ) +          \
                                constFact );                    \
                factor = factor /                           \
                  ((_atrRenderMaterial->specExponent -      \
                    factor *                                \
                    _atrRenderMaterial->specExponent +      \
                    factor) * dfactor );                    \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }


#define FLIGHT_ATTENUATED_DIFFSPEC_SINGLE                   \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                lightToVertexX;                             \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                lightToVertexY;                             \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                lightToVertexZ;                             \
            float ooHaDist = atmOOSqrt( sumX * sumX +       \
                                        sumY * sumY +       \
                                        sumZ * sumZ );      \
            float dist = 1.0f /                             \
             (atmOOSqrt( lightToVertexX * lightToVertexX +  \
                         lightToVertexY * lightToVertexY +  \
                         lightToVertexZ * lightToVertexZ )  \
              +0.001f);                                     \
            float dfactor = dist*(dist*(dist*quadFact+      \
                             linFact)+constFact);           \
            factor = factor / dfactor;                      \
            dest->r += light->ld.r * factor;                \
            dest->g += light->ld.g * factor;                \
            dest->b += light->ld.b * factor;                \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            if ( factor > 0.0f ) {                          \
                factor = factor /                           \
                  ((_atrRenderMaterial->specExponent -      \
                    factor *                                \
                    _atrRenderMaterial->specExponent +      \
                    factor) * dfactor );                    \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }

#define FLIGHT_DIRECTED_DIFFUSE_DOUBLE              \
    {                                               \
        float factor = light->vf[0] * src->i +      \
                       light->vf[1] * src->j +      \
                       light->vf[2] * src->k;       \
        if ( factor > 0.0f ) {                      \
            dest->r += light->ld.r * factor;        \
            dest->g += light->ld.g * factor;        \
            dest->b += light->ld.b * factor;        \
        }                                           \
    }

#define FLIGHT_DIRECTED_SPEC_DOUBLE                         \
    {                                                       \
        float factor = light->vf[0] * src->i +              \
                       light->vf[1] * src->j +              \
                       light->vf[2] * src->k;               \
        if ( factor > 0.0f ) {                              \
            /*Light To Vertex Assumed to Be VF */           \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                light->vf[0];                               \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                light->vf[1];                               \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                light->vf[2];                               \
            float ooHaDist;                                 \
            ooHaDist = atmOOSqrt( sumX * sumX +             \
                                  sumY * sumY +             \
                                  sumZ * sumZ );            \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            factor = factor /                               \
                     (                                      \
                        _atrRenderMaterial->specExponent -  \
                        factor *                            \
                        _atrRenderMaterial->specExponent +  \
                        factor                              \
                     );                                     \
            if ( factor > 0.0f ) {                          \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }


#define FLIGHT_DIRECTED_DIFFSPEC_DOUBLE                     \
    {                                                       \
        float factor = light->vf[0] * src->i +              \
                       light->vf[1] * src->j +              \
                       light->vf[2] * src->k;               \
        if ( factor > 0.0f ) {                              \
            /*Light To Vertex Assumed to Be VF */           \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                light->vf[0];                               \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                light->vf[1];                               \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                light->vf[2];                               \
            float ooHaDist;                                 \
            dest->r += light->ld.r * factor;                \
            dest->g += light->ld.g * factor;                \
            dest->b += light->ld.b * factor;                \
            ooHaDist = atmOOSqrt( sumX * sumX +             \
                                  sumY * sumY +             \
                                  sumZ * sumZ );            \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            factor = factor /                               \
                     (                                      \
                        _atrRenderMaterial->specExponent -  \
                        factor *                            \
                        _atrRenderMaterial->specExponent +  \
                        factor                              \
                     );                                     \
            if ( factor > 0.0f ) {                          \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }

#define FLIGHT_POSITIONAL_DIFFUSE_DOUBLE                    \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float ooDist =                                  \
               atmOOSqrt( lightToVertexX * lightToVertexX + \
                          lightToVertexY * lightToVertexY + \
                          lightToVertexZ * lightToVertexZ );\
            factor *= ooDist;                               \
            dest->r += light->ld.r * factor;                \
            dest->g += light->ld.g * factor;                \
            dest->b += light->ld.b * factor;                \
        }                                                   \
    }

#define FLIGHT_POSITIONAL_SPEC_DOUBLE                       \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                lightToVertexX;                             \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                lightToVertexY;                             \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                lightToVertexZ;                             \
            float ooHaDist = atmOOSqrt( sumX * sumX +       \
                                        sumY * sumY +       \
                                        sumZ * sumZ );      \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            factor = factor /                               \
                     (                                      \
                        _atrRenderMaterial->specExponent -  \
                        factor *                            \
                        _atrRenderMaterial->specExponent +  \
                        factor                              \
                     );                                     \
            if ( factor > 0.0f ) {                          \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }


#define FLIGHT_POSITIONAL_DIFFSPEC_DOUBLE                   \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                lightToVertexX;                             \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                lightToVertexY;                             \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                lightToVertexZ;                             \
            float ooHaDist = atmOOSqrt( sumX * sumX +       \
                                        sumY * sumY +       \
                                        sumZ * sumZ );      \
            float ooDist =                                  \
               atmOOSqrt( lightToVertexX * lightToVertexX + \
                          lightToVertexY * lightToVertexY + \
                          lightToVertexZ * lightToVertexZ );\
            factor *= ooDist;                               \
            dest->r += light->ld.r * factor;                \
            dest->g += light->ld.g * factor;                \
            dest->b += light->ld.b * factor;                \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            factor = factor /                               \
                     (                                      \
                        _atrRenderMaterial->specExponent -  \
                        factor *                            \
                        _atrRenderMaterial->specExponent +  \
                        factor                              \
                     );                                     \
            if ( factor > 0.0f ) {                          \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }

#define FLIGHT_ATTENUATED_DIFFUSE_DOUBLE                    \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float dist = 1.0f /                             \
             (atmOOSqrt( lightToVertexX * lightToVertexX +  \
                         lightToVertexY * lightToVertexY +  \
                         lightToVertexZ * lightToVertexZ )  \
              +0.001f);                                     \
            factor = factor /                               \
                     ( dist * ( dist * ( dist * quadFact +  \
                       linFact ) + constFact ));            \
            dest->r += light->ld.r * factor;                \
            dest->g += light->ld.g * factor;                \
            dest->b += light->ld.b * factor;                \
        }                                                   \
    }


#define FLIGHT_ATTENUATED_SPEC_DOUBLE                       \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                lightToVertexX;                             \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                lightToVertexY;                             \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                lightToVertexZ;                             \
            float ooHaDist = atmOOSqrt( sumX * sumX +       \
                                        sumY * sumY +       \
                                        sumZ * sumZ );      \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            if ( factor > 0.0f ) {                          \
                float dist = 1.0f /                             \
                 (atmOOSqrt( lightToVertexX * lightToVertexX +  \
                             lightToVertexY * lightToVertexY +  \
                             lightToVertexZ * lightToVertexZ )  \
                  +.001f );                                     \
                float dfactor = dist * ( dist * ( dist *        \
                                quadFact + linFact ) +          \
                                constFact );                    \
                factor = factor /                           \
                  ((_atrRenderMaterial->specExponent -      \
                    factor *                                \
                    _atrRenderMaterial->specExponent +      \
                    factor) * dfactor );                    \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }


#define FLIGHT_ATTENUATED_DIFFSPEC_DOUBLE                   \
    {                                                       \
        float lightToVertexX = light->lp[0] - src->x;       \
        float lightToVertexY = light->lp[1] - src->y;       \
        float lightToVertexZ = light->lp[2] - src->z;       \
        float factor = lightToVertexX * src->i +            \
                       lightToVertexY * src->j +            \
                       lightToVertexZ * src->k;             \
        if ( factor > 0.0f ) {                              \
            float sumX =                                    \
                _atrRenderCache->cameraPositionInLCS[0] -   \
                src->x +                                    \
                lightToVertexX;                             \
            float sumY =                                    \
                _atrRenderCache->cameraPositionInLCS[1] -   \
                src->y +                                    \
                lightToVertexY;                             \
            float sumZ =                                    \
                _atrRenderCache->cameraPositionInLCS[2] -   \
                src->z +                                    \
                lightToVertexZ;                             \
            float ooHaDist = atmOOSqrt( sumX * sumX +       \
                                        sumY * sumY +       \
                                        sumZ * sumZ );      \
            float dist = 1.0f /                             \
             (atmOOSqrt( lightToVertexX * lightToVertexX +  \
                         lightToVertexY * lightToVertexY +  \
                         lightToVertexZ * lightToVertexZ )  \
              +0.001f);                                     \
            float dfactor = dist*(dist*(dist*quadFact+      \
                             linFact)+constFact);           \
            factor = factor / dfactor;                      \
            dest->r += light->ld.r * factor;                \
            dest->g += light->ld.g * factor;                \
            dest->b += light->ld.b * factor;                \
            factor = ( sumX * src->i +                      \
                       sumY * src->j +                      \
                       sumZ * src->k ) *                    \
                       ooHaDist;                            \
            if ( factor > 0.0f ) {                          \
                factor = factor /                           \
                  ((_atrRenderMaterial->specExponent -      \
                    factor *                                \
                    _atrRenderMaterial->specExponent +      \
                    factor) * dfactor );                    \
                dest->r += light->ls.r * factor;            \
                dest->g += light->ls.g * factor;            \
                dest->b += light->ls.b * factor;            \
            }                                               \
        }                                                   \
    }


#define FLIGHT_PROJECTED_DIFFUSE_SINGLE
#define FLIGHT_PROJECTED_SPEC_SINGLE
#define FLIGHT_PROJECTED_DIFFSPEC_SINGLE
#define FLIGHT_PROJECTED_DIFFUSE_DOUBLE
#define FLIGHT_PROJECTED_SPEC_DOUBLE
#define FLIGHT_PROJECTED_DIFFSPEC_DOUBLE

/*----------------------------------------------------------
  Function Construction Macros
  ----------------------------------------------------------*/

#define MAKE_FUNCTION_NAME( _MODE, _TYPE, _INIT, _CLAMP, _MAT )  \
    _MODE ## _TYPE ## _INIT ## _CLAMP ## _MAT

#define FUNCTION_DATA( _TYPE )                             \
    DATA_ ## _TYPE

#define FUNCTION_LOOP_BEGIN                                \
    while( src < end ) {

#define FUNCTION_LOOP_END                                  \
    src++, dest++; }

#define FUNCTION_INIT( _INIT )                             \
    FINIT_ ## _INIT

#define FUNCTION_CLAMP( _CLAMP )                           \
    FCLAMP_ ## _CLAMP

#define FUNCTION_LIGHT( _TYPE, _MAT, _MODE )               \
    FLIGHT_ ## _TYPE ## _ ## _MAT ## _ ## _MODE
#if macintosh
#pragma require_prototypes off
#pragma warn_unusedarg off
#endif
#define MAKE_FUNCTION( _MODE, _TYPE, _INIT, _CLAMP, _MAT )     \
    void                                                       \
    MAKE_FUNCTION_NAME( _MODE, _TYPE, _INIT, _CLAMP, _MAT )    \
    ( AtrDstVertex *dest, AtrVertex *src,                      \
     AtrVertex *end, _AtrLightNode *light ) {                  \
    FUNCTION_DATA( _TYPE )                                     \
    FUNCTION_LOOP_BEGIN                                        \
    FUNCTION_INIT(  _INIT )                                    \
    FUNCTION_LIGHT( _TYPE, _MAT, _MODE )                       \
    FUNCTION_CLAMP( _CLAMP )                                   \
    FUNCTION_LOOP_END                                          \
    }

/*----------------------------------------------------------
  Function Definitions
  ----------------------------------------------------------*/
    MAKE_FUNCTION( SINGLE, AMBIENT,    INIT,  NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( SINGLE, AMBIENT,    INIT,  NOCLAMP, SPEC     )
    MAKE_FUNCTION( SINGLE, AMBIENT,    INIT,  NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( SINGLE, AMBIENT,    INIT,  CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( SINGLE, AMBIENT,    INIT,  CLAMP,   SPEC     )
    MAKE_FUNCTION( SINGLE, AMBIENT,    INIT,  CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( SINGLE, AMBIENT,    ACCUM, NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( SINGLE, AMBIENT,    ACCUM, NOCLAMP, SPEC     )
    MAKE_FUNCTION( SINGLE, AMBIENT,    ACCUM, NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( SINGLE, AMBIENT,    ACCUM, CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( SINGLE, AMBIENT,    ACCUM, CLAMP,   SPEC     )
    MAKE_FUNCTION( SINGLE, AMBIENT,    ACCUM, CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( SINGLE, DIRECTED,   INIT,  NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( SINGLE, DIRECTED,   INIT,  NOCLAMP, SPEC     )
    MAKE_FUNCTION( SINGLE, DIRECTED,   INIT,  NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( SINGLE, DIRECTED,   INIT,  CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( SINGLE, DIRECTED,   INIT,  CLAMP,   SPEC     )
    MAKE_FUNCTION( SINGLE, DIRECTED,   INIT,  CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( SINGLE, DIRECTED,   ACCUM, NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( SINGLE, DIRECTED,   ACCUM, NOCLAMP, SPEC     )
    MAKE_FUNCTION( SINGLE, DIRECTED,   ACCUM, NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( SINGLE, DIRECTED,   ACCUM, CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( SINGLE, DIRECTED,   ACCUM, CLAMP,   SPEC     )
    MAKE_FUNCTION( SINGLE, DIRECTED,   ACCUM, CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( SINGLE, POSITIONAL, INIT,  NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( SINGLE, POSITIONAL, INIT,  NOCLAMP, SPEC     )
    MAKE_FUNCTION( SINGLE, POSITIONAL, INIT,  NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( SINGLE, POSITIONAL, INIT,  CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( SINGLE, POSITIONAL, INIT,  CLAMP,   SPEC     )
    MAKE_FUNCTION( SINGLE, POSITIONAL, INIT,  CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( SINGLE, POSITIONAL, ACCUM, NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( SINGLE, POSITIONAL, ACCUM, NOCLAMP, SPEC     )
    MAKE_FUNCTION( SINGLE, POSITIONAL, ACCUM, NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( SINGLE, POSITIONAL, ACCUM, CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( SINGLE, POSITIONAL, ACCUM, CLAMP,   SPEC     )
    MAKE_FUNCTION( SINGLE, POSITIONAL, ACCUM, CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( SINGLE, ATTENUATED, INIT,  NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( SINGLE, ATTENUATED, INIT,  NOCLAMP, SPEC     )
    MAKE_FUNCTION( SINGLE, ATTENUATED, INIT,  NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( SINGLE, ATTENUATED, INIT,  CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( SINGLE, ATTENUATED, INIT,  CLAMP,   SPEC     )
    MAKE_FUNCTION( SINGLE, ATTENUATED, INIT,  CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( SINGLE, ATTENUATED, ACCUM, NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( SINGLE, ATTENUATED, ACCUM, NOCLAMP, SPEC     )
    MAKE_FUNCTION( SINGLE, ATTENUATED, ACCUM, NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( SINGLE, ATTENUATED, ACCUM, CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( SINGLE, ATTENUATED, ACCUM, CLAMP,   SPEC     )
    MAKE_FUNCTION( SINGLE, ATTENUATED, ACCUM, CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( SINGLE, PROJECTED, INIT,  NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( SINGLE, PROJECTED, INIT,  NOCLAMP, SPEC     )
    MAKE_FUNCTION( SINGLE, PROJECTED, INIT,  NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( SINGLE, PROJECTED, INIT,  CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( SINGLE, PROJECTED, INIT,  CLAMP,   SPEC     )
    MAKE_FUNCTION( SINGLE, PROJECTED, INIT,  CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( SINGLE, PROJECTED, ACCUM, NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( SINGLE, PROJECTED, ACCUM, NOCLAMP, SPEC     )
    MAKE_FUNCTION( SINGLE, PROJECTED, ACCUM, NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( SINGLE, PROJECTED, ACCUM, CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( SINGLE, PROJECTED, ACCUM, CLAMP,   SPEC     )
    MAKE_FUNCTION( SINGLE, PROJECTED, ACCUM, CLAMP,   DIFFSPEC )


    MAKE_FUNCTION( DOUBLE, AMBIENT,    INIT,  NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, AMBIENT,    INIT,  NOCLAMP, SPEC     )
    MAKE_FUNCTION( DOUBLE, AMBIENT,    INIT,  NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, AMBIENT,    INIT,  CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, AMBIENT,    INIT,  CLAMP,   SPEC     )
    MAKE_FUNCTION( DOUBLE, AMBIENT,    INIT,  CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, AMBIENT,    ACCUM, NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, AMBIENT,    ACCUM, NOCLAMP, SPEC     )
    MAKE_FUNCTION( DOUBLE, AMBIENT,    ACCUM, NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, AMBIENT,    ACCUM, CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, AMBIENT,    ACCUM, CLAMP,   SPEC     )
    MAKE_FUNCTION( DOUBLE, AMBIENT,    ACCUM, CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, DIRECTED,   INIT,  NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, DIRECTED,   INIT,  NOCLAMP, SPEC     )
    MAKE_FUNCTION( DOUBLE, DIRECTED,   INIT,  NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, DIRECTED,   INIT,  CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, DIRECTED,   INIT,  CLAMP,   SPEC     )
    MAKE_FUNCTION( DOUBLE, DIRECTED,   INIT,  CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, DIRECTED,   ACCUM, NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, DIRECTED,   ACCUM, NOCLAMP, SPEC     )
    MAKE_FUNCTION( DOUBLE, DIRECTED,   ACCUM, NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, DIRECTED,   ACCUM, CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, DIRECTED,   ACCUM, CLAMP,   SPEC     )
    MAKE_FUNCTION( DOUBLE, DIRECTED,   ACCUM, CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, POSITIONAL, INIT,  NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, POSITIONAL, INIT,  NOCLAMP, SPEC     )
    MAKE_FUNCTION( DOUBLE, POSITIONAL, INIT,  NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, POSITIONAL, INIT,  CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, POSITIONAL, INIT,  CLAMP,   SPEC     )
    MAKE_FUNCTION( DOUBLE, POSITIONAL, INIT,  CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, POSITIONAL, ACCUM, NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, POSITIONAL, ACCUM, NOCLAMP, SPEC     )
    MAKE_FUNCTION( DOUBLE, POSITIONAL, ACCUM, NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, POSITIONAL, ACCUM, CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, POSITIONAL, ACCUM, CLAMP,   SPEC     )
    MAKE_FUNCTION( DOUBLE, POSITIONAL, ACCUM, CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, ATTENUATED, INIT,  NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, ATTENUATED, INIT,  NOCLAMP, SPEC     )
    MAKE_FUNCTION( DOUBLE, ATTENUATED, INIT,  NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, ATTENUATED, INIT,  CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, ATTENUATED, INIT,  CLAMP,   SPEC     )
    MAKE_FUNCTION( DOUBLE, ATTENUATED, INIT,  CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, ATTENUATED, ACCUM, NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, ATTENUATED, ACCUM, NOCLAMP, SPEC     )
    MAKE_FUNCTION( DOUBLE, ATTENUATED, ACCUM, NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, ATTENUATED, ACCUM, CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, ATTENUATED, ACCUM, CLAMP,   SPEC     )
    MAKE_FUNCTION( DOUBLE, ATTENUATED, ACCUM, CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, PROJECTED, INIT,  NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, PROJECTED, INIT,  NOCLAMP, SPEC     )
    MAKE_FUNCTION( DOUBLE, PROJECTED, INIT,  NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, PROJECTED, INIT,  CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, PROJECTED, INIT,  CLAMP,   SPEC     )
    MAKE_FUNCTION( DOUBLE, PROJECTED, INIT,  CLAMP,   DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, PROJECTED, ACCUM, NOCLAMP, DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, PROJECTED, ACCUM, NOCLAMP, SPEC     )
    MAKE_FUNCTION( DOUBLE, PROJECTED, ACCUM, NOCLAMP, DIFFSPEC )

    MAKE_FUNCTION( DOUBLE, PROJECTED, ACCUM, CLAMP,   DIFFUSE  )
    MAKE_FUNCTION( DOUBLE, PROJECTED, ACCUM, CLAMP,   SPEC     )
    MAKE_FUNCTION( DOUBLE, PROJECTED, ACCUM, CLAMP,   DIFFSPEC )

/*----------------------------------------------------------
  NULL FUNCTIONS
  ----------------------------------------------------------*/
#define NULL_FUNCTION_NAME _nullLightFunc

static void _nullLightFunc( AtrDstVertex dest[],
                            AtrVertex      src[],
                            AtrVertex      *end,
                            _AtrLightNode  *light ) {
    FXUNUSED( dest );
    FXUNUSED( src );
    FXUNUSED( end );
    return;
}

/*----------------------------------------------------------
  Function Table
  ----------------------------------------------------------*/
_AtrLightFunc _atrLightTable[ATR_NUM_MAT_MODES]
                            [ATR_NUM_LIGHT_TYPES]
                            [ATR_NUM_ACCUM_MODES]
                            [ATR_NUM_CLAMP_MODES]
                            [ATR_NUM_MAT_TYPES] = {
    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    INIT,  NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    INIT,  NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    INIT,  NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    INIT,  CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    INIT,  CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    INIT,  CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    ACCUM, NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    ACCUM, NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    ACCUM, NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    ACCUM, CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    ACCUM, CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, AMBIENT,    ACCUM, CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,


    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   INIT,  NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   INIT,  NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   INIT,  NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   INIT,  CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   INIT,  CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   INIT,  CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   ACCUM, NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   ACCUM, NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   ACCUM, NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   ACCUM, CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   ACCUM, CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, DIRECTED,   ACCUM, CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,


    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, INIT,  NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, INIT,  NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, INIT,  NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, INIT,  CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, INIT,  CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, INIT,  CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, ACCUM, NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, ACCUM, NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, ACCUM, NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, ACCUM, CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, ACCUM, CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, POSITIONAL, ACCUM, CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,


    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, INIT,  NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, INIT,  NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, INIT,  NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, INIT,  CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, INIT,  CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, INIT,  CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, ACCUM, NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, ACCUM, NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, ACCUM, NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, ACCUM, CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, ACCUM, CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, ATTENUATED, ACCUM, CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, INIT,  NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, INIT,  NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, INIT,  NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, INIT,  CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, INIT,  CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, INIT,  CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, ACCUM, NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, ACCUM, NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, ACCUM, NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, ACCUM, CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, ACCUM, CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( SINGLE, PROJECTED, ACCUM, CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    INIT,  NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    INIT,  NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    INIT,  NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    INIT,  CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    INIT,  CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    INIT,  CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    ACCUM, NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    ACCUM, NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    ACCUM, NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    ACCUM, CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    ACCUM, CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, AMBIENT,    ACCUM, CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   INIT,  NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   INIT,  NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   INIT,  NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   INIT,  CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   INIT,  CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   INIT,  CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   ACCUM, NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   ACCUM, NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   ACCUM, NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   ACCUM, CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   ACCUM, CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, DIRECTED,   ACCUM, CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,


    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, INIT,  NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, INIT,  NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, INIT,  NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, INIT,  CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, INIT,  CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, INIT,  CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, ACCUM, NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, ACCUM, NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, ACCUM, NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, ACCUM, CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, ACCUM, CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, POSITIONAL, ACCUM, CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,


    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, INIT,  NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, INIT,  NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, INIT,  NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, INIT,  CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, INIT,  CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, INIT,  CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, ACCUM, NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, ACCUM, NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, ACCUM, NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, ACCUM, CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, ACCUM, CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, ATTENUATED, ACCUM, CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, INIT,  NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, INIT,  NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, INIT,  NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, INIT,  CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, INIT,  CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, INIT,  CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, ACCUM, NOCLAMP, DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, ACCUM, NOCLAMP, SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, ACCUM, NOCLAMP, DIFFSPEC ),
    NULL_FUNCTION_NAME,

    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, ACCUM, CLAMP,   DIFFUSE  ),
    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, ACCUM, CLAMP,   SPEC     ),
    MAKE_FUNCTION_NAME( DOUBLE, PROJECTED, ACCUM, CLAMP,   DIFFSPEC ),
    NULL_FUNCTION_NAME

};

#if macintosh
#pragma require_prototypes reset
#pragma warn_unusedarg reset
#endif

