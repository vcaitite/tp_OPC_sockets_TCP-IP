#include "global_variables.h"

/* 
* Script where global system variables are defined.
* Autor: V�tor Gabriel Reis Caitit� &
*        Joao Pedro Antunes Ferreira
*/

position_parameters_t positionParameters;
loading_parameters_t loadingParameters;
int countMessages = 0;
char dataMsg[POSITION_MSG_LENGHT] = "55|NNNNN|NNNNN|NNNNN|NNNNN|NNNN.NN";
char ackMsg[ACK_MSG_LENGHT] = "00|NNNNN";

std::mutex mtx_LoadingParameters;     
std::mutex mtx_PositioningParameters;// mutex for critical section
std::mutex mtx;
bool SHOULD_WRITE = false;
int seq_number = 0;
wchar_t* ITEM_IDS[ITEMS_QUANTITY] = { L"Random.UInt1", L"Random.UInt2", L"Random.UInt4", L"Saw-toothed Waves.Real4", L"Bucket Brigade.UInt1", L"Bucket Brigade.Real4" };