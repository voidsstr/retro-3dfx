-- util.lua
--

--------------------------------------
-- error code table:

-- don't make any error codes which are 0 or 1, they are reserved for:
-- 0 = success
-- 1 = user terminated by break/ctrl-c

test_bitfields = {
   pci = { 
        bus = 2,
        cfg = 3
   }, 

   agp = { 
        agp2x = 25
   }, 

   fbi = {
	memsize = 4,
	memtest = 5,
	constFill = 32,
	rndmFill = 33,	
	altFill = 34,
	slave_memory = 36
   },
   rom = {
        bios = 6,
        vpd_chk = 7,
	serial_rom = 30
   },

   vga = {
	ext_mode = 8,
	mode = 9,
	core = 31
   },

   b2d = {
	hwcursor_ = 10,
	flat_rect = 11,
	hw_overlay_bilinear = 12,
	desktopstart_reg = 24,
	stos = 35
   },

   ["3d"] = {
	tri_test = 13,
	point_test = 14,
	line_test = 15, 
	shaded_tri_test = 16,
	tex_tri_test = 17, 
	splash_frm = 18,
        unreal_test = 19,
        unreal_frm = 20
   },
   video = {
	dac_check = 21,
	color = 22,
	ntsc = 26,
	pal = 27,
	lcd = 28,
	crt = 29
   },
   ddc = {
        data = 23
   }
};



function sleep(seconds)
   local cur_time = time();
   local end_time = cur_time + seconds + 1;

   while (cur_time < end_time) do
      cur_time = time();
   end;
end;


function printTable(a_table)
	local i,v = next(a_table,nil)
	local count = 0;

	print(a_table);
	while i do
		write("  ["..type(i).."]: "..i.." = ["..type(v).."] ".. tostring(a_table[i]));
		if (type(a_table[i]) == "number") then
			write(format(" (0x%X)",a_table[i]));
		end;
		print("");
		i,v = next(a_table,i)
		count = count + 1;
	end
	if (count == 1) then
		print("1 entry.");
	else 
		print(count .. " entries.");
	end
end

function print_pci_info()
	local MAX_PCI_DEVICES = 20;
	local pci_dev_number = 0;

	while (pci_dev_number < MAX_PCI_DEVICES) do
		pci_info = pci_dev_info(pci_dev_number);

		if (pci_info ~= nil) then
			printTable(pci_info);
		end;

		pci_dev_number = pci_dev_number + 1;
	end;
end;

function test_vga_modes() 
    local vga_modes = { 0, 3, 5, 6, 13, 18, 19 };
    local i,v = next(vga_modes,nil)

    while i do
	if (type(vga_modes[i]) == "number") then
		set_vga_mode(vga_modes[i]);
		print("Mode ".. tostring(vga_modes[i]));
	else
		print("Invalid vga mode ".. tostring(vga_modes[i]) .. " in list.");
	end
      i,v = next(vga_modes,i);
    end
end


function nil_is_zero(value) 
	if (value == nil) then
	     return 0;
	else
	     return value;
	end
end

-- find_bitfield()
--
-- we want to look for the bitfield in the test_bitfields[][] table, but
-- we don't know the exact name, we need to substring match

function find_bitfield(module,testname) 
    local subtbl = test_bitfields[module];
    local length = strlen(testname);
    local bit;

    if (not subtbl) then
       return 30; -- missing name!! this shouldn't happen!
    end

    while ((length > 0) and (not subtbl[strsub(testname,1,length)])) do
       length = length - 1;
    end

    bit = subtbl[strsub(testname,1,length)];

    if (bit) then
       return bit;
    else 
      if (length == 0) then
       return 31; -- wow, that's even weirder
      else
       return 32;
      end
    end
end

