//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>
#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//
const char *studentName = "Ava Gonick";
const char *studentID = "A16729754";
const char *email = "agonick@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = {"Static", "Gshare",
                         "Tournament", "Custom"};

// define number of bits required for indexing the BHT here.
int ghistoryBits = 15; // Number of bits used for Global History
int bpType;            // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     // 
//------------------------------------//

//
// TODO: Add your own Branch Predictor data structures here
//
// gshare
uint8_t *bht_gshare;
uint64_t ghistory;

//tournament 

//local history table for tournament
uint16_t *lht_tournament;
//local prediction for tournament
uint8_t *lpred_tournament;

//global history for the tournament
uint16_t ghistory_tournament;
//global prediction for tournament
uint8_t *gpred_tournament;

//choice predictor
uint8_t *cpred_tournament;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

//tournament functions 
void init_tournament()
{

  //create the local history table, it has 1024 entires of size 10
  //can not get a size 10 value in C so using 16 and will only use the bottom 10 bits
  int local_entries = 2048;
  lht_tournament = (uint16_t *)malloc(local_entries * sizeof(uint16_t));
  int i = 0;

  //initiating history as all 0s 
  for (i = 0; i < local_entries; i++)
  {
    lht_tournament[i] = 0;
  }
 
  //create local prediction table 
  lpred_tournament = (uint8_t *)malloc(local_entries * sizeof(uint8_t));

  //initiating all predictions as weakly not taken
  for (i = 0; i < local_entries; i++)
  {
    lpred_tournament[i] = WN3;
  }

  //initiation the global predictor it has 4096 entires 
  gpred_tournament = (uint8_t *)malloc(8192 * sizeof(uint8_t));

  //initiating all predictions as weakly not taken
  for (i = 0; i < 8192; i++)
  {
    gpred_tournament[i] = WN;
  }

  ghistory_tournament = 0;


  //initiation the choice predictor it has 4096 entires 
  cpred_tournament = (uint8_t *)malloc(8192 * sizeof(uint8_t));

  //initiating all predictions as weakly not taken
  for (i = 0; i < 8192; i++)
  {
    cpred_tournament[i] = LW;
  }
}



uint8_t tournament_predict(uint32_t pc)
{
  //lht_entries 
  uint32_t lht_entries = 1 << 11;

  //get the bottom 10 bits in the pc this is your index
  //even though going to a different size int is is ok because it will just chop off the unused upper bits 
  uint32_t index = lht_tournament[pc & (lht_entries - 1)];

  int local;

  //if between 0 and 3 inclsuive you are not taken
  if (lpred_tournament[index] >= 0 && lpred_tournament[index] < 4){
    local = NOTTAKEN;
  }

  //if between 4 and 8 inclusive you are taken
  else if (lpred_tournament[index] >= 4 && lpred_tournament[index] < 8){
    local = TAKEN;
  }

  //debugging that mentions an invalid state 
  else {
     printf("Warning: Undefined state of entry in lpred_tournament!\n");
     return NOTTAKEN;
  }


  //get the global prediction
  int global;
  uint32_t bht_entries = 1 << 13;
  index = ghistory_tournament & (bht_entries - 1);

  //if between 0 and 1 inclsuive you are not taken
  if (gpred_tournament[index] >= 0 && gpred_tournament[index] < 2){
    global = NOTTAKEN;
  }

  //if between 2 and 3 inclusive you are taken
  else if (gpred_tournament[index] >= 2 && gpred_tournament[index] < 4){
    global = TAKEN;
  }

  //debugging that mentions an invalid state 
  else {
     printf("Warning: Undefined state of entry in gpred_tournament!\n");
    return NOTTAKEN;
  }

  if (global == local){
    return global;
  }

  //if they dont agree return based on if it should be the global or local predictor 
  else {
    if (cpred_tournament[index] == LW || cpred_tournament[index] == LS){
      return local;
     }

     else {
      return global;
     }
  }
}



