Sam,

Here is a drop of my object viewer for 3DSR4.  You will need to setup a
few variables in the makefile.  All of them are at the top.

You need to tell the makefile where your 3ds4\process directory is.

You will need to make a 3ds4\3dfx directory.

You should have dos4gw.exe in your path and you should have a texus.exe
in your path (I tried using texus.lib but all the textures came out slightly
screwed up, looked like texus was going beyond array bounds...).

You can do a "wmake /a" to do a new build or just copy the vw3dfx_i.pxp to 
the 3ds4\process directory.

You can build the plugin in two modes: using TEXUS.LIB and TEXUS.EXE.
Just do a "wmake /a" and both plugins (vw3dfx_i.pxp which uses
TEXUS.LIB and av3dfx_i.pxp which uses TEXUS.EXE) will be built. You
should see both versions of the plugin there now.  Note that the
plugin caches textures (*.3df files) in the 3ds4\3dfx directory.  You
need to delete these if you want to force the plugin to generate new 
textures, otherwise it will use whatever the last plugin execution
produced.

I use 4DOS as my shell, I am not sure if the makefile will work with
the standard shell.  You can get a shareware version of 4DOS from the
net:
      "You can download 4DOS upgrades from CompuServe, the Channel 1 BBS
      (see above), or our Internet ftp site and web page.  To download
      updates from CompuServe, go to the PC Vendor B forum (GO JPSOFT or GO
      PCVENB) and look for the files you want in Library 10.  On Channel 1,
      join conference 5 and then download the file you need (as long as you
      join conference 5 before downloading you will be able to download
      4DOS files even if they appear to be too large for your remaining
      time on the system).  On the Internet, download via ftp from
      ftp.std.com, directory /vendors/jpsoft (for general JP Software
      files) or directory /vendors/jpsoft/4dos (for 4DOS-specific files). 
      You can also download the files from our web site at
      http://www.jpsoft.com/.  See below for information on file names used
      for on-line copies of 4DOS."

Once you have a copy of the plugin in the 3ds4\process directory
launch 3ds.exe and load up a file (like balls.3ds) and then go to the
"Program/PXP Loader" and choose vw3dfx.  This should bring up a dialog
box.  Just click OK (or press return) and you should see a few dialogs
on the progress of texture and mesh conversions and then a rendering
should appear on the monitor hooked up to your 3dfx card.  Play with
the joystick.  

Some things you can do:

* hold button 1 and move the joystick to do translation
  in and out

* hold button 2 to do translation in the projection plane ("up" and
  "down" and "left" and "right")

* move the joystick to rotate

* move the hat (CH Flightstick Pro method) to translate in the
  projection plane, you can do this while rotating

* type 1, 2, 3, or 4 to switch viewports, 0 is the viewport you had
  selected when the plugin was invoked

* type r to reset the view

* type ESC to end

Right now I think the plugin leaves 3ds.exe in a slightly unstable
state after it is exited, so it is best to debug by starting a new 3ds
session, loading a file, starting the plugin and then debugging.  You
should redo that process rather than trying to load multiple files and
plugins in one session.  I think this is because I don't do complete
memory cleanup yet.

Email me if you have any problems.
