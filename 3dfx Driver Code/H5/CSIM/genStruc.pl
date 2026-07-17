#!/opt/perl/latest/bin/perl


print "//**********************************************************************\n";
print "//**********************************************************************\n";
print "//\n";
print "//                      Linear Mipmap Size tables\n";
print "//\n";
print "//**********************************************************************\n";
print "//**********************************************************************\n";
&mipMapSizeStruct("_sstMipMapSize_NormalTextures", 8, 1, 1, 1, 0);
&mipMapSizeStruct("_sstMipMapSize_BigTextures", 11, 1, 1, 1, 0);
&mipMapSizeStruct("_sstMipMapSize_NormalCompressedFourBitsPerTexelTextures", 8, 8, 4, .5, 1);
&mipMapSizeStruct("_sstMipMapSize_BigCompressedFourBitsPerTexelTextures", 11, 8, 4, .5, 1);
&mipMapSizeStruct("_sstMipMapSize_NormalCompressedEightBitsPerTexelTextures", 8, 4, 4, 1, 1);
&mipMapSizeStruct("_sstMipMapSize_BigCompressedEightBitsPerTexelTextures", 11, 4, 4, 1, 1);


print "\n";
print "//**********************************************************************\n";
print "//**********************************************************************\n";
print "//\n";
print "//                     Linear Mipmap offset tables\n";
print "//\n";
print "//**********************************************************************\n";
print "//**********************************************************************\n";
&mipMapOffsetStruct("_sstMipMapOffset_NormalTextures", 8, 0, 1, 1, 1, 0);
&mipMapOffsetStruct("_sstMipMapOffset_BigTextures", 11, 3, 1, 1, 1, 0);
&mipMapOffsetStruct("_sstMipMapOffset_NormalCompressedFourBitsPerTexelTextures", 8, 0, 8, 4, .5, 1);
&mipMapOffsetStruct("_sstMipMapOffset_BigCompressedFourBitsPerTexelTextures", 11, 3, 8, 4, .5, 1);
&mipMapOffsetStruct("_sstMipMapOffset_NormalCompressedEightBitsPerTexelTextures", 8, 0, 4, 4, 1, 1);
&mipMapOffsetStruct("_sstMipMapOffset_BigCompressedEightBitsPerTexelTextures", 11, 3, 4, 4, 1, 1);

print "\n";
print "//**********************************************************************\n";
print "//**********************************************************************\n";
print "//\n";
print "//                     Tiled Mipmap offset tables\n";
print "//\n";
print "//**********************************************************************\n";
print "//**********************************************************************\n";
&mipMapOffsetTiledStruct("_sstMipMapOffset_NormalTextures_Tiled", 
			 1, 1, 0, "r", "d", "r", "r", "d", "d", "d", "d");
&mipMapOffsetTiledStruct("_sstMipMapOffset_BigTextures_Tiled", 
			 1, 1, 3, "r", "d", "r", "r", "d", "r", "r", "d", "d", "d", "d");

print "\n";
print "//**********************************************************************\n";
print "//**********************************************************************\n";
print "//\n";
print "//           Tiled Mipmap offset tables for 4 bit Compressed Textures\n";
print "//\n";
print "//**********************************************************************\n";
print "//**********************************************************************\n";
&mipMapOffsetTiledStruct("_sstMipMapOffset_NormalCompressedFourBitsPerTexelTextures_Tiled", 
			 8, 4, 0, "r", "d", "r", "r", "d", "d", "d", "d");
&mipMapOffsetTiledStruct("_sstMipMapOffset_BigCompressedFourBitsPerTexelTextures_Tiled", 
			 8, 4, 3, "r", "d", "r", "r", "d", "r", "r", "d", "d", "d", "d");

print "\n";
print "//**********************************************************************\n";
print "//**********************************************************************\n";
print "//\n";
print "//           Tiled Mipmap offset tables for 8 bit Compressed Textures\n";
print "//\n";
print "//**********************************************************************\n";
print "//**********************************************************************\n";
&mipMapOffsetTiledStruct("_sstMipMapOffset_NormalCompressedEightBitsPerTexelTextures_Tiled", 
			 4, 4, 0, "r", "d", "r", "r", "d", "d", "d", "d");
