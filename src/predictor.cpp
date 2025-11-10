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

//Custom Predictor data structures 
uint16_t *b_pred; //base predictor table

uint16_t *t1; //table 1

uint16_t *t2; //table 2

uint16_t *t3; //table 3

uint16_t *t4; //table 4

int counter; //the second half of the global history that we want

uint64_t ghist1; //we need two values to hold all the global history 
uint64_t ghist2; //the second half of the global history that we want

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


//Custom functions 
void init_custom() {

 
  //initiate the local predictor
  int b_entries = 2048;
  b_pred = (uint16_t *)malloc(b_entries * sizeof(uint16_t));

  //initiating to weakly not taken 
  for (int i = 0; i < b_entries; i++)
  {
    b_pred[i] = WN3;
  }
 
  int t_entries = 1024;

  t1 = (uint16_t *)malloc(t_entries * sizeof(uint16_t));
  t2 = (uint16_t *)malloc(t_entries * sizeof(uint16_t));
  t3 = (uint16_t *)malloc(t_entries * sizeof(uint16_t));
  t4 = (uint16_t *)malloc(t_entries * sizeof(uint16_t));


  //init all to 0
    for(int i = 0; i < t_entries; i++){
      t1[i] = 0;
      t2[i] = 0;
      t3[i] = 0;
      t4[i] = 0;
    }

  //set the global history to 0 to begin with 
  ghist1 = 0;
  ghist2 = 0;
  counter = 0;

}

//a function to do the folding XOR that is needed to index with size 10
uint16_t fold_XOR_10(int size, uint32_t pc){

  //get the bottom 10 bits of the PC to start with
  uint16_t get_bottom_10 =  (1 << 10) - 1;
  uint16_t index;
  index = pc & (get_bottom_10);

  uint64_t hist1 = ghist1;
  uint64_t hist2 = ghist2;

  int use = 0;

  //if size less than or equal to 64 only need to use the first history bit
  if (size <= 64) {
    while ((size - use) >= 10) {
      uint16_t bottom_10 = hist1 & (get_bottom_10);
      use += 10;
      index = index ^ bottom_10;
      hist1 = hist1 >> 10; 
    }
    //need to get the last bits 
    index = index ^ (hist1 & ((1 << (size - use)) - 1));
    return index;
  }

  //else we are greater than 64 so we need to deal with the split that is caused do to that 
  else {
     while ((use + 10) <= 64) {
      uint16_t bottom_10 = hist1 & (get_bottom_10);
      use += 10;
      index = index ^ bottom_10;
      hist1 = hist1 >> 10; 
    }
    //the next part need get left that is in the first 64 and the rest from the second
    uint16_t left1 = 64 - use;
    uint16_t left2 = 10 - left1;
    uint16_t part1 = hist1 & ((1 << left1) - 1); // mask low 4 bits of h1
    uint16_t part2 = hist2 & ((1 << left2) - 1); // mask low 6 bits of h2
    index = index ^ ((part2 << left1) | part1);
    hist2 = hist2 >> left2;
    use += 10;

    while ((size - use) >= 10) {
      uint16_t bottom_10 = hist2 & (get_bottom_10);
      use += 10;
      index = index ^ bottom_10;
      hist2 = hist2 >> 10; 
    }
    //need to get the last bits 
    index = index ^ (hist2 & ((1 << (size - use)) - 1));
    return index;
  }
}


//a function to do the folding XOR that is needed to check the tag
uint16_t fold_XOR_9(int size, uint32_t pc){

  //get the bottom 10 bits of the PC to start with
  uint16_t get_bottom_9 =  (1 << 9) - 1;
  uint16_t index;
  index = pc & (get_bottom_9);

  uint64_t hist1 = ghist1;
  uint64_t hist2 = ghist2;

  int use = 0;
  //if size less than or equal to 64 only need to use the first history bit
  if (size <= 64) {
    while ((size - use) >= 9) {
      uint16_t bottom_9 = hist1 & (get_bottom_9);
      use += 9;
      index = index ^ bottom_9;
      hist1 = hist1 >> 9; 
    }
    //need to get the last bits 
    index = index ^ (hist1 & ((1 << (size - use)) - 1));
    return index;
  }

  //else we are greater than 64 so we need to deal with the split that is caused do to that 
  else {
     while ((use + 9) <= 64) {
      uint16_t bottom_9 = hist1 & (get_bottom_9);
      use += 9;
      index = index ^ bottom_9;
      hist1 = hist1 >> 9; 
    }
    //the next part need get left that is in the first 64 and the rest from the second
    uint16_t left1 = 64-use;
    uint16_t left2 = 9 - left1;
    uint16_t part1 = hist1 & ((1 << left1) - 1); // mask low 4 bits of h1
    uint16_t part2 = hist2 & ((1 << left2) - 1); // mask low 6 bits of h2
    index = index ^ ((part2 << left1) | part1);
    hist2 = hist2 >> left2;
    use += 9;

    while ((size - use) >= 9) {
      uint16_t bottom_9 = hist2 & (get_bottom_9);
      use += 9;
      index = index ^ bottom_9;
      hist2 = hist2 >> 9; 
    }
    //need to get the last bits 
    index = index ^ (hist2 & ((1 << (size - use)) - 1));
    return index;
  }
}

