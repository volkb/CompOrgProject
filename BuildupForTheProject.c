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
	char tag[25];
	char operation[25];
	char pipeline[9][4];
	int t_value;
	int s_value;
    int mainValue;
	bool nop;
	bool WBready;
	char firstVariable;
	int order;
	bool ready;
	bool hasBeenChecked;
    bool hasReachedMEM;
    bool hasReachedIF;
    bool hasReachedID;
    bool useAnotherID;
    bool checkedForNop;
    bool skip;
    int nopValue;
    bool isNotNull;
    bool okayToSkip;
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
	//char NOP = '*';

	char word[25];
	int wordCounter = 0;
	int WB_counter = 0;
    int max = 0;

    /*bool needsSpace = false;
    if(strcmp(argv[1],"ex07.s")==0 || strcmp(argv[1],"ex09.s")==0)
    {
        needsSpace = true;
    }*/
	if(fptr == NULL)
   	{
      printf("Error!\n");   
      exit(1);             
   	}
   	int rowCounter = 0;
   	ch = fgetc(fptr); 

   	struct pipe records[128];
    while (ch != EOF) 
    { 
    	if(ch != '\n'){
    	   word[wordCounter] = ch;
        }
    	if(ch == '\n')
    	{
            //printf("Word counter is: %d\n",wordCounter);
            max++;
    		rowCounter ++;
    		wordCounter = 0;
            //printf("%s\n",word);
    		memset(word, 0, sizeof word);
    	} else {
            if(ch != '\n'){
    		  records[rowCounter].tag[wordCounter] = ch;
    		  wordCounter ++;
            }
    	}
        //printf ("%c", ch); 
        ch = fgetc(fptr); 
        
    } 

    
    //char pipeArray[rowCounter*2][CPUcycles];
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


    //int row = 0;

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
    //int hasUsedWB = 0;

    int nopCounter = 0;
    //bool useNop = false;

    //int tmp = rowCounter;
    //int oldValue = 0;

    int nopStopper = 7;
    //bool anotherNOP = false;
    int nopHasIncreased = 0;
    //bool skipIncrement = false;
    //int adder = 0;
    bool thereIsAnotherNop = false;
    printf("START OF SIMULATION\n\n");
    while(WB_counter != max)
    {
    	printf("CPU Cycles ===>\t1\t2\t3\t4\t5\t6\t7\t8\t9\n");
    	for(int i = 0; i < rowCounter; i++)
    	{
            /*if(needsSpace)
            {
                printf("%s\t",records[i].tag);
            }else{
    		  printf("%s",records[i].tag);
            }*/
            printf("%s",records[i].tag);
    		if(!records[i].hasBeenChecked)
    		{
    			for(int y = 0; y < sizeof(records[i].tag); y++)
    			{
    				if(records[i].tag[y] == 't')
    				{
    					records[i].firstVariable = 't';
    					records[i].mainValue = records[i].tag[y+1] - '0';
    					//printf("THE T_VALUE IS %d\n",records[i].mainValue);
    					break;
    				}else if(records[i].tag[y] == 's')
    				{
    					records[i].firstVariable = 's';
    					records[i].mainValue = records[i].tag[y+1] - '0';
    					//printf("THE S_VALUE IS %d\n",records[i].mainValue);
    					break;
    				}
    			}
    			//printf("\n%ld\n",sizeof(records[i].tag));
    			records[i].hasBeenChecked = true;
    		}
            for(int z = i; z < rowCounter; z++)
            {
                //printf("\nThe record we are examining is %s\n",records[z].tag);
                if(records[z].order != records[i].order)
                {
                    //printf("\n-------------The record we are examining is %s -------------\n",records[z].tag);
                    //printf("\n%s\n",records[z].tag);
                    for(int l = 0; l < sizeof(records[z].tag); l++)
                    {
                        //CHECK SIZEOF VS STRINGLENGTH
                        if(isdigit(records[z].tag[l]))
                        {
                            if(!isdigit(records[z].tag[l+1]))
                            {
                                if(records[i].firstVariable == records[z].tag[l-1] && records[i].mainValue == (records[z].tag[l] - '0') && records[i].hasReachedMEM && records[z].hasReachedID && records[z].hasReachedIF && !records[i].WBready)
                                {
                                    //printf("\nThe record we are examining is %s\n",records[z].tag);
                                    records[z].useAnotherID = true;
                                    records[z].checkedForNop = true;
                                    for(int a = rowCounter - 1; a >= i+1; a--)
                                    {
                                        memcpy(&records[a+1],&records[a],128);
                                    }
                                    memset(records[z].tag,0,25);
                                    /*if(needsSpace)
                                    {
                                        strcat(records[z].tag,"nop\t");
                                    }else{
                                        strcat(records[z].tag,"nop\t\t");
                                    }*/
                                    strcat(records[z].tag,"nop\t\t");
                                    records[z].nop = true;
                                    for(int a = 0; a < 9; a++)
                                    {
                                        memset(records[z].pipeline[a],0,3);
                                        if(records[z-1].checkedForNop)
                                        {
                                            strcat(records[z].pipeline[a],records[z-1].pipeline[a]);
                                            //anotherNOP = true;
                                        }else{
                                            strcat(records[z].pipeline[a],records[z+1].pipeline[a]);
                                        }
                                    }
                                    records[z].checkedForNop = true;
                                    nopStopper--;
                                    rowCounter++;
                                    nopCounter = idCounter+1;

                                    z++;
                                    //printf("The value of z after ++ is %d\n",z);
                                    //printf("The idCounter value is %d\n",idCounter);
                                    /*if(!anotherNOP){
                                        nopCounter = idCounter+1;
                                    }*/
                                    /*if(records[z+1].isNotNull == false)
                                    {
                                        z--;
                                    }*/
                                    //printf("\n-------------The record we are examining is %s -------------\n",records[z+1].tag);
                                    
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
                                        //printf("\n-----------THE TAG WE ARE EXAMINING NOW IS %s---------\n",records[z+1].tag);
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
            }/////////////*/
            
            if(records[i].nop)
            {
                //printf("BEGIN nop here\n");
                for(int a = 0; a < 9; a++)
                {
                    if(a == nopCounter)
                    {
                        memset(records[i].pipeline[a],0,3);
                        records[i].pipeline[a][0] = '*';
                        //useNop = true;
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
                    /*if(needsSpace)
                    {
                        printf("%s\t",records[i].tag);
                    }else{
                        printf("%s",records[i].tag);
                    }*/
                    printf("%s",records[i].tag);
                    thereIsAnotherNop = false;
                    for(int a = 0; a < 9; a++)
                    {
                        if(a == nopCounter)
                        {
                            memset(records[i].pipeline[a],0,3);
                            records[i].pipeline[a][0] = '*';
                            //useNop = true;
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
                //return(0);

            }else{
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
    				    //printf("idCounter is %d\n",idCounter);
    				    memset(records[i].pipeline[j],0,3);
    				    strcat(records[i].pipeline[j], ID);
    				    useID = true;
                        records[i].hasReachedID = true;
    			    /*}else if(records[i].useAnotherID){
                        memset(records[i].pipeline[j],0,3);
                        strcat(records[i].pipeline[j], ID);
                        useID = true;
                        records[i].hasReachedID = true;
                        skipIncrement = true;*/
                        //printf("THINK OF A WAY TO CONTINUE\n");
                        //return(0);
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
    				    records[i].ready = true;
    				    //WB_counter += 1;
    			    }
    		  	    printf("\t%s",records[i].pipeline[j]);
    		    }
            }
    		if(useIF && whileCounter > ifCounter)
    		{
    			ifColumn += 1;
   				ifCounter += 1;
   			}
    		if(useID && whileCounter > idCounter)
    		{
   				idColumn += 1;
   				idCounter += 1;

                //printf("The idCounter value is %d\n",idCounter);
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
            
            /*if(useNop)
            {
                nopCounter += 1;
            }*/
    		printf("\n");
    	}
    	printf("\n");
    	progressCounter += 1;

    	whileCounter += 1;
    	WB_counter = 0;
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