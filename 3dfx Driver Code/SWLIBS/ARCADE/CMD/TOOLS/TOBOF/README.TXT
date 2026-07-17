tobof

tobof is a command to create a 3DFX BOF (Binary Object File) which puts one or more graphical objects into
a single file and provides a simply dictionary access to retrieve these objects.

To create a bof file at the command line enter:

tobof -f <command file name> -o <bof file name>

Supported commands

Each line in the command file describes a command to be processed. Blank lines and lines starting with a #
are treated as comments and ignored.

p[ath] <list of ';' separated directories>

specifies the directories to search when loading objects.

i[include] <filename>

In addition to the current command file also process the commands in the specified file.

o[bject] -name <object name> -file <filename> [-anim] [-vs <scale>]

Load an object from file <filename> and associate it with the name <object name> in the BOF dictionary. If
-anim is specified a data file with animation information will be expected and an AtsAnim object created. If the -vs option is psecified vertices will be scaled by the specified amount.

load the specified object

t[exture] -name <object name> -file <filename>

Load a texture from file <filename> and associate it with the name <object name> in the BOF dictionary. 

image -name <object name> -file <filename>

Loads a image from file <filename> and associate it with the name <object name> in the BOF dictionary. 

c[mesh] -name <object name> -file <filename> [-gridSize <gridsize>]

Load an collision mesh from file <filename> and associate it with the name <object name> in the BOF 
dictionary. If -gridSize is specified the mesh will be gridded with each cell of the specified dimensions.

seq[uence] -name <object name> -file <filename> -endFrame <endFrame> [-duration <time>] 
                 [-numreps <numReps>] [-startFrame <startFrame>] [-mode <mode>] [-vs <scale>]

Load a sequence object from file <filename> and associate it with the name <object name> in the BOF 
dictionary. You must specify the endFrame number (the start frame number defaults to 0). The filename is
treated as a sprintf format statement (say tuna%d.rtg) and the frame number used to specify the filenames.
If specified duration determines the time taken to display the entire animation (default is 1 second). If
specified -numreps specifies the number of times to repeat the animation (default is -1, indefinite repetition). If specified -mode specifies the sequence mode ATS_SEQ_CYCLE or ATS_SEQ_SWING 
(default is ATS_SEQ_CYCLE). If the -vs option is psecified vertices will be scaled by the specified amount.

Loading and accessing a BOF file


Below is a sample code sequence for loading a model from a BOF file. First the BOF file is loaded into
memory using atsFileLoad next the model is looked up in the BOF's directory.

    AtsObject *dict;
    AtsNode *model;
    AtsType *nodeType = atsGetTypeFromName("Node");

    if (( dict = atsFileLoad( WIZ_DATABASE, atsGetTypeFromName("Dict"))) == NULL ) {
        atuError(FXTRUE, " could not load wiz models %s\n", WIZ_DATABASE);
    }

    if (( model = atsDictFind(dict, modelName, t)) == NULL ) {
         atuError(FXTRUE, "Could not find model %s\n", modelName);
    }
