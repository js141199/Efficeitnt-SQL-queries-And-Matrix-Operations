#include "global.h"
/**
 * @brief 
 * SYNTAX: <new_table> <- GROUP BY <grouping_attribute> FROM <table_name> HAVING
<aggregate(attribute)> <bin_op> <attribute_value> RETURN
<aggregate_func(attribute)>
 */

string extractColumnName(string input) {
    size_t start = input.find('(');
    size_t end = input.find(')');

    if (start != string::npos && end != string::npos && end > start + 1) {
        return input.substr(start + 1, end - start - 1);
    } else {
        return "Error";
    }
}

bool syntacticParseGROUPBY() {
    logger.log("syntacticParseGROUPBY");

    if(tokenizedQuery.size() != 13 || tokenizedQuery[3] != "BY" || tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "HAVING" || tokenizedQuery[11] != "RETURN"){
        cout << "SYNTAX ERROR - query" << endl;
        cout << tokenizedQuery.size() << " " << tokenizedQuery[3] << " " << tokenizedQuery[5] << " " << tokenizedQuery[7] << " " << tokenizedQuery[11] << endl;  
        return false;
    }

    size_t pos1 = tokenizedQuery[8].find('(');
    size_t pos2 = tokenizedQuery[12].find('(');
    
    if (pos1 != string::npos && pos2 != string::npos) {

        string havingAGG = tokenizedQuery[8].substr(0 , pos1);
        string returnAGG = tokenizedQuery[12].substr(0 , pos2);

        if(havingAGG != "MAX" && havingAGG != "MIN" && havingAGG != "AVG" && havingAGG != "SUM") {
            cout << "SYNTAX ERROR - 'having' Aggregate function: " << havingAGG << endl;
            return false;
        }
        if(returnAGG != "MAX" && returnAGG != "MIN" && returnAGG != "AVG" && returnAGG != "SUM") {
            cout << "SYNTAX ERROR - 'return' Aggregate function: " << returnAGG << endl;
            return false;
        }

        parsedQuery.havingAGG = havingAGG;
        parsedQuery.returnAGG = returnAGG;
    }
    else {
        cout << "SYNTAX ERROR - can't find Aggregate functions" << endl;
        return false;
    }

    parsedQuery.queryType = GROUP_BY;
    parsedQuery.groupByResultantRelationName = tokenizedQuery[0];
    parsedQuery.sortRelationName = tokenizedQuery[6];

    parsedQuery.sortColumns = { {tokenizedQuery[4], ASC} };

    parsedQuery.groupByColumn = tokenizedQuery[4];
    parsedQuery.havingColumn = extractColumnName(tokenizedQuery[8]);
    parsedQuery.returnColumn = extractColumnName(tokenizedQuery[12]);

    logger.log("Column for GroupBy : " + parsedQuery.groupByColumn);

    return true;
}

