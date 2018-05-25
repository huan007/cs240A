//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <string.h>
#include "predictor.h"

#define PCBITS 9
#define HISBITS 10
#define DEBUG 0
#define K20 20000
uint8_t predict_gshare(uint32_t pc);
uint8_t predict_tour(uint32_t pc);
uint8_t predict_huan(uint32_t pc);
void train_gshare(uint32_t pc, uint8_t outcome);
void train_tour(uint32_t pc, uint8_t outcome);
void train_huan(uint32_t pc, uint8_t outcome);

//
// TODO:Student Information
//
const char *studentName = "Huan Nguyen";
const char *studentID   = "A12871523";
const char *email       = "hpn007@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
	"Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//
//Gshare variables
uint32_t history;
char* pht; 
unsigned int PHTSIZE = 2048;
unsigned int PSIZE;
unsigned int LSIZE;

//Custom Variables
char* l2List[CUS_LOCALSIZE];

//Tournament vars
uint32_t* localTable;
char* global;
char* choser;
char* localPre;

//Logging variables
int* seen;
int counterSeen = 0;
char* log_pc  ;
char* log_pat ;
char* log_bool;
int 	 log_inteferece;


//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

//Update function for two bit pht
void update(char* counter, uint8_t increment)
{
	if (increment && (*counter >= ST))
		return;	
	if (!increment && (*counter <= SN))
		return;	

	if (increment)
		(*counter)++;
	else
		(*counter)--;
}

// Initialize the predictor
//
	void
