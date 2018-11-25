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
	char tag[25];
	char pipeline[16][4];
	bool nop;
	int order;
	int sizeOf;
	int t_value;
	int s_value;
    int mainValue;
	bool WBready;
	char firstVariable;
	bool ready;
	bool hasBeenChecked;
    bool hasReachedMEM;
    bool hasReachedIF;
    bool hasReachedID;
    bool useAnotherID;
    bool checkedForNop;
    bool skip;
    bool valueReady;
    char operatorValue[5];
};

struct op
{
	char tag[4];
	char variable;
	char value[5];
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
	//cout << "There are " << rowCounter << " instructions in the file" << endl;
	/*for(int i = 0; i < rowCounter; i++)
	{
		cout << "the size of " << records[i].tag << " is " << records[i].sizeOf << endl;
	}*/
	

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
    						//cout << records[i].tag[j+1];
    						records[i].operatorValue[valueCounter] = records[i].tag[j+1];
    						valueCounter++;
    						j++;
    					}
    					//cout << records[i].operatorValue << endl;
    					valueCounter= 0;

    					break;
    				}else{
    					records[i].operatorValue[0] = '0';
    				}
    			}
    		}
    	}
    }
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
    int WB_Counter = 0;
    records[0].ready = true;

    while(WB_Counter != rowCounter)
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
						WB_Counter += 1;
						useWB = true;
					}
					cout << records[i].pipeline[j] << "\t";
				}
				cout << endl;
    		}
    	}
    	cout << endl;
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