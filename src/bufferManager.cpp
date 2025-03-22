#include "global.h"

BufferManager::BufferManager()
{
    logger.log("BufferManager::BufferManager");
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
    if (strncmp(tableName.c_str(), "temp/", 5) == 0) {
        tableName = tableName.substr(5);
    }
    
    string pageName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    
    // Check if page exists in pool
    for (auto& page : this->pages) {
        if (page.pageName == pageName) {
            return page;
        }
    }
    
    // If not in pool, create new page and add to pool
    Page newPage(tableName, pageIndex);
    if (this->pages.size() >= BLOCK_COUNT) {
        this->pages.pop_front();
    }
    this->pages.push_back(newPage);
    return newPage;
}

Page BufferManager::getPage(string matrixName, int pageIndex, int is_matrix)
{
    logger.log("BufferManager::getPage");
    string pageName = "../data/temp/"+matrixName + "_Page" + to_string(pageIndex);
    if (this->inPool(pageName)){
        // return this->getFromPool(pageName);
        this->deletePage(pageName);
        return this->insertIntoPool(matrixName, pageIndex, 1);
    }
    else{
        return this->insertIntoPool(matrixName, pageIndex, 1);
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

Page BufferManager::insertIntoPool(string tableName, int pageIndex, int is_matrix)
{
    logger.log("BufferManager::insertIntoPool");
    Page page(tableName, pageIndex, 1);
    if (this->pages.size() >= BLOCK_COUNT)
        pages.pop_front();
    pages.push_back(page);
    return page;
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
    string pageName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    
    // Create new page
    Page page(tableName, pageIndex, rows, rowCount);
    
    // Write to disk
    page.writePage();
    
    // Update in pool if exists
    for (auto& p : this->pages) {
        if (p.pageName == pageName) {
            p = page;
            return;
        }
    }
    
    // If not in pool, add it
    if (this->pages.size() >= BLOCK_COUNT) {
        this->pages.pop_front();
    }
    this->pages.push_back(page);
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

/**
 * @brief Removes a page from the buffer pool if it exists.
 *
 * @param pageName The name of the page to remove.
 */
void BufferManager::deletePage(string pageName)
{
    logger.log("BufferManager::deletePage");
    for (auto it = this->pages.begin(); it != this->pages.end(); ++it)
    {
        if (it->pageName == pageName)
        {
            this->pages.erase(it);
            logger.log("BufferManager::deletePage: Page removed from buffer");
            return;
        }
    }
    logger.log("BufferManager::deletePage: Page not found in buffer");
}
