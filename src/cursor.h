#include"bufferManager.h"
/**
 * @brief The cursor is an important component of the system. To read from a
 * table, you need to initialize a cursor. The cursor reads rows from a page one
 * at a time.
 *
 */
class Cursor{
    public:
    Page page;
    int pageIndex;
    string tableName;
    int pagePointer;

    public:
        Cursor();
        Cursor(string tableName, int pageIndex);
        Cursor(int pageIndex, string matrixName);
        vector<int> getNext();
        vector<int> getNextRow(bool isLastRow);
        vector<int> getCurrentRow();
        vector<int> getNextMatrix();
        void nextPage(int pageIndex);
        void nextMatrixPage(int pageIndex);
        vector<vector<int>> getPageData();
        vector<vector<int>> getRowBlocksData(int rowsPerBlock, int colBlocksReq, int totalRows, int totalCols);
};