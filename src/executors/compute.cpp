#include "global.h"
/**
 * @brief 
 * SYNTAX: COMPUTE relation_name
 */
bool syntacticParseCOMPUTE()
{
    logger.log("syntacticParseCOMPUTE");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = COMPUTE;
    parsedQuery.transposeRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseCOMPUTE()
{
    logger.log("semanticParseCOMPUTE");

    if(!matrixCatalogue.isMatrix(parsedQuery.transposeRelationName)){
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeCOMPUTE()
{
    logger.log("executeCOMPUTE");

    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.transposeRelationName);
    matrix->compute();

    return;
}