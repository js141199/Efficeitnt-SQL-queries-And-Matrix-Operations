#include"page.h"

/**
 * @brief The BufferManager is responsible for reading pages to the main memory.
 * Recall that large files are broken and stored as blocks in the hard disk. The
 * minimum amount of memory that can be read from the disk is a block whose size
 * is indicated by BLOCK_SIZE. within this system we simulate blocks by
 * splitting and storing the file as multiple files each of one BLOCK_SIZE,
 * although this isn't traditionally how it's done. You can alternatively just
 * random access to the point where a block begins within the same
 * file. In this system we assume that the the sizes of blocks and pages are the
 * same. 
 * 
 * <p>
 * The buffer can hold multiple pages quantified by BLOCK_COUNT. The
 * buffer manager follows the FIFO replacement policy i.e. the first block to be
 * read in is replaced by the new incoming block. This replacement policy should
 * be transparent to the executors i.e. the executor should not know if a block
 * was previously present in the buffer or was read in from the disk. 
 * </p>
 *
 */
class BufferManager{

    deque<Page> pages; 
    int blockReadCount;
    bool inPool(string pageName);
    Page getFromPool(string pageName);
    Page insertIntoPool(string tableName, int pageIndex);
    Page insertIntoPool(int pageIndex, string matrixName);

public:
    
    BufferManager();
    void setBlockReadCount(int blockReadCount);
    int getBlockReadCount();
    Page getPage(string tableName, int pageIndex);
    Page getPage(int pageIndex, string matrixName);
    void writePage(string pageName, vector<vector<int>> rows);
    void removePage(string pageName);
    void removePage(string tableName, int pageIndex);
    void deleteFile(string tableName, int pageIndex);
    void deleteFile(string fileName);
    void renameFile(string tableName, int pageIndex);
    void renameFile(string srcTableName, string destTableName, int pageIndex);
    void renameFile(string tableName, int pageIndex, string extra);
    void writePage(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount);
    void writeMatrixPage(string tableName, int pageIndex, vector<vector<int>>& rows, int rowCount);
    void clearQueue();
    void deleteFilesWithPrefix(const std::string& directory, const std::string& prefix);
};