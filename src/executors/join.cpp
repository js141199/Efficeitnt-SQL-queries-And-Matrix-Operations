#include "global.h"
/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[5] != "ON")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[3];
    parsedQuery.joinSecondRelationName = tokenizedQuery[4];
    parsedQuery.joinFirstColumnName = tokenizedQuery[6];
    parsedQuery.joinSecondColumnName = tokenizedQuery[8];

    string binaryOperator = tokenizedQuery[7];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.joinBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

void joinOnGreaterThanValues();
void joinOnGreaterEqualValues();

struct metaData
{
    int startPageIndex, endPageIndex;
};

void matchEqualRows(int firstColIndex, int secondColIndex, vector<int>& currentRow, Cursor &secondCursor, Table &secondTable, map<int, metaData>& mp, Table& resultantTable){
    logger.log("matchRows started");

    int secondTotalBlocks = secondTable.blockCount;
    int maxRowsInNewBlock = (BLOCK_SIZE*1000)/((currentRow.size() + secondTable.columnCount)*sizeof(int));


    while(secondCursor.pageIndex < secondTotalBlocks){
        vector<int> secondRow = secondCursor.getNext();
        if(secondRow.empty())
            break;

        if(currentRow[firstColIndex] == secondRow[secondColIndex]){
            secondRow.insert(secondRow.begin(), currentRow.begin(), currentRow.end());
            
            resultantTable.writeRow<int>(secondRow);

            if(!mp.empty() && mp.begin()->first != currentRow[firstColIndex]){
                mp.clear();
            }
            
            if(mp.empty()){
                metaData m;
                m.startPageIndex = secondCursor.pageIndex;
                m.endPageIndex = secondCursor.pageIndex;
                mp[currentRow[firstColIndex]] = m;
            }
            mp[currentRow[firstColIndex]].endPageIndex = max(mp[currentRow[firstColIndex]].endPageIndex, secondCursor.pageIndex);
        }
        else if(currentRow[firstColIndex] < secondRow[secondColIndex]){
            break;
        }
    }
    logger.log("matchRows ended");
}

void joinEqualBlocks(Cursor &firstCursor, Table &firstTable, Table &secondTable, map<int, metaData> &mp, Table& resultantTable){

    logger.log("joinEqualBlocks started");

    int rowsCnt = firstTable.rowsPerBlockCount[firstCursor.pageIndex];
    int firstColIndex = firstTable.getColumnIndex(parsedQuery.joinFirstColumnName);
    int secondColIndex = secondTable.getColumnIndex(parsedQuery.joinSecondColumnName);
    Cursor secondCursor;

    for (int r = 0; r < rowsCnt; r++)
    {
        vector<int> currentRow = firstCursor.getNext();
        if(currentRow.empty())
            break;
        int maxRowsInNewBlock = (BLOCK_SIZE*1000)/((currentRow.size() + secondTable.columnCount)*sizeof(int));

        // If map is empty, start with page 0 of second table
        if(mp.empty()){
            secondCursor = secondTable.getCursor(0, false);
        }
        else if(mp.find(currentRow[firstColIndex]) == mp.end()){
            secondCursor = secondTable.getCursor(mp.begin()->second.endPageIndex, false);
        }
        else{
            secondCursor = secondTable.getCursor(mp.begin()->second.startPageIndex, false);
        }

        matchEqualRows(firstColIndex, secondColIndex, currentRow, secondCursor, secondTable, mp, resultantTable);
        
    }
    logger.log("joinEqualBlocks ended");
}

