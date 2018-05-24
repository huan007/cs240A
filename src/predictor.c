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

#define PCBITS 8
#define DEBUGMODE 0
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
int PHTSIZE = 512;

//Custom Variables
uint32_t localTable[CUS_LOCALSIZE];
char* l2List[CUS_LOCALSIZE];


//Logging variables
int* seen;
int counterSeen = 0;
uint32_t* log_pc  ;
uint32_t* log_pat ;
char 	* log_bool;
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

	PHTSIZE = 1 << ghistoryBits;
	pht = malloc(sizeof(char) * PHTSIZE);
	seen = malloc(sizeof(int) * PHTSIZE);
	log_pc   = malloc(sizeof(uint32_t) * PHTSIZE);
	log_pat  = malloc(sizeof(uint32_t) * PHTSIZE);
	log_bool = malloc(sizeof(char) * PHTSIZE);
	fprintf(stderr, "Bits: %d\nSize: %d\n", ghistoryBits, getPHTSize());

	memset(pht, WN, (sizeof(char) * PHTSIZE));
	memset(seen, 0, (sizeof(int) * PHTSIZE));
	memset(log_pc , 0, (sizeof(uint32_t) * PHTSIZE));
	memset(log_pat, 0, (sizeof(uint32_t) * PHTSIZE));
	memset(log_bool,0, (sizeof(char) * PHTSIZE));

	//Custom setup
	int i = 0;
	for (i = 0; i < CUS_LOCALSIZE; i++)
	{
		l2List[i] = malloc(sizeof(char) * CUS_L2SIZE);
		memset(l2List[i], WN, (sizeof(char) * CUS_L2SIZE));
	}
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
			{
				return predict_gshare(pc);
			}
		case TOURNAMENT:
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
		case CUSTOM:
			train_huan(pc, outcome);
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

int getPHTSize()
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

	if (DEBUGMODE)
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

}

uint8_t predict_huan(uint32_t pc)
{
	uint32_t mask = -1;
	int numMask = 32 - PCBITS;
	mask = mask << numMask >> numMask;
	uint32_t hist = history & mask;
	uint32_t addr = pc & mask;

	//use addr to get the table
	//hash hist into table 
	mask = -1;
	numMask = 32 - 4;
	mask = mask << numMask >> numMask;
	int tableIndex = (hist & mask) ^ ( (hist >> 4) & mask); 
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

	if (DEBUGMODE)
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

}

void train_huan(uint32_t pc, uint8_t outcome)
{
	if (outcome != TAKEN && outcome != NOTTAKEN)
		exit(-1);
	
	uint32_t mask = -1;
	int numMask = 32 - PCBITS;
	mask = mask << numMask >> numMask;
	uint32_t hist = history & mask;
	uint32_t addr = pc & mask;

	//use addr to get the table
	//hash hist into table 
	mask = -1;
	numMask = 32 - 4;
	mask = mask << numMask >> numMask;
	int tableIndex = (hist & mask) ^ ( (hist >> 4) & mask); 
	char* table = l2List[addr%CUS_LOCALSIZE];

	//Update pattern history register
	history = (history << 1) + outcome;
	update(&(table[tableIndex]), outcome);
}
