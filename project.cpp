#include <iostream>
#include <fstream>
#include <cassert>
#include <vector>
#include <string>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <unordered_set>
#include <list>
using namespace std;

struct pipe
{
    //'tag' is so instructions like 'add $s1,$s1,$s1' can be outputted
    char tag[25];

    //'pipeline' is the array that stores the '.' or the pipeline values like 'ID', 'MEM', etc.    
    char pipeline[16][4];

    //'nop' is to inidicate whether a row in the array is a nop instruction or not
    bool nop;

    //'order' is used to reference the order in which the instructions go. ex: for the instructions add   sub   sw    add would be 1, sub would be 2, and sw would be 3
    int order;

    //This is really just used for spacing and does not play a major role in the program
    int sizeOf;

    //'mainValue' is used to reference the first numerical value in the instruction. ex: the main value for add $s1,$s2,$s3 would be 1 in this case
    int mainValue;

    //'WBready' is used in order to indicate when an insturction can and has used a WB value
    bool WBready;

    //'firstVariable' is used to reference the first variable ('s' or 't') in the instruction. ex: the first variable for add $t1,$s2,3 would be 't' in this case
    char firstVariable;

    //'ready' is used to see if an element can be examined
    bool ready;

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

    //'valueReady' is used to ensure that the 'mainValue' of the instruction can be examined and used
    bool valueReady;

    //'operatorValue' is used to express the value that is represented by the 'firstVariable' of the element. ex: add $s1,0,214    this add instruction would have a 'firstVariable' of $s1 and that would have an 'operatorValue' of 214
    int operatorValue;

    bool instructionBeforeLoop;

    string instructionCommand;
};

struct op
{
    //'tag' is so operators like 's1' can be outputted
    char tag[4];

    //'varaible' is used to identitfy whether the element is a 't' or an 's'
    char variable;

    //'value' will contain the value that will be outputted after the '=' for the operator. ex: $s1 = 124
    int value;

    //'mainValue' is used to reference the first numerical value in the instruction. ex: the main value for $s1 would be 1 in this case
    int mainValue;

    bool checked;

};