void joinOnEqualValues()
{
    logger.log("joinOnEqualValues");
    
    parsedQuery.sortRelationName = parsedQuery.joinFirstRelationName;
    parsedQuery.sortColumns = {{parsedQuery.joinFirstColumnName, ASC}};
    executeSORT();

    parsedQuery.sortRelationName = parsedQuery.joinSecondRelationName;
    parsedQuery.sortColumns = {{parsedQuery.joinSecondColumnName, ASC}};
    executeSORT();

    Table *firstTable = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table *secondTable = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);
    
    vector<string> columns;
    columns.insert(columns.end(), firstTable->columns.begin(), firstTable->columns.end());
    columns.insert(columns.end(), secondTable->columns.begin(), secondTable->columns.end());

    Table* resultantTable = new Table(parsedQuery.joinResultRelationName, columns);

    int firstTotalBlocks = firstTable->blockCount;

    map<int, metaData> mp;
    for (int b = 0; b < firstTotalBlocks; b++)
    {
        Cursor cursor = firstTable->getCursor(b, false);
        joinEqualBlocks(cursor, *firstTable, *secondTable, mp, *resultantTable);
    }

    if(resultantTable->blockify())
        tableCatalogue.insertTable(resultantTable);

}

bool matchLessThanRows(int firstColIndex, int secondColIndex, vector<int>& currentRow, Cursor &secondCursor, Table &secondTable, Table& resultantTable){
    logger.log("matchLessThanRows started");

    int secondTotalBlocks = secondTable.blockCount;
    int maxRowsInNewBlock = (BLOCK_SIZE*1000)/((currentRow.size() + secondTable.columnCount)*sizeof(int));
    int matchedCnt = 0;

    while(secondCursor.pageIndex < secondTotalBlocks){
        vector<int> secondRow = secondCursor.getNext();
        if(secondRow.empty())
            break;

        if(currentRow[firstColIndex] < secondRow[secondColIndex]){
            matchedCnt++;
            secondRow.insert(secondRow.begin(), currentRow.begin(), currentRow.end());
            resultantTable.writeRow<int>(secondRow);
        }
        else if(currentRow[firstColIndex] >= secondRow[secondColIndex]){
            break;
        }
    }

    logger.log("matchLessThanRows ended");
    return (matchedCnt > 0);
}

void joinLessThanBlocks(Cursor firstCursor, Table &firstTable, Table &secondTable, Table& resultantTable){

    logger.log("joinLessThanBlocks started");

    int rowsCnt = firstTable.rowsPerBlockCount[firstCursor.pageIndex];
    int firstColIndex = firstTable.getColumnIndex(parsedQuery.joinFirstColumnName);
    int secondColIndex = secondTable.getColumnIndex(parsedQuery.joinSecondColumnName);
    Cursor secondCursor;

    for (int r = 0; r < rowsCnt; r++)
    {
        vector<int> currentRow = firstCursor.getNext();
        if(currentRow.empty())
            break;

        secondCursor = secondTable.getCursor(0, false);
        if(!matchLessThanRows(firstColIndex, secondColIndex, currentRow, secondCursor, secondTable, resultantTable))
            break;
    }
    logger.log("joinLessThanBlocks ended");
}

void joinOnLessThanValues(){
    logger.log("joinOnLessThanValues started");

    Table *firstTable = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table *secondTable = tableCatalogue.getTable(parsedQuery.joinSecondRelationName); 

    vector<string> columns;
    columns.insert(columns.end(), firstTable->columns.begin(), firstTable->columns.end());
    columns.insert(columns.end(), secondTable->columns.begin(), secondTable->columns.end());

    Table* resultantTable = new Table(parsedQuery.joinResultRelationName, columns);

    parsedQuery.sortRelationName = parsedQuery.joinFirstRelationName;
    parsedQuery.sortColumns = {{parsedQuery.joinFirstColumnName, ASC}};
    executeSORT();

    parsedQuery.sortRelationName = parsedQuery.joinSecondRelationName;
    parsedQuery.sortColumns = {{parsedQuery.joinSecondColumnName, DESC}};
    executeSORT();

    int firstTotalBlocks = firstTable->blockCount;

    map<int, metaData> mp;
    for (int b = 0; b < firstTotalBlocks; b++)
    {
        Cursor cursor = firstTable->getCursor(b, false);
        joinLessThanBlocks(cursor, *firstTable, *secondTable, *resultantTable);
    }
    
    if(resultantTable->blockify())
        tableCatalogue.insertTable(resultantTable);

    logger.log("joinOnLessThanValues ended");
}

