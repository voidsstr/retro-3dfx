$debug
-- uniquef.lua
--
-- this creates a unique filename in the target directory which is 
-- derived from "base" 


localname = "mdcsnap.zip";

base = format("export\\devel\\%s\\mdcsn",date("%m%d%y"));
suffix = ".zip";


execute(format("mkdir export\\devel\\%s",date("%m%d%y")));

function unique_filename(name) 
   local test = readfrom(name);

   readfrom(_STDIN);
   return (not test);
end


counter = 0;
  try_name = format("%s%03d%s",base,counter,suffix);

while (not unique_filename(try_name)) do
  counter = counter + 1;
  try_name = format("%s%03d%s",base,counter,suffix);
end

print("name : ".. try_name);

execute(format("erase %s",localname));
execute(format("pkzip -p -R %s lua\\*.*",localname));
execute(format("pkzip %s d:\\devtools\\wat106\\binw\\dos4gw.exe",localname));

execute(format("copy /y %s %s",localname, try_name));

-- rem rm -f mdc_diag.zip
-- rem pkzip -p -R mdc_diag mdc.exe lua\*.*
-- rem pkzip mdc_diag D:\DEVTOOLS\WAT106\BINW/dos4gw.exe
