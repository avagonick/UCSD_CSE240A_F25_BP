//========================================================//
//  predictor.h                                           //
//  Header file for the Branch Predictor                  //
//                                                        //
//  Includes function prototypes and global predictor     //
//  variables and defines                                 //
//========================================================//

#ifndef PREDICTOR_H
#define PREDICTOR_H

#include <stdint.h>
#include <stdlib.h>

//
// Student Information
//
extern const char *studentName;
extern const char *studentID;
extern const char *email;

//------------------------------------//
//      Global Predictor Defines      //
//------------------------------------//
#define NOTTAKEN 0
#define TAKEN 1

// The Different Predictor Types
#define STATIC 0
#define GSHARE 1
#define TOURNAMENT 2
#define CUSTOM 3
extern const char *bpName[];

// Definitions for 2-bit counters
#define SN 0 // predict NT, strong not taken
#define WN 1 // predict NT, weak not taken
#define WT 2 // predict T, weak taken
#define ST 3 // predict T, strong taken

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//
extern int ghistoryBits; // Number of bits used for Global History
extern int lhistoryBits; // Number of bits used for Local History
extern int pcIndexBits;  // Number of bits used for PC index
extern int bpType;       // Branch Prediction Type
extern int verbose;

//------------------------------------//
//    Predictor Function Prototypes   //
//------------------------------------//

// Initialize the predictor
//
void init_predictor();

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint32_t make_prediction(uint32_t pc, uint32_t target, uint32_t direct);

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void train_predictor(uint32_t pc, uint32_t target, uint32_t outcome, uint32_t condition, uint32_t call, uint32_t ret, uint32_t direct);

// Please add your code below, and DO NOT MODIFY ANY OF THE CODE ABOVE
// 


//initialize 3 bit counter
#define SN3 0 //strongly not taken 
#define MN3 1 //mildly not taken
#define SLN3 2 //slightly not taken
#define WN3 3 //weakly not taken
#define WT3 4 //weakly taken
#define SLT3 5 //slightly taken
#define MT3 6 //mildly taken
#define ST3 7 //strongly taken


//define values for choice preidctor 
#define LS 0 //local strongly taken
#define LW 1 //local weakly taken
#define GW 2 //global weakly taken
#define GS 3 //global strongly taken 


#endif
