#include"global.h"
/**
 * @brief File contains method to process SORT commands.
 * 
 * syntax:
 * R <- SORT relation_name BY column_name IN sorting_order
 * 
 * sorting_order = ASC | DESC 
 */
bool syntacticParseSORT(){
    logger.log("syntacticParseSORT");
    int query_length = tokenizedQuery.size();
    if(query_length == 1){
        cout << "Please enter table name" << endl;
        return false;
    }
    cout << "Length of Query: " << query_length << endl;
    bool in = false;
    int in_index=-1;
    for(int i=0;i<query_length;i++){
        if(tokenizedQuery[i]=="IN"){
            in = true;
            in_index = i;
            break;
        }
    }
    if(tokenizedQuery[2]!="BY"){
        cout<<"BY Word is missing in Command or please make sure that by word is 3rd word, please give BY word"<<endl;
        return false;
    }
    if(in_index == -1){
        cout << "Missing in word in command, please give IN" << endl;
        return false;
    }
    parsedQuery.queryType = SORT;
    parsedQuery.sortRelationName = tokenizedQuery[1];
    int i = 3;
    while(tokenizedQuery[i]!="IN"){
        parsedQuery.sortColumns.push_back(tokenizedQuery[i]);
        cout << tokenizedQuery[i] << endl;
        i++;
    }
    i++;
    vector<string> sortingStrategy;
    while(i<query_length){
        sortingStrategy.push_back(tokenizedQuery[i]);
        i++;
    }
    for(int i=0;i<sortingStrategy.size();i++){
        if(sortingStrategy[i] == "ASC")
            parsedQuery.sortStrategy.push_back(ASC);
        else if(sortingStrategy[i] == "DESC")
            parsedQuery.sortStrategy.push_back(DESC);
        else{
            cout<<"SYNTAX ERROR 2"<<endl;
            return false;
        }   
    }
    if(parsedQuery.sortStrategy.size() != parsedQuery.sortColumns.size()){
        cout << "Please check whether the column count and sorting strategy size is same..." << endl;
        return false;
    }
    return true;
}

bool semanticParseSORT(){
    logger.log("semanticParseSORT");

    if(tableCatalogue.isTable(parsedQuery.sortResultRelationName)){
        cout<<"SEMANTIC ERROR: Resultant relation already exists"<<endl;
        return false;
    }

    if(!tableCatalogue.isTable(parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }
    Table* table = tableCatalogue.getTable(parsedQuery.sortRelationName);
    if (table) {
        for (const string& column : table->columns) {
            cout << column << " ";
        }
        cout << endl;
    }
    for (const string& sortColumn : parsedQuery.sortColumns) {
        if (!tableCatalogue.isColumnFromTable(sortColumn, parsedQuery.sortRelationName)) {
            cout << "SEMANTIC ERROR: Column '" << sortColumn << "' doesn't exist in relation" << endl;
            return false;
        }
    }
    return true;
}

void executeSORT(){
    logger.log("executeSORT");
    cout << "SORT FUNCTION IS Executing, will implement soon." << endl;
    Table* table = tableCatalogue.getTable(parsedQuery.sortRelationName);
    table->sortTable(true);
    return;
}