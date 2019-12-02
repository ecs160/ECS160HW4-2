/*  maxTweeter.c 
 *
 *  input:
 *      One program arguement, filepath to a CSV file. 
 * 
 * 
 *  restrictions:
 *      Max line length will not exceed 1024 characters.
 *      Length of file will not exceed 20,000 lines.
 *      Header for Name may be surrounded by quotes, but then all names must also be surrounded by quotes.
 *      Tweet field will not contain any commas.
 *  goals:
 *      Program correctness on input files.
 *      Program stability: should not crash on any input, including invalid files.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//DEBUG_MODE: 0=disabled, 1=enabled.
#define DEBUG_MODE 0

#define MAX_LINE_LENGTH 1024
#define MAX_FILE_LINES 20000
#define NAME "name"
#define NAME_QUOTES "\"name\""

#define FAILURE_EXIT exit(1)

#define dprintf(args ...) if(DEBUG_MODE) fprintf(stderr, args)

struct columnProperties{
    char name[MAX_LINE_LENGTH];
    int quotes;
};

struct nameCount{
    char name[MAX_LINE_LENGTH];
    int count;
    int taken;//0=false : 1=true
};

FILE* fileStream;
struct columnProperties column[MAX_LINE_LENGTH];
struct nameCount names[MAX_FILE_LINES];
char lineBuffer[MAX_LINE_LENGTH];
int total_names;
int name_index;
int name_quotes;
int last_column_index;
char* filePath;

//Prints the 10 names that had the highest count in the input file.
void print10Names(){
    int i;
    int listSize;

    if(total_names > 10){
        listSize = 10;
    }else{
        listSize = total_names;
    }

    for(i = 0; i < listSize; i++){
        printf("%s: %d\n", names[i].name, names[i].count);
    }

    return;
}

//Comparison function for the use of qsort.
int comparator(const void* p1, const void* p2){
    int r = ((struct nameCount*)p1)->count;
    int l = ((struct nameCount*)p2)->count;

    return (l - r);
}

//Uses qsort to sort the datastructure names from greatest to least.
void sortStructure(){
    int size = sizeof(names) / sizeof(names[0]);
    qsort((void*)names, size, sizeof(names[0]), comparator);
}

//
char* removeQuotes(char* string){
    size_t length = strlen(string);
    if(length > 0){
        string++;
    }
    if(length > 1){
        string[length-2] = '\0';
    }
    return string;
}

//Returns the last char in a string/buffer.
char lastChar(char* c){
    if(c == NULL || *c == '\0'){
        return 0;
    }
    return c[strlen(c)-1];
}

//Checks element for quote matching.
//Returns 0 for absent, 1 for present. FAILURE_EXIT on mismatch.
int checkFirstLast(const char* string){
    size_t stringLength = strlen(string);
    int out;

    dprintf("Checking string: %s\n", string);

    if(string[0] == '"'){
        if(string[stringLength-1] != '"'){
            dprintf("Field inconsistancy...Quote invalidation...\n");
            printf("Invalid Input Format.\n");
            fclose(fileStream);
            FAILURE_EXIT;
        }else{
            out = 1;
        }
    }else{
        if(string[stringLength-1] == '"'){
            dprintf("Field inconsistancy...Quote invalidation...\n");
            printf("Invalid Input Format.\n");
            fclose(fileStream);
            FAILURE_EXIT;
        }else{
            out = 0;
        }
    }

    dprintf("out = %d\n", out);
    return out;
}

void incrementNameCount(char* name){

    dprintf("IncrementNameCount enter: name=%s\n", name);

    int index;
    int found = 0;
    for(index = 0; found == 0 && index < MAX_FILE_LINES; index++ ){
        if(!strcmp(name, names[index].name) || names[index].taken == 0){
            found = 1;
            break;
        }
        dprintf("index = %d\n", index);
    }

    if(found){
        if(names[index].taken == 0){
            strcpy(names[index].name, name);
            names[index].count = 1;
            names[index].taken = 1;
            total_names++;
        }else{
            names[index].count++;
        }
        return;
    }else{
        //Believed to be dead code, there should always be room in...
        //...the names structure with the upper bound being MAX_FILE_LINES.
        dprintf("No room to add name to database...\n");
        fclose(fileStream);
        FAILURE_EXIT;
    }

}

void headerChecker(){

    dprintf("Entered headerChecker...\n");

    int nameCount = 0;
    int count = 0;
    int breakLoop = 0;
    char* element;
    char* workingElement = NULL;

    if(fgets(lineBuffer, MAX_LINE_LENGTH, fileStream) == NULL){
        dprintf("File contains no content...\n");
        printf("Invalid Input Format.\n");
        fclose(fileStream);
        FAILURE_EXIT;
    }

    if(lastChar(lineBuffer)!= '\n'){
        dprintf("Line does not end with a \\n char...line length exceeds maxiumum...");
        printf("Invalid Input Format.\n");
        fclose(fileStream);
        FAILURE_EXIT;
    }

    element = strtok(lineBuffer, ",");
    while(element !=  NULL)
    {   
        if(lastChar(element) == '\n'){
            break;
        }

        strcpy(column[count].name, element);
        if(checkFirstLast(element)){
            column[count].quotes = 1;
        }else{
            column[count].quotes = 0;
        }
        
        if(!strcmp(element, NAME)){
            nameCount++;
            name_quotes = 0;
            name_index = count;
        }
        if(!strcmp(element, NAME_QUOTES)){
            nameCount++;
            name_quotes = 1;
            name_index = count;
        }

        dprintf("Element %d name = %s\n", count, column[count].name);
        dprintf("Element %d quotes = %d\n", count, column[count].quotes);

        count++;
        element = strtok(NULL, ",");
    }

    //Decrements to go back to the last valid index.
    //count--;

    //Recalculates the last string field and if it uses quotes, removing the '\n' character.
    if(lastChar(element) == '\n'){

        dprintf("Enter exception area, removing \\n\n");
        
        element[strlen(element) -1] = '\0';
        //column[count].name[strlen(column[count].name) -1] = '\0';

        strcpy(column[count].name, element);
        //column[count].quotes = checkFirstLast(column[count].name);

        if(checkFirstLast(element)){
            column[count].quotes = 1;
        }else{
            column[count].quotes = 0;
        }

        if(!strcmp(column[count].name, NAME)){
            nameCount++;
            name_quotes = 0;
            name_index = count;
        }
        if(!strcmp(column[count].name, NAME_QUOTES)){
            nameCount++;
            name_quotes = 1;
            name_index = count;
        }

        dprintf("Element %d name = %s\n", count, column[count].name);
        dprintf("Element %d quotes = %d\n", count, column[count].quotes);
    }

    //Records the last column index for future reference in parsing lines.
    last_column_index = count;
    dprintf("Last Column Index = %d\n", last_column_index);

    //If no valid column "name" or name.
    if(nameCount != 1){
        dprintf("Invalid header... single valid NAME column not found");
        printf("Invalid Input Format.\n");
        fclose(fileStream);
        FAILURE_EXIT;
    }

    dprintf("Name field found in column %d, Contains quotes?:%d\n", name_index, name_quotes);

    for(int i = 0; i <= last_column_index; i++){
        dprintf("field:%s | index:%d | quotes:%d\n", column[i].name, i, column[i].quotes);
    }
}

int lineChecker(){
    int lineCount = 1;
    int index;
    char* element;
    char* workingElement = NULL;

    while(fgets(lineBuffer, MAX_LINE_LENGTH, fileStream) != NULL && lineCount < MAX_FILE_LINES){

            element = strtok(lineBuffer, ",");
        for(index = 0; index < last_column_index; index++){
            
            dprintf("Checking index = %d\n", index);

            if(!element){
                dprintf("Row does not line with header1...\n");
                printf("Invalid Input Format.\n");
                fclose(fileStream);
                FAILURE_EXIT;
            }

            if(column[index].quotes != checkFirstLast(element)){
                dprintf("Quote mismatch of element1...\n");
                printf("Invalid Input Format.\n");
                fclose(fileStream);
                FAILURE_EXIT;
            }

            if(index == name_index)
            {
                if(name_quotes){
                    incrementNameCount(removeQuotes(element));
                }else{
                    incrementNameCount(element);
                }
            }

            element = strtok(NULL, ",");
        }

        if(!element){
            dprintf("Row does not line with header2...\n");
            printf("Invalid Input Format.\n");
            fclose(fileStream);
            FAILURE_EXIT;
        }

        if(lastChar(element) != '\n'){
            dprintf("Row does not line with header3...\n");
            printf("Invalid Input Format.\n");
            fclose(fileStream);
            FAILURE_EXIT;
        }

        element[strlen(element)-1] = '\0';

        if(column[index].quotes != checkFirstLast(element)){
            dprintf("Quote mismatch of element2...\n");
            printf("Invalid Input Format.\n");
            fclose(fileStream);
            FAILURE_EXIT;
        }

        if(index == name_index)
        {
            if(name_quotes){
                incrementNameCount(removeQuotes(element));
            }else{
                incrementNameCount(element);
            }
        }

        dprintf("LineCount = %d\n", lineCount);
        lineCount++;
    }

    if(lineBuffer != NULL && lineCount == MAX_FILE_LINES){
        dprintf("LineBuffer = %s\n", lineBuffer);
        dprintf("Exceeded maximum number of lines in file...\n");
        printf("Invalid Input Format.\n");
        fclose(fileStream);
        FAILURE_EXIT;
    }

}

//Checks for file at provided filePath.
//If found, opens and stores in fileStream global var.
//Else, exit with failure state.
void fileChecker(char* filePath){
    
    fileStream = fopen(filePath, "r");

    if(!fileStream){
        dprintf("File with provided path not found...\n");
        printf("File not found.\n");
        FAILURE_EXIT;
    }else{
        dprintf("File with provided path found and opened...\n");
    }
    
    return;
}

void main(int argc, char **argv){

    total_names = 0;

    //Checks correct number of passed arguements.
    if( argc < 2){
        dprintf("No file path provided...\n");
        printf("Invalid number of arguements.\n");
        FAILURE_EXIT;
    }
    if( argc > 2){
        dprintf("Too many arguements...\n");
        printf("Invalid number of arguements.\n");
        FAILURE_EXIT;
    }
      
    filePath = malloc(sizeof(char) * strlen(argv[1]));

    if(!filePath){
        dprintf("Failed to allocate memory...\n");
        printf("Memory allocation failure.\n");
        FAILURE_EXIT;
    }

    //Save filePath arguement to filePath global variable.
    strcpy(filePath, argv[1]);

    //Checks validity of filePath.
    fileChecker(filePath);
    //free(filePath);

    //Checks validity of header of file.
    headerChecker();

    //Parses line-by-line, validating and collecting name data.
    lineChecker();

    //Sorts name data structure.
    sortStructure();

    //Outputs 10 names with most occurances.
    print10Names();


    return;   
}