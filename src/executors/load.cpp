#include "global.h"
/**
 * @brief 
 * SYNTAX: LOAD relation_name
 */
bool syntacticParseLOAD()
{
    logger.log("syntacticParseLOAD");
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

    parsedQuery.queryType =  loadType == "TABLE" ? LOAD : LOAD_MATRIX;
    parsedQuery.loadRelationName = loadType == "TABLE" ? tokenizedQuery[1] : tokenizedQuery[2];
    return true;
}

bool semanticParseLOAD()
{
    logger.log("semanticParseLOAD");

    if (tableCatalogue.isTable(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Relation already exists" << endl;
        return false;
    }

    if (!isFileExists(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }

    return true;
}

bool semanticParseLOAD_MATRIX()
{
    logger.log("semanticParseLOAD_MATRIX");

    if(matrixCatalogue.isMatrix(parsedQuery.loadRelationName)){
        cout << "SEMANTIC ERROR: Matrix already exist" << endl;
        return false;
    }

    if (!isFileExists(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }

    return true;
}


void executeLOAD()
{
    logger.log("executeLOAD");

    Table *table = new Table(parsedQuery.loadRelationName);
    if (table->load())
    {
        tableCatalogue.insertTable(table);
        cout << "Loaded Table. Column Count: " << table->columnCount << " Row Count: " << table->rowCount << endl;
    }
    return;
}
void executeMATRIXLOAD()
{
    logger.log("executeMATRIXLOAD");

    Matrix *matrix = new Matrix(parsedQuery.loadRelationName);

    if (matrix->load())
    {
        matrixCatalogue.insertMatrix(matrix);
        cout << "Loaded Matrix. Column Count: " << matrix->columnCount << " Row Count: " << matrix->rowCount << endl;

        cout << "Number of blocks read: 0" << endl;
        cout << "Number of blocks written: "  << matrix->blockCount << endl;
        cout << "Number of blocks accessed: " <<  matrix->blockCount << endl;
    }
    return;
}