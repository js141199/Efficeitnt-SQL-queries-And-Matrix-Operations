#include"logger.h"
/**
 * @brief The Page object is the main memory representation of a physical page
 * (equivalent to a block). The page class and the page.h header file are at the
 * bottom of the dependency tree when compiling files. 
 *<p>
 * Do NOT modify the Page class. If you find that modifications
 * are necessary, you may do so by posting the change you want to make on Moodle
 * or Teams with justification and gaining approval from the TAs. 
 *</p>
 */

class Page{

    string tableName;
    string pageIndex;
    int columnCount;
    int rowCount;
    vector<vector<int>> rows;

    public:

    string pageName = "";
    Page();
    Page(string tableName, int pageIndex);
    Page(int pageIndex, string matrixName);
    Page(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount);
    vector<int> getRow(int rowIndex);
    vector<vector<int>> getRows();
    void writePage();

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
        logger.log("Page::writeRow in file");
        for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
        {
            if (columnCounter != 0)
                fout << " ";
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
    void writeRows(vector<vector<T>> rows)
    {
        logger.log("Page::writeRow open file");
        ofstream fout(this->pageName, ios::trunc);

        for(vector<T> row : rows)
            this->writeRow(row, fout);
            
        fout.close();
    }

};