void train_tournament(uint32_t pc, uint8_t outcome)
{
  //train the local predictor 
  uint32_t lht_entries = 1 << 11;

  //get the bottom 10 bits in the pc this is your index
  //even though going to a different size int is is ok because it will just chop off the unused upper bits 
  uint32_t index = lht_tournament[pc & (lht_entries - 1)];

    //store the local prediction
  int local = lpred_tournament[index] >= 4 ? TAKEN : NOTTAKEN;


  // Update state of entry in the local prediction branch
  switch (lpred_tournament[index])
  {
  case SN3:
    lpred_tournament[index] = (outcome == TAKEN) ? MN3 : SN3;
    break;
  case MN3:
    lpred_tournament[index] = (outcome == TAKEN) ? SLN3 : SN3;
    break;
  case SLN3:
    lpred_tournament[index] = (outcome == TAKEN) ? WN3 : MN3;
    break;
  case WN3:
   lpred_tournament[index] = (outcome == TAKEN) ? WT3 : SLN3;
    break;
  case WT3:
    lpred_tournament[index] = (outcome == TAKEN) ? SLT3 : WN3;
    break;
  case SLT3:
    lpred_tournament[index] = (outcome == TAKEN) ? MT3 : WT3;
    break;
  case MT3:
    lpred_tournament[index] = (outcome == TAKEN) ? ST3 : SLT3;
    break;
  case ST3:
    lpred_tournament[index] = (outcome == TAKEN) ? ST3 : MT3;
    break;
  default:
    printf("Warning: Undefined state of entry in lpred_tournament!\n");
    break;
  }


//update history 
 lht_tournament[pc & (lht_entries - 1)] = ((lht_tournament[pc & (lht_entries - 1)] << 1) | outcome) & (lht_entries - 1);


  //train the global predictor 
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << 13;
  index = ghistory_tournament & (bht_entries - 1);


    //store the global prediction
  int global = gpred_tournament[index] >= 2 ? TAKEN : NOTTAKEN;


  // Update state of entry in bht based on outcome
  switch (gpred_tournament[index])
  {
  case WN:
    gpred_tournament[index] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    gpred_tournament[index] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    gpred_tournament[index] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    gpred_tournament[index] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in gpred_tournament!\n");
    break;
  }

  // Update history register
  ghistory_tournament = ((ghistory_tournament << 1) | outcome) & (bht_entries - 1);


  //update the choice predictor if needed
  if (global != local){
      switch (cpred_tournament[index])
            {
            case LS:
             cpred_tournament[index] = (outcome == global) ? LW : LS;
              break;
            case LW:
              cpred_tournament[index] = (outcome == global) ? GW : LS;
              break;
            case GW:
              cpred_tournament[index] = (outcome == global) ? GS : LW;
              break;
            case GS:
              cpred_tournament[index] = (outcome == global) ? GS : GW;
              break;
            default:
              printf("Warning: Undefined state of entry in cpred_tournament!\n");
              break;
            }
  }

}


// gshare functions
void init_gshare()
{
  int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t *)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for (i = 0; i < bht_entries; i++)
  {
    //is this the correct value 
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

uint8_t gshare_predict(uint32_t pc)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  switch (bht_gshare[index])
  {
  case WN:
    return NOTTAKEN;
  case SN:
    return NOTTAKEN;
  case WT:
    return TAKEN;
  case ST:
    return TAKEN;
  default:
    printf("Warning: Undefined state of entry in GSHARE BHT!\n");
    return NOTTAKEN;
  }
}

void train_gshare(uint32_t pc, uint8_t outcome)
{
  // get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries - 1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries - 1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  // Update state of entry in bht based on outcome
  switch (bht_gshare[index])
  {
  case WN:
    bht_gshare[index] = (outcome == TAKEN) ? WT : SN;
    break;
  case SN:
    bht_gshare[index] = (outcome == TAKEN) ? WN : SN;
    break;
  case WT:
    bht_gshare[index] = (outcome == TAKEN) ? ST : WN;
    break;
  case ST:
    bht_gshare[index] = (outcome == TAKEN) ? ST : WT;
    break;
  default:
    printf("Warning: Undefined state of entry in GSHARE BHT!\n");
    break;
  }

  // Update history register
  ghistory = ((ghistory << 1) | outcome);
}

void cleanup_gshare()
{
  free(bht_gshare);
}

void init_predictor()
{
  switch (bpType)
  {
  case STATIC:
    break;
  case GSHARE:
    init_gshare();
    break;
  case TOURNAMENT:
    init_tournament();
    break;
  case CUSTOM:
    break;
  default:
    break;
  }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint32_t make_prediction(uint32_t pc, uint32_t target, uint32_t direct)
{

  // Make a prediction based on the bpType
  switch (bpType)
  {
  case STATIC:
    return TAKEN;
  case GSHARE:
    return gshare_predict(pc);
  case TOURNAMENT:
    return tournament_predict(pc);
  case CUSTOM:
    return NOTTAKEN;
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

void train_predictor(uint32_t pc, uint32_t target, uint32_t outcome, uint32_t condition, uint32_t call, uint32_t ret, uint32_t direct)
{
  if (condition)
  {
    switch (bpType)
    {
    case STATIC:
      return;
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      return train_tournament(pc, outcome);
    case CUSTOM:
      return;
    default:
      break;
    }
  }
}
