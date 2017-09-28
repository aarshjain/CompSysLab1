#include<stdio.h>
#include <math.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>

int logOfTwo(int);
int whichSet(int, int);
int setIndexLength(int);
int offsetLength(int);
int tagBits(unsigned int, int, int);
int hitWay();
void updateOnHit(int, int, int, int, int **);
void updateOnMiss(int, int, int, int, int **, int **, int);
void print(int, int, int **,int **);


int main(int argc, char *argv[])
{
	int linesPerSet;		// Number of lines per set
	int lineSize;				// Size of each line in Bytes
	int cacheSize;			// Size of entire cache in kiloBytes
	int cacheSizeBits;	// Size of cache in bits
	int numSets;				// Number of sets
	int **tagArray;				// Tag array
	int **lruarray;			// LRU array
	int i;							// Counter (for) Function
	int j;							// Counter (for) Function
	int numIndexBits;		// Number of set index bits
	int numOffsetBits;	// Number of offset bits
	int numTagBits;			// Number of tag bits
	int indexvalue;// Set index of the current address
	int accesses=0;				// Counter (Total number of accesses
	int hits=0;						// Counter (number of hits)
	int misses=0;					// Counter (number of misses)
	int lineSizeBits;			// Size of line in bits

		assert( argc != 4 );		// Assert if all the variables were provided to main

		// Storing all the given parameters in local variables
		linesPerSet = strtol( argv[1], NULL, 10);			/*Number of Columns*/
		lineSize = strtol( argv[2], NULL, 10);				/*in Bytes*/
		lineSizeBits = lineSize*8;
		cacheSize = strtol( argv[3], NULL, 10);			/*in Kbytes*/
		cacheSizeBits = cacheSize*1024*8;												//Cache size in bits * kilo(1024) * bits in byte (8) = cache size in bits				//in Bytes
		numSets = cacheSizeBits/(lineSizeBits*linesPerSet);		/*Number of Rows*/
		
		/*INITIALIZE ARRAYS*/
		tagArray = (int **) malloc(numSets*sizeof(int *));
		for (i=0; i<numSets; i++) tagArray[i] = (int *) malloc(linesPerSet*sizeof(int));

		lruarray = (int **) malloc(numSets*sizeof(int *));
		for (i=0; i<numSets; i++) lruarray[i] = (int *) malloc(linesPerSet*sizeof(int));

		/*INITIALIZE ALL ELEMENTS IN ARRAYS AS -1*/
		for (i=0; i<numSets; i++){
			for (j=0; j<linesPerSet; j++){
				tagArray[i][j] = -1;
				lruarray[i][j] = -1;
			}
		}


		/******READ INPUT FILE******/
		
		unsigned int x;	/*variable for input read line*/
		FILE *fptr;
		fptr = fopen(argv[4], "r");
		while( !feof(fptr)){
			fscanf(fptr, "%x", &x);
			accesses++;
			numIndexBits = setIndexLength(numSets);
			numOffsetBits = offsetLength(lineSize);
			unsigned int theTagIndex = x >> numOffsetBits;
							//determine which set it belongs to using AND operation
			
			unsigned int theTag = tagBits(x, numIndexBits, numOffsetBits);
			indexvalue = theTag % numSets;	


			int safei, safej;
			for(i = 0; i < linesPerSet; i++){
				/*FOUND*/
				if(tagArray[indexvalue][i]==theTag){
					hits++;
					int hit = hitWay(linesPerSet, indexvalue, theTag, tagArray);

					safei = indexvalue;		/*element not to be updated in arrays*/
					safej = i;

					/*LRU UPDATE*/
					updateOnHit(safei,safej,numSets,linesPerSet,lruarray);
					/*printf("Updated CACHE\n");
					for (i=0; i<numSets; i++){
						for (j=0; j<linesPerSet; j++){
						printf("array[%2d][%2d]=%x\n", i, j, tagArray[i][j]);
						}
					}*/
					break;					/*end search for found element*/

					/*ELEMENT NOT FOUND IN CACHE*/
				}
				else{
					misses++;

					/*IF ARRAY IS FULL*/
					if(tagArray[indexvalue][linesPerSet-1] != -1){

						int lruIndex = 0;		//index for element LRU
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


							if(tagArray[indexvalue][i] == -1){
								safei = indexvalue;
								safej = i;
								i = linesPerSet;
							}
						}
					}
					updateOnMiss(numSets, linesPerSet, safei, safej, lruarray, tagArray, theTag);
					/*printf("Updated CACHE\n");
					for (i=0; i<numSets; i++){
						for (j=0; j<linesPerSet; j++){
						printf("array[%2d][%2d]=%x\n", i, j, tagArray[i][j]);
						}
					}*/
				}
				

			}

		}
		fclose(fptr);
		double mr = (double) misses/accesses;
		printf("accesses: %d\n", accesses);
		printf("hit rate: %d\n", hits);
		printf("misses: %d\n", misses);
		printf("miss rate: %lf\n", mr);
		
		/*printf("FINAL CACHE\n");
		for (i=0; i<numSets; i++){
			for (j=0; j<linesPerSet; j++){
			printf("array[%2d][%2d]=%x\n", i, j, tagArray[i][j]);
			}
		}*/
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

	int tagBits(unsigned int address,int indexBits, int offsetBits){
		return ( unsigned int )address>>(indexBits+offsetBits);
	}

	int hitWay(int linesPerSet, int indexvalue, int theTag, int **tagArray){
		int i;
		for(i = 0; i < linesPerSet; i++)
		if(tagArray[indexvalue][i] == theTag)
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

	void updateOnMiss(int numSets,int linesPerSet, int safei, int safej, int **lruarray, int **tagArray, int theTag){

		int i,j;
		i=j=0;

		for(i = 0; i < numSets; i++)
		for(j = 0; j < linesPerSet; j++)
		if(lruarray[i][j] != -1)
		lruarray[i][j] = lruarray[i][j] + 1;

		lruarray[safei][safej] = 0;
		tagArray[safei][safej] = theTag;
	}

	void print(int numSets,int linesPerSet, int **tagArray,int **lruarray){

		int i,j;
		i=j=0;

		printf("UPDATED CACHE\n");
		for (i=0; i<numSets; i++)
		for (j=0; j<linesPerSet; j++)
		printf("tagArray[%2d][%2d]=%x\n", i, j, tagArray[i][j]);

		printf("\n");

		for (i=0; i<numSets; i++)
		for (j=0; j<linesPerSet; j++)
		printf("lruarray[%2d][%2d]=%d\n", i, j, lruarray[i][j]);

		printf("\n");
	}
