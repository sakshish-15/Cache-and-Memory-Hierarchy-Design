#include <stdio.h>  
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stack>
#include <fstream>
#include <iostream>
#include <iomanip>
using namespace std;

class cache 
{
	public:
		unsigned int addr;
		int long L1_SIZE;
		int  BLOCKSIZE, L1_ASSOC, L2_ASSOC, L2_SIZE, L2_DATA_BLOCKS, L2_ADDRESS_TAGS, No_of_sets ;
		int index_size, Block_offset_size, No_of_blocks, LRU_Counter_bit_size;
		int block_to_be_evicted;
		int evicted_address,initialization_flag;
		int offset, tag_addr, index;

		int rd_counter;
		int rd_hit_counts, rd_miss_counts, wr_hit_counts, wr_miss_counts;
		int wr_counter;
		int write_back_counter;

		int **LRU_counter;
		int **MRU_matrix;
		int **tag_matrix;
		char **dirty_flag;
		bool **valid_flag;

/* Constructor */
cache()
{

	this->rd_hit_counts = 0;
	this->rd_miss_counts = 0;
	this->wr_hit_counts = 0;
	this->wr_miss_counts = 0;
	this->rd_counter = 0;
	this->wr_counter = 0;
	this->write_back_counter = 0;
	this->initialization_flag = 0;
}


	//public:

