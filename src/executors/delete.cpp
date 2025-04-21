#include "global.h"

/**
 * @brief 
 * SYNTAX: DELETE FROM relation_name WHERE column_name bin_op value
 */
bool syntacticParseDELETE()
{
    logger.log("syntacticParseDELETE");
    
    if (tokenizedQuery.size() != 7 || tokenizedQuery[1] != "FROM" || tokenizedQuery[3] != "WHERE")
    {
        cout << "SYNTAX ERROR: Expected format: DELETE FROM relation_name WHERE column_name bin_op value" << endl;
        return false;
    }
    
    parsedQuery.queryType = DELETE;
    parsedQuery.deleteRelationName = tokenizedQuery[2];
    parsedQuery.deleteColumnName = tokenizedQuery[4];
    
    string binaryOperator = tokenizedQuery[5];
    if (binaryOperator == "<")
        parsedQuery.deleteBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.deleteBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.deleteBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.deleteBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.deleteBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.deleteBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR: Invalid binary operator" << endl;
        return false;
    }
    
    // Check if value is a valid integer
    regex numeric("[-]?[0-9]+");
    string valueStr = tokenizedQuery[6];
    if (!regex_match(valueStr, numeric))
    {
        cout << "SYNTAX ERROR: Value must be an integer" << endl;
        return false;
    }
    
    parsedQuery.deleteIntLiteral = stoi(valueStr);
    return true;
}

bool semanticParseDELETE()
{
    logger.log("semanticParseDELETE");
    
    if (!tableCatalogue.isTable(parsedQuery.deleteRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    
    if (!tableCatalogue.isColumnFromTable(parsedQuery.deleteColumnName, parsedQuery.deleteRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    
    return true;
}

void executeDELETE()
{
    logger.log("executeDELETE");
    
    Table* table = tableCatalogue.getTable(parsedQuery.deleteRelationName);
    
    // Check if the table has an index on the delete column
    bool useIndex = table->isIndexed(parsedQuery.deleteColumnName);
    int rowsDeleted = 0;
    
    cout << "Deleting rows where " << parsedQuery.deleteColumnName << " ";
    switch(parsedQuery.deleteBinaryOperator) {
        case LESS_THAN: cout << "< "; break;
        case GREATER_THAN: cout << "> "; break;
        case LEQ: cout << "<= "; break;
        case GEQ: cout << ">= "; break;
        case EQUAL: cout << "== "; break;
        case NOT_EQUAL: cout << "!= "; break;
        default: cout << "? "; break;
    }
    cout << parsedQuery.deleteIntLiteral << " from " << parsedQuery.deleteRelationName << endl;
    
    vector<int> rowsToDelete;
    
    if (useIndex)
    {
        // Check if the index is a B+ tree
        if (table->indexingStrategy == BTREE) {
            cout << "Using existing B+ tree index on " << parsedQuery.deleteRelationName << "." << parsedQuery.deleteColumnName << endl;
        } else {
            cout << "Using existing index on " << parsedQuery.deleteRelationName << "." << parsedQuery.deleteColumnName << endl;
        }
        
        // Get matching row numbers from the index
        rowsToDelete = table->searchIndexed(parsedQuery.deleteColumnName, parsedQuery.deleteIntLiteral, parsedQuery.deleteBinaryOperator);
        rowsDeleted = rowsToDelete.size();
        
        if (rowsDeleted > 0) {
            cout << "Found " << rowsDeleted << " rows to delete using index" << endl;
        } else {
            cout << "No matching rows found to delete" << endl;
        }
    }
    else
    {
        cout << "No index found on " << parsedQuery.deleteColumnName << ", creating new B+ tree index..." << endl;
        
        // Create a new B+ tree index for this delete operation
        bool indexCreated = table->buildIndex(parsedQuery.deleteColumnName);
        
        if (indexCreated) {
            // Get matching row numbers from the index
            rowsToDelete = table->searchIndexed(parsedQuery.deleteColumnName, parsedQuery.deleteIntLiteral, parsedQuery.deleteBinaryOperator);
            rowsDeleted = rowsToDelete.size();
            
            if (rowsDeleted > 0) {
                cout << "Found " << rowsDeleted << " rows to delete using newly created B+ tree index" << endl;
            } else {
                cout << "No matching rows found to delete" << endl;
            }
        } else {
            cout << "Failed to create B+ tree index, falling back to sequential scan" << endl;
            
            // Fallback to sequential scan
            int columnIndex = table->getColumnIndex(parsedQuery.deleteColumnName);
            cout << "Doing sequential scan on " << parsedQuery.deleteRelationName << endl;
            Cursor cursor = table->getCursor();
            vector<int> row = cursor.getNext();
            int rowCounter = 0;
            
            while (!row.empty()) {
                if (columnIndex < row.size()) {
                    int value = row[columnIndex];
                    if (evaluateBinOp(value, parsedQuery.deleteIntLiteral, parsedQuery.deleteBinaryOperator)) {
                        rowsToDelete.push_back(rowCounter);
                        rowsDeleted++;
                    }
                }
                row = cursor.getNext();
                rowCounter++;
            }
            
            if (rowsDeleted > 0) {
                cout << "Found " << rowsDeleted << " rows to delete using sequential scan" << endl;
            } else {
                cout << "No matching rows found to delete" << endl;
            }
        }
    }
    
    // If we found rows to delete, perform the deletion
    if (rowsDeleted > 0) {
        // Sort row numbers in descending order to avoid shifting issues
        sort(rowsToDelete.begin(), rowsToDelete.end(), greater<int>());
        
        // Delete the rows from the table
        table->deleteRows(rowsToDelete);
        
        // Update the indices if any
        if (useIndex || table->indexed) {
            cout << "Updating indices after deletion..." << endl;
            
            // For each indexed column, rebuild the index
            for (const auto& col : table->columns) {
                if (table->isIndexed(col)) {
                    cout << "Rebuilding index on column " << col << endl;
                    table->buildIndex(col);
                }
            }
        }
        
        cout << "DELETE SUCCESSFUL" << endl;
        printRowCount(rowsDeleted);
    } else {
        cout << "No rows deleted" << endl;
    }
    
    return;
}