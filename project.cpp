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
    char operatorValue[5];
};

struct op
{
    //'tag' is so operators like 's1' can be outputted
	char tag[4];

    //'varaible' is used to identitfy whether the element is a 't' or an 's'
	char variable;

    //'value' will contain the value that will be outputted after the '=' for the operator. ex: $s1 = 124
	char value[5];

    //'mainValue' is used to reference the first numerical value in the instruction. ex: the main value for $s1 would be 1 in this case
	int mainValue;

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
		if(ch == '\n')
		{
			
			//cout << word << endl;
			word[0] = 0;
			n = 0;
			rowCounter++;
		}else{
			records[rowCounter].tag[n] = ch;
			records[rowCounter].sizeOf++;
			n++;
		}
	}
	
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
    						records[i].operatorValue[valueCounter] = records[i].tag[j+1];
    						valueCounter++;
    						j++;
    					}
    					valueCounter= 0;

    					break;
    				}else{
    					records[i].operatorValue[0] = '0';
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
    		operators[i].value[0] = '0';
    	}else{
    		operators[i].tag[0] = '$';
    		operators[i].tag[1] = 't';
    		operators[i].tag[2] = '0' + opCounter;
    		operators[i].mainValue = opCounter;
    		operators[i].variable = 't';
    		operators[i].value[0] = '0';
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
					}
					if(j == idCounter && i == idColumn && progressCounter >= startID)
					{
						records[i].pipeline[j][0] = 'I';
						records[i].pipeline[j][1] = 'D';
						useID = true;
					}
					if(j == exCounter && i == exColumn && progressCounter >= startEX)
					{
						records[i].pipeline[j][0] = 'E';
						records[i].pipeline[j][1] = 'X';
						useEX = true;
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

                        //Each time an instruction reaches WB this will increment by 1 and once it reaches the same value as the number of instructions that was implemented into the program at the beginning it will end the simulation
						WB_Counter += 1;
						useWB = true;
					}
					cout << records[i].pipeline[j] << "\t";
				}
				cout << endl;
    		}
    	}
    	cout << endl;

        //This for loop contains a sequence of code that will output each of the operators ($s0-$s7 and $t0-$t9) and show what their 'value' is by checking to see if they match any of the 'firstVariable' values in the instructions and give them the 'operatorValue' of that instruction
    	for(int z = 0; z < 18; z++)
    	{
    		for(int y = 0; y < rowCounter; y++)
    		{
    			if(records[y].mainValue == operators[z].mainValue && records[y].valueReady && operators[z].variable == records[y].firstVariable)
    			{
    				for(int x = 0; x < 5; x++)
    				{
    					operators[z].value[x] = records[y].operatorValue[x];
    				}
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