--
-- glide tests
--

gt = {}; -- new package

function gt.fb_check(test_name,short_name)
   local lchksum;
   local status;

   grSstIdle();

   lchksum = HWbansheeCrc();
   --print(format("checksum '%s' = %s",test_name,lchksum));
   status = chksum.checkChecksum(test_name,lchksum);
      
   if (getCfgVar("glide", "image_check")) then
      -- only check the raw images if they have "glide.image_check" set.
      -- in lua\cfgvar.lua
    if (test_name == "unreal_frm0") then
      if (getCfgVar("chksum","generate")) then
        system("if not exist gld\\nul mkdir gld"); -- create a directory
        checksumGlideFb(format("gld\\%s.sbi",short_name),1);
      else   -- "chksum" "generate"
       if (unreal_count <= 3) then
         if (status) then	-- CRC check passed
            good_crc_count = good_crc_count + 1;
	    if ((good_crc_count == 2) or (good_pixel_count >= 2)) then
                end_unreal_frm0 = 1;               -- signal end of test
	    end
            global_status = 1; -- make sure we continue testing
            status = 1; -- passed
	 else     -- status   
            checksum_status, pixel_error = checksumGlideFb(format("gld\\%s.sbi",short_name),0);
            if (pixel_error == 0) then
               good_pixel_count = good_pixel_count + 1;		-- signal no pixel errors
               global_status = 1; -- make sure we continue testing
               status = 1; -- passed
	    else     -- exit test and return with error any time a pixel error occurs
               print(format("checksum '%s' failed (ex=%s,rec=%s)",
                        test_name,render(chksum.golden[test_name]),render(lchksum)));
               global_status = nil; -- make sure we stop testing and signal failure
               status = nil; -- failure!
               pixel_error_end = 1;                -- signal end of test
            end  -- pixel_error
         end    -- status
        end    -- unreal_count
      end     -- "chksum" "generate"
    end	  -- test_name

    if (test_name == "unreal_frm1") then
      if (getCfgVar("chksum","generate")) then
        system("if not exist gld\\nul mkdir gld"); -- create a directory
        checksumGlideFb(format("gld\\%s.sbi",short_name),1);
      else   -- "chksum" "generate"
       if (unreal_count <= 3) then
         if (status) then	-- CRC check passed
            good_crc_count_1 = good_crc_count_1 + 1;
            if ((good_crc_count_1 == 2) or (good_pixel_count_1 >= 2)) then
                end_unreal_frm1 = 1;               -- signal end of test
	    end
            global_status = 1; -- make sure we stop testing and signal failure
            status = 1; -- passed
	 else     -- status   
            checksum_status, pixel_error = checksumGlideFb(format("gld\\%s.sbi",short_name),0);
            if (pixel_error == 0) then
               good_pixel_count_1 = good_pixel_count_1 + 1;         -- signal no pixel errors
               global_status = 1; -- make sure we continue testing
               status = 1; -- passed
	    else     -- exit test and return with error any time a pixel error occurs
               print(format("checksum '%s' failed (ex=%s,rec=%s)",
                        test_name,render(chksum.golden[test_name]),render(lchksum)));
               global_status = nil; -- make sure we stop testing and signal failure
               status = nil; -- failure!
               pixel_error_end = 1;                -- signal end of test
            end  -- pixel_error
         end    -- status
        end    -- unreal_count
      end     -- "chksum" "generate"
    end	  -- test_name
   end;  -- image_check

   if ((test_name == "unreal_frm0") or (test_name == "unreal_frm1")) then
     if (unreal_count == 3)  then        -- only record results after last test
      recordTestResult("3d",test_name,status);
     end
     if ((unreal_count == 2) and (good_crc_count == 2) and (good_crc_count_1 == 2)) then
      recordTestResult("3d",test_name,status);
     end
     if (pixel_error_end == 1) then
      recordTestResult("3d",test_name,status);
     end
   else
      recordTestResult("3d",test_name,status);
   end
   return (status);
end

function gt.point_test()
   local lchksum;
   local ScreenX, ScreenY;
   local num_points = 40;

   grBufferClear();

   -- we want a solid color
   grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
	GR_COMBINE_FACTOR_NONE,
	GR_COMBINE_LOCAL_CONSTANT,
	GR_COMBINE_OTHER_NONE,
	0); 

   ScreenX = 640;
   ScreenY = 480; -- these should come from somewhere

   local ix = ScreenX/num_points;
   local iy = ScreenY/num_points;
   local i = 0;

   while ((ix * i) < ScreenX) do
       grDrawPoint({ x = (i * ix), y = (i * iy)});
       i = i + 1;
   end

   grBufferSwap(1);

   -- check the framebuffer!
   return (gt.fb_check("point_test","point"));
end

