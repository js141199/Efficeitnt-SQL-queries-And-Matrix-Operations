#include"semanticParser.h"

void executeCommand();

void executeCLEAR();
void executeCROSS();
void executeDISTINCT();
void executeEXPORT();
void executeINDEX();
void executeJOIN();
void executeLIST();
void executeLOAD();
void executeMATRIXLOAD();
void executeMATRIXTRANSPOSE();
void executeCHECKSYMMETRY();
void executeEXPORTMATRIX();
void executeCOMPUTE();
void executePRINT();
void executeMATRIXPRINT();
void executePROJECTION();
void executeRENAME();
void executeRENAME_MATRIX();
void executeSELECTION();
void executeSORT();
void executeSOURCE();
void executeORDERBY();
void executeGROUPBY();

bool evaluateBinOp(int value1, int value2, BinaryOperator binaryOperator);
void printRowCount(int rowCount);