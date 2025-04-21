#include "global.h"

/**
 * @brief Deletes rows from the table based on the provided row indices
 * 
 * @param rowIndices Vector of row indices to delete (should be sorted in descending order)
 */
void Table::deleteRows(const vector<int>& rowIndices) {
    logger.log("Table::deleteRows");
    
    if (rowIndices.empty()) {
        cout << "No rows to delete" << endl;
        return;
    }
    
    // Make sure rowIndices are sorted in descending order to avoid shifting issues
    // This should be done by the caller, but we'll check anyway
    vector<int> sortedIndices = rowIndices;
    sort(sortedIndices.begin(), sortedIndices.end(), greater<int>());
    
    // Calculate which blocks contain the rows to be deleted
    unordered_map<int, vector<int>> blockToRowIndices;
    int currentBlockRows = 0;
    int blockIndex = 0;
    
    // Map row indices to their respective blocks
    for (int rowIndex : sortedIndices) {
        // Find which block contains this row
        int rowsSoFar = 0;
        bool found = false;
        
        for (blockIndex = 0; blockIndex < this->blockCount; blockIndex++) {
            int rowsInBlock = this->rowsPerBlockCount[blockIndex];
            if (rowIndex < rowsSoFar + rowsInBlock) {
                // This row is in this block
                int localRowIndex = rowIndex - rowsSoFar;
                blockToRowIndices[blockIndex].push_back(localRowIndex);
                found = true;
                break;
            }
            rowsSoFar += rowsInBlock;
        }
        
        if (!found) {
            cout << "Warning: Row index " << rowIndex << " is out of bounds" << endl;
        }
    }
    
    // Now process each block
    for (auto& [blockIndex, localRowIndices] : blockToRowIndices) {
        // Sort local indices in descending order
        sort(localRowIndices.begin(), localRowIndices.end(), greater<int>());
        
        // Load the block
        Page page = bufferManager.getPage(this->tableName, blockIndex);
        vector<vector<int>> rows = page.getAllRows();
        
        // Delete rows from this block
        for (int localRowIndex : localRowIndices) {
            if (localRowIndex < rows.size()) {
                rows.erase(rows.begin() + localRowIndex);
            }
        }
        
        // Update the block
        this->rowsPerBlockCount[blockIndex] = rows.size();
        bufferManager.writePage(this->tableName, blockIndex, rows, rows.size());
    }
    
    // Update the total row count
    this->rowCount -= rowIndices.size();
    
    // Rebalance blocks if needed
    rebalanceBlocks();
    
    // We don't call makePermanent() here because we only want to modify the data in the temp directory
    // The original CSV files should remain unchanged as they are only used for LOAD operations
    // If the user wants to save the changes to a CSV file, they should use the EXPORT command
}

/**
 * @brief Rebalances blocks after deletion to maintain optimal block usage
 */
void Table::rebalanceBlocks() {
    logger.log("Table::rebalanceBlocks");
    
    // If we have no blocks or just one block, no need to rebalance
    if (this->blockCount <= 1) {
        return;
    }
    
    // Collect all rows from all blocks
    vector<vector<int>> allRows;
    int totalRows = 0;
    
    for (int blockIndex = 0; blockIndex < this->blockCount; blockIndex++) {
        Page page = bufferManager.getPage(this->tableName, blockIndex);
        vector<vector<int>> rows = page.getAllRows();
        allRows.insert(allRows.end(), rows.begin(), rows.end());
        totalRows += rows.size();
    }
    
    // Calculate how many blocks we need
    int newBlockCount = (totalRows + this->maxRowsPerBlock - 1) / this->maxRowsPerBlock;
    if (newBlockCount == 0) newBlockCount = 1;  // Always have at least one block
    
    // Redistribute rows across blocks
    this->rowsPerBlockCount.clear();
    this->blockCount = newBlockCount;
    
    for (int blockIndex = 0; blockIndex < newBlockCount; blockIndex++) {
        int startRow = blockIndex * this->maxRowsPerBlock;
        int endRow = std::min(startRow + (int)this->maxRowsPerBlock, totalRows);
        int rowsInBlock = endRow - startRow;
        
        if (rowsInBlock <= 0) {
            // Empty block, just create an empty one
            vector<vector<int>> emptyBlock;
            bufferManager.writePage(this->tableName, blockIndex, emptyBlock, 0);
            this->rowsPerBlockCount.push_back(0);
            continue;
        }
        
        // Extract rows for this block
        vector<vector<int>> blockRows(allRows.begin() + startRow, allRows.begin() + endRow);
        
        // Write the block
        bufferManager.writePage(this->tableName, blockIndex, blockRows, rowsInBlock);
        this->rowsPerBlockCount.push_back(rowsInBlock);
    }
    
    cout << "Rebalanced table into " << newBlockCount << " blocks" << endl;
}