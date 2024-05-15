#include "cursor.h"

/**
 * @brief The Table class holds all information related to a loaded table. It
 * also implements methods that interact with the parsers, executors, cursors
 * and the buffer manager. There are typically 2 ways a table object gets
 * created through the course of the workflow - the first is by using the LOAD
 * command and the second is to use assignment statements (SELECT, PROJECT,
 * JOIN, SORT, CROSS and DISTINCT). 
 *
 */
class Matrix
{

public:
    string sourceFileName = "";
    string matrixName = "";
    long long int columnCount = 0;
    long long int rowCount = 0;
    unsigned int blockCount = 0;
    unsigned int subMatrixDim = 0; // d x d
    vector<pair<unsigned int, unsigned int>> perBlockDim; // pair<row, col>

 
    Matrix();
    Matrix(string matrixName);

    bool load();
    void transpose();
    void transposeMatrices(int rowBlockIndex, int colBlockIndex, int& blocksWriten);
    bool isBlockSymmetric(int rowBlockIndex, int colBlockIndex);
    void compute();
    vector<vector<int>> getBlockTranspose(vector<vector<int>> mat);
    vector<vector<int>> calcMatMinusMatTranspose(vector<vector<int>> &matBlock, vector<vector<int>> &matBlockTranspose);
    bool blockify();
    bool updateMatrixInfo(string firstLine);
    void addBlocks(int& rowCounter, vector<vector<int>>& rows);
    void print();
    void getNextPage(Cursor *cursor);
    void unload();
    bool checksymmetry();
    bool exportMatrix();
    bool renameMatrix();

    template <typename T>
    void writeRowBlockData(vector<vector<T>> row, ostream &fout)
    {
        logger.log("Matrix::writeRowBlockData");

        for (int r = 0; r < row.size(); r++){
            for (int c = 0; c < row[0].size(); c++){
                if (c != 0)
                    fout << ", ";
                fout << row[r][c];
            }
            fout << endl;
        }
    }

    /**
     * @brief Static function that takes a vector of valued and prints them out in a
     * comma seperated format.
     *
     * @tparam T current usaages include int and string
     * @param row
     */
    template <typename T>
    void writeRow(vector<T> row, ostream &fout)
    {
        logger.log("Matrix::printRow");
        for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
        {
            if (columnCounter != 0)
                fout << ", ";
            fout << row[columnCounter];
        }
        fout << endl;
    }

    /**
    * @brief Static function that takes a vector of valued and prints them out in a
    * comma seperated format.
    *
    * @tparam T current usaages include int and string
    * @param row 
    */
    template <typename T>
    void writeRow(vector<T> row)
    {
        logger.log("Matrix::printRow");
        ofstream fout(this->sourceFileName, ios::app);
        this->writeRow(row, fout);
        fout.close();
    }
};