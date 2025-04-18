#include "global.h"
/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */
// bool syntacticParseJOIN()
// {
//     logger.log("syntacticParseJOIN");
//     if (tokenizedQuery.size() != 9 || tokenizedQuery[5] != "ON")
//     {
//         cout << "SYNTAX ERROR" << endl;
//         return false;
//     }
//     parsedQuery.queryType = JOIN;
//     parsedQuery.joinResultRelationName = tokenizedQuery[0];
//     parsedQuery.joinFirstRelationName = tokenizedQuery[3];
//     parsedQuery.joinSecondRelationName = tokenizedQuery[4];
//     parsedQuery.joinFirstColumnName = tokenizedQuery[6];
//     parsedQuery.joinSecondColumnName = tokenizedQuery[8];

//     string binaryOperator = tokenizedQuery[7];
//     if (binaryOperator == "<")
//         parsedQuery.joinBinaryOperator = LESS_THAN;
//     else if (binaryOperator == ">")
//         parsedQuery.joinBinaryOperator = GREATER_THAN;
//     else if (binaryOperator == ">=" || binaryOperator == "=>")
//         parsedQuery.joinBinaryOperator = GEQ;
//     else if (binaryOperator == "<=" || binaryOperator == "=<")
//         parsedQuery.joinBinaryOperator = LEQ;
//     else if (binaryOperator == "==")
//         parsedQuery.joinBinaryOperator = EQUAL;
//     else if (binaryOperator == "!=")
//         parsedQuery.joinBinaryOperator = NOT_EQUAL;
//     else
//     {
//         cout << "SYNTAX ERROR" << endl;
//         return false;
//     }
//     return true;
// }

/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1, column_name2
 * Only EQUI-JOIN supported.
 */

bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 8 || tokenizedQuery[5] != "ON")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[3];
    parsedQuery.joinSecondRelationName = tokenizedQuery[4];
    parsedQuery.joinFirstColumnName = tokenizedQuery[6];
    parsedQuery.joinSecondColumnName = tokenizedQuery[7];

    // parsedQuery.joinBinaryOperator = EQUAL;
    return true;
}






bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

void executeJOIN()
{
    logger.log("executeJOIN");

    Table* table1 = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table* table2 = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    if (!table1 || !table2) {
        cout << "ERROR: One or both tables do not exist in catalogue." << endl;
        return;
    }

    table1->joinTables();  // You can keep this if joinTables uses parsedQuery for both tables

    parsedQuery.clear();
}
