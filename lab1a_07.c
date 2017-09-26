#include<stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

int logOfTwo(int);
int whichSet(int, int);
int setIndexLength(int);
int offsetLength(int);
int tagBits(int, int, int);
int hitWay();
void updateOnHit(int, int, int, int, int **);
void updateOnMiss(int, int, int, int, int **, int **, int);
void print(int, int, int **,int **);


int main(int argc, char *argv[])
{
	int linesPerSet;		//Number of lines per set
	int lineSize;				//Size of each line in Bytes
	int cacheSize;			//Size of entire cache in kiloBytes
	int w;
	int v;
	int numSets;				//Number of sets
	int **array;				//Tag array
	int **lruarray;			//LRU array
	int i;							//counter (for) Function
	int j;							//counter (for) Function
	int hit;						//counter (number of hits)
	int miss;						//counter (number of misses)
	int numIndexBits;		//Number of set index bits
	int numOffsetBits;	//Number of offset bits
	int numTagBits;			//Number of tag bits
	int multiplier;
	int indexvalue;
	int accesses=0;
	int hits=0;
	int misses=0;
	int lineSizeBits;

	linesPerSet = strtol( argv[1], NULL, 10);			/*Number of Columns*/
	lineSize = strtol( argv[2], NULL, 10);				/*in Bytes*/
	lineSizeBits = lineSize*8;
	cacheSize = strtol( argv[3], NULL, 10);			/*in Kbytes*/
	w = cacheSize*1024*8;												//Cache size in kiloBytes * kilo(1024) * bits in byte (8) = cache size in bits
	//v = logOfTwo(w) + 1;
	//cacheSize = (int) pow(2, v);					//in Bytes
	numSets = w/(lineSizeBits*linesPerSet);		/*Number of Rows*/

	//printf("size of cache: %d\n", cacheSize);
	//printf("num of sets: %d\n", numSets);
	/*INITIALIZE ARRAYS*/
	array = (int **) malloc(numSets*sizeof(int *));
	for (i=0; i<numSets; i++) array[i] = (int *) malloc(linesPerSet*sizeof(int));

	lruarray = (int **) malloc(numSets*sizeof(int *));
	for (i=0; i<numSets; i++) lruarray[i] = (int *) malloc(linesPerSet*sizeof(int));

	/*INITIALIZE ALL ELEMENTS IN ARRAYS AS -1*/
	for (i=0; i<numSets; i++){
		for (j=0; j<linesPerSet; j++){
			array[i][j] = -1;
			lruarray[i][j] = -1;
		}
	}

	//INITIAL CACHE
	/*printf("INITIAL CACHE\n");
	for (i=0; i<numSets; i++){
	for (j=0; j<linesPerSet; j++){
	printf("array[%2d][%2d]=%d\n", i, j, array[i][j]);
}
}
printf("\n");
for (i=0; i<numSets; i++){
for (j=0; j<linesPerSet; j++){
printf("lruarray[%2d][%2d]=%d\n", i, j, lruarray[i][j]);
}
}
printf("\n");
*/
/*READ INPUT FILE*/
unsigned int x;	/*variable for input read line*/
//char buf[10];
FILE *fptr;
fptr = fopen(argv[4], "r");
while( !feof(fptr)){
	fscanf(fptr, "%x", &x);
	accesses++;
	numIndexBits = setIndexLength(numSets);
	numOffsetBits = offsetLength(lineSize);
	//printf("num of inx bits: %d\n", numIndexBits);
	//printf("num of offset bits: %d\n", numOffsetBits);
	//unsigned int theTag = strtol(buf, NULL, 16);
	//unsigned int theTagIndex = strtol(buf, NULL, 16);	//address without offset
	unsigned int theTagIndex = x >> numOffsetBits;
	multiplier = (int) pow(2, numIndexBits) - 1;
	indexvalue = theTagIndex % numSets;		//determine which set it belongs to using AND operation
	unsigned int theTag = x >> (numIndexBits+numOffsetBits);


	//int tbpRow = whichSet(x,numSets);		/*calculate which set to access*/


	int safei, safej;
	int hit = hitWay(linesPerSet, indexvalue, theTag, array);
	for(i = 0; i < linesPerSet; i++){
		/*FOUND*/
		if(array[indexvalue][i]==theTag){
			hits++;
			//printf("ELEMENT FOUND IN CACHE\n");
//			lruarray[indexvalue][i] = 0;
			safei = indexvalue;				/*element not to be updated in arrays*/
			safej = i;

			/*LRU UPDATE*/
			updateOnHit(safei,safej,numSets,linesPerSet,lruarray);
			print(numSets,linesPerSet,array,lruarray);
			break;	/*end search for found element*/

			/*ELEMENT NOT FOUND IN CACHE*/
		}
		else{
			misses++;

			/*IF ARRAY IS FULL*/
			if(array[indexvalue][linesPerSet-1] != -1){

				int lruIndex = 0;					/*index for element LRU*/
				for(j = 0; j < linesPerSet; j++){
					/*Search lruarray for element LRU within row*/
					if(lruarray[indexvalue][j] > lruarray[indexvalue][lruIndex]){
						lruIndex = j;
					}
				}
				safei = indexvalue;
				safej = lruIndex;
			}
			/*LOOK FOR NEXT EMPTY BLOCK IN CACHE*/
			else{
				for(i = 0; i < linesPerSet; i++){


					if(array[indexvalue][i] == -1){
						safei = indexvalue;
						safej = i;
						i = linesPerSet;
					}
				}
			}
			updateOnMiss(numSets, linesPerSet, safei, safej, lruarray, array, theTag);
			print(numSets,linesPerSet,array,lruarray);
		}

	}

}
fclose(fptr);
/*printf("\n");
printf("FINAL CACHE\n");
for (i=0; i<numSets; i++){
for (j=0; j<linesPerSet; j++){
printf("array[%2d][%2d]=%x\n", i, j, array[i][j]);
}
}
*/
double mr = (double) misses/accesses;
printf("accesses: %d\n", accesses);
printf("hit rate: %d\n", hits);
printf("misses: %d\n", misses);
printf("miss rate: %lf\n", mr);
}