bool matchLessEqualRows(int firstColIndex, int secondColIndex, vector<int>& currentRow, Cursor &secondCursor, Table &secondTable, Table &resultantTable){
    logger.log("matchLessEqualRows started");

    int secondTotalBlocks = secondTable.blockCount;
    int maxRowsInNewBlock = (BLOCK_SIZE*1000)/((currentRow.size() + secondTable.columnCount)*sizeof(int));
    int matchedCnt = 0;

    while(secondCursor.pageIndex < secondTotalBlocks){
        vector<int> secondRow = secondCursor.getNext();
        if(secondRow.empty())
            break;

        if(currentRow[firstColIndex] <= secondRow[secondColIndex]){
            matchedCnt++;
            secondRow.insert(secondRow.begin(), currentRow.begin(), currentRow.end());
            resultantTable.writeRow<int>(secondRow);
        }
        else if(currentRow[firstColIndex] > secondRow[secondColIndex]){
            break;
        }
    }

    logger.log("matchLessEqualRows ended");
    return (matchedCnt > 0);
}

void joinLessEqualBlocks(Cursor firstCursor, Table &firstTable, Table &secondTable, Table& resultantTable){

    logger.log("joinLessEqualBlocks started");

    int rowsCnt = firstTable.rowsPerBlockCount[firstCursor.pageIndex];
    int firstColIndex = firstTable.getColumnIndex(parsedQuery.joinFirstColumnName);
    int secondColIndex = secondTable.getColumnIndex(parsedQuery.joinSecondColumnName);
    Cursor secondCursor;

    for (int r = 0; r < rowsCnt; r++)
    {
        vector<int> currentRow = firstCursor.getNext();
        if(currentRow.empty())
            break;

        secondCursor = secondTable.getCursor(0, false);
        if(!matchLessEqualRows(firstColIndex, secondColIndex, currentRow, secondCursor, secondTable, resultantTable))
            break;
    }
    logger.log("joinLessThanBlocks ended");
}

void joinOnLessEqualValues(){
    logger.log("joinOnLessEqualValues started");
    
    Table *firstTable = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table *secondTable = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    parsedQuery.sortRelationName = parsedQuery.joinFirstRelationName;
    parsedQuery.sortColumns = {{parsedQuery.joinFirstColumnName, ASC}};
    executeSORT();

    parsedQuery.sortRelationName = parsedQuery.joinSecondRelationName;
    parsedQuery.sortColumns = {{parsedQuery.joinSecondColumnName, DESC}};
    executeSORT();

    vector<string> columns;
    columns.insert(columns.end(), firstTable->columns.begin(), firstTable->columns.end());
    columns.insert(columns.end(), secondTable->columns.begin(), secondTable->columns.end());

    Table* resultantTable = new Table(parsedQuery.joinResultRelationName, columns);

    int firstTotalBlocks = firstTable->blockCount;

    for (int b = 0; b < firstTotalBlocks; b++)
    {
        Cursor cursor = firstTable->getCursor(b, false);
        joinLessEqualBlocks(cursor, *firstTable, *secondTable, *resultantTable);
    }

    if(resultantTable->blockify())
        tableCatalogue.insertTable(resultantTable);

    logger.log("joinOnLessEqualValues ended");
}

bool matchGreaterThanRows(int firstColIndex, int secondColIndex, vector<int>& currentRow, Cursor &secondCursor, Table &secondTable, Table &resultantTable){
    logger.log("matchGreaterThanRows started");

    int secondTotalBlocks = secondTable.blockCount;
    int maxRowsInNewBlock = (BLOCK_SIZE*1000)/((currentRow.size() + secondTable.columnCount)*sizeof(int));
    int matchedCnt = 0;

    while(secondCursor.pageIndex < secondTotalBlocks){
        vector<int> secondRow = secondCursor.getNext();
        if(secondRow.empty())
            break;

        if(currentRow[firstColIndex] > secondRow[secondColIndex]){
            matchedCnt++;
            secondRow.insert(secondRow.begin(), currentRow.begin(), currentRow.end());
            resultantTable.writeRow<int>(secondRow);
        }
        else if(currentRow[firstColIndex] <= secondRow[secondColIndex]){
            break;
        }
    }

    logger.log("matchGreaterThanRows ended");
    return (matchedCnt > 0);
}