	int decode_tag_index_from_addr (int long L1_SIZE,  int BLOCKSIZE, int L1_ASSOC, unsigned int addr)
{
	int shift;
	No_of_sets = L1_SIZE/(L1_ASSOC*BLOCKSIZE);
	index_size = log2(No_of_sets);
	Block_offset_size = log2(BLOCKSIZE);
 	No_of_blocks = L1_SIZE/(BLOCKSIZE);
 	LRU_Counter_bit_size = log2(L1_ASSOC);

	index = (addr >> Block_offset_size) % (No_of_sets);
	shift = (index_size + Block_offset_size);
	tag_addr= addr >>shift;

	
	if(this->initialization_flag == 0)
	{
		tag_matrix = new int* [No_of_sets];
		dirty_flag = new char* [No_of_sets];
		valid_flag = new bool* [No_of_sets];
		LRU_counter= new int* [No_of_sets];
		MRU_matrix= new int* [No_of_sets];

	for(int i = 0 ; i < No_of_sets ; i++)
		{
			tag_matrix[i] = new int[L1_ASSOC];
			dirty_flag[i] = new char[L1_ASSOC];
			valid_flag[i] = new bool[L1_ASSOC];
			LRU_counter[i] = new int[L1_ASSOC];
			MRU_matrix[i] = new int[L1_ASSOC];
		}

		for (int i = 0 ; i < No_of_sets ; i ++)
		{
			for ( int j = 0 ; j < L1_ASSOC ; j++)
			{
				this->dirty_flag[i][j] = 'N';
				this->valid_flag[i][j] = false;
				this->tag_matrix[i][j] = 0;
				this->LRU_counter[i][j] = -1;
			}
		}
		
	this->initialization_flag = 1;
	}


return 0;

}


void LRU_Update ( int hit_index, int assoc)
{
	
	int temp;
		if (valid_flag[hit_index][assoc]== false)
	{
		LRU_counter[hit_index][assoc]=0;

		for (int i =0; i< L1_ASSOC ; i++)
		{
			//printf("Valid flag for i = %d is %d", i, valid_flag[hit_index][i]);
			if(valid_flag[hit_index][i] == true)
			{	
				LRU_counter[hit_index][i] = LRU_counter[hit_index][i] + 1;
				//printf("\nIncremented value for i = %d is %d",i,LRU_counter[hit_index][i]);
			//}
			}	
		
		} 
		LRU_counter[hit_index][assoc]=0;
	}
	else if (LRU_counter[hit_index][assoc] >= 0)
	{
		//printf("second hit");
		//printf("\n");
		//printf("%d",hit_index);
		//printf("\n");
		//printf("%d",assoc);
		//printf("\n");

		temp = LRU_counter[hit_index][assoc];
		LRU_counter[hit_index][assoc] = 0;

		for (int i =0; i< L1_ASSOC ; i++)
		{
			if(valid_flag[hit_index][i] == true)
			{	
				//printf("Hit index is %d", hit_index);
				if (i == assoc)
				{
					continue;
				}

				else if(LRU_counter[hit_index][i] <= temp)
					{  
						LRU_counter[hit_index][i] = LRU_counter[hit_index][i] + 1;
					}
			}	
			
		}
	}
}


int LRU_policy (int hit_index)
{	
	int evicted_block;
	int max = -9999999;
	int row = -1;
	for(int i=0;i<L1_ASSOC;i++)
	{
		if(valid_flag[hit_index][i] == true)
		{
			if(LRU_counter[hit_index][i]>max)
			{
				max = LRU_counter[hit_index][i];
				row = i;
			}
		}
	}

	evicted_block = row;
	//printf("LRU policy : Evicted Block is [%0d][%0d]",hit_index,row);
 return evicted_block;
}



bool readFromAddress( unsigned int addr)
{ 	
	//printf ("Address from read func is %x", addr);
	decode_tag_index_from_addr (L1_SIZE, BLOCKSIZE, L1_ASSOC,  addr);
	//printf("No of sets = %d\n",No_of_sets);
	//printf("block offset size = %d\n",Block_offset_size);
	//printf("No of Blocks= %d\n",No_of_blocks);
	//printf("Index_size is %d\n", index_size);
	//printf ("Index is %d", index);
	//printf("Tag is %d", tag_addr);
	//printInBase2(index);
	//printf ("\n");    
    //printInBase2(tag_addr);
    
	

	int hit_flag = 0;;
	int empty_tag_flag = 0;
	int choice;
	rd_counter ++;
	////printf ("Tag_matrix[6][1] = %d\n", tag_matrix[6][1]);
	////printf("first L_Assoc is %d",L1_ASSOC);

	for ( int i=0 ; i < L1_ASSOC; i++)
	{
	////printf("Tag_addr = %d and valid_flag[%0d][%0d] = %d, tag_matrix[%0d][%0d] = %d -> For Loop \n ", tag_addr, index,i,valid_flag[index][i],index,i,tag_matrix[index][i]);
	if (tag_matrix[index][i] == tag_addr) // Cache Hit
		{
			rd_hit_counts ++;
			//printf ("\nREAD HIT at tag_matrix[%0d][%0d], rd_hit_counts = %d\n\n\n", index,i,rd_hit_counts);
			LRU_Update(index,i);
			//for ( int i = 0 ; i < L1_ASSOC ; i++)
			//{
				//printf("\nLRU value for matrix[%0d][%0d] is %d\n", index,i,LRU_counter[index][i]);
			//}

			// //printf("LRU_counter[%0d][%0d] = %d",index,i,LRU_value);

			hit_flag = 1;
			break;
		}
	////printf("tag_matix[index][i] = %d", tag_addr);
}
	//printf("Hit Flag");
	if ( hit_flag == 0)

	{
	
		rd_miss_counts ++;
		//printf ("\nREAD MISS, rd_miss_counts =%d\n", rd_miss_counts);
		////printf("L_Assoc is %d, index is %d", this->L1_ASSOC, index);
		for (int i =0 ; i < L1_ASSOC ; i++)
		{
			////printf ("\nvalid_flag[%0d][%0d]= %d", index,i,valid_flag[index][i]);
			if (valid_flag[index][i] == false)
			{
				choice = i;
				tag_matrix[index][choice] = tag_addr;
				//printf("\nWrite the read Miss data to tag[%0d][%0d]\n and valid_flag[%0d][%0d] is %d\n", index,i,index,i,valid_flag[index][i]);
				LRU_Update(index, choice);
				valid_flag[index][choice]=true;
				//printf ("Insert After Miss\n");
				empty_tag_flag = 1;
			break;
			}
		}
			
		
				
			if (empty_tag_flag == 0)
			{
				//printf("Replacement needed\n");
			
				block_to_be_evicted = LRU_policy (index);
				if(dirty_flag[index][block_to_be_evicted] == 'D') // checking if this block to be 
				//updated back in the lower level or not
				{
					write_back_counter ++;
					dirty_flag[index][block_to_be_evicted] = 'N';
					//valid_flag[index][block_to_be_evicted] = true;

				}
			
				//printf ("Write back counter = %d\n" ,write_back_counter );
				tag_matrix[index][block_to_be_evicted] = tag_addr;
				//printf("Evicted Block is tag_matrix[%0d][%0d] and tag_addr is %d , tag_matrix new one = %d\n\n\n", index, 
				//block_to_be_evicted,tag_addr,tag_matrix[index][block_to_be_evicted]);
				LRU_Update(index, block_to_be_evicted);
				valid_flag[index][block_to_be_evicted]=true;
		
			}
	}
		
	
return false;
}
	////printf ("Write back counter = %d\n" ,write_back_counter );



bool writeToAddress(unsigned int addr)
{
	decode_tag_index_from_addr (L1_SIZE,  BLOCKSIZE, L1_ASSOC, addr);
	//printf("No of sets = %d\n",No_of_sets);
	//printf("block offset size = %d\n",Block_offset_size);
	//printf("No of blocks= %d\n",No_of_blocks);
	//printf("Index_size is %d\n", index_size);
	
	//printInBase2(index);
	//printf ("Index is %d", index);
	//printf("Tag is %d", tag_addr);


    

	int hit_flag = 0;
	int choice ;
	int empty_tag_flag = 0;


	
	wr_counter ++;

	for (int i =0; i < L1_ASSOC; i++)
	{
		if (tag_matrix[index][i] == tag_addr) // Hit
		{
			wr_hit_counts ++;
			//printf("L1 Write Hit");
			dirty_flag[index][i] = 'D'; 
			LRU_Update (index, i);
			hit_flag = 1;
			break;
		}

	}
		if ( hit_flag == 0)
		{
			wr_miss_counts ++;

			for (int i =0 ; i < L1_ASSOC ; i++)
			{
				//printf("\nDebug : valid_flag[%0d][%0d] = %d\n", index, i, valid_flag[index][i]);
				if (valid_flag[index][i] == false)
				{
					choice = i;
					//printf("\nDebug_IF : valid_flag[%0d][%0d] = %d\n", index, choice, valid_flag[index][choice]);

					tag_matrix[index][choice] = tag_addr;
					dirty_flag[index][choice] = 'D';
					
					LRU_Update ( index, choice);
					valid_flag[index][choice] = true;
					//printf("\nDebug_IF_latr : valid_flag[%0d][%0d] = %d\n", index, choice, valid_flag[index][choice]);
					//printf("\n\nData written to empty block\n");
					empty_tag_flag = 1;
					break;
				}
			}
			
				
					if( empty_tag_flag == 0)
						{
						////printf("\nENtered else part\n");
							block_to_be_evicted = LRU_policy (index);
							//printf ( "Evicted block = %d\n", block_to_be_evicted);

							if(dirty_flag[index][block_to_be_evicted] == 'D') // checking if this block to be 
							//updated back in the lower level or not
								{
								//evicted_address = {tag_matrix[index][block_to_be_evicted] & index & offset};
								// concat
								write_back_counter ++;
								}

							tag_matrix[index][block_to_be_evicted] = tag_addr;
							//valid_flag[index][block_to_be_evicted] = true;
							dirty_flag[index][block_to_be_evicted] = 'D';
							LRU_Update(index, block_to_be_evicted);



				
				}
			}
			

	return false;
	}	
	