int logOfTwo(int value){
	int i=2;
	int j=0;
	while(i<=value){
		i*=2;
		j++;
	}
	return j;
}

int whichSet(int input, int numOfSets){
	return input%numOfSets;
}

int setIndexLength(int numOfSets){
	return logOfTwo(numOfSets);
}

int offsetLength(int lineSize){
	return logOfTwo(lineSize);
}

int tagBits(int address,int numOfSets,int lineSize){
	return address>>(setIndexLength(numOfSets)+offsetLength(lineSize));
}

int hitWay(int linesPerSet, int indexvalue, int theTag, int **array){
	int i=0;
	for(i = 0; i < linesPerSet; i++)
	if(array[indexvalue][i] == theTag)
	return i;
	return -1;
}

void updateOnHit(int safei, int safej, int numSets, int linesPerSet, int **lruarray){

	int i,j;
	i=j=0;

	for(i = 0; i < numSets; i++)
	for(j = 0; j < linesPerSet; j++)
	if(lruarray[i][j] != -1)
	lruarray[i][j] = lruarray[i][j] + 1;

	lruarray[safei][safej]=0;
}

void updateOnMiss(int numSets,int linesPerSet, int safei, int safej, int **lruarray, int **array, int theTag){

	int i,j;
	i=j=0;

	for(i = 0; i < numSets; i++)
	for(j = 0; j < linesPerSet; j++)
	if(lruarray[i][j] != -1)
	lruarray[i][j] = lruarray[i][j] + 1;

	lruarray[safei][safej] = 0;
	array[safei][safej] = theTag;
}

void print(int numSets,int linesPerSet, int **array,int **lruarray){

	int i,j;
	i=j=0;

	printf("UPDATED CACHE\n");
	for (i=0; i<numSets; i++)
	for (j=0; j<linesPerSet; j++)
	printf("array[%2d][%2d]=%x\n", i, j, array[i][j]);

	printf("\n");

	for (i=0; i<numSets; i++)
	for (j=0; j<linesPerSet; j++)
	printf("lruarray[%2d][%2d]=%d\n", i, j, lruarray[i][j]);

	printf("\n");
}
