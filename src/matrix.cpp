#include "global.h"

/**
 * @brief Construct a new Table:: Table object
 *
 */
Matrix::Matrix()
{
    logger.log("Matrix::Matrix");
}

/**
 * @brief Construct a new Table:: Table object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param matrixName 
 */
Matrix::Matrix(string matrixName)
{
    logger.log("Matrix::Matrix");
    this->sourceFileName = "../data/" + matrixName + ".csv";
    this->matrixName = matrixName;
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates table
 * statistics.
 *
 * @return true if the table has been successfully loaded 
 * @return false if an error occurred 
 */
bool Matrix::load()
{
    logger.log("Matrix::load");
    fstream fin(this->sourceFileName, ios::in);
    string line;
    if (getline(fin, line))
    {
        fin.close();
        if (this->updateMatrixInfo(line))
            if (this->blockify())
                return true;
    }
    fin.close();
    return false;
}

void Matrix::transpose()
{
    logger.log("Matrix::transpose");

    int rowBlocks = ceil(this->rowCount / (double)this->subMatrixDim);
    int colBlocks = ceil(this->columnCount / (double)this->subMatrixDim);
    int blocksWriten = 0;
    bufferManager.setBlockReadCount(0);

    for (int i = 0; i < rowBlocks; i++){
        for (int j = i; j < colBlocks; j++){
            int rowPageIndex = i * colBlocks + j;
            int colPageIndex = j * rowBlocks + i;
            transposeMatrices(rowPageIndex, colPageIndex, blocksWriten) ;
        }
    }

    cout << "Number of blocks read: " << bufferManager.getBlockReadCount() << endl;
    cout << "Number of blocks written: " << blocksWriten << endl;
    cout << "Number of blocks accessed: " << (bufferManager.getBlockReadCount() + blocksWriten) << endl;

    bufferManager.setBlockReadCount(0);

}

void Matrix::transposeMatrices(int rowBlockIndex, int colBlockIndex, int& blocksWriten){
    logger.log("Matrix::transposeMatrices");

    Cursor *c1 = new Cursor(rowBlockIndex, this->matrixName);
    vector<vector<int>> page1 = c1->page.getRows();

    if (rowBlockIndex == colBlockIndex)
    {
        for (int r = 0; r < page1.size(); r++){
            for (int c = r; c < page1[0].size(); c++){
                swap(page1[r][c], page1[c][r]);
            }
        }
    }

    else{
        Cursor *c2 = new Cursor(colBlockIndex, this->matrixName);
        vector<vector<int>> page2 = c2->page.getRows();

        for (int r = 0; r < page1.size(); r++){
            for (int c = 0; c < page1[0].size(); c++){
                swap(page1[r][c], page2[c][r]);
            }
        }
        c2->page.writeRows(page2);
        bufferManager.removePage(c2->page.pageName);
        blocksWriten++;
    }

    c1->page.writeRows(page1);

    bufferManager.removePage(c1->page.pageName);
    blocksWriten++;

}

bool Matrix::checksymmetry()
{
    logger.log("Matrix::checksymmetry");

    int rowBlocks = ceil(this->rowCount / (double)this->subMatrixDim);
    int colBlocks = ceil(this->columnCount / (double)this->subMatrixDim);

    bufferManager.setBlockReadCount(0);

    for (int i = 0; i < rowBlocks; i++){
        for (int j = i; j < colBlocks; j++){
            int rowPageIndex = i * colBlocks + j;
            int colPageIndex = j * rowBlocks + i;
            if(!isBlockSymmetric(rowPageIndex, colPageIndex))
                return false;
        }
    }

    cout << "Number of blocks read: " << bufferManager.getBlockReadCount() << endl;
    cout << "Number of blocks written: 0"  << endl;
    cout << "Number of blocks accessed: " << bufferManager.getBlockReadCount() << endl;

    bufferManager.setBlockReadCount(0);

    return true;
}

bool Matrix::isBlockSymmetric(int rowBlockIndex, int colBlockIndex){
    logger.log("Matrix::isBlockSymmetric");

    Cursor *c1 = new Cursor(rowBlockIndex, this->matrixName);
    vector<vector<int>> page1 = c1->page.getRows();

    if (rowBlockIndex == colBlockIndex)
    {
        for (int r = 0; r < page1.size(); r++){
            for (int c = r; c < page1[0].size(); c++){
                if(page1[r][c] != page1[c][r])
                    return false;
            }
        }
        
    }
    else{
        Cursor *c2 = new Cursor(colBlockIndex, this->matrixName);
        vector<vector<int>> page2 = c2->page.getRows();

        for (int r = 0; r < page1.size(); r++){
            for (int c = 0; c < page1[0].size(); c++){
                if(page1[r][c] != page2[c][r])
                    return false;
            }
        }
    }
    return true;
}

void Matrix::compute(){
    logger.log("Matrix::compute");

    int rowBlocks = ceil(this->rowCount / (double)this->subMatrixDim);
    int colBlocks = ceil(this->columnCount / (double)this->subMatrixDim);

    int blockWritten = 0;

    bufferManager.setBlockReadCount(0);

    for (int i = 0; i < rowBlocks; i++)
    {
        for (int j = 0; j < colBlocks; j++)
        {
            int rowPageIndex = i * colBlocks + j;
            int colPageIndex = j * rowBlocks + i;

            Cursor *c1 = new Cursor(rowPageIndex, this->matrixName);
            vector<vector<int>> matBlock = c1->page.getRows();

            Cursor *c2 = new Cursor(colPageIndex, this->matrixName);
            vector<vector<int>> matBlockTranspose = getBlockTranspose(c2->page.getRows());

            vector<vector<int>> res = calcMatMinusMatTranspose(matBlock, matBlockTranspose);

            c1->page.writeRows(res);
            blockWritten++;

            string newFileName = "../data/temp/" + this->matrixName + "_RESULT_Page" + to_string(rowPageIndex);
            ofstream fout(newFileName, ios::trunc);

            // write one block
            this->writeRowBlockData(res, fout);      

            fout.close();
        }
    }

    Matrix *resultMatrix = new Matrix(this->matrixName + "_RESULT");
    resultMatrix->rowCount = this->rowCount;
    resultMatrix->columnCount = this->columnCount;
    resultMatrix->subMatrixDim = this->subMatrixDim;
    resultMatrix->perBlockDim = this->perBlockDim;
    resultMatrix->blockCount = this->blockCount;

    cout << "Number of blocks read: " << bufferManager.getBlockReadCount() << endl;
    cout << "Number of blocks written: " << blockWritten  << endl;
    cout << "Number of blocks accessed: " << (bufferManager.getBlockReadCount() + blockWritten) << endl;

    matrixCatalogue.insertMatrix(resultMatrix);

    bufferManager.setBlockReadCount(0);
}

vector<vector<int>> Matrix::getBlockTranspose(vector<vector<int>> mat){
    logger.log("Matrix::getBlockTranspose");
    vector<vector<int>> transpose(mat[0].size(), vector<int>(mat.size()));

    for (int i = 0; i < mat.size(); i++){
        for (int j = 0; j < mat[0].size(); j++){
            transpose[j][i] = mat[i][j];
        }
    }
    return transpose;
}

vector<vector<int>> Matrix::calcMatMinusMatTranspose(vector<vector<int>>& matBlock, vector<vector<int>>& matBlockTranspose){
    logger.log("Matrix::calcMatMinusMatTranspose");

    for (int i = 0; i < matBlock.size(); i++){
        for (int j = 0; j < matBlock[0].size(); j++){
            matBlock[i][j] -= matBlockTranspose[i][j];
        }
    }
    return matBlock;
}

/**
 * @brief Function extracts column names from the header line of the .csv data
 * file. 
 *
 * @param line 
 * @return true if column names successfully extracted (i.e. no column name
 * repeats)
 * @return false otherwise
 */
bool Matrix::updateMatrixInfo(string firstLine)
{
    logger.log("Matrix::updateMatrixInfo");

    string word;
    stringstream s(firstLine);
    int columnLength = 0;
    while (getline(s, word, ','))
    {
        word.erase(remove_if(word.begin(), word.end(), ::isspace), word.end());
        if(word == "")
            return false;
        columnLength++;
    }
    this->columnCount = columnLength;
    int maxNumberPerBlock = (unsigned int)(BLOCK_SIZE * 1000) / (sizeof(int));
    this->subMatrixDim = sqrt(maxNumberPerBlock);

    logger.log(to_string(this->columnCount));
    logger.log(to_string(this->subMatrixDim));

    return true;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 * @param rows => rowCounter x columnCount
 * @return true if successfully blockified
 * @return false otherwise
 */
void Matrix::addBlocks(int& rowCounter, vector<vector<int>>& rows) 
{
    int col = 0;
    
    for (col = 0; col < this->columnCount; col += this->subMatrixDim){
        vector<vector<int>> subBlock;
        for (int row = 0; row < rowCounter; row++)
        {
            subBlock.push_back(vector<int>(rows[row].begin() + col, rows[row].begin() + col + fmin(this->subMatrixDim, this->columnCount - col)));
        }
        bufferManager.writeMatrixPage(this->matrixName, this->blockCount, subBlock, rowCounter);    
        this->blockCount++;
        this->perBlockDim.push_back({subBlock.size(), subBlock[0].size()});
    }
    rowCounter = 0;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size. 
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Matrix::blockify()
{
    logger.log("Matrix::blockify");
    ifstream fin(this->sourceFileName, ios::in);
    string line, word;
    vector<vector<int>> rows;
    vector<int> row(this->columnCount);
    int subMaxtrixRowCount = 0;
    int rowCounter = 0;
   
    while (getline(fin, line))
    {
        stringstream s(line);
        
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (!getline(s, word, ','))
                return false;
            row[columnCounter] = stoi(word);
        }
        rowCounter++;
        this->rowCount++;
        rows.push_back(row);

        if (rowCounter == this->subMatrixDim)
        {
            this->addBlocks(rowCounter, rows);
            rows.clear();
        }
        subMaxtrixRowCount++;
    }
    if (rowCounter)
    {
        this->addBlocks(rowCounter, rows);
    }

    if (this->rowCount == 0)
        return false;
    return true;
}


/**
 * @brief Function prints the first few rows of the table. If the table contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Matrix::print()
{
    logger.log("Matrix::print");
    unsigned int rowCount = min((long long)PRINT_COUNT, this->rowCount);
    unsigned int colCount = min((long long)PRINT_COUNT, this->columnCount);

    unsigned int rowBlocksReq = ceil(rowCount / (double)this->subMatrixDim);
    unsigned int colBlocksReq = ceil(colCount / (double)this->subMatrixDim);

    unsigned int totalColBlocks = ceil(this->rowCount/(double)this->subMatrixDim);

    int currRow = 0, rowBlock = 0;

    bufferManager.setBlockReadCount(0);

    for (rowBlock = 0, currRow = 0; rowBlock < rowBlocksReq; rowBlock++, currRow+=this->subMatrixDim)
    {
        int pageIndex = totalColBlocks * rowBlock;
        Cursor cursor(pageIndex, this->matrixName);
        vector<vector<int>> rowBlocksData = cursor.getRowBlocksData(this->perBlockDim[pageIndex].first, colBlocksReq, min(this->subMatrixDim, rowCount - currRow), colCount);
        this->writeRowBlockData(rowBlocksData, cout);
    }

    printRowCount(rowCount);
    
    cout << "Number of blocks read: " << bufferManager.getBlockReadCount() << endl;
    cout << "Number of blocks written: 0"  << endl;
    cout << "Number of blocks accessed: " << bufferManager.getBlockReadCount() << endl;

    bufferManager.setBlockReadCount(0);
}

void Matrix::getNextPage(Cursor *cursor)
{
    logger.log("Matrix::getNextPage");

    if (cursor->pageIndex < this->blockCount - 1)
    {
        cursor->nextMatrixPage(cursor->pageIndex+1);
    }
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Matrix::unload(){
    logger.log("Matrix::~unload");
    for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
        bufferManager.deleteFile(this->matrixName, pageCounter);
}


bool Matrix::exportMatrix(){

    int rowBlocks = ceil(this->rowCount / (double) this->subMatrixDim);
    int colBlocks = ceil(this->columnCount / (double) this->subMatrixDim);

    string newSourceFile = "../data/" + this->matrixName + ".csv";
    ofstream fout(newSourceFile, ios::trunc);

    bufferManager.setBlockReadCount(0);

    for(int startBlockIndex = 0; startBlockIndex < this->blockCount; startBlockIndex += colBlocks){
        Cursor *cursor = new Cursor(startBlockIndex, this->matrixName);
        int totalRows = this->perBlockDim[startBlockIndex].first;
        vector<vector<int>> data = cursor->getRowBlocksData(totalRows, colBlocks, totalRows, this->columnCount);
        this->writeRowBlockData(data, fout);

    }

    fout.close();    

    cout << "Number of blocks read: " << bufferManager.getBlockReadCount() << endl;
    cout << "Number of blocks written: 0"  << endl;
    cout << "Number of blocks accessed: " << bufferManager.getBlockReadCount() << endl;
    
    bufferManager.setBlockReadCount(0);
    
    return true;
}

bool Matrix::renameMatrix(){

    logger.log("Matrix::rename");

    string oldMatrixName = this->matrixName;

    // rename the matrix name in matrix object
    this->matrixName = parsedQuery.renameToRelationName;
    
    matrixCatalogue.removeMatrix(oldMatrixName);

    matrixCatalogue.insertMatrix(this);

    // rename all the old pages in memory
    for(int pageIndex = 0; pageIndex < this->blockCount; pageIndex++){

        string oldPage = "../data/temp/" + oldMatrixName + "_Page" + to_string(pageIndex);
        string renamePage = "../data/temp/" + this->matrixName + "_Page" + to_string(pageIndex);
        if(rename(oldPage.c_str(), renamePage.c_str()) != 0){
            cout << "Error in renaming the page " << pageIndex << endl;
            return false;
        }
    }

    return true;
}
