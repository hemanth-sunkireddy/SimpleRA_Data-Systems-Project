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

    public:

    vector<vector<int>> rows;
    string pageName = "";
    Page();
    Page(string tableName, int pageIndex);
    Page(string tableName, int pageIndex, int is_matrix);
    Page(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount);
    vector<int> getRow(int rowIndex);
    void updateRow(int rowIndex, vector<int> newRow);
    vector<vector<int>> getAllRows();
    int getrowcount();
    void writePage();
};