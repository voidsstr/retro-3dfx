// TdfxProfiler.c


#include <Profiler.h>
#include <stdio.h>

#pragma profile off
#pragma global_optimizer off


static short profilerStatus = false;


typedef struct {
	char * functionName;
	unsigned long count;
	unsigned long totalTime;
	unsigned long netTime;			// netTime = totalTime - time spent in children
	unsigned long min;
	unsigned long max;
} ProfilerInfo;

typedef struct {
	ProfilerInfo * function;
	unsigned long start1;
	unsigned long start2;
	unsigned long childTime;
} ProfilerStack;


ProfilerInfo functions[3000];
int functionCount = 0;

ProfilerStack stackBuffer[2000];
ProfilerStack * stackSpace;
ProfilerStack * stack;
int stackCount;

static ProfilerInfo * FindFunction(char * functionName);


#define COUNT_ONLY 0

static void ProfilerLog(char * string)
{
#if 0
	FILE * f;
	f = fopen("profiler log.txt", "a");
	fprintf(f, "%s\n", string);
	fclose(f);
#endif
}


pascal void __PROFILE_ENTRY(char *functionName)
{
	register unsigned long timeBase;
	ProfilerInfo * function;

	asm{
		mftb	timeBase
	}
	
	if(!profilerStatus) return;
	
	stackCount++;

	function = FindFunction(functionName);
	function->count += 1;

#if !COUNT_ONLY
			
	stack++;
		
	if(stackCount > 500) debugstr("stackCount > 500");
	
	stack->function = function;
	stack->start1 = timeBase;
	stack->childTime = 0;

	asm{
		mftb	timeBase
	}
	
	stack->start2 = timeBase;
#endif // !COUNT_ONLY
	
}




pascal void __PROFILE_EXIT(void)
{
	register unsigned long timeBase;
	register unsigned long save;
	ProfilerInfo * function;
	unsigned long elapsedTime;
	unsigned long start1;
	unsigned long childTime;


	// must save and restore r3
	// calling function sets r3 (return value) before calling __PROFILE_EXIT()
	asm{
		mr		save, r3
		mftb	timeBase
	}

	if(!profilerStatus) goto exit;
	
	stackCount--;
#if !COUNT_ONLY
	if(stack <= stackSpace){
		goto exit;
	}

	elapsedTime = timeBase - stack->start2;
	
	function = stack->function;
	
	if(function < functions || function > functions + (functionCount - 1)){
		debugstr("function is bad");
		goto exit;
	}
	
//	function->count += 1;
	function->totalTime += elapsedTime;
	elapsedTime -= stack->childTime;
	function->netTime += elapsedTime;
	
	if(elapsedTime < function->min) function->min = elapsedTime;
	if(elapsedTime > function->max) function->max = elapsedTime;
	
	// childTime(of parrent) += timeBase - start1(of me);
	// math rearanged to lower overhead
	
	start1 = stack->start1;
	stack--;
	childTime = stack->childTime - start1;
	
	asm{
		mftb	timeBase
	}

	stack->childTime = childTime + timeBase;
#endif // !COUNT_ONLY
exit:

	// restore r3 before returning
	asm{
		mr	r3, save
	}


}

pascal OSErr ProfilerInit(short /*ProfilerCollectionMethod*/ method, short /*ProfilerTimeBase*/ timeBase, short numFunctions, short stackDepth)
{
	ProfilerClear();
	ProfilerSetStatus(true);	// matches what the codewarrior profiling lib does
	return noErr;
}


pascal void ProfilerTerm(void)
{

}

pascal void ProfilerSetStatus(short on)
{
	profilerStatus = on;
}

pascal short ProfilerGetStatus(void)
{
	return profilerStatus;
}

pascal void ProfilerGetDataSizes(long *functionSize, long *stackSize)
{


}



pascal OSErr ProfilerDump(StringPtr filename)
{

	FILE * f;
	int i;
	ProfilerInfo * function;
	unsigned long totalTotal;
	unsigned long totalNet;
	
	f = fopen("profiler dump.txt", "w");
#if COUNT_ONLY
	fprintf(f, "stackcount = %d\n", stackCount);
	for(i = 0; i < functionCount; i++){
		function = &functions[i];
		
		fprintf(f, "%s\t%ld\n", 
			function->functionName,
			function->count
			);
	}
#else
	fprintf(f, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n", 
		"Function Name",
		"count",
		"totalTime",
		"totalTime%",
		"netTime",
		"netTime%",
		"min",
		"max",
		"average");

	totalTotal = 0;
	totalNet = 0;
	for(i = 0; i < functionCount; i++){
		function = &functions[i];
		totalTotal += function->totalTime;
		totalNet += function->netTime;
	}

	for(i = 0; i < functionCount; i++){
		function = &functions[i];
		
		fprintf(f, "%s\t%ld\t%ld\t%6.2f\t%ld\t%6.2f\t%ld\t%ld\t%6.2f\n", 
			function->functionName,
			function->count,
			function->totalTime,
			(100.0f * (float)function->totalTime / (float)totalTotal),
			function->netTime,
			(100.0f * (float)function->netTime / (float)totalNet),
			function->min,
			function->max,
			((float)function->netTime / (float)function->count)
			);
	}
#endif
	fclose(f);

	return noErr;
}

pascal void ProfilerClear(void)
{
	stackSpace = stackBuffer + 1000;
	stack = stackSpace;
	functionCount = 0;
	stackCount = 0;
}

static ProfilerInfo * FindFunction(char * functionName)
{
	int i;
	ProfilerInfo * function;



	for (i = 0; i < functionCount; i++){
		if(functions[i].functionName == functionName){
			return &functions[i];
		}
	}
	
	if(functionCount > 2000) debugstr("functionCount > 2000");
		
	function = &functions[functionCount];
	functionCount++;

	function->functionName = functionName;
	function->count = 0;
	function->totalTime = 0;
	function->netTime = 0;			// netTime = totalTime - time spent in children
	function->min = -1;
	function->max = 0;

	return function;
	
}


