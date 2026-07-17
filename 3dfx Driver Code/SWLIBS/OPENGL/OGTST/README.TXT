
BUILDING OGTST

We have provided Visual C++ 4.2 project files and nmake makefiles either
of which can be used to build ogtst.

To use the nmake makefiles, execute the makeall.bat batch file.  This
will produce an ogtst.exe in the ogtst/env subdirectory.

To use the Visual C++ project files.  Open the project file with msdev,
select a build target, and build the application.  This will produce
an ogtst.exe in either the ogtst/Debug or ogtst/Release subdirectories.

RUNNING OGTST

You should run ogtst in a shell window from the ogtst/env subdirectory.

There is an extensive set of help menus available from within ogtst
(run ogtst -h) which explain the many command line options.

Many of the tests in ogtst are IMAGED tests.  This means that the test is
performed by rendering an image, generating a checksum from the pixels
in the image, and then comparing the new checksum with the previously
computed checksum of a known good image.

The checksums for known good images are stored in files in the env/F and
env/B directories for images drawn to the front buffer and back buffer
respectively.  Checksum files are indexed based on the name of the
OpenGL driver as returned by glGetString(GL_RENDERER).

The -u command line flag will cause ogtst to recompute checksums for
all IMAGED tests.  This option can be used to bootstrap the testing
process.