int main(int argc, char* argv[]){

    ifstream myReadFile;
    myReadFile.open(argv[2]);
    char output[50];
    if (myReadFile.fail()) {
        std::cerr << "ERROR: cannot open " << argv[2] << " to read" << std::endl;
        exit(0);
    }

    //This is used to hold the 2nd element of the argument that is entered by the user and sees if forwarding is used ('Y') or if it isn't used ('N')
    string isForwarding = argv[1];
    int n = 0;
    char ch;
    char word[25];
    int rowCounter = 0;
    int closeCounter = 0;
    while(!myReadFile.eof())
    {
        myReadFile.get(ch);;
        if(ch == '\n')
        {
            closeCounter++;
        }
    }

    struct pipe records[128];

    ifstream myReadFile2;
    myReadFile2.open(argv[2]);

    //This while loop will go through the file being read and create the 'tag' value for each element in the 'records' array
    while(!myReadFile2.eof())
    {

        if(rowCounter == closeCounter-1)
        {
            break;
        }
        myReadFile2.get(ch);
        //cout << ch;
        word[n] = ch;
        if(ch == ' ' || ch == ':')
        {
            if(records[rowCounter].tag[n] == ':')
            {
                for(int k = 0; k < n; k++)
                {
                    records[rowCounter].instructionCommand[k] = records[rowCounter].tag[k];
                    //cout << records[rowCounter].tag[k];
                }
                //cout << records[rowCounter].instructionCommand << endl;
            }else{
                records[rowCounter].instructionCommand = records[rowCounter].tag;
                //cout << records[rowCounter].instructionCommand << endl;
            }
        }
        if(ch == '\n')
        {
            
            //cout << word << endl;
            word[0] = 0;
            n = 0;
            if(strcmp(records[rowCounter].tag,"loop:") == 0)
            {
                memset(records[rowCounter].tag,25,0);
                records[rowCounter].sizeOf -= 5;
                records[rowCounter-1].instructionBeforeLoop = true;
                closeCounter--;
            }else{
                rowCounter++;
            }
        }else{
            records[rowCounter].tag[n] = ch;
            records[rowCounter].sizeOf++;
            n++;
        }
    }
    //cout << rowCounter << endl;
    //These next few lines instantiate the 'counter' and 'nop' values of the 'records' array and fills the 'pipeline' array with '.' values
    int orderNum = 1;
    for(int i = 0; i < rowCounter; i++)
    {
        records[i].order = orderNum;
        records[i].nop = false;
        orderNum ++;
        for(int j = 0; j < 16; j++)
        {
            records[i].pipeline[j][0] = '.';
        }
    }

    //This next for loop contains a sequence of code to give each 'record' element its 'operatorValue' which will give it a numerical value if a number is in the instruction or a '0' if there isn't
    int commaCounter = 0;
    int valueCounter = 0;
    for(int i = 0; i < rowCounter; i++)
    {
        for(int j = 0; j < 25; j++)
        {
            if(records[i].tag[j] == ',')
            {
                commaCounter++;
                if(commaCounter == 2)
                {
                    commaCounter = 0;
                    if(isdigit(records[i].tag[j+1]))
                    {
                        while(isdigit(records[i].tag[j+1]))
                        {
                            
                            records[i].operatorValue = records[i].operatorValue * 10;
                            records[i].operatorValue += records[i].tag[j+1] - '0';
                            
                            //records[i].operatorValue[valueCounter] = records[i].tag[j+1];
                            valueCounter++;
                            j++;
                        }
                        valueCounter= 0;

                        break;
                    }else{
                        records[i].operatorValue = 0;
                    }
                }
            }
        }
    }

    //This next for loop will create the struct array 'operators' which are the operators $s0-$s7 and $t0-$t9. These will be outputted at the end of each simulation step 
    struct op operators[18];
    int opCounter = 0;
    for(int i = 0; i < 18; i++)
    {
        if(i == 8)
        {
            opCounter = 0;
        }
        if(i < 8)
        {
            operators[i].tag[0] = '$';
            operators[i].tag[1] = 's';
            operators[i].tag[2] = '0' + opCounter;
            operators[i].mainValue = opCounter;
            operators[i].variable = 's';
            operators[i].value = 0;
        }else{
            operators[i].tag[0] = '$';
            operators[i].tag[1] = 't';
            operators[i].tag[2] = '0' + opCounter;
            operators[i].mainValue = opCounter;
            operators[i].variable = 't';
            operators[i].value = 0;
        }
        opCounter++;
    }


    if(isForwarding == "N")
    {
        cout << "START OF SIMULATION (no forwarding)\n";
    }else{
        cout << "START OF SIMULATION (forwarding)\n";
        
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

    //Each time an instruction reaches WB this will increment by 1 and once it reaches the same value as the number of instructions that was implemented into the program at the beginning it will end the simulation
    int WB_Counter = 0;

    records[0].ready = true;

    int max = rowCounter;

    int nopCounter = 0;

    int nopStopper = 7;
    int nopHasIncreased = 0;
    bool thereIsAnotherNop = false;

    //This 'while' loop contains the entire process for doing the simulation
    while(WB_Counter != max)
    {
        cout << "--------------------------------------------------------------------------------------------------------------------------------------------------\n";
        cout << "CPU Cycles ===>\t\t1\t2\t3\t4\t5\t6\t7\t8\t9\t10\t11\t12\t13\t14\t15\t16\n";
        for(int i = 0; i < rowCounter; i++)
        {
            if(records[i].ready)
            {
                if(records[i].sizeOf > 16)
                {
                    cout << records[i].tag << "\t";
                }else {
                    cout << records[i].tag << "\t\t";
                }

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
                //////////////////////////////////
                //      INPUT CODE FOR NOP      //
                //////////////////////////////////
                if(isForwarding == "N")
                {
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
                                            //printf("BEGIN NOP OPERATION\n");
                                            records[z].checkedForNop = true;

                                            //This for loop will make sure that all of the elements in the 'records' array that are in the current index placement and beyond will increment by one so that the current index will have a 'nop' instruction in it
                                            for(int a = rowCounter - 1; a >= i+1; a--)
                                            {
                                                memcpy(&records[a+1],&records[a],128);
                                            }

                                            //These next 3 lines will clear the current 'tag' of its previous instruction and replace it with a 'nop' as well as give the record variable a true for its 'nop' struct value to indicate that it is indeed a 'nop' instruction
                                            memset(records[z].tag,0,25);
                                            strcat(records[z].tag,"nop\t");
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
                                            nopCounter = idCounter;
                                            z++;
                                            break;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                if(records[i].nop)
                {
                    for(int a = 0; a < 16; a++)
                    {
                        if(a == nopCounter)
                        {
                            memset(records[i].pipeline[a],0,3);
                            records[i].pipeline[a][0] = '*';
                        }
                        if(a != 15)
                        {
                            //printf("%s\t",records[i].pipeline[a]);
                            cout << records[i].pipeline[a] << "\t";
                        }else{
                            //printf("%s",records[i].pipeline[a]);
                            cout << records[i].pipeline[a];
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

                    for(int j = 0; j < 16; j++)
                    {
                       if(j == ifCounter && i == ifColumn && progressCounter >= startIF)
                       {
                          records[i].pipeline[j][0] = 'I';
                          records[i].pipeline[j][1] = 'F';
                          useIF = true;
                          records[i].hasReachedIF = true;
                       }
                       if(j == idCounter && i == idColumn && progressCounter >= startID)
                       {
                          records[i].pipeline[j][0] = 'I';
                          records[i].pipeline[j][1] = 'D';
                          useID = true;
                          records[i].hasReachedID = true;
                       }
                       if(j == exCounter && i == exColumn && progressCounter >= startEX)
                       {
                          records[i].pipeline[j][0] = 'E';
                          records[i].pipeline[j][1] = 'X';
                          useEX = true;
                          records[i].hasReachedMEM = true;
                       }
                       if(j == memCounter && i == memColumn && progressCounter >= startMEM)
                       {
                          records[i].pipeline[j][0] = 'M';
                          records[i].pipeline[j][1] = 'E';
                          records[i].pipeline[j][2] = 'M';
                          useMEM = true;
                       }
                       if(j == wbCounter && i == wbColumn && progressCounter >= startWB)
                       {
                          records[i].pipeline[j][0] = 'W';
                          records[i].pipeline[j][1] = 'B';
                          records[i].valueReady = true;
                          records[i].WBready = true;

                            //Each time an instruction reaches WB this will increment by 1 and once it reaches the same value as the number of instructions that was implemented into the program at the beginning it will end the simulation
                          WB_Counter += 1;
                          useWB = true;
                       }
                       cout << records[i].pipeline[j] << "\t";
                    }
                   cout << endl;
                }//end of if records[i].nop
           }//End of if records[i].ready
           if(records[i].nop)
           {
                cout << endl;
           }
           cout << endl;
        }//End of main for loop

        //This for loop contains a sequence of code that will output each of the operators ($s0-$s7 and $t0-$t9) and show what their 'value' is by checking to see if they match any of the 'firstVariable' values in the instructions and give them the 'operatorValue' of that instruction
        for(int z = 0; z < 18; z++)
        {
            for(int y = 0; y < rowCounter; y++)
            {
                if(records[y].mainValue == operators[z].mainValue && records[y].valueReady && operators[z].variable == records[y].firstVariable && operators[z].checked == false)
                {
                    bool skipThis = false;
                    //for(int x = 0; x < 5; x++)
                    //{
                    //cout << "(";
                    for(int p = 0; p < sizeof(records[y].tag); p++)
                    {
                        for(int q = 0; q < rowCounter; q++)
                        {
                            if(records[y].tag[p] == records[q].firstVariable && records[y].order != records[q].order)
                            {
                                if((records[y].tag[p+1] - '0') == records[q].mainValue)
                                {
                                    //cout << records[y].tag << " MATCHES " << records[q].tag << " for the operator value: " << operators[z].tag << " and the value " << records[q].operatorValue << " will be put in";
                                    operators[z].value += records[q].operatorValue;
                                    /*for(int n = 0; n < z; n++)
                                    {
                                        if(records[y].tag[1] == operators[n].tag[1] && records[n].tag[2] == operators[z].tag[2])
                                        {
                                            cout << records[y].tag[p] << records[y].tag[p+1] << " MATCHES " << records[q].firstVariable << records[q].mainValue;
                                        }
                                    }*/
                                    skipThis = true;
                                }
                            }
                        }
                    }
                    //cout << ")"; 
                    if(skipThis == false)
                    {
                        operators[z].value += records[y].operatorValue;
                    }
                    operators[z].checked = true;
                    //}
                }
            }
            cout << operators[z].tag << " = " << operators[z].value << " \t";
            if(operators[z].mainValue == 3 || operators[z].mainValue == 7)
            {
                cout << endl;
            }
        }
        cout << endl;
        whileCounter++;

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
        progressCounter++;
        if(progressCounter < rowCounter)
        {
            records[progressCounter].ready = true;
        }
        
    }
    cout << "--------------------------------------------------------------------------------------------------------------------------------------------------\n";
    cout << "END OF SIMULATION" << endl;




    myReadFile.close();
    return 0;
}