&mipMapOffsetTiledStruct("_sstMipMapOffset_BigCompressedEightBitsPerTexelTextures_Tiled", 
			 4, 4, 3, "r", "d", "r", "r", "d", "r", "r", "d", "d", "d", "d");



sub mipMapOffsetTiledStruct
{
    my($name, $minWidth, $minHeight, $startLod, @directions) = @_;

    printf("tiledStruct %s[2][4][16] = {\n", $name);

    $log2Size = @directions;
    $size = 1 << $log2Size;

    $wideTexture = 0;
    
    while($wideTexture < 2)
    {
	if($wideTexture)
	{
	    print "  {   // short and wide (s_is_wider==1)\n";
	}
	else
	{
	    print "  {   // tall and thin (s_is_wider==0)\n";
	}    

	$aspectRatio = 1;
	while($aspectRatio <= 8)
	{

	    if($wideTexture)
	    {
		print "    {   // $aspectRatio:1 aspect ratio\n";
	    }
	    else
	    {
		print "    {   // 1:$aspectRatio aspect ratio\n";
	    }    

	    $i = 0;
	    while($i <= $log2Size)
	    {
		if($i > 0)
		{
		    if($directions[$i-1] eq "r")
		    {
			$bboxMinX[$i] = $bboxMaxX[$i-1];
			$bboxMinY[$i] = $bboxMinY[$i-1];
		    }
		    elsif($directions[$i-1] eq "d")
		    {
			$bboxMinX[$i] = $bboxMinX[$i-1];
			$bboxMinY[$i] = $bboxMaxY[$i-1];
		    }
		    else
		    {
			die("shit");
		    }
		    
		    $height[$i] = $height[$i-1] / 2;
		    if($height[$i] < 1)
		    {
			$height[$i] = 1;
		    }

		    $width[$i] = $width[$i-1] / 2;	    
		    if($width[$i] < 1)
		    {
			$width[$i] = 1;
		    }

		}
		else
		{
		    $bboxMinX[$i] = 0;
		    $bboxMinY[$i] = 0;
		    
		    if($wideTexture)
		    {
			$width[0] = $size;
			$height[0] = $size / $aspectRatio;
		    }
		    else
		    {
			$width[0] = $size / $aspectRatio; 
			$height[0] = $size; 
		    }
		}
		
		$dw = $width[$i];
		if($dw < $minWidth)
		{
		    $dw = $minWidth;
		}
		$dh = $height[$i];
		if($dh < $minHeight)
		{
		    $dh = $minHeight;
		}

		$bboxMaxX[$i] = $bboxMinX[$i] + $dw;
		$bboxMaxY[$i] = $bboxMinY[$i] + $dh;

		$i=$i+1;
	    }

	    $i = $log2Size;
	    while($i >= 0)
	    {
		if($i == $log2Size)
		{
		    $totalBBoxMinX[$i] = $bboxMinX[$i];
		    $totalBBoxMinY[$i] = $bboxMinY[$i];
		    $totalBBoxMaxX[$i] = $bboxMaxX[$i];
		    $totalBBoxMaxY[$i] = $bboxMaxY[$i];
		}
		else
		{
		    $totalBBoxMinX[$i] = $totalBBoxMinX[$i+1];
		    $totalBBoxMinY[$i] = $totalBBoxMinY[$i+1];
		    $totalBBoxMaxX[$i] = $totalBBoxMaxX[$i+1];
		    $totalBBoxMaxY[$i] = $totalBBoxMaxY[$i+1];

		    if($bboxMinX[$i] < $totalBBoxMinX[$i])
		    {
			$totalBBoxMinX[$i] = $bboxMinX[$i];
		    }

		    if($bboxMinY[$i] < $totalBBoxMinY[$i])
		    {
			$totalBBoxMinY[$i] = $bboxMinY[$i];
		    }

		    if($bboxMaxX[$i] > $totalBBoxMaxX[$i])
		    {
			$totalBBoxMaxX[$i] = $bboxMaxX[$i];
		    }

		    if($bboxMaxY[$i] > $totalBBoxMaxY[$i])
		    {
			$totalBBoxMaxY[$i] = $bboxMaxY[$i];	    
		    }
		}

		$totalWidth[$i] = $totalBBoxMaxX[$i] - $totalBBoxMinX[$i];
		$totalHeight[$i] = $totalBBoxMaxY[$i] - $totalBBoxMinY[$i];

		$i=$i-1;
	    }

	    $xOffset = $bboxMinX[$startLod];
	    $yOffset = $bboxMinY[$startLod];

	    $i = 0;
	    while($i <= $log2Size)
	    {
		printf("      {%4d, %4d, %4d, %4d},   // %2d: %4dx%d\n", 
		       $bboxMinX[$i] - $xOffset, $bboxMinY[$i] - $yOffset, 
		       $totalWidth[$i], $totalHeight[$i],
		       $i, $width[$i], $height[$i]);
		$i=$i+1;
	    }

	    if($aspectRatio < 8)
	    {
		print"    },\n";
	    }
	    else
	    {
		print"    }\n";
	    }
	    
	    $aspectRatio = $aspectRatio * 2;
	}
	
	if($wideTexture)
	{
	    print"  }\n";
	}
	else
	{
	    print"  },\n";
	}

	$wideTexture = $wideTexture + 1;
    }
    print "};\n\n";
}



