#include "global.h"
/**
 * @brief 
 * SYNTAX: RENAME column_name TO column_name FROM relation_name
 */
bool syntacticParseRENAME()
{
    logger.log("syntacticParseRENAME");

    if(tokenizedQuery[1] == "MATRIX"){
        if(tokenizedQuery.size() != 4){
            cout << "SYNTAX ERROR" << endl;
            return false;
        }
        parsedQuery.queryType =  RENAME_MATRIX;
        parsedQuery.renameToRelationName = tokenizedQuery[3];
        parsedQuery.renameRelationName =  tokenizedQuery[2];
    }
    
    else if (tokenizedQuery.size() != 6 || tokenizedQuery[2] != "TO" || tokenizedQuery[4] != "FROM")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    else{
        parsedQuery.queryType = RENAME;
        parsedQuery.renameFromColumnName = tokenizedQuery[1];       
        parsedQuery.renameToColumnName = tokenizedQuery[3];
        parsedQuery.renameRelationName =  tokenizedQuery[5];
    }
    
    return true;
}

bool semanticParseRENAME()
{
    logger.log("semanticParseRENAME");

    if (!tableCatalogue.isTable(parsedQuery.renameRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.renameFromColumnName, parsedQuery.renameRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }

    if (tableCatalogue.isColumnFromTable(parsedQuery.renameToColumnName, parsedQuery.renameRelationName))
    {
        cout << "SEMANTIC ERROR: Column with name already exists" << endl;
        return false;
    }

    return true;
}

bool semanticParseRENAME_MATRIX(){

    logger.log("semanticParseRENAME_MATRIX");

    if(!matrixCatalogue.isMatrix(parsedQuery.renameRelationName)){
        cout << "SEMANTIC ERROR:: Matrix does not exist" << endl;
        return false;
    }
    else if(matrixCatalogue.isMatrix(parsedQuery.renameToRelationName)){
        cout << "SEMANTIC ERROR:: Matrix with new name already exist" << endl;
        return false;
    }

    return true;
}

void executeRENAME()
{
    logger.log("executeRENAME");
    Table* table = tableCatalogue.getTable(parsedQuery.renameRelationName);
    table->renameColumn(parsedQuery.renameFromColumnName, parsedQuery.renameToColumnName);
    return;
}

void executeRENAME_MATRIX(){
    logger.log("executeRENAME_MATRIX");

    Matrix *matrix = matrixCatalogue.getMatrix(parsedQuery.renameRelationName);

    if(matrix->renameMatrix()){
        cout << "Renaming done successfully";
        return ;
    }

    cout << "Error:: renaming not done";
}