void joinGreaterThanBlocks(Cursor &firstCursor, Table &firstTable, Table &secondTable, Table& resultantTable){

    logger.log("joinGreaterThanBlocks started");

    int rowsCnt = firstTable.rowsPerBlockCount[firstCursor.pageIndex];
    int firstColIndex = firstTable.getColumnIndex(parsedQuery.joinFirstColumnName);
    int secondColIndex = secondTable.getColumnIndex(parsedQuery.joinSecondColumnName);
    Cursor secondCursor;

    for (int r = 0; r < rowsCnt; r++)
    {
        vector<int> currentRow = firstCursor.getNext();
        if(currentRow.empty())
            break;
        int maxRowsInNewBlock = (BLOCK_SIZE*1000)/((currentRow.size() + secondTable.columnCount)*sizeof(int));

        secondCursor = secondTable.getCursor(0, false);
        if(!matchGreaterThanRows(firstColIndex, secondColIndex, currentRow, secondCursor, secondTable, resultantTable))
            break;
    }
    logger.log("joinGreaterThanBlocks ended");
}

void joinOnGreaterThanValues(){
    logger.log("joinOnGreaterThanValues started");

    Table *firstTable = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table *secondTable = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    parsedQuery.sortRelationName = parsedQuery.joinFirstRelationName;
    parsedQuery.sortColumns = {{parsedQuery.joinFirstColumnName, DESC}};
    executeSORT();

    parsedQuery.sortRelationName = parsedQuery.joinSecondRelationName;
    parsedQuery.sortColumns = {{parsedQuery.joinSecondColumnName, ASC}};
    executeSORT();

    vector<string> columns;
    columns.insert(columns.end(), firstTable->columns.begin(), firstTable->columns.end());
    columns.insert(columns.end(), secondTable->columns.begin(), secondTable->columns.end());

    Table* resultantTable = new Table(parsedQuery.joinResultRelationName, columns);

    int firstTotalBlocks = firstTable->blockCount;

    for (int b = 0; b < firstTotalBlocks; b++)
    {
        Cursor cursor = firstTable->getCursor(b, false);
        joinGreaterThanBlocks(cursor, *firstTable, *secondTable, *resultantTable);
    }

    if(resultantTable->blockify())
        tableCatalogue.insertTable(resultantTable);

    logger.log("joinOnGreaterThanValues ended");
}

bool matchGreaterEqualRows(int firstColIndex, int secondColIndex, vector<int>& currentRow, Cursor &secondCursor, Table &secondTable, Table &resultantTable){
    logger.log("matchLessEqualRows started");

    int secondTotalBlocks = secondTable.blockCount;
    int maxRowsInNewBlock = (BLOCK_SIZE*1000)/((currentRow.size() + secondTable.columnCount)*sizeof(int));
    int matchedCnt = 0;

    while(secondCursor.pageIndex < secondTotalBlocks){
        vector<int> secondRow = secondCursor.getNext();
        if(secondRow.empty())
            break;

        if(currentRow[firstColIndex] >= secondRow[secondColIndex]){
            matchedCnt++;
            secondRow.insert(secondRow.begin(), currentRow.begin(), currentRow.end());
            resultantTable.writeRow<int>(secondRow);
        }
        else if(currentRow[firstColIndex] < secondRow[secondColIndex]){
            break;
        }
    }

    logger.log("matchLessEqualRows ended");
    return (matchedCnt > 0);
}

