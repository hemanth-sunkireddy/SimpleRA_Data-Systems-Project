#include "global.h"

bool syntacticParseGROUP_BY()
{
    logger.log("syntacticParseGROUPBY");
    
    if (tokenizedQuery.size() != 13)
    {
        cout << "SYNTAX ERROR [Query length is not correct. please see grammer]" << endl;
        return false;
    }

    parsedQuery.queryType = GROUP_BY;
    parsedQuery.groupByResultRelationName = tokenizedQuery[0];
    parsedQuery.groupAttribute = tokenizedQuery[4];
    parsedQuery.groupRelation = tokenizedQuery[6];
    parsedQuery.havingAggregate = "";
    parsedQuery.havingAttribute = "";
    parsedQuery.returnAggregate = "";
    parsedQuery.returnAttribute = "";

    int i = 0;
    for (; i < tokenizedQuery[8].size(); i++) {
        if (tokenizedQuery[8][i] == '(') break;
        parsedQuery.havingAggregate += tokenizedQuery[8][i];
    }
    i++;
    for (; i < tokenizedQuery[8].size() - 1; i++) {
        parsedQuery.havingAttribute += tokenizedQuery[8][i];
    }

    i = 0;
    for (; i < tokenizedQuery[12].size(); i++) {
        if (tokenizedQuery[12][i] == '(') break;
        parsedQuery.returnAggregate += tokenizedQuery[12][i];
    }
    i++;
    for (; i < tokenizedQuery[12].size() - 1; i++) {
        parsedQuery.returnAttribute += tokenizedQuery[12][i];
    }

    if (parsedQuery.havingAggregate == "MAX")
        parsedQuery.havingAgg = MAX;
    else if (parsedQuery.havingAggregate == "MIN")
        parsedQuery.havingAgg = MIN;
    else if (parsedQuery.havingAggregate == "AVG")
        parsedQuery.havingAgg = AVG;
    else if (parsedQuery.havingAggregate == "SUM")
        parsedQuery.havingAgg = SUM;
    else if (parsedQuery.havingAggregate == "COUNT")
        parsedQuery.havingAgg = COUNT;
    else {
        cout << "SYNTAX ERROR: Incorrect HAVING aggregate function" << endl;
        return false;
    }
    if (parsedQuery.returnAggregate == "MAX")
        parsedQuery.returnAgg = MAX;
    else if (parsedQuery.returnAggregate == "MIN")
        parsedQuery.returnAgg = MIN;
    else if (parsedQuery.returnAggregate == "AVG")
        parsedQuery.returnAgg = AVG;
    else if (parsedQuery.returnAggregate == "SUM")
        parsedQuery.returnAgg = SUM;
    else if (parsedQuery.havingAggregate == "COUNT")
        parsedQuery.returnAgg = COUNT;
    else {
        cout << "SYNTAX ERROR: Incorrect RETURN aggregate function" << endl;
        return false;
    }
    parsedQuery.havingValue = stoi(tokenizedQuery[10]);
    string binaryOperator = tokenizedQuery[9];
    if (binaryOperator == "<")
        parsedQuery.groupBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.groupBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.groupBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.groupBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.groupBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.groupBinaryOperator = NOT_EQUAL;
    else {
        cout << "SYNTAX ERROR: Incorrect Binary Operator" << endl;
        return false;
    }
    cout << "SYNTAX ORDER is correct" << endl;
    return true;
}

