#include "global.h"

Cursor::Cursor(string tableName, int pageIndex)
{
    logger.log("Cursor::Cursor");
    this->page = bufferManager.getPage(tableName, pageIndex);
    this->pagePointer = 0;
    this->tableName = tableName;
    this->pageIndex = pageIndex;
}

Cursor::Cursor(string matrixName, int pageIndex, int is_matrix)
{
    logger.log("Cursor :: MatrixCursor");
    this -> page = bufferManager.getPage(matrixName, pageIndex, 1);
    this->pagePointer = 0;
    this -> tableName = matrixName;
    this -> pageIndex = pageIndex;
    this -> is_it_matrix = 1;
}

vector<int> Cursor::getNextPageRow() {
    logger.log("Cursor::geNext");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    // if (result.empty())
    // {
    //     tableCatalogue.getTable(this->tableName)->getNextPage(this);
    //     if (!this->pagePointer)
    //     {
    //         result = this->page.getRow(this->pagePointer);
    //         this->pagePointer++;
    //     }
    // }
    return result;
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
    logger.log("Cursor::geNext");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    if(result.empty()){
        if (this->is_it_matrix == 1)
        {
            // Its a matrix
            matrixCatalogue.getmatrix(this->tableName)->getNextPage(this);
        }else{
            tableCatalogue.getTable(this->tableName)->getNextPage(this);
        }
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
    if (this->is_it_matrix == 1)
        this->page = bufferManager.getPage(this->tableName, pageIndex, 1);
    else
        this->page = bufferManager.getPage(this->tableName, pageIndex);
    this->pageIndex = pageIndex;
    this->pagePointer = 0;
}