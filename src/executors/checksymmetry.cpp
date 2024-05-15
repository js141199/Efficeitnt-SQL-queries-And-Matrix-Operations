#include "global.h"
/**
 * @brief 
 * SYNTAX: CHECKSYMMETRY relation_name
 */
bool syntacticParseCHECKSYMMETRY()
{
    logger.log("syntacticParseCHECKSYMMETRY");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = CHECKSYMMETRY;
    parsedQuery.transposeRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseCHECKSYMMETRY()
{
    logger.log("semanticParseCHECKSYMMETRY");

    if(!matrixCatalogue.isMatrix(parsedQuery.transposeRelationName)){
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeCHECKSYMMETRY()
{
    logger.log("executeCHECKSYMMETRY");

    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.transposeRelationName);
    if(matrix->checksymmetry()){
        cout << "TRUE" << endl;
    }
    else{
        cout << "FALSE" << endl;
    }
    return;
}