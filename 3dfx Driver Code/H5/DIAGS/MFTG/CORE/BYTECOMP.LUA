-- bytecomp.lua
--
-- this byte compiles a string of bytes into a C data structure
--

-- write the preamble

write("/*\n"..
      " * <filename>\n" ..
      " *\n" ..
      " * this is an automatically generated binary file, don't mess with it!\n" ..
      " */\n");

write("\n\n\n");


-- write the octals

local data;
local octals_per_line = 9;
local cur_octal = 0;
local octal_count = 0;
local line_indent = "    ";


write("unsigned char bin_data[] = {\n".. line_indent);

data = read(".");
while (data) do
     write(format("0x%02X, ",ascii(data)));
     cur_octal = cur_octal + 1;
     octal_count = octal_count + 1;
     if (cur_octal >= octals_per_line) then
         cur_octal = 0;
         write("\n" .. line_indent);
     end
     data = read(".");
end;

-- write the last octal

write("0x00\n");

-- write the postamble

write("};\n\n");

write(format("unsigned int bin_len = %d;\n",octal_count));
write("\n\n");