sub mipMapOffsetStruct
{
    my($name, $maxLOD, $startLOD, $minU, $minV,  $scalar, $asymetric) = @_;
    
    #Non split 
    &mipMapOffset($name, $maxLOD, $startLOD, $minU, $minV, $scalar, $asymetric, 0);
    
    #Split
    $name = $name . "_Tsplit";
    &mipMapOffset($name, $maxLOD, $startLOD, $minU, $minV, $scalar, $asymetric, 1);
}



sub mipMapOffset
{
    my($name, $maxLOD, $startLOD, $minU, $minV, $scalar, $asymetric, $isSplit) = @_;

    $logAspectRatio = 0;

    if($asymetric)
    {
	printf("FxI32 %s[2][4][16] = \n{\n", $name);
	$wideTexture = 0;
    }
    else
    {
	printf("FxI32 %s[4][16] = \n{\n", $name);
	$wideTexture = 1;
    }

    while($wideTexture <= 1)
    {
	if($asymetric)
	{
	    if($wideTexture)
	    {
		printf(" {  // wideTexture\n");
	    }
	    else
	    {
		printf(" {  // tallTexture\n");
	    }
	}

	
	#Do the non-split structure first
	$logAspectRatio = 0;
	while($logAspectRatio < 4)
	{
	    if($wideTexture)
	    {
		printf("  {  // %d:1 aspect ratio\n", (1 << $logAspectRatio));
	    }
	    else
	    {
		printf("  {  // 1:%d aspect ratio\n", (1 << $logAspectRatio));
	    }
	    
	    if($isSplit)
	    {
		$offset[$startLOD] = 0;
		$offset[$startLOD + 1] = 0;
		
		$lod = $startLOD + 2;
	    }
	    else
	    {
		$offset[$startLOD] = 0;

		$lod = $startLOD + 1;
	    }

	    while($lod <= $maxLOD)
	    {
		if($isSplit)
		{
		    $offset[$lod] = $offset[$lod - 2] + &mipMapSize($lod-2, $maxLOD, $logAspectRatio, $wideTexture, 
								    $minU, $minV, $scalar);
		}
		else
		{
		    $offset[$lod] = $offset[$lod - 1] + &mipMapSize($lod-1, $maxLOD, $logAspectRatio, $wideTexture,
								    $minU, $minV, $scalar);
		}
		
		$lod = $lod + 1;
	    }
	    
	    #Do the LOD's below the start LOD
	    $lod = $startLOD - 1;
	    while($lod >= 0)
	    {
		if($isSplit)
		{
		    $offset[$lod] = $offset[$lod+2] - &mipMapSize($lod, $maxLOD, $logAspectRatio, $wideTexture,
								  $minU, $minV, $scalar);
		}
		else
		{
		    $offset[$lod] = $offset[$lod+1] - &mipMapSize($lod, $maxLOD, $logAspectRatio, $wideTexture,
								  $minU, $minV, $scalar);
		}

		$lod = $lod - 1;
	    }

	    #print the results
	    $lod = 0;
	    while($lod <= $maxLOD)
	    {
		$width = 1 << ($maxLOD - $lod);
		$height = $width / (1 << $logAspectRatio);
		if($height < 1)
		{
		    $height = 1;
		}

		if(!($wideTexture))
		{
		    #Need to swap width and height
		    $temp = $width;
		    $width = $height;
		    $height = $temp;
		}
		
		$actualU = $width;
		$actualV = $height;
		
		if($actualU < $minU)
		{
		    $actualU = $minU;
		}
		if($actualV < $minV)
		{
		    $actualV = $minV;
		}

		printf("    0x%08x,    // (%11d) LOD %-2d : %4dx%-4d  =>  (%4dx%-4d)\n", $offset[$lod], $offset[$lod], $lod,
		       $width, $height, $actualU, $actualV);

		$lod = $lod + 1;
	    }

	    printf("  },\n");

	    
	    $logAspectRatio = $logAspectRatio + 1;
	}

	if($asymetric)
	{
	    if($wideTexture)
	    {
		printf(" }\n");
	    }
	    else
	    {
		printf(" },\n");
	    }
	}

	$wideTexture = $wideTexture + 1;
    }
    print "};\n\n";
}