-- setupErrorExit
--
-- This parses the pass/fail summary table and stores error exit code.
--
function setupErrorExit(table) 
   local i,v;
   local ti,tv;
   local module;
   local print_count = 0;
   local error_codes = {};

   if (type(table) ~= "table") then   -- abort if non table
      return;
   end

   i,v = next(table,nil);
   
   while (i) do  -- first go through the module tables
      module = i; -- record the module name

      if (type(v) ~= "table") then -- abort if non-table
	 return;
      end

      ti,tv = next(v,nil);


      while (ti) do -- second go through the test tables
	 if (type(tv) ~= "table") then  -- abort if non-table
	    return;
	 end

	 if (tv["fail_count"]) then
	     print_count = print_count + 1;
             local this_error_code = find_bitfield(i,ti);
             add_element(error_codes,this_error_code);

             if (first_error == nil) then
                first_error = this_error_code;
             end;

	 end

	 ti,tv = next(v,ti);
      end

      i,v = next(table,i);
   end

   if (print_count == 0) then
	 print("-- no failures, see STDOUT.LOG for full summary");

   else 
      local i = 0;

      write("Error Summary Codes: ");

      while (error_codes[i] ~= nil) do
        write(format("%u ",error_codes[i]));
        i = i + 1;
      end
   end;

end


-- printTableSummary
--
-- This prints a pass/fail summary in a column format. If show_all is
-- true it prints both pass and fail rows, else it just prints failure rows.
--
  function add_element(tbl, element) 
     local i = 0;

     while (tbl[i] ~= nil) do
         if (tbl[i] == element) then
            return;
         end;
         i = i + 1;
     end

     tbl[i] = element;
  end


function printTableSummary(table,show_all) 
   local i,v;
   local ti,tv;
   local module;
   local print_count = 0;
   local error_codes = {};

   print(format("%-9s %-30s %10s %10s","Module","Test","Pass","Fail"));

   if (type(table) ~= "table") then   -- abort if non table
      return;
   end

   i,v = next(table,nil);
   
   while (i) do  -- first go through the module tables
      module = i; -- record the module name

      if (type(v) ~= "table") then -- abort if non-table
	 return;
      end

      ti,tv = next(v,nil);


      while (ti) do -- second go through the test tables
	 if (type(tv) ~= "table") then  -- abort if non-table
	    return;
	 end

	 if ((show_all ~= nil) or (tv["fail_count"] ~= nil)) then
	    print_count = print_count + 1;
	    print(format("%-9s %-30s %10s %10s",module,ti,
		"pass:"..nil_is_zero(tv["pass_count"]),
		"fail:"..nil_is_zero(tv["fail_count"])));
	 end

	 if (tv["fail_count"]) then
	     -- print(format("lookup: %s/%s = %d",i,ti,find_bitfield(i,ti)));
             local this_error_code = find_bitfield(i,ti);
             add_element(error_codes,this_error_code);

             if (first_error == nil) then
                first_error = this_error_code;
             end;

             -- this is the old bitwise error code method!
             --  error_code = bitor(error_code,(2^(find_bitfield(i,ti))));
	 end

	 ti,tv = next(v,ti);
      end

      i,v = next(table,i);
   end

   if (print_count == 0) then
      if (show_all) then
	 print("-- no results, probably a bad table, contents:");
	 printTableLua(table);
	 print("");
      else
	 print("-- no failures, see STDOUT.LOG for full summary");
      end

   else 
      local i = 0;

      write("Error Summary Codes: ");

      while (error_codes[i] ~= nil) do
        write(format("%u ",error_codes[i]));
        i = i + 1;
      end
   end;

end

-- printTableLua()
-- 
-- This prints a table formatted to be valid 'lua' input syntax.
--

function printTableLua(table,cur_indent) 
	local i,v = next(table,nil);
	local old_v;
	local indent;

	if (cur_indent == nil) then
	     indent = "  ";
	else 
	     indent = cur_indent;
	end

	write("{\n");

	while (i) do
	  if ((type(v) ~= "function") and (type(v) ~= "cfunction")) then
	    write(indent .. tostring(i) .. " = ");
	    if (type(v) == "string") then
	       write("\"".. v .. "\"");
	    elseif (type(v) == "number") then