init_predictor()
{
	//
	//TODO: Initialize Branch Predictor Data Structures
	//
	history = 0;
	log_inteferece = 0;

	//Gshare size
	PHTSIZE = 1 << ghistoryBits;

	//Tournament size
	LSIZE = 1 << lhistoryBits;
	PSIZE = 1 << pcIndexBits;
	//Custom size

	switch (bpType) {
		case STATIC:
			break;
		case GSHARE:
			{
				//GShare vars
				pht = malloc (sizeof(char) * PHTSIZE);
				memset(pht, WN, (sizeof(char) * PHTSIZE));
				break;
			}
		case TOURNAMENT:
			{
				//Tournament vars
				localTable = malloc (sizeof(uint32_t) * PSIZE);
				global = malloc (sizeof(char) * PHTSIZE);
				choser = malloc (sizeof(char) * PHTSIZE);
				localPre = malloc(sizeof(char) * LSIZE);

				//Tournament Setup
				memset(localTable, 0, (sizeof(uint32_t) * PSIZE));
				memset(global, WN, (sizeof(char) * PHTSIZE));
				memset(choser, WT, (sizeof(char) * PHTSIZE));
				memset(localPre, WN, (sizeof(char) * LSIZE));
				break;
			}
		case CUSTOM:
			{
				//Custom setup
				int i = 0;
				for (i = 0; i < CUS_LOCALSIZE; i++)
				{
					l2List[i] = malloc(sizeof(char) * CUS_L2SIZE);
					memset(l2List[i], WN, (sizeof(char) * CUS_L2SIZE));
				}
			}
		default:
			break;
	}

	//Logging variables
	int* seen;
	char* log_pc  ;
	char* log_pat ;
	char* log_bool;
	seen = malloc(sizeof(int) * PHTSIZE);
	log_pc = malloc(sizeof(char) * PHTSIZE);
	log_pat = malloc(sizeof(char) * PHTSIZE);
	log_bool = malloc(sizeof(char) * PHTSIZE);

	memset(seen, 0, (sizeof(int) * PHTSIZE));
	memset(log_pc , 0, (sizeof(char) * PHTSIZE));
	memset(log_pat, 0, (sizeof(char) * PHTSIZE));
	memset(log_bool,0, (sizeof(char) * PHTSIZE));
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
	uint8_t
make_prediction(uint32_t pc)
{
	//
	//TODO: Implement prediction scheme
	//

	// Make a prediction based on the bpType
	switch (bpType) {
		case STATIC:
			return TAKEN;
		case GSHARE:
			return predict_gshare(pc);
		case TOURNAMENT:
			return predict_tour(pc);
		case CUSTOM:
			return predict_huan(pc);
		default:
			break;
	}

	// If there is not a compatable bpType then return NOTTAKEN
	return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
	void
train_predictor(uint32_t pc, uint8_t outcome)
{
	//
	//TODO: Implement Predictor training
	//
	switch (bpType) {
		case STATIC:
			break;
		case GSHARE:
			{
				train_gshare(pc, outcome);
				return;
			}
		case TOURNAMENT:
			{
				train_tour(pc, outcome);
				return;
			}
		case CUSTOM:
			{
				train_huan(pc, outcome);
				return;
			}
		default:
			break;
	}
}

int slotsUsed()
{
	return counterSeen;
}

int getInterference()
{
	return log_inteferece;
}

int getSize()
{
	return PHTSIZE;
}

uint8_t predict_gshare(uint32_t pc)
{
	uint32_t mask = -1;
	int numMask = 32 - ghistoryBits;
	mask = mask << numMask >> numMask;
	uint32_t hist = history & mask;
	uint32_t addr = pc & mask;
	//printf ("hist: %d\naddr: %d\n", hist, addr);

	unsigned int index = (hist ^ addr) % PHTSIZE;

	if (DEBUG)
	{
		//Log what is first seen at the pHT slot
		if (log_pc[index] == 0 && log_pat[index] == 0)
		{
			log_pc [index] = addr;
			log_pat[index] = hist;
		}
		//Different PC or differet pattern map to the same slot
		else if (log_pc[index] != addr || log_pat[index] != hist)
		{
			if (log_bool[index] == FALSE)
			{
				log_bool[index] = TRUE;
				log_inteferece++;
			}
		}
	}
	if (pht[index] > WN)
		return TAKEN;
	else
		return NOTTAKEN;
}

uint8_t predict_tour(uint32_t pc)
{
	//Global 
	uint32_t mask = -1;
	int numMask = 32 - ghistoryBits;
	mask = mask << numMask >> numMask;
	uint32_t hist = history & mask;
	int globalIndex = hist % 512;

	//Choser
	char resultGlobal = global[globalIndex];
	char resultChoser = choser[globalIndex];

	if (DEBUG)
	{
		fprintf(stderr, "----------Prediction Phase--------------\n");
		fprintf(stderr, "GlobalIndex: %d\nglobal: %d\nchoser: %d\n", globalIndex, 
					resultGlobal, resultChoser);
	}

	//Local
	mask = -1;
	numMask = 32 - pcIndexBits;
	mask = mask << numMask >> numMask;
	int localIndex = (pc & mask) % 1024;
	//TODO: Need to modify later
	uint32_t localPat = localTable[localIndex] % 1024;
	char resultLocal = localPre[localPat];

	if (DEBUG)
	{
		uint32_t temp = localPat;
		fprintf(stderr, "localIndex: %d\nlocalPattern: ", localIndex);

		
		while (temp > 0)
		{
			if (temp & 1)
				fprintf(stderr, "1");

			else
				fprintf(stderr, "0");

			temp = temp >> 1;
		}
		fprintf(stderr, "\nlocal: %d\n", resultLocal);
	}

	char result;

	//Choose result base on the choser
	if (resultChoser > WN)
		result = resultGlobal;
	else
		result = resultLocal;

	if (DEBUG)
		fprintf(stderr, "Final result: %d\n", result);

	//Process result and return final answer
	if (result > WN)
		return TAKEN;
	else
		return NOTTAKEN;
}

uint8_t predict_huan(uint32_t pc)
{
	//Get history bits to selec entry
	uint32_t mask = -1;
	int numMask = 32 - HISBITS;
	mask = mask << numMask >> numMask;
	uint32_t hist = history & mask;

	//Get PC bits to select table
	mask = -1;
	numMask = 32 - PCBITS;
	mask = mask << numMask >> numMask;
	uint32_t addr = pc & mask;

	//use addr to get the table
	//hash hist into table 
	mask = -1;
	numMask = 32 - HISBITS;
	mask = mask << numMask >> numMask;
	int tableIndex = (hist & mask) ^ ( (hist >> HISBITS) & mask); 
	char* table = l2List[addr%CUS_LOCALSIZE];

	if (table[tableIndex] > WN)
		return TAKEN;
	else
		return NOTTAKEN;
}

void train_gshare(uint32_t pc, uint8_t outcome)
{
	if (outcome != TAKEN && outcome != NOTTAKEN)
		exit(-1);

	uint32_t mask = -1;
	int numMask = 32 - ghistoryBits;
	mask = mask << numMask >> numMask;
	uint32_t hist = history & mask;
	uint32_t addr = pc & mask;
	unsigned int index = (hist ^ addr) % PHTSIZE;
	//printf("Index: %d\n", index);
	//printf("Prediction: %d\n", pht[index]);
	//printf("Outcome: %d\n", outcome);

	//Update pattern history register
	history = (history << 1) + outcome;
	update(&(pht[index]), outcome);

	if (DEBUG)
	{
		//Logging
		int i = 0;
		for (i = 0; i < counterSeen; i++)
		{
			//Check in the seen list to see if index show up
			if (seen[i] == index)
				//If index is already in there, then we have used that slot
				return;
		}

		//New slot in PHT being used, we take note
		seen[i] = index;
		counterSeen++;
	}
}

void train_tour(uint32_t pc, uint8_t outcome)
{
	if (outcome != TAKEN && outcome != NOTTAKEN)
		exit(-1);

	//Global
	uint32_t mask = -1;
	int numMask = 32 - ghistoryBits;
	mask = mask << numMask >> numMask;
	uint32_t hist = history & mask;
	unsigned int globalIndex = hist % 512;
	
	uint32_t resultGlobal = global[globalIndex];

	if (DEBUG)
	{
		fprintf(stderr, "----------Training Phase--------------\n");
		fprintf(stderr, "OUTCOME: %d\nGlobalIndex: ", outcome);
		
		unsigned int temp = globalIndex;
		while (temp > 0)
		{
			if (temp & 1)
				fprintf(stderr, "1");

			else
				fprintf(stderr, "0");

			temp = temp >> 1;
		}
		fprintf(stderr, "\nGlobal: %d\n", resultGlobal);
	}
	//Local
	mask = -1;
	numMask = 32 - pcIndexBits;
	mask = mask << numMask >> numMask;
	int localIndex = (pc & mask) % 1024;
	uint32_t localPat = localTable[localIndex] % 1024;
	uint32_t resultLocal = localPre[localPat];

	if (DEBUG)
	{
		uint32_t temp = localPat;
		fprintf(stderr, "localIndex: %d\nlocalPattern: ", localIndex);

		
		while (temp > 0)
		{
			if (temp & 1)
				fprintf(stderr, "1");

			else
				fprintf(stderr, "0");

			temp = temp >> 1;
		}
		fprintf(stderr, "\nlocal: %d\n", resultLocal);
	}

	uint8_t outGlobal;
	uint8_t outLocal;

	//Process global's prediction
	if (resultGlobal > WN)
		outGlobal = TAKEN;
	else
		outGlobal = NOTTAKEN;

	//Process local's prediction
	if (resultLocal > WN)
		outLocal = TAKEN;
	else
		outLocal = NOTTAKEN;

	if (DEBUG)
	{
		fprintf(stderr, "outGlobal: %d\noutLocal:  %d\n", outGlobal, outLocal);	
	}

	//Update choice predictor
	if (outGlobal == outLocal)
	{
		//Do nothing
		if (DEBUG)
		{
			fprintf(stderr, "Choser kept the same\n");
		}
	}
	//Global was correct
	else if (outGlobal == outcome)
		update(&(choser[globalIndex]), TRUE);
	//Local was correct
	else if (outLocal == outcome)
		update(&(choser[globalIndex]), FALSE);

	//Update pattern history register
	history = (history << 1) + outcome;

	//Update local and globals
	update(&(global[globalIndex]), outcome);
	update(&(localPre[localPat]), outcome);

	if (DEBUG)
	{
		fprintf(stderr, "Update global: %d\nUpdate local: %d\n", global[globalIndex],
					localPre[localPat]);
		fprintf(stderr, "Updated Chose: %d\n", choser[globalIndex]);
	}

	//Update local's PHT
	localTable[localIndex % 1024] = (localTable[localIndex % 1024] << 1) + outcome;
}

void train_huan(uint32_t pc, uint8_t outcome)
{
	if (outcome != TAKEN && outcome != NOTTAKEN)
		exit(-1);
	
	//Get history bits to selec entry
	uint32_t mask = -1;
	int numMask = 32 - HISBITS;
	mask = mask << numMask >> numMask;
	uint32_t hist = history & mask;

	//Get PC bits to select table
	mask = -1;
	numMask = 32 - PCBITS;
	mask = mask << numMask >> numMask;
	uint32_t addr = pc & mask;

	//use addr to get the table
	//hash hist into table 
	mask = -1;
	numMask = 32 - HISBITS;
	mask = mask << numMask >> numMask;
	//Index into prediction table by XOR lower and higher half of history
	int tableIndex = (hist & mask) ^ ( (hist >> HISBITS) & mask); 
	char* table = l2List[addr%CUS_LOCALSIZE];

	//Update pattern history register
	history = (history << 1) + outcome;
	update(&(table[tableIndex]), outcome);
}

void clean()
{
	switch (bpType) {
		case STATIC:
			break;
		case GSHARE:
			{
				//GShare vars
				free(pht);
				break;
			}
		case TOURNAMENT:
			{
				//Tournament Setup
				free(localTable);
				free(global);
				free(choser);
				free(localPre);
				break;
			}
		case CUSTOM:
			{
				//Custom setup
				//int i = 0;
				//for (i = 0; i < CUS_LOCALSIZE; i++)
				//	free(l2List[i]);
			}
		default:
			break;
	}

	free(seen);
	free(log_pc);
	free(log_pat);
	free(log_bool);
}