function gt.line_test()
    local ScreenX, ScreenY;
    local num_lines = 40;

    grBufferClear();

    -- solid color
    grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
	GR_COMBINE_FACTOR_NONE,
	GR_COMBINE_LOCAL_CONSTANT,
	GR_COMBINE_OTHER_NONE,
	0); 

    ScreenX = 640; ScreenY = 480; -- get this from somewhere

    local ix = ScreenX/num_lines;
    local iy = ScreenY/num_lines;
    local i = 0;

    while ((iy * (i + 1)) < ScreenY) do
        grDrawLine({ x = (ix * i), y = 0 }, { x = (ScreenX), y = (iy * (i + 1)) });
        i = i + 1;
    end

    grBufferSwap(1);
    -- check the framebuffer
    return (gt.fb_check("line_test","line"));
end

function gt.flat_tri_test()
   local a = { x = 2, y = 2, a = 100, r = 100, g = 100, b = 100 };
   local b = { x = 10, y = 300, a = 170, r = 200, g = 50, b = 50 };
   local c = { x = 300, y = 300, a = 50, r = 50, g = 200, b = 50 };
   
   -- solid color
   grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
	GR_COMBINE_FACTOR_NONE,
	GR_COMBINE_LOCAL_CONSTANT,
	GR_COMBINE_OTHER_NONE,
	0); 
   grBufferClear();  
   grDrawTriangle(a,b,c);

   grBufferSwap(1);
   return (gt.fb_check("tri_test","tri"));
end

function gt.shaded_tri_test()
   local a = { x = 2, y = 2, a = 100, r = 100, g = 100, b = 100 };
   local b = { x = 10, y = 300, a = 170, r = 200, g = 50, b = 50 };
   local c = { x = 300, y = 300, a = 50, r = 50, g = 200, b = 50 };

   -- iterated color
   grColorCombine(GR_COMBINE_FUNCTION_LOCAL,
	GR_COMBINE_FACTOR_NONE,
	GR_COMBINE_LOCAL_ITERATED,
	GR_COMBINE_OTHER_NONE,
	0);
 
   grBufferClear();
   grDrawTriangle(a,b,c);
   grBufferSwap(1); 

   return (gt.fb_check("shaded_tri_test","shtri"));
end

function gt.tex_tri_test()
   if (gt.texture == nil) then
      gt.texture = loadTextureFile("3df\\miro.3df",GR_TMU0);
   end
   
   if (gt.texture) then
      guTexSource(gt.texture);
      local i = 5;
      local tex_status = 1;
        
      local a = { x = 0 , y = 0 , a = 255, r = 255, g = 255, b = 255, oow = 1 ,
                  tmuvtx = { { oow = 1.0, sow = 0 * 1 , tow = 0 * 1 } }
                };
      local b = { x = 600 , y = 0 , a = 255, r = 255, g = 255, b = 255 , oow = 1,
                  tmuvtx = { { oow = 1.0, sow = 255 * 1, tow = 0 * 1 } }
                };
      local c = { x = 0, y = 400, a = 50, r = 255, g = 255, b = 255, oow = 1,
                  tmuvtx = { { oow = 1.0, sow = 0 * 1, tow = 255 * 1 } } 
                };
      local d = { x = 600, y = 400, a = 50, r = 255, g = 255, b = 255, oow = 1,
                  tmuvtx = { { oow = 1.0, sow = 255 * 1, tow = 255 * 1 } } 
                };
      

      while (i < 255) do
       c.tmuvtx[1].tow = i;
       d.tmuvtx[1].tow = i;

       i = i + 5;
       grBufferClear();
       grDrawTriangle(a,b,c);
       grDrawTriangle(b,c,d);
       grBufferSwap(1);
       tex_status = gt.fb_check("tex_tri_test"..i,format("text%X",i))
              and tex_status;
      end;

      return (tex_status);
   else
     return nil; -- failed!
   end
end

