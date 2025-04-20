#include "global.h"

BufferManager::BufferManager()
{
    logger.log("BufferManager::BufferManager");
    // Initialize the set of tables being indexed as empty
    tablesBeingIndexed.clear();
}

/**
 * @brief Mark a table as being indexed to prevent its pages from being evicted
 * 
 * @param tableName The name of the table being indexed
 */
void BufferManager::markTableAsBeingIndexed(string tableName)
{
    logger.log("BufferManager::markTableAsBeingIndexed");
    // cout << "DEBUG: Marking table " << tableName << " as being indexed" << endl;
    tablesBeingIndexed.insert(tableName);
}

/**
 * @brief Unmark a table as being indexed
 * 
 * @param tableName The name of the table no longer being indexed
 */
void BufferManager::unmarkTableAsBeingIndexed(string tableName)
{
    logger.log("BufferManager::unmarkTableAsBeingIndexed");
    // cout << "DEBUG: Unmarking table " << tableName << " as being indexed" << endl;
    tablesBeingIndexed.erase(tableName);
}

/**
 * @brief Check if a table is currently being indexed
 * 
 * @param tableName The name of the table to check
 * @return true if the table is being indexed, false otherwise
 */
bool BufferManager::isTableBeingIndexed(string tableName)
{
    logger.log("BufferManager::isTableBeingIndexed");
    return tablesBeingIndexed.find(tableName) != tablesBeingIndexed.end();
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
    
    // cout << "DEBUG: BufferManager::getPage - Requesting page: " << pageName << endl;
    // cout << "DEBUG: Current buffer size: " << this->pages.size() << "/" << BLOCK_COUNT << endl;
    
    // Check if page exists in pool
    for (auto& page : this->pages) {
        if (page.pageName == pageName) {
            // cout << "DEBUG: Page found in buffer pool: " << pageName << endl;
            // Move the page to the end of the deque to mark it as recently used
            Page foundPage = page;
            // We'll move it to the end later after removing it
            return foundPage;
        }
    }
    
    // cout << "DEBUG: Page not found in buffer pool, creating new page: " << pageName << endl;
    
    // If not in pool, create new page and add to pool
    try {
        Page newPage(tableName, pageIndex);
        
        // If buffer is full, we need to evict a page
        if (this->pages.size() >= BLOCK_COUNT) {
            // Check if the page we're about to evict is an index page
            string frontPageName = this->pages.front().pageName;
            bool isIndexPage = frontPageName.find("_index") != string::npos || 
                              frontPageName.find("_bptree") != string::npos;
            
            // Extract the table name from the page name
            string frontPageTableName = frontPageName;
            size_t pos = frontPageTableName.find("_Page");
            if (pos != string::npos) {
                frontPageTableName = frontPageTableName.substr(11, pos - 11); // 11 is the length of "../data/temp/"
            }
            
            // Check if the table is currently being indexed
            bool isTableBeingIndexed = this->isTableBeingIndexed(frontPageTableName);
            
            // cout << "DEBUG: Buffer full, need to evict a page" << endl;
            // cout << "DEBUG: Oldest page is: " << frontPageName << endl;
            // cout << "DEBUG: Is index page: " << (isIndexPage ? "YES" : "NO") << endl;
            // cout << "DEBUG: Is from table being indexed: " << (isTableBeingIndexed ? "YES" : "NO") << endl;
            
            // If it's an index page or from a table being indexed, try to find a page to evict that is neither
            if (isIndexPage || isTableBeingIndexed) {
                bool foundPageToEvict = false;
                for (auto it = this->pages.begin(); it != this->pages.end(); ++it) {
                    string currentPageName = it->pageName;
                    
                    // Check if this page is an index page
                    bool isCurrentIndexPage = currentPageName.find("_index") != string::npos || 
                                             currentPageName.find("_bptree") != string::npos;
                    
                    // Extract the table name from the current page name
                    string currentPageTableName = currentPageName;
                    size_t pos = currentPageTableName.find("_Page");
                    if (pos != string::npos) {
                        currentPageTableName = currentPageTableName.substr(11, pos - 11); // 11 is the length of "../data/temp/"
                    }
                    
                    // Check if the current table is being indexed
                    bool isCurrentTableBeingIndexed = this->isTableBeingIndexed(currentPageTableName);
                    
                    // If this page is neither an index page nor from a table being indexed, evict it
                    if (!isCurrentIndexPage && !isCurrentTableBeingIndexed) {
                        // cout << "DEBUG: Found page to evict: " << currentPageName << endl;
                        this->pages.erase(it);
                        foundPageToEvict = true;
                        break;
                    }
                }
                
                // If we couldn't find a suitable page to evict, we have to remove the oldest page
                // that is not from a table being indexed
                if (!foundPageToEvict) {
                    // First try to find any page that is not from a table being indexed
                    for (auto it = this->pages.begin(); it != this->pages.end(); ++it) {
                        string currentPageName = it->pageName;
                        
                        // Extract the table name from the current page name
                        string currentPageTableName = currentPageName;
                        size_t pos = currentPageTableName.find("_Page");
                        if (pos != string::npos) {
                            currentPageTableName = currentPageTableName.substr(11, pos - 11); // 11 is the length of "../data/temp/"
                        }
                        
                        // Check if the current table is being indexed
                        bool isCurrentTableBeingIndexed = this->isTableBeingIndexed(currentPageTableName);
                        
                        // If this page is not from a table being indexed, evict it
                        if (!isCurrentTableBeingIndexed) {
                            // cout << "DEBUG: Found page not from table being indexed to evict: " << currentPageName << endl;
                            this->pages.erase(it);
                            foundPageToEvict = true;
                            break;
                        }
                    }
                    
                    // If we still couldn't find a page to evict, we have to remove the oldest page
                    // even if it's an index page or from a table being indexed
                    if (!foundPageToEvict) {
                        // cout << "DEBUG: No suitable pages found to evict, evicting oldest page: " << frontPageName << endl;
                        this->pages.pop_front();
                    }
                }
            } else {
                // Not an index page and not from a table being indexed, just remove the oldest page
                // cout << "DEBUG: Evicting oldest page: " << frontPageName << endl;
                this->pages.pop_front();
            }
        }
        
        this->pages.push_back(newPage);
        // cout << "DEBUG: Added new page to buffer pool: " << pageName << endl;
        // cout << "DEBUG: New buffer size: " << this->pages.size() << "/" << BLOCK_COUNT << endl;
        return newPage;
    } catch (const exception& e) {
        cerr << "ERROR: Error creating page: " << e.what() << endl;
        // Return an empty page as fallback
        return Page();
    }
}