bool semanticParseGROUP_BY()
{
    logger.log("semanticParseGROUPBY");

    if (tableCatalogue.isTable(parsedQuery.groupByResultRelationName)) {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }
    if (!tableCatalogue.isTable(parsedQuery.groupRelation)) {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupAttribute, parsedQuery.groupRelation)) {
        cout << "SEMANTIC ERROR: Grouping Column doesn't exist in relation" << endl;
        return false;
    }
    if (!tableCatalogue.isColumnFromTable(parsedQuery.havingAttribute, parsedQuery.groupRelation)) {
        cout << "SEMANTIC ERROR: Having Column doesn't exist in relation" << endl;
        return false;
    }
    if (!tableCatalogue.isColumnFromTable(parsedQuery.returnAttribute, parsedQuery.groupRelation)) {
        cout << "SEMANTIC ERROR: Return Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

void executeGROUP_BY() {
    logger.log("executeGROUPBY");
    Table* table = tableCatalogue.getTable(parsedQuery.groupRelation);
    table->groupBy();
    parsedQuery.clear();
}

bool syntacticParseORDERBY()
{
    logger.log("syntacticParseORDERBY()");

    if(tokenizedQuery.size()!=8){
        cout<<"SYNATX ERROR: PLEASE GIVE 8 Arguments"<<endl;
        return false;
    }
    parsedQuery.queryType = ORDERBY;
    parsedQuery.orderResultRelation = tokenizedQuery[0];
    parsedQuery.orderAttribute = tokenizedQuery[4];
    string sortingStrategy = tokenizedQuery[5];
    parsedQuery.orderRelationName = tokenizedQuery[7];
    if(sortingStrategy == "ASC")
        parsedQuery.sortingStrategy = ASC;
    else if(sortingStrategy == "DESC")
        parsedQuery.sortingStrategy = DESC;
    else{
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }
    cout << "PASSED" << endl;
    return true;
}

bool semanticParseORDERBY(){
    logger.log("semanticParseORDERBY");

    if(tableCatalogue.isTable(parsedQuery.orderResultRelation)){
        cout<<"SEMANTIC ERROR: Resultant relation already exists"<<endl;
        return false;
    }

    if(!tableCatalogue.isTable(parsedQuery.orderRelationName)){
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    if(!tableCatalogue.isColumnFromTable(parsedQuery.orderAttribute, parsedQuery.orderRelationName)){
        cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
        return false;
    }

    return true;
}




void executeORDER_BY() {
    logger.log("executeGROUPBY");
    Table* table = tableCatalogue.getTable(parsedQuery.groupRelation);
    table->orderBy();
    parsedQuery.clear();
}



bool syntaticParseInsert()
{
    logger.log("syntacticParseINSERT()");
    cout << "INSERT OPERATION" << endl;
    // if (tokenizedQuery.size() != 4) {
    //     cout << "SYNTAX ERROR: Please give 4 arguments in this format: INSERT INTO table_name (col1=val1, col2=val2)" << endl;
    //     return false;
    // }

    if (tokenizedQuery[1] != "INTO") {
        cout << "SYNTAX ERROR: Please make sure 2nd argument is INTO" << endl;
        return false;
    }

    parsedQuery.queryType = INSERT;
    parsedQuery.loadRelationName = tokenizedQuery[2];
    string valueStr = tokenizedQuery[3];  // "(col1=1,col2=2)"
    // Remove parentheses
    if (valueStr.front() == '(') valueStr.erase(0, 1);
    if (valueStr.back() == ')') valueStr.pop_back();
    
    // Now parse key=value pairs
    stringstream ss(valueStr);
    string pair;
    while (getline(ss, pair, ',')) {
        size_t pos = pair.find('=');
        if (pos == string::npos) continue;
    
        string key = pair.substr(0, pos);
        string value = pair.substr(pos + 1);
    
        parsedQuery.insertKeyValue[key] = value;
    }


    return true;
}


bool semanticParseInsert() {
    logger.log("semanticParseINSERT");

    if (!tableCatalogue.isTable(parsedQuery.loadRelationName)) {
        cout << "SEMANTIC ERROR: Table does not exist" << endl;
        return false;
    }

    Table* table = tableCatalogue.getTable(parsedQuery.loadRelationName);

    for (const string& col : table->columns) {
        cout << col << endl;
    }

    // Debug print: Parsed key-value pairs
    cout << "Parsed INSERT values:" << endl;
    for (auto& [key, val] : parsedQuery.insertKeyValue) {
        cout << key << " = " << val << endl;
    }

    return true;
}



void executeINSERT() {
    logger.log("executeINSERT");
    cout << "EXECUTING INSERT OPERATION" << endl;

    Table* table = tableCatalogue.getTable(parsedQuery.loadRelationName);

    vector<string> row;
    for (const string& col : table->columns) {
        row.push_back(parsedQuery.insertKeyValue[col]);
    }

    table->insertRow(row);
}