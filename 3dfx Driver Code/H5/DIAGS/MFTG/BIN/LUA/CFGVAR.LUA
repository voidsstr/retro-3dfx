
--
-- these are the configuration variables for modules
-- 


cfgvars = {
	main = {
	   test = "foo"
	},

	memerr = {
	   full_report = nil,         -- extended memory report by default?
	   alternate_fill_report = 1  -- set = nil to disable
				      -- output bit errors for alternate fill memory test
	},
	rom = {
	   dump = nil          -- should we dump the rom contents to the log?
	},
	glide = {
 	   image_check = 1     -- should we count pixel errors in glide tests?
        }
};