uint8_t prediction(int pred){
   //if between 0 and 3 inclsuive you are not taken
  if (pred >= 0 && pred < 4){
    return NOTTAKEN;
  }

  //if between 4 and 8 inclusive you are taken
  else if (pred >= 4 && pred < 8){
    return TAKEN;
  }

  else {
    printf("Warning: Undefined state of entry in custom predictor!\n");
    return NOTTAKEN;
  }

}


uint8_t custom_predict(uint32_t pc, uint16_t *source)
{
  //if just go from largest to smallest looking for a tag hit will do this in the correct order 
  
  //get tag and index for 128 bits of history 
  int index128 = fold_XOR_10(128, pc);
  int tag4 = fold_XOR_9(128, pc);
  uint16_t tagBits = (t4[index128] >> 2) & ((1 << 9) - 1);

  //if tag in 128bits of history return that 
  if (tagBits == tag4) {
    int pred = (t4[index128] >> 11) & ((1 << 3) - 1);
    *source = 128;
    return prediction(pred);
  }

  //get tag and index for 64 bits of history 
  int index64 = fold_XOR_10(64, pc);
  int tag3 = fold_XOR_9(64, pc);
  tagBits = (t3[index64] >> 2) & ((1 << 9) - 1);

  //if tag in 64 bits return that 
  if (tagBits == tag3) {
    int pred = (t3[index64] >> 11) & ((1 << 3) - 1);
    *source = 64;
    return prediction(pred);
  }

  //get tag and history for 32 bits of history 
  int index32 = fold_XOR_10(32, pc);
  int tag2 = fold_XOR_9(32, pc);
  tagBits = (t2[index32] >> 2) & ((1 << 9) - 1);

  //if tag in 32 bits return that 
  if (tagBits == tag2) {
    int pred = (t2[index32] >> 11) & ((1 << 3) - 1);
    *source = 32;
    return prediction(pred);
  }

  //get tag and history for 16 bits of history 
  int index16 = fold_XOR_10(16, pc);
  int tag1 = fold_XOR_9(16, pc);
  tagBits = (t1[index16] >> 2) & ((1 << 9) - 1);

  //if tag in 16 bits return that 
  if (tagBits == tag1) {
    int pred = (t1[index16] >> 11) & ((1 << 3) - 1);
    *source = 16;
    return prediction(pred);
  }


//else just use the base predictor 

int pred = b_pred[pc & ((1 << 11) -1)];
*source = 0;
return prediction(pred);
}

uint8_t updatePred(uint8_t pred, uint8_t outcome){
  switch (pred)
        {
        case SN3:
          pred = (outcome == TAKEN) ? MN3 : SN3;
          break;
        case MN3:
          pred = (outcome == TAKEN) ? SLN3 : SN3;
          break;
        case SLN3:
          pred = (outcome == TAKEN) ? WN3 : MN3;
          break;
        case WN3:
          pred = (outcome == TAKEN) ? WT3 : SLN3;
          break;
        case WT3:
          pred = (outcome == TAKEN) ? SLT3 : WN3;
          break;
        case SLT3:
          pred = (outcome == TAKEN) ? MT3 : WT3;
          break;
        case MT3:
          pred = (outcome == TAKEN) ? ST3 : SLT3;
          break;
        case ST3:
          pred = (outcome == TAKEN) ? ST3 : MT3;
          break;
        default:
          printf("Warning: Undefined state of entry in train custom!\n");
          break;
        }

    return pred;
}

