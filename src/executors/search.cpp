#include "global.h"

/**
 * @brief 
 * SYNTAX: R <- SEARCH FROM relation_name WHERE column_name bin_op value
 */
bool syntacticParseSEARCH()
{
    logger.log("syntacticParseSEARCH");
    
    if (tokenizedQuery.size() != 9|| tokenizedQuery[3] != "FROM" || tokenizedQuery[5] != "WHERE")
    {
        cout << "SYNTAX ERROR: Expected format: R <- SEARCH FROM relation_name WHERE column_name bin_op value" << endl;
        return false;
    }
    
    parsedQuery.queryType = SEARCH;
    parsedQuery.searchResultRelationName = tokenizedQuery[0];
    parsedQuery.searchRelationName = tokenizedQuery[4];
    parsedQuery.searchColumnName = tokenizedQuery[6];
    
    string binaryOperator = tokenizedQuery[7];
    if (binaryOperator == "<")
        parsedQuery.searchBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.searchBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.searchBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.searchBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.searchBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.searchBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR: Invalid binary operator" << endl;
        return false;
    }
    
    // Check if value is a valid integer
    regex numeric("[-]?[0-9]+");
    string valueStr = tokenizedQuery[8];
    if (!regex_match(valueStr, numeric))
    {
        cout << "SYNTAX ERROR: Value must be an integer" << endl;
        return false;
    }
    
    parsedQuery.searchIntLiteral = stoi(valueStr);
    return true;
}

bool semanticParseSEARCH()
{
    logger.log("semanticParseSEARCH");
    
    if (tableCatalogue.isTable(parsedQuery.searchResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }
    
    if (!tableCatalogue.isTable(parsedQuery.searchRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    
    if (!tableCatalogue.isColumnFromTable(parsedQuery.searchColumnName, parsedQuery.searchRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    
    return true;
}

void executeSEARCH()
{
    logger.log("executeSEARCH");
    
    Table* table = tableCatalogue.getTable(parsedQuery.searchRelationName);
    Table* resultantTable = new Table(parsedQuery.searchResultRelationName, table->columns);
    
    // Check if the table has an index on the search column
    bool useIndex = table->isIndexed(parsedQuery.searchColumnName);
    int rowsMatched = 0;
    
    cout << "Searching for rows where " << parsedQuery.searchColumnName << " ";
    switch(parsedQuery.searchBinaryOperator) {
        case LESS_THAN: cout << "< "; break;
        case GREATER_THAN: cout << "> "; break;
        case LEQ: cout << "<= "; break;
        case GEQ: cout << ">= "; break;
        case EQUAL: cout << "== "; break;
        case NOT_EQUAL: cout << "!= "; break;
        default: cout << "? "; break;
    }
    cout << parsedQuery.searchIntLiteral << " in " << parsedQuery.searchRelationName << endl;
    
    vector<int> matchingRows;
    
    if (useIndex)
    {
        // Check if the index is a B+ tree
        if (table->indexingStrategy == BTREE) {
            cout << "Using existing B+ tree index on " << parsedQuery.searchRelationName << "." << parsedQuery.searchColumnName << endl;
        } else {
            cout << "Using existing index on " << parsedQuery.searchRelationName << "." << parsedQuery.searchColumnName << endl;
        }
        
        // Get matching row numbers from the index
        matchingRows = table->searchIndexed(parsedQuery.searchColumnName, parsedQuery.searchIntLiteral, parsedQuery.searchBinaryOperator);
        rowsMatched = matchingRows.size();
        
        if (rowsMatched > 0) {
            cout << "Found " << rowsMatched << " matching rows using index" << endl;
            
            // Retrieve the actual rows
            for (int rowNum : matchingRows)
            {
                // Get the row from the table
                Cursor cursor = table->getCursor();
                vector<int> row;
                for (int i = 0; i <= rowNum; i++)
                {
                    row = cursor.getNext();
                    if (row.empty()) break;
                }
                
                if (!row.empty())
                {
                    resultantTable->writeRow<int>(row);
                }
            }
        } else {
            cout << "No matching rows found" << endl;
        }
    }
    else
    {
        cout << "No index found on " << parsedQuery.searchColumnName << ", creating new B+ tree index..." << endl;
        
        // Create a new B+ tree index for this search
        bool indexCreated = table->buildIndex(parsedQuery.searchColumnName);
        
        if (indexCreated) {
            // Get matching row numbers from the index
            matchingRows = table->searchIndexed(parsedQuery.searchColumnName, parsedQuery.searchIntLiteral, parsedQuery.searchBinaryOperator);
            rowsMatched = matchingRows.size();
            
            if (rowsMatched > 0) {
                cout << "Found " << rowsMatched << " matching rows using newly created B+ tree index" << endl;
                
                // Retrieve the actual rows
                for (int rowNum : matchingRows)
                {
                    // Get the row from the table
                    Cursor cursor = table->getCursor();
                    vector<int> row;
                    for (int i = 0; i <= rowNum; i++)
                    {
                        row = cursor.getNext();
                        if (row.empty()) break;
                    }
                    
                    if (!row.empty())
                    {
                        resultantTable->writeRow<int>(row);
                    }
                }
            } else {
                cout << "No matching rows found" << endl;
            }
        } else {
            cout << "Failed to create B+ tree index, falling back to sequential scan" << endl;
            
            // Fallback to sequential scan
            int columnIndex = table->getColumnIndex(parsedQuery.searchColumnName);
            cout << "Doing sequential scan on " << parsedQuery.searchRelationName << endl;
            Cursor cursor = table->getCursor();
            vector<int> row = cursor.getNext();
            
            while (!row.empty()) {
                if (columnIndex < row.size()) {
                    int value = row[columnIndex];
                    if (evaluateBinOp(value, parsedQuery.searchIntLiteral, parsedQuery.searchBinaryOperator)) {
                        resultantTable->writeRow<int>(row);
                        rowsMatched++;
                    }
                }
                row = cursor.getNext();
            }
            
            if (rowsMatched > 0) {
                cout << "Found " << rowsMatched << " matching rows using sequential scan" << endl;
            } else {
                cout << "No matching rows found" << endl;
            }
        }
    }
    
    if (resultantTable->blockify())
    {
        tableCatalogue.insertTable(resultantTable);
        cout << "SEARCH SUCCESSFUL" << endl;
        cout << "Result stored in table: " << parsedQuery.searchResultRelationName << endl;
        printRowCount(rowsMatched);
    }
    else
    {
        cout << "Empty Table" << endl;
        resultantTable->unload();
        delete resultantTable;
    }
    
    return;
}