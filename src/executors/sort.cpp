#include"global.h"
/**
 * @brief File contains method to process SORT commands.
 * 
 * syntax:
 * R <- SORT relation_name BY column_name IN sorting_order
 * 
 * sorting_order = ASC | DESC 
 * 
 * SORT TABLE_NAME BY COL1, COL2 IN ASC, DESC
 * 0     1         2    3     4   5  6    7 
 * 
 */
bool syntacticParseSORT(){
    logger.log("syntacticParseSORT");

    int numTokens = tokenizedQuery.size();
    int numColumns = (numTokens - 4) / 2;

    if(numTokens < 6 || tokenizedQuery[2] != "BY" || tokenizedQuery[numTokens - numColumns - 1] != "IN"){
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    vector<pair<string, SortingStrategy>> sortColumns(numColumns);

    for(int pos = 3; pos < numTokens - numColumns - 1; pos++){
        if(tokenizedQuery[pos + numColumns + 1] != "ASC" && tokenizedQuery[pos + numColumns + 1] != "DESC"){
            cout<<"SYNTAX ERROR"<<endl;
            return false;
        }
        sortColumns[pos - 3] = {tokenizedQuery[pos],
                        tokenizedQuery[pos + numColumns + 1] == "ASC" ? ASC : DESC };
        logger.log("Start");
        logger.log(tokenizedQuery[pos] + " " + tokenizedQuery[pos + numColumns + 1]);
    }

    logger.log("tokenization done");

    parsedQuery.queryType = SORT;
    parsedQuery.sortRelationName = tokenizedQuery[1];
    parsedQuery.sortColumns = sortColumns; 
    
    return true;
}

bool semanticParseSORT(){
    logger.log("semanticParseSORT");

    // Result table is not needed
    // if(tableCatalogue.isTable(parsedQuery.sortResultRelationName)){
    //     cout<<"SEMANTIC ERROR: Resultant relation already exists"<<endl;
    //     return false;
    // }

    if(!tableCatalogue.isTable(parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    // Need to check for all the columns
    // if(!tableCatalogue.isColumnFromTable(parsedQuery.sortColumnNames, parsedQuery.sortRelationName)){
    //     cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
    //     return false;
    // }

    Table table = *tableCatalogue.getTable(parsedQuery.sortRelationName);
    for (auto col : parsedQuery.sortColumns)
    {
        if (!table.isColumn(col.first))
        {
            cout << "SEMANTIC ERROR: Column doesn't exist in relation";
            return false;
        }
    }

    return true;
}

bool individualBlockComparator(const vector<int>& r1, const vector<int>& r2, vector<pair<int, SortingStrategy>>& sortingOrder) {

    for(auto it: sortingOrder){
        if(r1[it.first] != r2[it.first]){
            if (it.second == DESC)
                return r1[it.first] > r2[it.first];
            else
                return r1[it.first] < r2[it.first];
        }
    }
    return false;
}

void sortIndividualBlocks(){

    logger.log("sortIndividualBlocks started...");

    Table *table = tableCatalogue.getTable(parsedQuery.sortRelationName);

    Cursor currentPage = table->getCursor();

    int pageNo = 0;

    int columnToSortBy;
    SortingStrategy order;

    int sortColumnsCount = parsedQuery.sortColumns.size();

    do {
        int blockRows = table->rowsPerBlockCount[pageNo];
        vector<vector<int>> rows(blockRows);

        for(int r = 0; r < blockRows; r++){
            rows[r] = currentPage.getNext();
        }
        vector<pair<int, SortingStrategy>> sortingOrder;

        // sort the rows
        for (int c = 0; c < sortColumnsCount; c++){
            // sort the rows based on given column index
            columnToSortBy = table->getColumnIndex(parsedQuery.sortColumns[c].first);
            order = parsedQuery.sortColumns[c].second;
            sortingOrder.push_back({columnToSortBy, order});
            logger.log("Col : " + parsedQuery.sortColumns[c].first + " order :" + to_string(parsedQuery.sortColumns[c].second));
        }
        stable_sort(rows.begin(), rows.end(), [&](const auto &v1, const auto &v2) {
            return individualBlockComparator(v1, v2, sortingOrder);
        });

        // dump rows back to page
        currentPage.page.writeRows(rows);
        
        // remove the page from the deque of buffer manager.
        bufferManager.removePage(table->tableName, pageNo++);

        table->getNextPage(&currentPage);

    } while(pageNo < table->blockCount);

    logger.log("sortIndividualBlocks completed...");
}

struct pageComparator {
    vector<pair<int, SortingStrategy>> sortingOrder;

    pageComparator(const vector<pair<int, SortingStrategy>>& order) : sortingOrder(order) {}

    bool operator()(pair<Cursor, int> p1, pair<Cursor,int> p2) {
        vector<int> row1 = p1.first.getCurrentRow();
        vector<int> row2 = p2.first.getCurrentRow();

        for (auto it = sortingOrder.begin(); it != sortingOrder.end(); it++)
        {
            if(it->second == DESC) {
                if (row1[it->first] != row2[it->first]){
                    return row1[it->first] < row2[it->first];
                }
            }
            else{
                if(row1[it->first] != row2[it->first]){
                    return row1[it->first] > row2[it->first];
                }
            }
        }
        return p1.first.pageIndex > p2.first.pageIndex;
    }
};

void mergeLevelWiseBlocks(const vector<pair<Cursor, int>>& cursorList, const int& pageStart){
    logger.log("Inside mergeLevelWiseBlocks started...");
    vector<vector<int>> rows;
    int pageWriteIndex = pageStart;

    int sortColumnsCount = parsedQuery.sortColumns.size();
    Table *table = tableCatalogue.getTable(parsedQuery.sortRelationName);

    vector<pair<int, SortingStrategy>> sortingOrder;
    for (int c = 0; c < sortColumnsCount; c++)  {
        // sort the rows based on given column index
        int columnToSortBy = table->getColumnIndex(parsedQuery.sortColumns[c].first);
        SortingStrategy order = parsedQuery.sortColumns[c].second;
        sortingOrder.push_back({columnToSortBy, order});
    }
    priority_queue<pair<Cursor, int>, vector<pair<Cursor, int>>, pageComparator> pq(sortingOrder);
    
    for(auto it: cursorList)    pq.push({it.first, it.second});

    while(!pq.empty()){
        pair<Cursor, int> page = pq.top();
        pq.pop();

        bool isLastRow = page.second == 1;
        rows.push_back(page.first.getNextRow(isLastRow));

        if (rows.size() == table->maxRowsPerBlock)
        {
            logger.log("Printing Page");
            for (auto it : rows)
            {
                string intString = std::accumulate(it.begin(), it.end(), std::string(), [](const std::string& a, int b) -> std::string {
                    return a + (a.length() > 0 ? ", " : "") + std::to_string(b);
                });
                logger.log(intString);
            }
            Page newPage("TEMP_" + table->tableName, pageWriteIndex, rows, rows.size()); // INTERMEDIATE PAGES
            newPage.writePage();
            rows.clear();
            pageWriteIndex++;
        }
        page.second--;
        if(page.second > 0)
            pq.push(page);
    }
    if(!rows.empty()) {
        Page newPage(table->tableName, pageWriteIndex, rows, rows.size());
        newPage.writePage();
        pageWriteIndex++;
    }
    logger.log("Inside mergeLevelWiseBlocks ended...");
}

void mergeBlocks(){
    Table *table = tableCatalogue.getTable(parsedQuery.sortRelationName);

    int totalBlocks = table->blockCount;
    int totalLevels = ceil(log(totalBlocks) / (double)(log(BLOCK_COUNT - 1)));

    logger.log("totalBlocks : " + to_string(totalBlocks));
    logger.log("totalLevels : " + to_string(totalLevels));

    vector<int> prefixRowCount(totalBlocks);
    partial_sum(table->rowsPerBlockCount.begin(), table->rowsPerBlockCount.end(), prefixRowCount.begin());

    logger.log("PRINTING prefixRowCount");
    for (auto it : prefixRowCount)
    {
        logger.log(to_string(it) + " ");
    }

    for(int lev = 0; lev < totalLevels; lev++){
        int pagePtr = 0, pageStart = 0;
        logger.log("----Level----: " + to_string(lev));
        bufferManager.clearQueue();
        while (pagePtr < totalBlocks)
        {
            logger.log("----Start Page----: " + to_string(pagePtr));
            logger.log("Inside inner while");
            vector<pair<Cursor, int>> cursorList;

            pageStart = pagePtr;
            int cursors = BLOCK_COUNT - 1;
            int mergeBlockCount = pow(BLOCK_COUNT - 1, lev);

            while(pagePtr < totalBlocks && cursors--){
                logger.log("----Iteration----: " + to_string(cursors) + " PagePtr: "+ to_string(pagePtr));
                Cursor cursor = table->getCursor(pagePtr, true);
                logger.log("----CURSOR PAGE INDEX----: " + to_string(cursor.pageIndex));

                int lastPageIndex = fmin(pagePtr + mergeBlockCount - 1, totalBlocks - 1);
                int prevPageIndex = pagePtr - 1;
                int rowsToBeTaken = prefixRowCount[lastPageIndex] - (prevPageIndex < 0 ? 0 : (prefixRowCount[prevPageIndex]));

                logger.log("lastPageIndex: " + to_string(lastPageIndex) + " prevPageIndex: "+ to_string(prevPageIndex) + " rowsToBeTaken: "+ to_string(rowsToBeTaken));
                cursorList.push_back({cursor, rowsToBeTaken});
                pagePtr += mergeBlockCount;
                logger.log("Inside innermost while ended");
            }

            mergeLevelWiseBlocks(cursorList, pageStart);
        }
        for (int pageIndex = 0; pageIndex < totalBlocks; pageIndex++)
            bufferManager.renameFile(table->tableName, pageIndex);
    }
}

void executeSORT() {
    
    logger.log("executeSORT started...");

    // Step-1 sort rows of individual block by given columns 
    sortIndividualBlocks();
    bufferManager.clearQueue();

    // Step-2 Merge sorted blocks level by level
    mergeBlocks();
    bufferManager.clearQueue();

    logger.log("executeSORT completed...");

    return;
}