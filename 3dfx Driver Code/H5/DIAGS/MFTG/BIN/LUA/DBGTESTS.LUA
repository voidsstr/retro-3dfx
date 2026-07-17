
-- this is a test of lua's hexidecimal

local test = format("0x%04X",300);

if (test ~= "0x012C") then
  error("hex test failed");
end;








