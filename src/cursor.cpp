#include "global.h"
Cursor::Cursor()
{
}

Cursor::Cursor(string tableName, int pageIndex)
{
    logger.log("Cursor::Cursor");
    this->page = bufferManager.getPage(tableName, pageIndex);
    this->pagePointer = 0;
    this->tableName = tableName;
    this->pageIndex = pageIndex;
}

Cursor::Cursor(int pageIndex, string matrixName)
{
    logger.log("Cursor::Cursor");
    this->page = bufferManager.getPage(pageIndex, matrixName);
    this->pagePointer = 0;
    this->tableName = matrixName;
    this->pageIndex = pageIndex;
}

/**
 * @brief This function reads the next row from the page. The index of the
 * current row read from the page is indicated by the pagePointer(points to row
 * in page the cursor is pointing to).
 *
 * @return vector<int> 
 */
vector<int> Cursor::getNext()
{
    logger.log("Cursor::getNext start");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    if(result.empty()){
        tableCatalogue.getTable(this->tableName)->getNextPage(this);
        if(!this->pagePointer){
            result = this->page.getRow(this->pagePointer);
            this->pagePointer++;
        }
    }
    logger.log("Cursor::getNext end");
    return result;
}

vector<int> Cursor::getNextRow(bool isLastRow)
{
    logger.log("Cursor::getNextRow");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++; // RECORD INDEX
    if(this->pagePointer >= tableCatalogue.getTable(this->tableName)->rowsPerBlockCount[this->pageIndex] && !isLastRow){
        tableCatalogue.getTable(this->tableName)->getNextPage(this);
        bufferManager.deleteFile(this->tableName, this->pageIndex);
    }
    return result;
}

vector<int> Cursor::getCurrentRow()
{
    logger.log("Cursor::getCurrentRow rowIndex: " + to_string(this->pagePointer) + " pageIndex: " + to_string(this->pageIndex));
    return this->page.getRow(this->pagePointer);
}

vector<int> Cursor::getNextMatrix()
{
    logger.log("Cursor::getNextMatrix");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    if(result.empty()){
        matrixCatalogue.getMatrix(this->tableName)->getNextPage(this);
        if(!this->pagePointer){
            result = this->page.getRow(this->pagePointer);
            this->pagePointer++;
        }
    }
    return result;
}
/**
 * @brief Function that loads Page indicated by pageIndex. Now the cursor starts
 * reading from the new page.
 *
 * @param pageIndex 
 */
void Cursor::nextPage(int pageIndex)
{
    logger.log("Cursor::nextPage");
    this->page = bufferManager.getPage(this->tableName, pageIndex);
    this->pageIndex = pageIndex;
    this->pagePointer = 0;
}

void Cursor::nextMatrixPage(int pageIndex)
{
    logger.log("Cursor::nextMatrixPage");
    this->page = bufferManager.getPage(pageIndex, this->tableName);
    this->pageIndex = pageIndex;
    this->pagePointer = 0;
}

/**
 * @brief Get the Row Blocks Data object
 * 
 * @param startPageIndex 
 * @param endPageIndex 
 * @param totalRows number of rows we need for a given page
 * @param totalColumns number of columns we need across all pages from startPageIndex to endPageIndex
 * @return vector<vector<int>> 
 */
vector<vector<int>> Cursor::getRowBlocksData(int rowsPerBlock, int colBlocksReq, int totalRows, int totalCols){
    vector<vector<int>> subMatrix(rowsPerBlock);

    for (int r = 0; r < rowsPerBlock * colBlocksReq; r++){
        vector<int> row = this->getNextMatrix();
        subMatrix[r % rowsPerBlock].insert(subMatrix[r % rowsPerBlock].end(), row.begin(), row.end());

        // removing extra columns
        if (subMatrix[r%rowsPerBlock].size() > totalCols) subMatrix[r%rowsPerBlock].resize(totalCols);
    }

    if(subMatrix.size() > totalRows) subMatrix.resize(totalRows);
    return subMatrix;
}