function gt.tex_tri_test_b()
   if (gt.texture == nil) then
      gt.texture = loadTextureFile("3df\\miro.3df",GR_TMU0);
   end
   if (gt.texture_2 == nil) then
      gt.texture_2 = loadTextureFile("3df\\matt1.3df",GR_TMU1);
   end
   
   if (gt.texture) then
      guTexSource(gt.texture);
      guTexSource(gt.texture_2);
      local i = 5;
      local tex_status = 1;
           

      local a = { x = 0 , y = 0 , a = 255, r = 255, g = 255, b = 255, oow = 1 ,
                  tmuvtx = { { oow = 1.0, sow = 0 * 1 , tow = 0 * 1 } ,
                             { oow = 1.0, sow = 0 * 1 , tow = 0 * 1 }}
                };
      local b = { x = 600 , y = 0 , a = 255, r = 255, g = 255, b = 255 , oow = 1,
                  tmuvtx = { { oow = 1.0, sow = 255 * 1, tow = 0 * 1 } ,
                             { oow = 1.0, sow = 255 * 1, tow = 0 * 1 } }
                };
      local c = { x = 0, y = 400, a = 50, r = 255, g = 255, b = 255, oow = 1,
                  tmuvtx = { { oow = 1.0, sow = 0 * 1, tow = 255 * 1 },
                             { oow = 1.0, sow = 0 * 1, tow = 255 * 1 }}
                };
      local d = { x = 600, y = 400, a = 50, r = 255, g = 255, b = 255, oow = 1,
                  tmuvtx = { { oow = 1.0, sow = 255 * 1, tow = 255 * 1 } ,
                             { oow = 1.0, sow = 255 * 1, tow = 255 * 1 }}
                };

        -- setup render state, decal texture, bilinear
        grColorCombine( GR_COMBINE_FUNCTION_SCALE_OTHER,
          GR_COMBINE_FACTOR_ONE, GR_COMBINE_LOCAL_NONE,
          GR_COMBINE_OTHER_TEXTURE, FXFALSE);

        grTexFilterMode( GR_TMU0,
          GR_TEXTUREFILTER_BILINEAR,
          GR_TEXTUREFILTER_BILINEAR);
        grTexFilterMode( GR_TMU1,
          GR_TEXTUREFILTER_BILINEAR,
          GR_TEXTUREFILTER_BILINEAR);
  
        grTexMipMapMode( GR_TMU0,
          GR_MIPMAP_NEAREST,
          FXTRUE);
        grTexMipMapMode( GR_TMU1,
          GR_MIPMAP_NEAREST,
          FXTRUE);

        grTexCombine( GR_TMU0,
          GR_COMBINE_FUNCTION_BLEND_OTHER,
          GR_COMBINE_FACTOR_LOCAL,
          GR_COMBINE_FUNCTION_ZERO,
          GR_COMBINE_FACTOR_NONE,
          FXFALSE,
          FXFALSE);

        grTexCombine( GR_TMU1,
          GR_COMBINE_FUNCTION_LOCAL,
          GR_COMBINE_FACTOR_ONE,
          GR_COMBINE_FUNCTION_ZERO,
          GR_COMBINE_FACTOR_NONE,
          FXFALSE,
          FXFALSE);

--      grHints(GR_HINT_STWHINT, 8); -- we want separate TMU0/1 S&T
       grHints(GR_HINT_STWHINT, GR_STWHINT_ST_DIFF_TMU1); -- we want separate TMU0/1 S&T

      while (i < 255) do
       c.tmuvtx[1].tow = i;
       d.tmuvtx[1].tow = i;


       i = i + 5;
       grBufferClear();
       grDrawTriangle(a,b,c);
       grDrawTriangle(b,c,d);
       grBufferSwap(1);
       tex_status = gt.fb_check("tex_tri_test_b"..i,format("texb%X",i))
              and tex_status;
      end;

      return (tex_status);
   else
     return nil; -- failed!
   end
end


function gt.splash_test()
   local ScreenX, ScreenY;
   local frame = 1;
   local max_frame = 75;
   gt.my_status = 1;

   ScreenX = 640;
   ScreenY = 480;

   grSplash(0,0,ScreenX,ScreenY,1,
       function(frm)
         grSstIdle();
         gt.my_status = gt.fb_check("splash_frm".. tostring(frm), "splsh"..tostring(frm)) and gt.my_status;
       end
   );
  
   return (gt.my_status);
end

 gt.testlist = { "point_test", "line_test", "flat_tri_test", "shaded_tri_test", 
                "tex_tri_test", "tex_tri_test_b" };
-- Don't do splash test for now!
--                "tex_tri_test", "tex_tri_test_b", "splash_test" };

function gt.run(sli, aa)
   local i = 1;
   local fn;
   local test_status;
   local overall_status = 1; -- pass

--   if (sli == 1) then
--      grSliCtrl();	-- enable sli
--   end

--   if (aa == 1) then
--      grAASetup(1);	-- enable anti_aliasing
--   end

   startglide();
   while (gt.testlist[i]) do
      fn = gt[gt.testlist[i]];
      if (fn and (type(fn) == "function")) then
        test_status = fn();
        if (test_status == nil) then
           -- signalFailure();
           -- check onFailure();

           overall_status = nil; -- return that one of our tests failed
        end
      else
         overall_status = nil;
         print(format("Missing glide test function '%s'",gt.testlist[i]));
      end
      i = i + 1;
   end

   grGlideShutdown(); -- be nice and shutdown

--   if (sli == 1) then
--      grDisableSliCtrl();	-- disable sli
--   end

--   if (aa == 1) then
--      grAASetup(0);	-- disable anti_aliasing
--   end
  
   return (overall_status);
end;