	int compare (const void * a, const void * b) 
    {
    	double diff = LRU_counter[index][*(int*)a] - LRU_counter[index][*(int*)b];
    	return  (0 < diff) - (diff < 0);
	}

};

		
int main(int agrc , const char *argv[])
{
	char wr[2];
	char address[8];
	
	cache Cache_L1;
        
	std::string  trace_file;
	//printf ("RD miss is %d", Cache_L1.rd_miss_counts);
	

 	Cache_L1.BLOCKSIZE = atoi(argv[1]);
 	Cache_L1.L1_SIZE = atoi(argv[2]);
 	Cache_L1.L1_ASSOC = atoi(argv[3]);
	Cache_L1.L2_ASSOC = atoi(argv[4]);
	Cache_L1.L2_SIZE = atoi(argv[5]);
	Cache_L1.L2_DATA_BLOCKS = atoi(argv[6]);
	Cache_L1.L2_ADDRESS_TAGS = atoi(argv[7]);
	trace_file = atoi(argv[8]);
        Cache_L1.decode_tag_index_from_addr (Cache_L1.L1_SIZE, Cache_L1.BLOCKSIZE, Cache_L1.L1_ASSOC,  Cache_L1.addr);
	
	FILE *file;

	file = fopen(argv[8], "r");
    if (file == NULL) 
    {
        fprintf(stderr, "ERROR: cannot open input file. \n");
        return 1;
    }
    
    while (fscanf(file, "%s  %s", wr, address) != EOF) 
    {
        //printf("\n\n\n\n\nAddr is %s\n",address);
        std::string s = address;
		unsigned int  addr1 = std::stol(s, 0, 16);
		//printf ("\nString to hex is %x\n\n", addr1);
		////printf ("\nString to hex is %d\n\n", addr1);

		

        if(strcmp(wr, "#eof") == 0)
        {
            break;
        }
        if(strcmp(wr, "r") == 0)
        {
        	//printf("READ \n");
        	Cache_L1.readFromAddress(addr1);
        	
        }
        else if(strcmp(wr, "w") == 0)
        {
        	//printf("WRITE\n" );
        	Cache_L1.writeToAddress(addr1);
        }
    }
		
    fclose(file);
    

    
    	int a = Cache_L1.rd_miss_counts + Cache_L1.wr_miss_counts;
	int b = Cache_L1.wr_counter + Cache_L1.rd_counter;
	float Miss_rate = (float)a/b;


cout << "  ===== Simulator configuration ===== " << endl;
cout << "  BLOCKSIZE:                        " << argv[1] << endl;
cout << "  L1_SIZE:                          " << argv[2] << endl;
cout << "  L1_ASSOC:                         " << argv[3] << endl;
cout << "  L2_SIZE:                          " << argv[4] << endl;
cout << "  L2_ASSOC:                         " << argv[5] << endl;
cout << "  L2_DATA_BLOCKS:                   " << argv[6] << endl;
cout << "  L2_ADDRESS_TAGS:                  " << argv[7] << endl;
cout << "  trace_file:                       " << argv[8] << endl;

cout << "\n===== L1 contents =====			" << endl;

	///Sorting Logic////
	 
	int temp_value, temp_2;
	char temp_1;
	for (int k = 0; k < Cache_L1.No_of_sets ; k++)
	{
		for (int i =0; i < Cache_L1.L1_ASSOC; i++)
			{

				for ( int j= i; j < Cache_L1.L1_ASSOC ; j++)
				{	
					if(Cache_L1.valid_flag[k][i] == true && Cache_L1.valid_flag[k][j] == true)
					{
					if(Cache_L1.LRU_counter[k][i] > Cache_L1.LRU_counter[k][j])

					{
						temp_value = Cache_L1.tag_matrix[k][j];
						temp_2 = Cache_L1.LRU_counter[k][j];
						Cache_L1.tag_matrix[k][j]=Cache_L1.tag_matrix[k][i];
						Cache_L1.LRU_counter[k][j]=Cache_L1.LRU_counter[k][i];
						Cache_L1.tag_matrix[k][i]=temp_value;
						Cache_L1.LRU_counter[k][i]=temp_2;
						temp_1 = Cache_L1.dirty_flag[k][j];
						Cache_L1.dirty_flag[k][j]=Cache_L1.dirty_flag[k][i];
						Cache_L1.dirty_flag[k][i]=temp_1;
					}
				}	
				}

			}
	}


	  for (int i = 0; i < Cache_L1.No_of_sets ; ++i)
	  {
		if(i==0)
		{
		printf("set  %d:\t",i);
		}
	
		else
		{	
		printf ("\nset	%d:\t",i);
		}

	for ( int j =0 ; j < Cache_L1.L1_ASSOC ; ++j)
	{ 

		cout.setf(ios::hex, ios::basefield);
		//cout<< " " << Cache_L1.LRU_counter[i][j] << " " << "||\t";
		cout <<" " << Cache_L1.tag_matrix[i][j] << " " << Cache_L1.dirty_flag[i][j] << " ||\t"  ;
	}

} 


	cout.unsetf(ios::hex);
	cout <<  " \n                               " << endl;
	cout <<  "===== Simulation Results =====    " << endl;
	cout <<  "a. number of L1 reads:	    " << Cache_L1.rd_counter << endl;
	cout <<  "b. number of L1 read misses:	    " << Cache_L1.rd_miss_counts << endl;
	cout <<  "c. number of L1 writes:           " << Cache_L1.wr_counter << endl;
	cout <<  fixed<<std::setprecision(4) << "d. number of L1 write misses:		" << Cache_L1.wr_miss_counts << endl;
	//std::cout << std::fixed;
	cout <<  "e. L1 miss rate:                      	" << Miss_rate << endl;
	cout <<  "f. number of writebacks from L1 memory:	" << Cache_L1.write_back_counter << endl;
	cout <<  "g. total memory traffic:		" << Cache_L1.write_back_counter + Cache_L1.rd_miss_counts + Cache_L1.wr_miss_counts << endl;
return 0;

};
