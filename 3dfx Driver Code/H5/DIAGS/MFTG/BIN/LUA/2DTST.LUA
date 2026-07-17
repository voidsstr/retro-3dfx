--
-- 2d tests
--

b2d = {}

function b2d.fb_check(test_name) 
   local lchksum;
   local status;

   lchksum = HWbansheeCrc();
--   print(format("checksum '%s' = %s",test_name,lchksum));
   status = chksum.checkChecksum(test_name,lchksum);

   recordTestResult("b2d",test_name,status);
   return (status);
end

function b2d.flat_rect(iteration)
       local rect = { 
                        dstX = 0, 
                        dstY = 0, 
                        colorFore = 4660, 
		        dstSizeX = 100, 
                        dstSizeY = 100, 
                        blitType = 5  -- flat color rectangle
                    };
       local loop_i;

        rect.colorFore = 31;

       loop_i = iteration;
       while (loop_i < 40) do
          loop_i = loop_i + 1;
          rect.dstX = rect.dstX + 10;
          rect.dstY = rect.dstY + 1;
          rect.colorFore = rect.colorFore - 1;
          blit2d(rect);
       end

       rect.colorFore = 2016;
       rect.dstX = 0;
       rect.dstY = 110;

       loop_i = iteration;
       while (loop_i < 40) do
          loop_i = loop_i + 1;
          rect.dstX = rect.dstX + 10;
--          rect.dstY = rect.dstY + 1;
          rect.colorFore = rect.colorFore - 32;
          blit2d(rect);
       end

       rect.colorFore = 63488;
       rect.dstX = 0;
       rect.dstY = 220;

       loop_i = 0;
       while (loop_i < 40) do
          loop_i = loop_i + 1;
          rect.dstX = rect.dstX + 10;
          rect.dstY = rect.dstY - 1;
          rect.colorFore = rect.colorFore - 2048;
          blit2d(rect);
       end

       return (b2d.fb_check("flat_rect" .. tostring(iteration)));

end

function b2d.hwcursor()
	local x = 100;
        local y = 100;
        local status = 1;

	while (y < 300) do
		while (x < 300) do
			hwCursor(1,x,y);
       			status = b2d.fb_check(format("hwcursor_%d_%d",x,y))
				 and status;
			x = x + 30;
		end
		y = y + 30;
		x = y;
	end

        return (status);

end


-- screen to screen blit
function b2d.stos() 
       local rect = { 
                        dstX = 200, 
                        dstY = 0, 
		        dstSizeX = 100, 
                        dstSizeY = 100, 

                        colorFore = 0,

                        srcX = 0,
                        srcY = 0,
                        srcSizeX = 100,
                        srcSizeY = 100,

                        blitType = 1 -- screen to screen blit
                    };
       local loop_i

--       loop_i = 0;
--       while (loop_i < 40) do
--          loop_i = loop_i + 1;
--          rect.dstY = rect.dstY + 10;
--       end

          blit2d(rect);

       return (b2d.fb_check("stos"));
        
end

function b2d.run()
       local status = 1;
       local loopcount = 15;
       print("2d blit test");
       set_vga_mode(273);

--       while (loopcount > 0) do
  --         status = b2d.flat_rect(loopcount) and status;
    --       loopcount = loopcount - 1;
      -- end

       banBOvl();
       status = b2d.fb_check(format("hw_overlay_bilinear")) and status;

       status = b2d.hwcursor() and status;

       status =  b2d.stos() and status;
       return (status);
--    getch();
end
