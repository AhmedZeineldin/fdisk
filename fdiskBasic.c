#include <stdio.h>
#include <sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <unistd.h>
#include <inttypes.h>

typedef struct {
    uint8_t status;
    uint8_t first_chs[3];
    uint8_t partition_type;
    uint8_t last_chs[3];
    uint32_t lba;
    uint32_t sector_count;
} PartitionEntry;

int main(int arg, char **argv)
{
    int currentSectorNumber = 0;
    char extendedFlag = 0;
    int extendedSectorStartNum = 0;
    char buf[512];
    int fd = open(argv[1], O_RDONLY);
    read(fd, buf, 512);
    PartitionEntry * extended_table_entry_ptr = NULL;
    PartitionEntry *table_entry_ptr = (PartitionEntry *) & buf[446];
    printf("%-5s %-17s %-10s %-10s %-10s %-10s %-10s %-10s\n", "Device","Boot", "Start","End","Sectors","Size","Id","Туре");

    for (int i = 0; i < 4; i++) 
    {
    	 printf("%s%-5d %-10c %-10u %-10u %-10u %.1fG      %-10X\n",
	 argv[1], 
	 i + 1,
	 table_entry_ptr[i].status == 0x80 ? '*' : ' ', 
	 table_entry_ptr[i].lba,
	 table_entry_ptr[i].lba + table_entry_ptr[i].sector_count - 1,
	 table_entry_ptr[i].sector_count,
	 (((uint64_t) table_entry_ptr[i].sector_count * 512.0) / (1024 * 1024 * 1024)),
	 table_entry_ptr[i].partition_type);
	 
	if(table_entry_ptr[i].partition_type == 5)
    	{
    		extendedFlag = 1;
    	}
	if(extendedFlag != 0)
	{
	    /**
	      * save the start of the extended section as the offsets of the logical will be calculated as offsets from it.
	      */
	    extendedSectorStartNum = table_entry_ptr[i].lba;
	    currentSectorNumber    = extendedSectorStartNum;
	    
    	    lseek(fd, (table_entry_ptr[i].lba) * 512, SEEK_SET);
    	    read(fd, buf, 512);
            extended_table_entry_ptr = (PartitionEntry *) & buf[446]; // read the first EBR as the last 16 bytes in the first section of the extended partition
            
            i++;                                                      // increment the partition number.
            
            //check that the contents of the second section is not zeroes with dereferencing the pointer after being casted to (16*4 = 64) bit pointer
            while(*((uint64_t*)extended_table_entry_ptr) != 0)
            {
            
            	//print the contents of the logical partition
                printf("%s%-5d %-10c %-10u %-10u %-10u %.1fG      %-10X\n", 
                argv[1], 
                ++i, 
                extended_table_entry_ptr->status == 0x80 ? '*' : ' ', 
                extended_table_entry_ptr->lba + currentSectorNumber,
	 	 extended_table_entry_ptr->lba + extended_table_entry_ptr->sector_count - 1 + currentSectorNumber, 
	 	 extended_table_entry_ptr->sector_count,
	 	(((uint64_t) extended_table_entry_ptr->sector_count * 512.0) / (1024 * 1024 * 1024)), extended_table_entry_ptr->partition_type);
	 	
            	//increment current sector number to include the next EBR by adding to it 1 sector of the EBR and the LP's sector number
            	currentSectorNumber += (extended_table_entry_ptr->sector_count + 1 + 1);
            	
            	//move to the second entry
		extended_table_entry_ptr++;                                                
		
		
		//check if the second entry is not zeroes then move your pointers
		if(*((uint64_t*)extended_table_entry_ptr) != 0)
		{
			lseek(fd, (extended_table_entry_ptr->lba + extendedSectorStartNum) * 512, SEEK_SET); //move the fd to the beginning of the next EBR with the lba as offset directly
			read(fd, buf, 512);                                                                  //read the whole section
            		extended_table_entry_ptr = (PartitionEntry *) & buf[446];                            //read the EBR's first section
		}
	    }
	    break;
    	}
     }
     return 0;
}
