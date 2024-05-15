#include "global.h"

/**
 * @brief 
 * SYNTAX: EXPORT <relation_name> 
 */

bool syntacticParseEXPORT()
{

    logger.log("syntacticParseEXPORT");
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

    parsedQuery.queryType =  loadType == "TABLE" ? EXPORT : EXPORT_MATRIX;
    parsedQuery.exportRelationName = loadType == "TABLE" ? tokenizedQuery[1] : tokenizedQuery[2];
    return true;

}

bool semanticParseEXPORT()
{
    logger.log("semanticParseEXPORT");
    
    if (!tableCatalogue.isTable(parsedQuery.exportRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    return true;
}


bool semanticParseEXPORT_MATRIX()
{
    logger.log("semanticParseEXPORT");

    if(!matrixCatalogue.isMatrix(parsedQuery.exportRelationName)){
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeEXPORT()
{
    logger.log("executeEXPORT");
    Table* table = tableCatalogue.getTable(parsedQuery.exportRelationName);
    table->makePermanent();
    return;
}

void executeEXPORTMATRIX()
{
    logger.log("executeEXPORTMATRIX");

    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.exportRelationName);
    
    if(matrix->exportMatrix()){
        logger.log("Export Matrix done successfully for Matrix " + parsedQuery.exportRelationName);
    }
    else{
        logger.log("ERROR:: export matrix not done for matrix " + parsedQuery.exportRelationName);
    }

}