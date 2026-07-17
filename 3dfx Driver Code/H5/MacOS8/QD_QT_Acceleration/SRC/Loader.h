/********************************************************************************
	Loader.h
	
	Chall Fry
	Critical Path Software		
	
*/

// Enums
enum
{
	kFailureIcon = 128,
	kSuccessIcon = 129,
	
	kLoaderControlTableVersion = 1,

	k3DFXGetLoaderTable = '3DLt'
};

// Types
typedef struct LoaderControlTable
{
	UInt32		tableVersion;		// Version # of the table struct itself
	UInt32		productID;			// Product identifier
	UInt32		productVersion;		// From vers resource
	LoaderControlTable *next;		// Next table in linked list
	
	void		(*loadAccel)(Boolean openInitFile);
	void		(*unloadAccel)();
	
} LoaderControlTable;	