Page BufferManager::getPage(string matrixName, int pageIndex, int is_matrix)
{
    logger.log("BufferManager::getPage");
    string pageName = "../data/temp/"+matrixName + "_Page" + to_string(pageIndex);
    
    // Check if page exists in pool
    for (auto& page : this->pages) {
        if (page.pageName == pageName) {
            // Move the page to the end of the deque to mark it as recently used
            Page foundPage = page;
            return foundPage;
        }
    }
    
    // If not in pool, create new page and add to pool
    try {
        Page newPage(matrixName, pageIndex, 1);
        
        // If buffer is full, we need to evict a page
        if (this->pages.size() >= BLOCK_COUNT) {
            // Check if the page we're about to evict is an index page
            string frontPageName = this->pages.front().pageName;
            bool isIndexPage = frontPageName.find("_index") != string::npos || 
                              frontPageName.find("_bptree") != string::npos;
            
            // If it's an index page, try to find a non-index page to evict first
            if (isIndexPage) {
                bool foundNonIndexPage = false;
                for (auto it = this->pages.begin(); it != this->pages.end(); ++it) {
                    string currentPageName = it->pageName;
                    if (currentPageName.find("_index") == string::npos && 
                        currentPageName.find("_bptree") == string::npos) {
                        // Found a non-index page, remove it
                        this->pages.erase(it);
                        foundNonIndexPage = true;
                        break;
                    }
                }
                
                // If we couldn't find a non-index page, we have to remove the oldest page
                if (!foundNonIndexPage) {
                    this->pages.pop_front();
                }
            } else {
                // Not an index page, just remove the oldest page
                this->pages.pop_front();
            }
        }
        
        this->pages.push_back(newPage);
        return newPage;
    } catch (const exception& e) {
        cerr << "Error creating page: " << e.what() << endl;
        // Return an empty page as fallback
        return Page();
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
    
    // If page not found, return an empty page
    cerr << "Error: Page " << pageName << " not found in pool" << endl;
    return Page();
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
    try {
        Page page(tableName, pageIndex);
        
        // If buffer is full, we need to evict a page
        if (this->pages.size() >= BLOCK_COUNT) {
            // Check if the page we're about to evict is an index page
            string frontPageName = this->pages.front().pageName;
            bool isIndexPage = frontPageName.find("_index") != string::npos || 
                              frontPageName.find("_bptree") != string::npos;
            
            // If it's an index page, try to find a non-index page to evict first
            if (isIndexPage) {
                bool foundNonIndexPage = false;
                for (auto it = this->pages.begin(); it != this->pages.end(); ++it) {
                    string currentPageName = it->pageName;
                    if (currentPageName.find("_index") == string::npos && 
                        currentPageName.find("_bptree") == string::npos) {
                        // Found a non-index page, remove it
                        this->pages.erase(it);
                        foundNonIndexPage = true;
                        break;
                    }
                }
                
                // If we couldn't find a non-index page, we have to remove the oldest page
                if (!foundNonIndexPage) {
                    this->pages.pop_front();
                }
            } else {
                // Not an index page, just remove the oldest page
                this->pages.pop_front();
            }
        }
        
        pages.push_back(page);
        return page;
    } catch (const exception& e) {
        cerr << "Error inserting page into pool: " << e.what() << endl;
        // Return an empty page as fallback
        return Page();
    }
}

Page BufferManager::insertIntoPool(string tableName, int pageIndex, int is_matrix)
{
    logger.log("BufferManager::insertIntoPool");
    try {
        Page page(tableName, pageIndex, 1);
        
        // If buffer is full, we need to evict a page
        if (this->pages.size() >= BLOCK_COUNT) {
            // Check if the page we're about to evict is an index page
            string frontPageName = this->pages.front().pageName;
            bool isIndexPage = frontPageName.find("_index") != string::npos || 
                              frontPageName.find("_bptree") != string::npos;
            
            // If it's an index page, try to find a non-index page to evict first
            if (isIndexPage) {
                bool foundNonIndexPage = false;
                for (auto it = this->pages.begin(); it != this->pages.end(); ++it) {
                    string currentPageName = it->pageName;
                    if (currentPageName.find("_index") == string::npos && 
                        currentPageName.find("_bptree") == string::npos) {
                        // Found a non-index page, remove it
                        this->pages.erase(it);
                        foundNonIndexPage = true;
                        break;
                    }
                }
                
                // If we couldn't find a non-index page, we have to remove the oldest page
                if (!foundNonIndexPage) {
                    this->pages.pop_front();
                }
            } else {
                // Not an index page, just remove the oldest page
                this->pages.pop_front();
            }
        }
        
        pages.push_back(page);
        return page;
    } catch (const exception& e) {
        cerr << "Error inserting page into pool: " << e.what() << endl;
        // Return an empty page as fallback
        return Page();
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
    string pageName = "../data/temp/" + tableName + "_Page" + to_string(pageIndex);
    
    try {
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
            // Check if the page we're about to evict is an index page
            string frontPageName = this->pages.front().pageName;
            bool isIndexPage = frontPageName.find("_index") != string::npos || 
                              frontPageName.find("_bptree") != string::npos;
            
            // If it's an index page, try to find a non-index page to evict first
            if (isIndexPage) {
                bool foundNonIndexPage = false;
                for (auto it = this->pages.begin(); it != this->pages.end(); ++it) {
                    string currentPageName = it->pageName;
                    if (currentPageName.find("_index") == string::npos && 
                        currentPageName.find("_bptree") == string::npos) {
                        // Found a non-index page, remove it
                        this->pages.erase(it);
                        foundNonIndexPage = true;
                        break;
                    }
                }
                
                // If we couldn't find a non-index page, we have to remove the oldest page
                if (!foundNonIndexPage) {
                    this->pages.pop_front();
                }
            } else {
                // Not an index page, just remove the oldest page
                this->pages.pop_front();
            }
        }
        
        this->pages.push_back(page);
    } catch (const exception& e) {
        cerr << "Error writing page: " << e.what() << endl;
    }
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
