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

    bool skipThisDuringSequence;

    int cycle_stop = 17;
	
	bool isNotNull;
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

};

int main(int argc, char* argv[]){

    ifstream myReadFile;
    myReadFile.open(argv[2]);
    //char output[50];
    if (myReadFile.fail()) {
        std::cerr << "ERROR: cannot open " << argv[2] << " to read" << std::endl;
        exit(0);
    }

    //This is used to hold the 2nd element of the argument that is entered by the user and sees if forwarding is used ('Y') or if it isn't used ('N')
    string isForwarding = argv[1];
    int n = 0;
    char ch;
    //char word[25];
    int rowCounter = 0;
    int closeCounter = 0;
    while(!myReadFile.eof())
    {
        myReadFile.get(ch);
        if(ch == '\n')
        {
            closeCounter++;
        }
    }

    struct pipe records[128];

    ifstream myReadFile2;
    myReadFile2.open(argv[2]);
	int label = -1;
    //This while loop will go through the file being read and create the 'tag' value for each element in the 'records' array
    while(!myReadFile2.eof())
    {

        if(rowCounter == closeCounter-1)
        {
            break;
        }
        myReadFile2.get(ch);
        //word[n] = ch;

        //These next 12 lines of code will give each of the struct variables a value for their 'instructionCommad' property. So it will have values like 'add', 'slti', etc. so that later on in the program these instructions can be pulled off if need be
        if(ch == ' ' || ch == ':')
        {
			if (ch == ':') {
				label = rowCounter;
			}
            if(records[rowCounter].tag[n] == ':')
            {
				
                for(int k = 0; k < n; k++)
                {
                    records[rowCounter].instructionCommand[k] = records[rowCounter].tag[k];
                }
            }else{
                records[rowCounter].instructionCommand = records[rowCounter].tag;
            }
        }
        if(ch == '\n')
        {
            //word[0] = 0;
            n = 0;
            if(strcmp(records[rowCounter].tag,"loop:") == 0)
            {
                memset(records[rowCounter].tag,0,25);
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
        //cout << "RECORDS[I].TAG IS: " << records[i].tag << endl;
        records[i].isNotNull = true;
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

	for(int i = 0; i < rowCounter; i++) {
        if(records[i].instructionCommand == "and")
            cout << records[i].instructionCommand << endl;
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
    //bool thereIsAnotherNop = false;
    //This 'while' loop contains the entire process for doing the simulation
	int cycle = 0;
    while(WB_Counter != max && cycle < 16)
    {
        cout << "----------------------------------------------------------------------------------\n";
        cout << "CPU Cycles ===>     1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16\n";
        for(int i = 0; i < rowCounter; i++)
        {
            if(records[i].ready)
            {
                cout << records[i].tag;
				if (records[i].tag[0] == 'n') {
					cout << "                 ";
				}
				else {
					for (int a = records[i].sizeOf; a < 20; a++) {
						cout << " ";
					}
				}

                //if a 'record' has not been checked it will receive its 'firstvariable' and 'mainValue' values 
                if(!records[i].hasBeenChecked)
                {
                    for(int y = 0; y < (int)sizeof(records[i].tag); y++)
                    {
                        if(records[i].tag[y] == 't' && records[i].tag[y-1] == '$')
                        {
                            records[i].firstVariable = 't';
                            records[i].mainValue = records[i].tag[y+1] - '0';
                            //cout << "The first variable for " << records[i].tag << " is " << records[i].firstVariable << " and its main value is " << records[i].mainValue << endl;
                            break;
                        }else if(records[i].tag[y] == 's' && records[i].tag[y-1] == '$')
                        {
                            records[i].firstVariable = 's';
                            records[i].mainValue = records[i].tag[y+1] - '0';
                            //cout << "The first variable for " << records[i].tag << " is " << records[i].firstVariable << " and its main value is " << records[i].mainValue << endl;
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
                            for(int l = 0; l < (int)sizeof(records[z].tag); l++)
                            {
                                //These next 3 'if' statements will check to see if the 'mainValue' and 'firstVariable' of a previous instruction matches any of the varaibles in the current instruction being checked then it will also check to make sure that the previous instruction also has reached its MEM phase and has not reached its WB phase. If all of these requirements are met then that means a 'nop' instruction has to be created and inputted
                                if(isdigit(records[z].tag[l]))
                                {
                                    if(!isdigit(records[z].tag[l+1]))
                                    {
                                        if(records[i].firstVariable == records[z].tag[l-1] && records[i].mainValue == (records[z].tag[l] - '0') && records[i].hasReachedMEM && records[z].hasReachedID && records[z].hasReachedIF && !records[i].WBready && records[z].checkedForNop == false)
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
                                            strcat(records[z].tag,"nop");
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
                                            //records[z+1].instructionCommand = records[z].instructionCommand;
                                            //records[z].instructionCommand = "nops";

                                            //These next 4 lines will help with outputting for 'nop' instructions as well as insure that no segmentation faults occur by decreasing the number of '*' a nop instruction can use, adding an extra value to the 'rowCounter' to ensure that the new 'nop' instruction can be viewed, and incrementing the current for loop so the same value is not examined again
                                            nopStopper--;
                                            rowCounter++;
                                            nopCounter = idCounter;
                                            z++;
                                            /*if(!records[z+1].isNotNull)
                                            {
                                                if(records[z].tag[0]=='s' && records[z].tag[1] == 'w')
                                                {
                                                    nopCounter = idCounter;
                                                    if(nopHasIncreased < 1){
                                                            nopStopper++;
                                                    }
                                                }
                                            }*/
                                            //cout << "HERE HERE HERE HERE HERE HERE HERE HERE HERE PASS" << endl;
                                            //cout << "RECORDS[Z+1] IS " << records[z+1].tag << " AND ITS VALUE FOR isNotNull IS " << records[z+1].isNotNull << endl;
                                            bool skipNopStopper = false;
                                            if(records[z+1].order > 0)
                                            {
                                                records[z+1].isNotNull = true;
                                                skipNopStopper = true;
                                            }
                                            if(records[z+1].order != records[i].order && records[z+1].isNotNull)
                                            {
                                                //cout << "PASS" << endl;
                                        
                                                for(int m = 0; m < (int)sizeof(records[z+1].tag); m++)
                                                {
                                                    if(isdigit(records[z+1].tag[m]))
                                                    {
                                                        if(records[i].firstVariable == records[z+1].tag[m-1] && records[i].mainValue == (records[z+1].tag[m] - '0') && records[z+1].skip == false)
                                                        {
                                                            thereIsAnotherNop = true;
                                                            records[z+1].skip = true;
                                                            if(!skipNopStopper){
                                                                nopStopper--;
                                                            }

                                                            //records[z+1].nop = true;
                                                            //rowCounter++;

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
                }

                if(records[i].nop) {
					for(int a = 0; a < 16; a++) {
                        if(a == nopCounter)
                        {
                            memset(records[i].pipeline[a],0,3);
                            records[i].pipeline[a][0] = '*';
                        }
						
						cout << records[i].pipeline[a];
						if (a != 15) {
						   if (records[i].pipeline[a][0] == '.' || records[i].pipeline[a][0] == '*') {
							   cout << "   ";
						   }
						   else {
							  cout << "  "; 
						   }
					    }
					   
                    }
					cout << endl;
                    if(thereIsAnotherNop)
                    {
                        printf("%s",records[i].tag);
						cout << "                 ";
                        //thereIsAnotherNop = false;
                        for(int a = 0; a < 16; a++) {
							if(a == nopCounter) {
								memset(records[i].pipeline[a],0,3);
								records[i].pipeline[a][0] = '*';
							}
							
							cout << records[i].pipeline[a];
							if (a != 15) {
							   if (records[i].pipeline[a][0] == '.' || records[i].pipeline[a][0] == '*') {
								   cout << "   ";
							   }
							   else {
								  cout << "  "; 
							   }
							}
						}
						cout << endl;
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
						   if (j > records[i].cycle_stop) {
							   records[i].pipeline[j][0] = '*';
						   }
						   else {
							records[i].pipeline[j][0] = 'I';
							records[i].pipeline[j][1] = 'F';
						   }
							useIF = true;
							records[i].hasReachedIF = true;
                          
                       }
                       if(j == idCounter && i == idColumn && progressCounter >= startID)
                       {
						   if (j > records[i].cycle_stop) {
							   records[i].pipeline[j][0] = '*';
						   }
						   else {
                          records[i].pipeline[j][0] = 'I';
                          records[i].pipeline[j][1] = 'D';
						   }
                          useID = true;
                          records[i].hasReachedID = true;
                       }
                       if(j == exCounter && i == exColumn && progressCounter >= startEX)
                       {
						   if (j > records[i].cycle_stop) {
							   records[i].pipeline[j][0] = '*';
						   }
						   else {
                          records[i].pipeline[j][0] = 'E';
                          records[i].pipeline[j][1] = 'X';
						   }
                          useEX = true;
                          records[i].hasReachedMEM = true;
                       }
                       if(j == memCounter && i == memColumn && progressCounter >= startMEM)
                       {
						   if (j > records[i].cycle_stop) {
							   records[i].pipeline[j][0] = '*';
						   }
						   else {
                          records[i].pipeline[j][0] = 'M';
                          records[i].pipeline[j][1] = 'E';
                          records[i].pipeline[j][2] = 'M';
						   }
                          useMEM = true;
                       }
                       if(j == wbCounter && i == wbColumn && progressCounter >= startWB)
                       {
						   // cout << j << " " << records[i].cycle_stop << endl;
						   if (j > records[i].cycle_stop) {
							   records[i].pipeline[j][0] = '*';
						   }
						   else {
							   records[i].pipeline[j][0] = 'W';
						       records[i].pipeline[j][1] = 'B';
						   }
                          
                          records[i].valueReady = true;
                          records[i].WBready = true;

                            //Each time an instruction reaches WB this will increment by 1 and once it reaches the same value as the number of instructions that was implemented into the program at the beginning it will end the simulation
                          WB_Counter += 1;
                          useWB = true;
                       }
                       cout << records[i].pipeline[j];
					   if (j != 15) {
						   if (records[i].pipeline[j][0] == '.' || records[i].pipeline[j][0] == '*') {
							   cout << "   ";
						   }
						   else if (records[i].pipeline[j][0] == 'M') {
							   cout << " ";
						   }
						   else {
							  cout << "  "; 
						   }
					   }
                    }
					if (label != -1 && i+1 == rowCounter && records[i].hasReachedIF == true && records[i].hasReachedID == false) {
						WB_Counter = -1000;
						// cout << "Repeat here\n" << endl;
						for (int a = 0; a < rowCounter-label; a++) {
							strcat(records[rowCounter+a].tag, records[label+a].tag);
							records[rowCounter+a].sizeOf = records[label+a].sizeOf;
							records[rowCounter+a].order = records[label+a].order;
							records[rowCounter+a].mainValue = records[label+a].mainValue;
							records[rowCounter+a].firstVariable = records[label+a].firstVariable;
							records[rowCounter+a].operatorValue = records[label+a].operatorValue;
							records[rowCounter+a].instructionCommand = records[label+a].instructionCommand;
							for(int b = 0; b < 16; b++)
							{
								records[rowCounter+a].pipeline[b][0] = '.';
							}
							// cout << rowCounter+a << " is now " << records[a+label].tag << endl;
						}
						// cout << endl;
						for (int a = rowCounter-1; a >= 0; a--) {
							if (records[a].instructionCommand == "bne") {
								// cout << a << " at " << records[rowCounter-a].tag << " is a bne" << endl;
								break;
							}
							else {
								// cout << a << " at " << records[a].tag << " is now stoppped at " << cycle << endl;
								records[a].cycle_stop = cycle;
							}
						}
						rowCounter+=(rowCounter-label);
					}
                   cout << endl;
                }//end of if records[i].nop
           }//End of if records[i].ready
		   // cout << i << endl;
			
        }
		cout << endl;
		//End of main for loop
        /*
        This next for loop contains a sequence of code that will output each of the operators ($s0-$s7 and $t0-$t9) and show what 
        their 'value' is by checking to see if they match any of the 'firstVariable' values in the instructions and give them 
        the 'operatorValue' of that instruction
        It will give these operators values based on their instruction. For example instructions like add, addi, and an ori 
        instruction that is the first instruction of the entire sequence will give values based on their operatorValue. But 
        if said operatorValue is 0 then we will go through all of the operators of the current instruction and add values to 
        the current operator based on their values. Then for instructions like 'bne', 'beq', etc. we will still go through the 
        instruction, take the values of operators and actual numbers that are in the instruction, put them in temporary variables, 
        and then compare them and put the value from the comparison into the operator we are currently on, meaning that the operator 
        will either have a '1' or '0' value for these types of instructions.
        */
        for(int z = 0; z < 18; z++)
        {
            for(int y = 0; y < rowCounter; y++)
            {
                
                if(records[y].valueReady && records[y].mainValue == operators[z].mainValue && operators[z].variable == records[y].firstVariable && records[y].skipThisDuringSequence == false)
                {
                    if((records[y].instructionCommand == "add" || records[y].instructionCommand == "addi" || (records[y].instructionCommand == "ori" && y == 0)) && records[y].operatorValue != 0)
                    {
                        operators[z].value += records[y].operatorValue;
                    }else if(records[y].operatorValue == 0)
                    {
                        int commas = 0;
                        int val1 = 0;
                        int val2 = 0;
                        for(int instruct = 0; instruct < (int)sizeof(records[y].tag); instruct++)
                        {
                            if(records[y].tag[instruct] == ',')
                            {
                                commas++;
                            }
                            if((commas < 2 && commas > 0) && (records[y].tag[instruct] == 't' || records[y].tag[instruct] == 's'))
                            {
                                char ins[2];
                                ins[0] = records[y].tag[instruct];
                                ins[1] = records[y].tag[instruct+1];
                                for(int r = 0; r < 18; r++ ){
                                    if(ins[0] == operators[r].variable && (ins[1]-'0') == operators[r].mainValue)
                                    {
                                        val1 = operators[r].value;
                                    }
                                }
                                instruct++;
                            }else if((commas > 1) && (records[y].tag[instruct] == 't' || records[y].tag[instruct] == 's'))
                            {
                                char ins[2];
                                ins[0] = records[y].tag[instruct];
                                ins[1] = records[y].tag[instruct+1];
                                for(int r = 0; r < 18; r++ ){
                                    if(ins[0] == operators[r].variable && (ins[1]-'0') == operators[r].mainValue)
                                    {
                                        val2 = operators[r].value;
                                    }
                                }
                                instruct++;
                            }
                        }
                        operators[z].value += (val1+val2);   
                    }else if(records[y].instructionCommand == "slti")
                    {
                        int commas = 0;
                        int val1 = 0;
                        int val2 = 0;
                        for(int instruct = 0; instruct < (int)sizeof(records[y].tag); instruct++)
                        {
                            if(records[y].tag[instruct] == ',')
                            {
                                commas++;
                            }
                            if((commas < 2 && commas > 0) && (records[y].tag[instruct] == 't' || records[y].tag[instruct] == 's'))
                            {
                                char ins[2];
                                ins[0] = records[y].tag[instruct];
                                ins[1] = records[y].tag[instruct+1];
                                for(int r = 0; r < 18; r++ ){
                                    if(ins[0] == operators[r].variable && (ins[1]-'0') == operators[r].mainValue)
                                    {
                                        val1 = operators[r].value;
                                    }
                                }
                                instruct++;
                            }else if((commas < 2 && commas > 0) && isdigit(records[y].tag[instruct]) && records[y].tag[instruct-1] == ',')
                            {
                                char ins[4];
                                int insCounter = 0;
                                while(isdigit(records[y].tag[instruct]))
                                {
                                    ins[insCounter] = records[y].tag[instruct];
                                    insCounter++;
                                    instruct++;
                                }
                                val1 = stoi(ins);

                            }else if((commas > 1) && (records[y].tag[instruct] == 't' || records[y].tag[instruct] == 's'))
                            {
                                char ins[2];
                                ins[0] = records[y].tag[instruct];
                                ins[1] = records[y].tag[instruct+1];
                                for(int r = 0; r < 18; r++ ){
                                    if(ins[0] == operators[r].variable && (ins[1]-'0') == operators[r].mainValue)
                                    {
                                        val2 = operators[r].value;
                                    }
                                }
                                instruct++;
                            }else if((commas > 1) && isdigit(records[y].tag[instruct]) && records[y].tag[instruct-1] == ',')
                            {
                                char ins[4];
                                int insCounter = 0;
                                while(isdigit(records[y].tag[instruct]))
                                {
                                    ins[insCounter] = records[y].tag[instruct];
                                    insCounter++;
                                    instruct++;
                                }
                                val2 = stoi(ins);

                            }
                        }
                        operators[z].value = (val1 < val2);

                    }else if(records[y].instructionCommand == "beq")
                    {
                        int commas = 0;
                        int val1 = 0;
                        int val2 = 0;
                        for(int instruct = 0; instruct < (int)sizeof(records[y].tag); instruct++)
                        {
                            if(records[y].tag[instruct] == ',')
                            {
                                commas++;
                            }
                            if((commas < 2 && commas > 0) && (records[y].tag[instruct] == 't' || records[y].tag[instruct] == 's'))
                            {
                                char ins[2];
                                ins[0] = records[y].tag[instruct];
                                ins[1] = records[y].tag[instruct+1];
                                for(int r = 0; r < 18; r++ ){
                                    if(ins[0] == operators[r].variable && (ins[1]-'0') == operators[r].mainValue)
                                    {
                                        val1 = operators[r].value;
                                    }
                                }
                                instruct++;
                            }else if((commas < 2 && commas > 0) && isdigit(records[y].tag[instruct]) && records[y].tag[instruct-1] == ',')
                            {
                                char ins[4];
                                int insCounter = 0;
                                while(isdigit(records[y].tag[instruct]))
                                {
                                    ins[insCounter] = records[y].tag[instruct];
                                    insCounter++;
                                    instruct++;
                                }
                                val1 = stoi(ins);

                            }else if((commas > 1) && (records[y].tag[instruct] == 't' || records[y].tag[instruct] == 's'))
                            {
                                char ins[2];
                                ins[0] = records[y].tag[instruct];
                                ins[1] = records[y].tag[instruct+1];
                                for(int r = 0; r < 18; r++ ){
                                    if(ins[0] == operators[r].variable && (ins[1]-'0') == operators[r].mainValue)
                                    {
                                        val2 = operators[r].value;
                                    }
                                }
                                instruct++;
                            }else if((commas > 1) && isdigit(records[y].tag[instruct]) && records[y].tag[instruct-1] == ',')
                            {
                                char ins[4];
                                int insCounter = 0;
                                while(isdigit(records[y].tag[instruct]))
                                {
                                    ins[insCounter] = records[y].tag[instruct];
                                    insCounter++;
                                    instruct++;
                                }
                                val2 = stoi(ins);

                            }
                        }
                        operators[z].value = (val1 == val2);

                    }else if(records[y].instructionCommand == "bne")
                    {
                        int commas = 0;
                        int val1 = 0;
                        int val2 = 0;
                        for(int instruct = 0; instruct < (int)sizeof(records[y].tag); instruct++)
                        {
                            if(records[y].tag[instruct] == ',')
                            {
                                commas++;
                            }
                            if((commas < 2 && commas > 0) && (records[y].tag[instruct] == 't' || records[y].tag[instruct] == 's'))
                            {
                                char ins[2];
                                ins[0] = records[y].tag[instruct];
                                ins[1] = records[y].tag[instruct+1];
                                //cout << ins << endl;
                                for(int r = 0; r < 18; r++ ){
                                    if(ins[0] == operators[r].variable && (ins[1]-'0') == operators[r].mainValue)
                                    {
                                        //cout << "The tag we are getting a value from is: " << operators[r].tag << endl;
                                        val1 = operators[r].value;
                                    }
                                }
                                instruct++;
                            }else if((commas < 2 && commas > 0) && isdigit(records[y].tag[instruct]) && records[y].tag[instruct-1] == ',')
                            {
                                char ins[4];
                                int insCounter = 0;
                                while(isdigit(records[y].tag[instruct]))
                                {
                                    ins[insCounter] = records[y].tag[instruct];
                                    insCounter++;
                                    instruct++;
                                }
                                val1 = stoi(ins);

                            }else if((commas > 1) && (records[y].tag[instruct] == 't' || records[y].tag[instruct] == 's'))
                            {
                                char ins[2];
                                ins[0] = records[y].tag[instruct];
                                ins[1] = records[y].tag[instruct+1];
                                for(int r = 0; r < 18; r++ ){
                                    if(ins[0] == operators[r].variable && (ins[1]-'0') == operators[r].mainValue)
                                    {
                                        val2 = operators[r].value;
                                    }
                                }
                                instruct++;
                            }else if((commas > 1) && isdigit(records[y].tag[instruct]) && records[y].tag[instruct-1] == ',')
                            {
                                char ins[4];
                                int insCounter = 0;
                                while(isdigit(records[y].tag[instruct]))
                                {
                                    ins[insCounter] = records[y].tag[instruct];
                                    insCounter++;
                                    instruct++;
                                }
                                
                                val2 = stoi(ins);

                            }
                        }
                        operators[z].value = (val1 != val2);

                    }
                    records[y].skipThisDuringSequence = true;
                }
            }
            
            cout << operators[z].tag << " = " << operators[z].value;
            if(operators[z].mainValue == 3 || operators[z].mainValue == 7 || operators[z].mainValue == 9)
            {
                cout << endl;
            }
			else {
				for (int a = std::to_string(operators[z].value).length(); a < 14; a++) {
					cout << " ";
				}
			}
        }
        // cout << endl;
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
        cycle++;
    }
    cout << "----------------------------------------------------------------------------------\n";
    cout << "END OF SIMULATION" << endl;
	// cout << "Label " << label << endl;



    myReadFile.close();
    return 0;
}
