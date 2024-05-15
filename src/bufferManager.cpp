#include "global.h"

BufferManager::BufferManager()
{
    logger.log("BufferManager::BufferManager");
    this->blockReadCount = 0;
}

void BufferManager::setBlockReadCount(int blockReadCount){
    this->blockReadCount = blockReadCount;
}

int BufferManager::getBlockReadCount(){
    return this->blockReadCount;
}

/**
 * @brief Function called to read a page from the buffer manager. If the page is
 * not present in the pool, the page is read and then inserted into the pool.
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page 
 */
Page BufferManager::getPage(string tableName, int pageIndex)
{
    logger.log("BufferManager::getPage");
    string pageName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    if (this->inPool(pageName))
        return this->getFromPool(pageName);
    else
        return this->insertIntoPool(tableName, pageIndex);
}

Page BufferManager::getPage(int pageIndex, string matrixName)
{
    logger.log("BufferManager::getPage overloaded");
    string pageName = "../data/temp/"+matrixName + "_Page" + to_string(pageIndex);
    if (this->inPool(pageName))
        return this->getFromPool(pageName);
    else{
        this->blockReadCount++;
        return this->insertIntoPool(pageIndex, matrixName);
    }
}

/**
 * @brief Checks to see if a page exists in the pool
 *
 * @param pageName 
 * @return true 
 * @return false 
 */
bool BufferManager::inPool(string pageName)
{
    logger.log("BufferManager::inPool");
    for (auto page : this->pages)
    {
        if (pageName == page.pageName)
            return true;
    }
    return false;
}

/**
 * @brief If the page is present in the pool, then this function returns the
 * page. Note that this function will fail if the page is not present in the
 * pool.
 *
 * @param pageName 
 * @return Page 
 */
Page BufferManager::getFromPool(string pageName)
{
    logger.log("BufferManager::getFromPool");
    for (auto page : this->pages)
        if (pageName == page.pageName)
            return page;
}

/**
 * @brief Inserts page indicated by tableName and pageIndex into pool. If the
 * pool is full, the pool ejects the oldest inserted page from the pool and adds
 * the current page at the end. It naturally follows a queue data structure. 
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page 
 */
Page BufferManager::insertIntoPool(string tableName, int pageIndex)
{
    logger.log("BufferManager::insertIntoPool");
    Page page(tableName, pageIndex);
    if (this->pages.size() >= BLOCK_COUNT)
        pages.pop_front();
    pages.push_back(page);
    return page;
}

Page BufferManager::insertIntoPool(int pageIndex, string matrixName)
{
    logger.log("BufferManager::insertIntoPool overloaded");
    Page page(pageIndex, matrixName);
    if (this->pages.size() >= BLOCK_COUNT)
        pages.pop_front();
    pages.push_back(page);
    return page;
}

void BufferManager::removePage(string pageName){

    if(this->inPool(pageName)){
        int count = 0;
        int index = -1;
        for(auto it : pages){
            if(it.pageName == pageName){
                index = count;
                break;
            }
            count++;
        }
        if(index != -1){
            this->pages.erase(this->pages.begin() + index);
        }
    }
}

/**
 * @brief The buffer manager is also responsible for writing pages. This is
 * called when new tables are created using assignment statements.
 *
 * @param tableName 
 * @param pageIndex 
 * @param rows 
 * @param rowCount 
 */
void BufferManager::writePage(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount)
{
    logger.log("BufferManager::writePage");
    Page page(tableName, pageIndex, rows, rowCount);
    page.writePage();
}

/**
 * @brief The buffer manager is also responsible for writing pages. This is
 * called when new tables are created using assignment statements.
 *
 * @param tableName 
 * @param pageIndex 
 * @param rows 
 * @param rowCount 
 */
void BufferManager::writeMatrixPage(string matrixName, int pageIndex, vector<vector<int>>& rows, int rowCount)
{
    logger.log("BufferManager::writeMatrixPage");
    Page page(matrixName, pageIndex, rows, rowCount);
    page.writePage();
}

/**
 * @brief Deletes file names fileName
 *
 * @param fileName 
 */
void BufferManager::deleteFile(string fileName)
{
    
    if (remove(fileName.c_str()))
        logger.log("BufferManager::deleteFile: Err");
        else logger.log("BufferManager::deleteFile: Success");
}

/**
 * @brief Overloaded function that calls deleteFile(fileName) by constructing
 * the fileName from the tableName and pageIndex.
 *
 * @param tableName 
 * @param pageIndex 
 */
void BufferManager::deleteFile(string tableName, int pageIndex)
{
    logger.log("BufferManager::deleteFile");
    string fileName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    this->deleteFile(fileName);
}

void BufferManager::renameFile(string tableName, int pageIndex)
{
    logger.log("BufferManager::renameFile");
    string oldFileName = "../data/temp/TEMP_"+tableName + "_Page" + to_string(pageIndex);
    string newFileName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    rename(oldFileName.c_str(), newFileName.c_str());
}


void BufferManager::renameFile(string tableName, int pageIndex, string extra)
{
    logger.log("BufferManager::renameFile");
    string oldFileName = "../data/temp/" + extra + tableName + "_Page" + to_string(pageIndex);
    string newFileName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    rename(oldFileName.c_str(), newFileName.c_str());
}

void BufferManager::renameFile(string srcTableName, string destTableName, int pageIndex)
{
    logger.log("BufferManager::renameFile");
    string oldFileName = "../data/temp/" + srcTableName + "_Page" + to_string(pageIndex);
    string newFileName = "../data/temp/" + destTableName + "_Page" + to_string(pageIndex);
    rename(oldFileName.c_str(), newFileName.c_str());
}


void BufferManager::removePage(string tableName, int pageIndex){
    logger.log("BufferManager::removePage");
    string pageName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    removePage(pageName);
}


void BufferManager::clearQueue(){
    pages.clear();
}

void BufferManager::deleteFilesWithPrefix(const std::string& directory, const std::string& prefix) {
    DIR* dir = opendir(directory.c_str());
    if (!dir) {
        std::cerr << "Error opening directory" << std::endl;
        return;
    }

    dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename = entry->d_name;
        if (filename.find(prefix) == 0) {
            std::string fullPath = directory + "/" + filename;
            if (remove(fullPath.c_str()) == 0){}
        }
    }

    closedir(dir);
}