void joinGreaterEqualBlocks(Cursor &firstCursor, Table &firstTable, Table &secondTable, map<int, metaData> &mp, Table& resultantTable){

    logger.log("joinGreaterEqualBlocks started");

    int rowsCnt = firstTable.rowsPerBlockCount[firstCursor.pageIndex];
    int firstColIndex = firstTable.getColumnIndex(parsedQuery.joinFirstColumnName);
    int secondColIndex = secondTable.getColumnIndex(parsedQuery.joinSecondColumnName);
    Cursor secondCursor;

    for (int r = 0; r < rowsCnt; r++)
    {
        vector<int> currentRow = firstCursor.getNext();
        if(currentRow.empty())
            break;
        int maxRowsInNewBlock = (BLOCK_SIZE*1000)/((currentRow.size() + secondTable.columnCount)*sizeof(int));

        secondCursor = secondTable.getCursor(0, false);
        if(!matchGreaterEqualRows(firstColIndex, secondColIndex, currentRow, secondCursor, secondTable, resultantTable))
            break;
    }
    logger.log("joinGreaterEqualBlocks ended");
}

void joinOnGreaterEqualValues(){
    logger.log("joinOnGreaterEqualValues started");

    Table *firstTable = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table *secondTable = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    parsedQuery.sortRelationName = parsedQuery.joinFirstRelationName;
    parsedQuery.sortColumns = {{parsedQuery.joinFirstColumnName, DESC}};
    executeSORT();

    parsedQuery.sortRelationName = parsedQuery.joinSecondRelationName;
    parsedQuery.sortColumns = {{parsedQuery.joinSecondColumnName, ASC}};
    executeSORT();

    vector<string> columns;
    columns.insert(columns.end(), firstTable->columns.begin(), firstTable->columns.end());
    columns.insert(columns.end(), secondTable->columns.begin(), secondTable->columns.end());

    Table* resultantTable = new Table(parsedQuery.joinResultRelationName, columns);

    int firstTotalBlocks = firstTable->blockCount;

    map<int, metaData> mp;
    for (int b = 0; b < firstTotalBlocks; b++)
    {
        Cursor cursor = firstTable->getCursor(b, false);
        joinGreaterEqualBlocks(cursor, *firstTable, *secondTable, mp, *resultantTable);
    }

     if(resultantTable->blockify())
        tableCatalogue.insertTable(resultantTable);

    logger.log("joinOnGreaterEqualValues ended");
}

void executeJOIN()
{
    logger.log("executeJOIN");

    string directory_path = "../data/temp"; // Replace with the path to the directory where you want to delete files.
    string prefix = parsedQuery.joinResultRelationName; // Replace with the prefix you want to match.

    bufferManager.deleteFilesWithPrefix(directory_path, prefix);

    Table *firstTable = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table *secondTable = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    // make copy of the file pages
    firstTable->generateCopies("JOIN_TEMP_");
    secondTable->generateCopies("JOIN_TEMP_");

    // return ;
    int totalNewBlocks;

    switch(parsedQuery.joinBinaryOperator){
        case EQUAL:
            joinOnEqualValues();
            break;
        case LESS_THAN:
            joinOnLessThanValues();
            break;
        case LEQ:
            joinOnLessEqualValues();
            break;
        case GREATER_THAN:
            joinOnGreaterThanValues();
            break;
        case GEQ:
            joinOnGreaterEqualValues();
            break;
    }

    prefix = parsedQuery.joinFirstRelationName;
    bufferManager.deleteFilesWithPrefix(directory_path, prefix);

    prefix = parsedQuery.joinSecondRelationName;
    bufferManager.deleteFilesWithPrefix(directory_path, prefix);

    // rename all the temporary pages
    for(int pageIndex = 0; pageIndex < firstTable->blockCount; pageIndex++){
        bufferManager.renameFile(firstTable->tableName, pageIndex, "JOIN_TEMP_");
    }

    for(int pageIndex = 0; pageIndex < secondTable->blockCount; pageIndex++){
        bufferManager.renameFile(secondTable->tableName, pageIndex, "JOIN_TEMP_");
    }
}