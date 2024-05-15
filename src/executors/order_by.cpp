#include "global.h"
/**
 * @brief 
 * SYNTAX: <new_table> <- ORDER BY <attribute> ASC|DESC ON <table_name>
 */
bool syntacticParseORDERBY()
{
    logger.log("syntacticParseORDERBY");
    
    if(tokenizedQuery.size() != 8 || tokenizedQuery[3] != "BY" || tokenizedQuery[6] != "ON"){
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    string orderType = tokenizedQuery[5];

    if(!(orderType != "ASC" || orderType != "DESC")){
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.queryType = ORDER_BY;
    parsedQuery.sortRelationName = tokenizedQuery[7];
    parsedQuery.sortColumns = { {tokenizedQuery[4], orderType == "ASC" ? ASC : DESC} };
    parsedQuery.orderByResultantRelationName = tokenizedQuery[0];

    logger.log("Column name : " + parsedQuery.sortColumns[0].first);

    return true;
}

bool semanticParseORDERBY()
{
    logger.log("semanticParseORDERBY");

    if (tableCatalogue.isTable(parsedQuery.orderByResultantRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.sortColumns[0].first, parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}


/**
 * @brief 
 * Step-1: Create new resultant table using copy constructor
 * Step-2: Add data of each page into the csv file
 * Step-3: Add table into the table catelog 
 * 
 */
void convertPagesToFile(Table* table){

    logger.log("convertPagesToFile started...");

    Table *resultantTable = new Table(parsedQuery.orderByResultantRelationName, table->columns);
    
    resultantTable->blockCount = table->blockCount;
    resultantTable->rowCount = table->rowCount;
    resultantTable->rowsPerBlockCount = table->rowsPerBlockCount;
        
    string pagePath = "../data/temp/" + parsedQuery.orderByResultantRelationName + "_Page";

    for(int pageIndex = 0; pageIndex < resultantTable->blockCount; pageIndex++){
        ifstream page(pagePath + to_string(pageIndex)); // Open input file for reading

        string line;
        vector<int> row;
        while (getline(page, line)) {
            stringstream ss(line);
            string element;
            while (ss >> element) {
                row.push_back(stoi(element));
            }
            resultantTable->writeRow<int>(row);
            row.clear();
        }

        page.close();
    }

    tableCatalogue.insertTable(resultantTable);

    logger.log("convertPagesToFile ended...");
}

void executeORDERBY(){

    logger.log("executeORDERBY started...");

    string directory_path = "../data/temp"; // Replace with the path to the directory where you want to delete files.
    string prefix = parsedQuery.orderByResultantRelationName; // Replace with the prefix you want to match.

    bufferManager.deleteFilesWithPrefix(directory_path, prefix);

    Table *table = tableCatalogue.getTable(parsedQuery.sortRelationName);

    table->generateCopies("ORDER_BY_TEMP_");

    executeSORT();

    // rename the real pages of original table as resultant page
    for(int pageIndex = 0; pageIndex < table->blockCount; pageIndex++){
        bufferManager.renameFile(table->tableName, parsedQuery.orderByResultantRelationName, pageIndex);
    }

    // convert pages to single csv file
    convertPagesToFile(table);

    // rename all the temporary pages
    for(int pageIndex = 0; pageIndex < table->blockCount; pageIndex++){
        bufferManager.renameFile(table->tableName, pageIndex, "ORDER_BY_TEMP_");
    }
    
    logger.log("executeORDERBY ended...");
}