sub mipMapSizeStruct
{
    my($name, $maxLOD, $minU, $minV, $scalar, $asymetric) = @_;

    if($asymetric)
    {
	printf("FxI32 %s[2][4][16] = \n{\n", $name);
	$wideTexture = 0;
    }
    else
    {
	printf("FxI32 %s[4][16] = \n{\n", $name);
	$wideTexture = 1;
    }
    

    while($wideTexture <= 1)
    {
	if($asymetric)
	{
	    if($wideTexture)
	    {
		printf(" {  // wideTexture\n");
	    }
	    else
	    {
		printf(" {  // tallTexture\n");
	    }
	}

	$logAspectRatio = 0;
	
	while($logAspectRatio < 4)
	{
	    if($wideTexture)
	    {		
		printf("  {  // %d:1 aspect ratio\n", (1 << $logAspectRatio));
	    }
	    else
	    {
		printf("  {  // 1:%d aspect ratio\n", (1 << $logAspectRatio));
	    }

	    $lod = 0;
	    while($lod <= $maxLOD)
	    {
		$width = 1 << ($maxLOD - $lod);

		$logHeight = ($maxLOD - $lod - $logAspectRatio);
		if($logHeight < 0)
		{
		    $logHeight = 0;
		}
		$height = 1 << $logHeight;

		if(!($wideTexture))
		{
		    #Need to swap width and height
		    $temp = $width;
		    $width = $height;
		    $height = $temp;
		}

		$actualU = $width;
		$actualV = $height;

		if($actualU < $minU)
		{
		    $actualU = $minU;
		}
		if($actualV < $minV)
		{
		    $actualV = $minV;
		}
		$size = $actualU * $actualV * $scalar;

		printf("    0x%08x,    // (%11d) LOD %-2d : %4dx%-4d  =>  (%4dx%-4d) \n", $size, $size, $lod,
		       $width, $height, $actualU, $actualV);

		$lod = $lod + 1;
	    }
	    
	    printf("  },\n");
	    $logAspectRatio = $logAspectRatio + 1;
	}

	if($asymetric)
	{
	    if($wideTexture)
	    {
		printf(" }\n");
	    }
	    else
	    {
		printf(" },\n");
	    }
	}

	$wideTexture = $wideTexture + 1;
    }
    print "};\n\n";
}


sub mipMapSize
{
    my($lod, $maxLOD, $logAspectRatio, $wideTexture, $minU, $minV, $scalar) = @_;
    my($width, $height);

    if($wideTexture)
    {
	$width = 1 << ($maxLOD - $lod);
	$height = $width / (1 << $logAspectRatio);
    }
    else
    {
	$height = 1 << ($maxLOD - $lod);
	$width = $height / (1 << $logAspectRatio);
    }

    if($width < $minU)
    {
	$width = $minU;
    }
    if($height < $minV)
    {
	$height = $minV;
    }

    $size = $width * $height * $scalar;
        
    if($size < $minSize)
    {
	$size = $minSize;
    }
    
    return($size);
}
