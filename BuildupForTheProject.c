/* hw5.c */
/* NAME: Travis Peterson */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>


struct pipe
{
    //'tag' is so instructions like 'add $s1,$s1,$s1' can be outputted
    char tag[25];

    //'pipeline' is the array that stores the '.' or the pipeline values like 'ID', 'MEM', etc.
    char pipeline[9][4];

    //'mainValue' is used to reference the first numerical value in the instruction. ex: the main value for add $s1,$s2,$s3 would be 1 in this case
    int mainValue;

    //'nop' is to inidicate whether a row in the array is a nop instruction or not
    bool nop;

    //'WBready' is used in order to indicate when an insturction can and has used a WB value
    bool WBready;

    //'firstVariable' is used to reference the first variable ('s' or 't') in the instruction. ex: the first variable for add $t1,$s2,3 would be 't' in this case
    char firstVariable;

    //'order' is used to reference the order in which the instructions go. ex: for the instructions add   sub   sw    add would be 1, sub would be 2, and sw would be 3
    int order;

    //'hasBeenChecked' is used in order to make sure that an instruction has already been checked and received its mainValues and firstVariables
    bool hasBeenChecked;

    //'hasReachedMEM' is used to check when an instruction has reached the MEM value
    bool hasReachedMEM;

    //'hasReachedIF' is used to check when an instruction has reached the IF value
    bool hasReachedIF;

    //'hasReachedID' is used to check when an instruction has reached the ID value
    bool hasReachedID;

    //'checkedForNop' is used in order to make sure that an instruction has already been examined for a nops instruction.
    bool checkedForNop;

    //'skip' is used in order to see if an examination for a secondary nops instruction can be skipped
    bool skip;

    //'isNotNull' is used in order to tell the difference between a column that has instruction values and empty values
    bool isNotNull;
};


