
-- something like the perl package requires, it will only do it once per session

function require(package)
   local filename;

   if (type(package) ~= "string") then
        error("require: passed ".. type(package) .." argument, must be string");
   else
        filename = "lua\\" .. package .. ".lua";
	if (_requirelist == nil) then
	     _requirelist = {};  -- create the require list
        end
        if (_requirelist[filename] == nil) then
	        _requirelist[filename] = "loaded";
	        dofile(filename);
        end;
   end
end;


-- BOOL findArg(argstr)
--
-- something to parse the args list. 
-- returns the arg number if it finds it, or nil if it dosn't.

function findArg(argstr) 
    local i = 0;
    local curarg;

    if (_args) then
        curarg = _args[i];
	while (curarg) do
           if (("-".. argstr) == curarg) then
              return i;
           elseif (("/".. argstr) == curarg) then
              return i;
           end
           i = i + 1;
           curarg = _args[i];
        end;
    end;

    return nil;
end



function num_lines_in_string(a_string) 
        local pos;

        pos = strfind(a_string,"\n",1,1); -- start at beginning, no regex
        if (pos) then
                return 2;
        end

        return 1;
end

-- here is a nice printGlobalVariables();

function printGlobalVariables(new_width)
	local i;
        local scrwidth = 80; -- screen width
        local maxwidth = 0;
        local formatstring;

        if (type(new_width) == "number") then
            scrwidth = new_width;
        end

        -- first found out the max function name width

        i = nextvar(nil);
	while (i) do
	  if (type(getglobal(i)) == "function") then
		maxwidth = max(maxwidth,strlen(tostring(i)));
          end
           i = nextvar(i);
        end

        maxwidth = maxwidth + 2;

        local numcols = floor(scrwidth / maxwidth);
        local curcol = 0;

	formatstring = format("%%%ds  ",maxwidth);

        -- next column print all the function names

        i = nextvar(nil);
	while (i) do
	  if (type(getglobal(i)) == "function") then
		write(format(formatstring,tostring(i)));
                curcol = curcol + 1;
                if (curcol >= numcols) then
                   curcol = 0;
                   write("\n");
                end
          end
           i = nextvar(i);
        end

        

        write("\n");

	-- next, print out the variables:

	i = nextvar(nil);
 	while (i) do
 	  if (type(getglobal(i)) ~= "function") then
                 if (type(getglobal(i)) == "string") then
                   local string_rep;

                   if (num_lines_in_string(getglobal(i)) <= 1) then
              	        string_rep = getglobal(i);
                
                        write(format("%25s = '%25s' %10s\n", tostring(i), 
		           tostring(string_rep), 
			   type(getglobal(i))));
                   else
                        string_rep = format("<multi-line string, %d chars>", strlen(getglobal(i)));
                        write(format("%25s =  %25s  %10s\n", tostring(i),
                           tostring(string_rep),
                           type(getglobal(i))));
                   end


                 elseif (type(getglobal(i)) == "number") then
              	   write(format("%25s = %25s %10s\n", tostring(i), 
			tostring(getglobal(i)), 
			type(getglobal(i))));
                 else -- who knows what type it is, just print it...
                   write(format("%25s = '%25s' %10s\n", tostring(i),
			tostring(getglobal(i)), 
			type(getglobal(i))));
                 end;
          end
          i = nextvar(i);
        end

end

-- load configuration variables

dofile("lua\\cfgvar.lua");


-- print warning banner about this being an alpha version!!!

print("*******************************************************************");
print("***                                                             ***");
print("***               3dfx Board Manufacturing Diagnostic           ***");
print("***                                                             ***");
print("*******************************************************************");