--               if (v == strtoul(format("0x%X",v),nil,16)) then
		    write("X(\"" .. format("0x%X",v) .. "\")");
--               else 
--                    write(tostring(v));
--               end
	    elseif (type(v) == "table") then
	       printTableLua(v,indent .. "  ");
	    else
	       write(tostring(v)); -- who knows what type it is!
	    end

	    old_v = v;
	    i,v = next(table,i);

	    if (i and (type(v) ~= "function") and (type(v) ~= "cfunction")) then
	       write(",");
	    end

	    if ((type(old_v) == "number")) then
	       write(format(" -- 0x%X, %d (%f)",old_v,old_v,old_v));
	    end
	    write("\n");
	 else
	    i,v = next(table,i);
	 end
	end

	write(indent.."}");     
end

function persistTableToFile(table_name, table,file_name)
   local  result, errstr = writeto(file_name);

   if (result ~= nil) then
       write(table_name .. " = ");
       printTableLua(table);
       write(";\n");
   else
       print("Could not persist table, file open failed");
       print(errstr);
   end
   writeto(); -- restore stdout
end

function option_menu() 
    local key = ascii("B");
    local timeout = 10; -- ten second timeout
    local now;
    local start_time;
    local last_print = 0;

    write(format("Default = %c, seconds remaining: --",key));

    now = time();
    start_time = now;
    while ((now - start_time) < timeout) do
      if (last_print ~= now) then
	write(format("%c%c%02d",8,8,time() - now));
	last_print = now;
      end
      now = time();
    end

    print("time elapsed!");
    
end


-- convert a number to from hex
function X(num_string)
   return (strtoul(num_string,nil,16)); 
end

function get_yes_no() 
	while (1) do
	     key = getch();

	     if ((key == ascii("y")) or (key == ascii("Y"))) then
		 return 1; -- pass
	     elseif ((key == ascii("n")) or 
		     (key == ascii("N"))) then
		 return nil; -- fail
	     end
			 
	end
end

function sectionBanner(level_num, banner_name) 
    local i;
    local length = strlen(banner_name);
    local edgelen = 4;
    local edgepad = 8;


    local repeat_char = function(char, count) 
	 local i = 0;
	 while (i < count) do
	     write(char);
	     i = i + 1;
	 end
    end

    write("\n");

    if (level_num == 1) then
     -- top line
     repeat_char("*",length + ((edgelen + edgepad) * 2) );
     write("\n");

     -- middle line
     repeat_char("*",edgelen);
     repeat_char(" ",edgepad);
     write(banner_name); 
     repeat_char(" ",edgepad);
     repeat_char("*",edgelen);
     write("\n");


     -- bottom line
     repeat_char("*",length + ((edgelen + edgepad) * 2) );
     write("\n");

    else

    print(format("**** %s ****",banner_name));
    end;
 
    write("\n");
      
end

function make_a_dir(dir)
   -- create a directory
   system(format("if not exist %s\\nul mkdir %s",dir,dir)); 
end

function make_path(path_str)
   local cur_pos = 1;
   local keep_going = 1;
   local next_separator;

   if (strsub(path_str,2,2) == ":") then
     -- we have a dos "C:" style identifier...
     cur_pos = 4;
   end

   repeat
     next_separator = strfind(path_str, "\\" , cur_pos, 1);
     if (next_separator) then
       make_a_dir(strsub(path_str,1,next_separator - 1));
       cur_pos = next_separator + 1;
     else
       -- we are going to assume that this is actuall a filename path!
       -- make_a_dir("mkdir ".. path_str);
       keep_going = nil
     end;
   until (not keep_going);
end


    
 
   
     


