
typedef struct {
    char    *name;
    FxU32   materialIndex;
    FxU32   meshIndex;
    FxU32   transformIndex;
    FxU32   animationIndex;
} RenderTarget;

typedef struct {
    FxU32 numRenderTargets;
    FxU32 numTransforms;
    FxU32 numMaterials;
    FxU32 numMeshes;
    FxU32 numAnimations;
    AtrAnimation *animationArray;
    AtrMaterial *materialArray;
    AtrTransform *transformArray;
    AtrMesh *meshArray;
    RenderTarget *renderTargetArray;
} Scene;

void storeScene( Scene *s, FILE *stream ) {
    FxU32 index;
    FxU32 len;
    fwrite( &(s->numRenderTargets), sizeof( s->numRenderTargets ), 1, stream );    
    fwrite( &(s->numTransforms), sizeof( s->numTransforms ), 1, stream );    
    fwrite( &(s->numAnimations), sizeof( s->numAnimations ), 1, stream );    
    fwrite( &(s->numMaterials), sizeof( s->numMaterials ), 1, stream );    
    fwrite( &(s->numMeshes), sizeof( s->numMeshes ), 1, stream );
    for( index = 0;index < s->numRenderTargets;index++ ) {
        len = strlen( s->renderTargetArray[index].name ) + 1;
        fwrite( &len, sizeof( len ), 1, stream );
        fwrite( s->renderTargetArray[index].name, sizeof( char ), len, stream );
        fwrite( &(s->renderTargetArray[index].materialIndex), sizeof( FxU32 ), 1, stream );
        fwrite( &(s->renderTargetArray[index].meshIndex), sizeof( FxU32 ), 1, stream );
        fwrite( &(s->renderTargetArray[index].transformIndex), sizeof( FxU32 ), 1, stream );
        fwrite( &(s->renderTargetArray[index].animationIndex), sizeof( FxU32 ), 1, stream );
    }
    for( index = 0;index < s->numTransforms;index++ ) 
        atrTransformStore( s->transformArray + index, stream );
    for( index = 0;index < s->numAnimations;index++ ) 
        atrAnimationStore( s->animationArray + index, stream );
    for( index = 0;index < s->numMaterials;index++ ) 
        atrMaterialStore( s->materialArray + index, stream );
    for( index = 0;index < s->numMeshes;index++ ) 
        atrMeshStore( s->meshArray + index, stream );

}

Scene *loadScene( FILE *stream ) {
    FxU32 index, len;
    Scene *retval;
    retval = calloc( sizeof( Scene ), 1 );
    fread( &(retval->numRenderTargets), sizeof( retval->numRenderTargets ), 1, stream );    
    fread( &(retval->numTransforms), sizeof( retval->numTransforms ), 1, stream );    
    fread( &(retval->numAnimations), sizeof( retval->numAnimations ), 1, stream );    
    fread( &(retval->numMaterials), sizeof( retval->numMaterials ), 1, stream );    
    fread( &(retval->numMeshes), sizeof( retval->numMeshes ), 1, stream );
    retval->renderTargetArray = calloc( sizeof( RenderTarget ), retval->numRenderTargets );
    for( index = 0;index < retval->numRenderTargets;index++ ) {
        fread( &len, sizeof( len ), 1, stream );
        retval->renderTargetArray[index].name = calloc( sizeof( char ), len );
        fread( retval->renderTargetArray[index].name, sizeof( char ), len, stream );
        fread( &(retval->renderTargetArray[index].materialIndex), sizeof( FxU32 ), 1, stream );
        fread( &(retval->renderTargetArray[index].meshIndex), sizeof( FxU32 ), 1, stream );
        fread( &(retval->renderTargetArray[index].transformIndex), sizeof( FxU32 ), 1, stream );
        fread( &(retval->renderTargetArray[index].animationIndex), sizeof( FxU32 ), 1, stream );
    }
    retval->transformArray = atrTransformAllocate( retval->numTransforms );
    for( index = 0;index < retval->numTransforms;index++ ) 
        atrTransformLoad( retval->transformArray + index, stream );
    retval->animationArray = atrAnimationAllocate( retval->numAnimations );
    for( index = 0;index < retval->numAnimations;index++ ) 
        atrAnimationLoad( retval->animationArray + index, stream );
    retval->materialArray = atrMaterialAllocate( retval->numMaterials );
    for( index = 0;index < retval->numMaterials;index++ ) 
        atrMaterialLoad( retval->materialArray + index, stream );
    retval->meshArray = atrMeshAllocate( retval->numMeshes );
    for( index = 0;index < retval->numMeshes;index++ ) 
        atrMeshLoad( retval->meshArray + index, stream );
    return retval;
}

RenderTarget *findTarget( char *name, RenderTarget *list, FxU32 num_targets ) {
    if ( !num_targets ) return 0;
    for ( num_targets--;num_targets >= 0;num_targets--) {
        if ( !strcmp( list[num_targets].name, name ) ) {
            return list + num_targets;
        }
    }
    return 0;
}

