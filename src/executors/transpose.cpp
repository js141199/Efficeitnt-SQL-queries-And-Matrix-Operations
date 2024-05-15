#include "global.h"
/**
 * @brief 
 * SYNTAX: TRANSPOSE relation_name
 */
bool syntacticParseTRANSPOSE()
{
    logger.log("syntacticParseTRANSPOSE");
    if (tokenizedQuery[1] != "MATRIX" || tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = TRANSPOSE;
    parsedQuery.transposeRelationName = tokenizedQuery[2];
    return true;
}

bool semanticParseTRANSPOSE()
{
    logger.log("semanticParseTRANSPOSE");

    if(!matrixCatalogue.isMatrix(parsedQuery.transposeRelationName)){
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeMATRIXTRANSPOSE()
{
    logger.log("executeMATRIXTRANSPOSE");

    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.transposeRelationName);
    matrix->transpose();
    cout << "Matrix Transpose Done Successfully" << endl;

    return;
}