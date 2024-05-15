#include "global.h"
/**
 * @brief 
 * SYNTAX: PRINT relation_name
 */
bool syntacticParsePRINT()
{

    logger.log("syntacticParsePRINT");

    string loadType = tokenizedQuery.size() == 2 ? "TABLE" : tokenizedQuery[1];
    
    if (loadType == "MATRIX"){
        if(tokenizedQuery.size() != 3){
            cout << "SYNTAX ERROR" << endl;
            return false;  
        }
    }
    else if(loadType != "TABLE"){
        cout << "SYNTAX ERROR" << endl;
        return false;  
    }

    parsedQuery.queryType =  loadType == "TABLE" ? PRINT : PRINT_MATRIX;
    parsedQuery.printRelationName = loadType == "TABLE" ? tokenizedQuery[1] : tokenizedQuery[2];
    return true;

}

bool semanticParsePRINT()
{
    logger.log("semanticParsePRINT");

    if (!tableCatalogue.isTable(parsedQuery.printRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    return true;
}

bool semanticParsePRINT_MATRIX()
{
    logger.log("semanticParsePRINT_MATRIX");

    if(!matrixCatalogue.isMatrix(parsedQuery.printRelationName)){
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    return true;
}

void executePRINT()
{
    logger.log("executePRINT");
    Table* table = tableCatalogue.getTable(parsedQuery.printRelationName);
    table->print();
    return;
}

void executeMATRIXPRINT()
{
    logger.log("executeMATRIXPRINT");
    Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.printRelationName);
    matrix->print();
    return;
}