bool semanticParseGROUPBY() {
    logger.log("semanticParseGROUPBY");

    if (tableCatalogue.isTable(parsedQuery.groupByResultantRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists"  << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist - " << parsedQuery.sortRelationName << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupByColumn, parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: GroupBy Column doesn't exist in relation - " << parsedQuery.groupByColumn << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.havingColumn, parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: Having Column doesn't exist in relation - " << parsedQuery.havingColumn << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.returnColumn, parsedQuery.sortRelationName))
    {
        cout << "SEMANTIC ERROR: Return Column doesn't exist in relation - " << parsedQuery.returnColumn << endl;
        return false;
    }

    return true;
}

void executeGROUPBY() {

    logger.log("executeGROUPBY started...");


    Table *table = tableCatalogue.getTable(parsedQuery.sortRelationName);
    // removing the header
    // resultantTable->removeRow();

    int groupByIdx = table->getColumnIndex(parsedQuery.groupByColumn);
    int havingColumnIdx = table->getColumnIndex(parsedQuery.havingColumn);
    int returnColumnIdx = table->getColumnIndex(parsedQuery.returnColumn);
   
    vector<string> resColumns{table->columns[groupByIdx], parsedQuery.returnAGG + table->columns[returnColumnIdx]};
    Table *resultantTable = new Table(parsedQuery.groupByResultantRelationName, resColumns);
    
    table->generateCopies("GROUP_BY_TEMP_");

    executeSORT();


    string pagePath = "../data/temp/" + parsedQuery.sortRelationName + "_Page";

    int minReturn = INT_MAX , maxReturn = INT_MIN , sumReturn = 0 , countReturn = 0;

    // cout << parsedQuery.havingAGG << endl;

    if (parsedQuery.havingAGG == "MAX" || parsedQuery.havingAGG == "MIN") {
        
        int maxHaving = INT_MIN , minHaving = INT_MAX , prev = -1;

        for(int pageIndex = 0; pageIndex < table->blockCount; pageIndex++){
            ifstream page(pagePath + to_string(pageIndex)); // Open input file for reading

            string line;
            vector<int> row;
            while (getline(page, line)) {
                stringstream ss(line);
                string element;
                while (ss >> element) {
                    row.push_back(stoi(element));
                }

                // if new group encountered
                if(row[groupByIdx] != prev){

                    if(prev != -1) {
                        double checkValue = ((parsedQuery.havingAGG == "MIN") ? minHaving : maxHaving);
                        int required = stoi(tokenizedQuery[10]);

                        // write into the table only if having condition is satisfied
                        if((tokenizedQuery[9] == ">" && checkValue > required) ||
                        (tokenizedQuery[9] == "<" && checkValue < required) ||
                        (tokenizedQuery[9] == ">=" && checkValue >= required) ||
                        (tokenizedQuery[9] == "<=" && checkValue <= required) ||
                        (tokenizedQuery[9] == "==" && checkValue == required)) {

                            if(parsedQuery.returnAGG == "MIN") 
                                resultantTable->writeRow<int>({prev , minReturn});

                            else if(parsedQuery.returnAGG == "MAX") 
                                resultantTable->writeRow<int>({prev , maxReturn});

                            else if(parsedQuery.returnAGG == "SUM")
                                resultantTable->writeRow<int>({prev , sumReturn});

                            else 
                                resultantTable->writeRow<int>({prev , sumReturn / countReturn});
                        }
                    }

                    // updating values for next iteration
                    prev = row[groupByIdx];
                    minHaving = row[havingColumnIdx] , maxHaving = row[havingColumnIdx];
                    countReturn = 1;
                    minReturn = row[returnColumnIdx] , maxReturn = row[returnColumnIdx] , sumReturn = row[returnColumnIdx];
                
                }
                
                else {
                    minHaving = min(minHaving , row[havingColumnIdx]);
                    maxHaving = max(maxHaving , row[havingColumnIdx]);

                    // updating return value
                    minReturn = min(minReturn , row[returnColumnIdx]);
                    maxReturn = max(maxReturn , row[returnColumnIdx]);
                    sumReturn += row[returnColumnIdx];
                    countReturn++;
                }
                row.clear();
            }

            page.close();
        }

        // checking for the case when all pages are ended
        double checkValue = ((parsedQuery.havingAGG == "MIN") ? minHaving : maxHaving);
        int required = stoi(tokenizedQuery[10]);

        // write into the table only if having condition is satisfied
        if((tokenizedQuery[9] == ">" && checkValue > required) ||
        (tokenizedQuery[9] == "<" && checkValue < required) || 
        (tokenizedQuery[9] == ">=" && checkValue >= required) ||
        (tokenizedQuery[9] == "<=" && checkValue <= required) ||
        (tokenizedQuery[9] == "==" && checkValue == required)) {
            if(parsedQuery.returnAGG == "MIN") 
                resultantTable->writeRow<int>({prev , minReturn});
            else if(parsedQuery.returnAGG == "MAX") 
                resultantTable->writeRow<int>({prev , maxReturn});
            else if(parsedQuery.returnAGG == "SUM") 
                resultantTable->writeRow<int>({prev , sumReturn});
            else 
                resultantTable->writeRow<int>({prev , sumReturn / countReturn});
        }
    }

    else {

        int sumHaving = 0 , countHaving = 0 , prev = -1;

        for(int pageIndex = 0; pageIndex < table->blockCount; pageIndex++){
            ifstream page(pagePath + to_string(pageIndex)); // Open input file for reading

            string line;
            vector<int> row;
            while (getline(page, line)) {
                stringstream ss(line);
                string element;
                while (ss >> element) {
                    row.push_back(stoi(element));
                }

                // if new group encountered
                if(row[groupByIdx] != prev){

                    if(prev != -1) {
                        double checkValue = ((parsedQuery.havingAGG == "AVG") ? (sumHaving * 1.0) / countHaving : sumHaving);
                        int required = stoi(tokenizedQuery[10]);

                        // write into the table only if having condition is satisfied
                        if((tokenizedQuery[9] == ">" && checkValue > required) ||
                        (tokenizedQuery[9] == "<" && checkValue < required) ||
                        (tokenizedQuery[9] == ">=" && checkValue >= required) ||
                        (tokenizedQuery[9] == "<=" && checkValue <= required) ||
                        (tokenizedQuery[9] == "==" && checkValue == required)) {

                            if(parsedQuery.returnAGG == "MIN") 
                                resultantTable->writeRow<int>({prev , minReturn});

                            else if(parsedQuery.returnAGG == "MAX") 
                                resultantTable->writeRow<int>({prev , maxReturn});

                            else if(parsedQuery.returnAGG == "SUM")
                                resultantTable->writeRow<int>({prev , sumReturn});

                            else 
                                resultantTable->writeRow<int>({prev , sumReturn / countReturn});
                        }
                    }

                    // updating values for next iteration
                    prev = row[groupByIdx];
                    sumHaving = row[havingColumnIdx];
                    countHaving = 1 , countReturn = 1;
                    minReturn = row[returnColumnIdx] , maxReturn = row[returnColumnIdx] , sumReturn = row[returnColumnIdx];
                
                }
                
                else {
                    sumHaving += row[havingColumnIdx];
                    countHaving++;

                    // updating return value
                    minReturn = min(minReturn , row[returnColumnIdx]);
                    maxReturn = max(maxReturn , row[returnColumnIdx]);
                    sumReturn += row[returnColumnIdx];
                    countReturn++;
                }
                row.clear();
            }

            page.close();
        }

        // checking for the case when all pages are ended
        double checkValue = ((parsedQuery.havingAGG == "AVG") ? (sumHaving * 1.0) / countHaving : sumHaving);
        int required = stoi(tokenizedQuery[10]);

        // write into the table only if having condition is satisfied
        if((tokenizedQuery[9] == ">" && checkValue > required) ||
        (tokenizedQuery[9] == "<" && checkValue < required) ||
        (tokenizedQuery[9] == ">=" && checkValue >= required) ||
        (tokenizedQuery[9] == "<=" && checkValue <= required) ||
        (tokenizedQuery[9] == "==" && checkValue == required)) {
            if(parsedQuery.returnAGG == "MIN") 
                resultantTable->writeRow<int>({prev , minReturn});
            else if(parsedQuery.returnAGG == "MAX") 
                resultantTable->writeRow<int>({prev , maxReturn});
            else if(parsedQuery.returnAGG == "SUM") 
                resultantTable->writeRow<int>({prev , sumReturn});
            else 
                resultantTable->writeRow<int>({prev , (sumReturn / countReturn)});
        }
    }
    if(resultantTable->blockify())
        tableCatalogue.insertTable(resultantTable);

    // rename the real pages of original table as resultant page
    for(int pageIndex = 0; pageIndex < resultantTable->blockCount; pageIndex++){
        bufferManager.renameFile(resultantTable->tableName, parsedQuery.groupByResultantRelationName, pageIndex);
    }

    // rename all the temporary pages
    for(int pageIndex = 0; pageIndex < table->blockCount; pageIndex++){
        bufferManager.renameFile(table->tableName, pageIndex, "GROUP_BY_TEMP_");
    }
    

    logger.log("executeGROUPBY ended...");
}