int allocate(uint16_t *table, uint16_t size, uint32_t pc, uint8_t outcome){
 //try to allocate to table above
      int index = fold_XOR_10(size, pc);
      int newRow = 0;
      int row = table[index];

      //get the usefullness
      int useful = table[index] & ((1 << 2) - 1);
      if (useful == 0){
        //then we replace the row 
        //either weakly taken or weakly not taken based on what it does 
        int pred = (outcome == TAKEN) ? WT3 : WN3;
        int tag = fold_XOR_9(size, pc);
        newRow = newRow | (pred << 11);
        newRow = newRow | (tag << 2);
        //update the newRow
        table[index] = newRow;
        return 1;
      }

      else {
        useful -= 1;
        row = row & ~((1 << 2) - 1);      // clear bottom 2 bits
        newRow = row | useful;
        table[index] = newRow;
        return 0;
      }

    }

void train_custom(uint32_t pc, uint8_t outcome) {
  //now have to do the training
  //will have to essentially redo prediction in order to get the values to update for this or not
  //need to know the prediction as well as which value was predicted
    uint16_t src;
    uint8_t prediction = custom_predict(pc, &src);

    uint16_t *table;

    //get what table was used based on what the prediction returned
    switch(src){
      case 0:
        table = b_pred;
        break;
      case 16:
        table = t1;
        break;
      case 32:
        table = t2;
        break;
      case 64:
        table = t3;
        break;
      case 128:
        table = t4;
        break;
    }

    //if correct update usefulness bit up else update it down and always update the 3 bit counter
    //so this always happens and then the second part is to allocate if the prediction is incorrect
    uint16_t row;
    uint16_t index;
    uint8_t useful;
    uint8_t pred;
    if (src != 0){
      index = fold_XOR_10(src, pc);

      row = table[index];

      useful = row & ((1 << 2) - 1);

      //update usefullness up if correct and down if incorrect
      if (prediction == outcome){
        //since 2 bits largest it can be is 3
        if (useful < 3){
          useful += 1;
        }
      }

      //else update it down
      else {
        if (useful > 0){
          useful -= 1;
        }
      }

      //now need to decide how to update the actual prediction with is 3 values 
      pred = (row >> 11) & ((1 << 3) - 1);

      pred = updatePred(pred, outcome);

      //now need to actually do the update 
      row = row & ~((1 << 2) - 1);      // clear bottom 2 bits
      row = row & ~(7 << 11);       // clear bits 11-13

      //now that these values are 0 can just or the values i want in
      row = row | useful;
      row = row | (pred << 11);

      //update the table
      table[index] = row;

    }

    //else you are using the base predictor and update this one
    else {
      index = pc & ((1 << 11) - 1);
      pred = table[index];
      pred = updatePred(pred, outcome);
      table[index] = pred;
    }


    //in this case prediction does not equal outcome need to do allocation

    //lets start without probability and just trying to allocate to one table above 
    uint16_t newRow = 0;

    if (prediction != outcome){
      if (src == 64){
        allocate(t4, 128, pc, outcome);
      }

      //here have the change to do the 
      if (src == 32){
         if (allocate(t3, 64, pc, outcome) == 0){
             allocate(t4, 128, pc, outcome);
         }
      }

      if (src == 16){
        if (allocate(t2, 32, pc, outcome) == 0){
          if (allocate(t3, 64, pc, outcome) == 0){
              allocate(t4, 128, pc, outcome);
          }
      }
    }

      if (src == 0){
        if (allocate(t1, 16, pc, outcome) == 0){
          if (allocate(t2, 32, pc, outcome) == 0){
            if (allocate(t3, 64, pc, outcome) == 0){
                allocate(t4, 128, pc, outcome);
            }
        }
      }
  }
}
  

    //update global history 
    ghist2 = (ghist2 << 1) | (1 & (ghist1 >> 63));
    ghist1 = (ghist1 << 1) | (outcome & 1);

    uint16_t mask;
    //next see if we need to update the counter
    if (counter == 256000){
      //if 256000 we are going to go update all of the usefulness counter
      mask = ~1;

      for(int i = 0; i < 1024; i++){
        t1[i] = t1[i] & mask;
        t2[i] = t2[i] & mask;
        t3[i] = t3[i] & mask;
        t4[i] = t4[i] & mask;
      }

    }

    if (counter == 512000){

       mask = ~(1 << 1);

      for(int i = 0; i < 1024; i++){
        t1[i] = t1[i] & mask;
        t2[i] = t2[i] & mask;
        t3[i] = t3[i] & mask;
        t4[i] = t4[i] & mask;
      }

      counter = 0;
    }

    counter += 1;

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
    init_custom();
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
    uint16_t src;
    return custom_predict(pc, &src);
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
      return train_custom(pc, outcome);
    default:
      break;
    }
  }
}