int main( int argc, char * argv[] )
{
    FILE *fptr;
    char ch;
    fptr = fopen(argv[1],"r");
    int CPUcycles = 9;
    char IF[] = "IF";
    char ID[] = "ID";
    char EX[] = "EX";
    char MEM[] = "MEM";
    char WB[] = "WB";

    char word[25];
    int wordCounter = 0;
    int WB_counter = 0;
    int max = 0;

    
    if(fptr == NULL)
    {
      printf("Error!\n");   
      exit(1);             
    }
    int rowCounter = 0;
    ch = fgetc(fptr); 

    //These next few lines create an array of 'struct' 'pipe' values and fills the 'tag' values of each element
    struct pipe records[128];
    while (ch != EOF) 
    { 
        if(ch != '\n'){
           word[wordCounter] = ch;
        }
        if(ch == '\n')
        {
            max++;
            rowCounter ++;
            wordCounter = 0;
            memset(word, 0, sizeof word);
        } else {
            if(ch != '\n'){
              records[rowCounter].tag[wordCounter] = ch;
              wordCounter ++;
            }
        }
        ch = fgetc(fptr); 
        
    } 

    //These next few lines instantiate the 'counter', 'nop', and 'isNotNull' values of the 'records' array and fills the 'pipeline' array with '.' values
    int orderNum = 1;
    for(int i = 0; i < rowCounter; i++)
    {
        records[i].order = orderNum;
        records[i].nop = false;
        records[i].isNotNull = true;
        orderNum ++;
        for(int j = 0; j < CPUcycles; j++)
        {
            records[i].pipeline[j][0] = '.';
        }
    }


    //These next 34 lines instantiate local variables that will help with doing the algorithm for the simulation
    int ifColumn = 0;
    int ifCounter = 0;
    int startIF = 0;
    bool useIF = false;

    int idColumn = 0;
    int idCounter = 1;
    int startID = 1;
    bool useID = false;

    int exColumn = 0;
    int exCounter = 2;
    int startEX = 2;
    bool useEX = false;

    int memColumn = 0;
    int memCounter = 3;
    int startMEM = 3;
    bool useMEM = false;

    int wbColumn = 0;
    int wbCounter = 4;
    int startWB = 4;
    bool useWB = false;

    
    int progressCounter = 0;
    int whileCounter = 0;

    int nopCounter = 0;

    int nopStopper = 7;
    int nopHasIncreased = 0;
    bool thereIsAnotherNop = false;
    printf("START OF SIMULATION\n\n");

    //This 'while' loop contains the entire process for doing the simulation
    while(WB_counter != max)
    {
        printf("CPU Cycles ===>\t1\t2\t3\t4\t5\t6\t7\t8\t9\n");
        for(int i = 0; i < rowCounter; i++)
        {
            printf("%s",records[i].tag);

            //if a 'record' has not been checked it will receive its 'firstvariable' and 'mainValue' values 
            if(!records[i].hasBeenChecked)
            {
                for(int y = 0; y < sizeof(records[i].tag); y++)
                {
                    if(records[i].tag[y] == 't')
                    {
                        records[i].firstVariable = 't';
                        records[i].mainValue = records[i].tag[y+1] - '0';
                        break;
                    }else if(records[i].tag[y] == 's')
                    {
                        records[i].firstVariable = 's';
                        records[i].mainValue = records[i].tag[y+1] - '0';
                        break;
                    }
                }
                records[i].hasBeenChecked = true;
            }

            //this for loop contains the sequence that checks and inserts the 'nop' operator if it fulfills the requirements
            for(int z = i; z < rowCounter; z++)
            {
                //this if statement will make sure that only 2 different instructions are being checked
                if(records[z].order != records[i].order)
                {
                    //This for loop will go through the the current 'tag' of the 'record'
                    for(int l = 0; l < sizeof(records[z].tag); l++)
                    {
                        //These next 3 'if' statements will check to see if the 'mainValue' and 'firstVariable' of a previous instruction matches any of the varaibles in the current instruction being checked then it will also check to make sure that the previous instruction also has reached its MEM phase and has not reached its WB phase. If all of these requirements are met then that means a 'nop' instruction has to be created and inputted
                        if(isdigit(records[z].tag[l]))
                        {
                            if(!isdigit(records[z].tag[l+1]))
                            {
                                if(records[i].firstVariable == records[z].tag[l-1] && records[i].mainValue == (records[z].tag[l] - '0') && records[i].hasReachedMEM && records[z].hasReachedID && records[z].hasReachedIF && !records[i].WBready)
                                {
                                    
                                    records[z].checkedForNop = true;

                                    //This for loop will make sure that all of the elements in the 'records' array that are in the current index placement and beyond will increment by one so that the current index will have a 'nop' instruction in it
                                    for(int a = rowCounter - 1; a >= i+1; a--)
                                    {
                                        memcpy(&records[a+1],&records[a],128);
                                    }

                                    //These next 3 lines will clear the current 'tag' of its previous instruction and replace it with a 'nop' as well as give the record variable a true for its 'nop' struct value to indicate that it is indeed a 'nop' instruction
                                    memset(records[z].tag,0,25);
                                    strcat(records[z].tag,"nop\t\t");
                                    records[z].nop = true;

                                    //This for loop will take the values in the 'pipeline' array from the previous instruction and input it into the 'nop' 'pipeline' array
                                    for(int a = 0; a < 9; a++)
                                    {
                                        memset(records[z].pipeline[a],0,3);
                                        if(records[z-1].checkedForNop)
                                        {
                                            strcat(records[z].pipeline[a],records[z-1].pipeline[a]);
                                            
                                        }else{
                                            strcat(records[z].pipeline[a],records[z+1].pipeline[a]);
                                        }
                                    }
                                    records[z].checkedForNop = true;

                                    //These next 4 lines will help with outputting for 'nop' instructions as well as insure that no segmentation faults occur by decreasing the number of '*' a nop instruction can use, adding an extra value to the 'rowCounter' to ensure that the new 'nop' instruction can be viewed, and incrementing the current for loop so the same value is not examined again
                                    nopStopper--;
                                    rowCounter++;
                                    nopCounter = idCounter+1;
                                    z++;
                                    
                                    //These next 2 lines are used to see if the next instruction is also one that needs a 'nop' instruction
                                    if(!records[z+1].isNotNull)
                                    {
                                        if(records[z].tag[0]=='s' && records[z].tag[1] == 'w')
                                        {
                                            nopCounter = idCounter;
                                            if(nopHasIncreased < 1){
                                                nopStopper++;
                                            }
                                        }
                                    }
                                    if(records[z+1].order != records[i].order && records[z+1].isNotNull)
                                    {
                                        
                                        for(int m = 0; m < sizeof(records[z+1].tag); m++)
                                        {
                                            if(isdigit(records[z+1].tag[m]))
                                            {
                                                if(records[i].firstVariable == records[z+1].tag[m-1] && records[i].mainValue == (records[z+1].tag[m] - '0') && records[z+1].skip == false)
                                                {
                                                    thereIsAnotherNop = true;
                                                    records[z+1].skip = true;
                                                }
                                            }
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            
            //this if statement checks to see if a record element is a nop and if it is a sequence of code lines will be implemented to print out the 'nop' instruction
            if(records[i].nop)
            {
                for(int a = 0; a < 9; a++)
                {
                    if(a == nopCounter)
                    {
                        memset(records[i].pipeline[a],0,3);
                        records[i].pipeline[a][0] = '*';
                    }
                    if(a != 8)
                    {
                        printf("%s\t",records[i].pipeline[a]);
                    }else{
                        printf("%s",records[i].pipeline[a]);
                    }
                }
                if(thereIsAnotherNop)
                {
                    printf("\n");
                    printf("%s",records[i].tag);
                    thereIsAnotherNop = false;
                    for(int a = 0; a < 9; a++)
                    {
                        if(a == nopCounter)
                        {
                            memset(records[i].pipeline[a],0,3);
                            records[i].pipeline[a][0] = '*';
                        }
                        if(a != 8)
                        {
                            printf("%s\t",records[i].pipeline[a]);
                        }else{
                            printf("%s",records[i].pipeline[a]);
                        }
                    }
                }
                if(!records[i+1].nop)
                {
                    if(nopCounter < nopStopper)
                    {
                        nopCounter ++;
                        nopHasIncreased++;
                    }
                }

            }else{
                /*
                This entire for loop contains various 'if' statements that check to see if any of the pipeline values can be inserted, such as IF,ID,MEM,etc.

                this is to see if they reach the point where these values can be used.

                This is done by checking the 'column', 'counter', and 'start' values of each pipeline value as well as the progress in which the simulation has reached

                Once one of these values has been reached the 'column' and 'counter' values will be incremented so that the next instruction can use the pipeline value
                */
                for(int j = 0; j < 9; j++)
                {
                    if(i == ifColumn && j == ifCounter && progressCounter >= startIF && j == progressCounter)
                    {
                        memset(records[i].pipeline[j],0,3);
                        strcat(records[i].pipeline[j], IF);
                        useIF = true;
                        records[i].hasReachedIF = true;
                    }else if(i == idColumn && j == idCounter && progressCounter >= startID && i <= progressCounter)
                    {
                        memset(records[i].pipeline[j],0,3);
                        strcat(records[i].pipeline[j], ID);
                        useID = true;
                        records[i].hasReachedID = true;
                    }else if(i == exColumn && j == exCounter && progressCounter >= startEX && i <= progressCounter)
                    {
                        memset(records[i].pipeline[j],0,3);
                        strcat(records[i].pipeline[j], EX);
                        useEX = true;
                        records[i].hasReachedMEM = true;
                    }else if(i == memColumn && j == memCounter && progressCounter >= startMEM && i <= progressCounter)
                    {
                        memset(records[i].pipeline[j],0,3);
                        strcat(records[i].pipeline[j], MEM);
                        useMEM = true;
                    
                    }else if(i == wbColumn && j == wbCounter && progressCounter >= startWB && i <= progressCounter)
                    {
                        memset(records[i].pipeline[j],0,3);
                        strcat(records[i].pipeline[j], WB);
                        useWB = true;
                        records[i].WBready = true;
                        //records[i].ready = true;
                    }
                    printf("\t%s",records[i].pipeline[j]);
                }
            }

            //These next sequence of 'if' statements will check to see if the variables that are used to implement the pipeline values can be incremented so that the next instruction can use them
            if(useIF && whileCounter > ifCounter)
            {
                ifColumn += 1;
                ifCounter += 1;
            }
            if(useID && whileCounter > idCounter)
            {
                idColumn += 1;
                idCounter += 1;

            }
            if(useEX && whileCounter > exCounter)
            {
                exColumn += 1;
                exCounter += 1;
            }
            if(useMEM && whileCounter > memCounter)
            {
                memColumn += 1;
                memCounter += 1;
            }
            if(useWB && whileCounter > wbCounter)
            {
                wbColumn += 1;
                wbCounter += 1;
            }
            
            printf("\n");
        }
        printf("\n");
        progressCounter += 1;

        whileCounter += 1;
        WB_counter = 0;

        //This 'for' loop will got through each of the elements in the array and see how many have reached the WB stage and add a number to the 'WB_counter' which is used as the base case for the while loop. If the value of the 'WB_counter' is the same as the number of instructions that were first implemented into te program then the while loop will end thus ending the simulation
        for(int v = 0; v < rowCounter; v++)
        {
            if(records[v].WBready)
            {
                WB_counter += 1;
            }
        }
    }
    printf("END OF SIMULATION\n");

    fclose(fptr);
    return EXIT_SUCCESS;
}