#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include "helper.h"
//Name: Drew Diettrich
//Operating Systems Lab 3B
//Description: This program takes in a image file as a command line input. After the program recognizes the image file and it prompts the user for input you
//are able to use DIR, EXTRACT AND QUIT for what you want to do. DIR displays all of the content/entries inside of the root directory with
//all of its relevant data. With EXTRACT you are able to do input EXTRACT <filename> after it prompts you for input and then the 
//program will find the file in the root directory and create it in the projects directory. If you type in QUIT you are able to quit out 
//of the application.



int main(int argc, char* argv[], char* env[]){

    while(1){//while loop continues unless the user enters QUIT or there is an invalid input

        char* current = argv[1];//user input which is a file

        //printf("requested file: %s\n", current);//prints the file that the user has requested
        FILE *file;     
        file = fopen(current, "rb");//opens the image Images: FlashImage.img, CFImage.img
        //file = fopen("FlashImage.img", "rb");

        if(file == NULL){
            printf("File not found\n");
            return 0;
        }
        char inputString[150];//char array for the extract thing

        printf("\n\\>");//this line prompts the user for input
        fgets(inputString, sizeof(inputString), stdin);//gets the input string

        int extractSpaceIndex = 0;
        //finds the index of the space
        for(int x = 0; x < 150; x++){
            if (inputString[x] == '\n') {//At the end of the input it turns the \n into \0 which makes it a null terminator then breaks
                inputString[x] = '\0';
                break;
            } 
            if (inputString[x] == ' ') { //inserts the Null termination at the first space
                inputString[x] = '\0';
                extractSpaceIndex = x;//index of the space
                break;
            } 
        }


        //booleans for deciding whether the user entered DIR or EXTRACT
        bool DIR = false;
        bool EXTRACT = false;

        char extractFileName[150];




        if(strcmp(inputString, "QUIT") == 0){//if QUIT end the execution
            return 0;
        }
        else if(strcmp(inputString, "DIR") == 0){//if DIR make DIR true so that it will print out all of the values
            DIR = true;
        }
        else if(strcmp(inputString, "EXTRACT") == 0){//if EXTRACT seperate the filename from the EXTRACT
            EXTRACT = true;

            //Get the file that the user wants
            extractSpaceIndex += 1;//increases the number past the space
            for(int x= 0; x<150; x++){
                extractFileName[x] = inputString[extractSpaceIndex];//starts reading input after the space into extractFileName
                extractSpaceIndex++;
                if(inputString[extractSpaceIndex] == '"'){//end of input
                    extractFileName[x+1] = '\n';
                    break;
                }
            }
        }
        else{//anything other than the 3 expected inputs closes the program
            printf("Input not valid\nquitting\n");
            return 0;
        }
        
        
        //Information that will be useful if the filename is equal to a filename in the dir
        bool extractIsFile = false;
        int extractFileStartingClusterNumber = 0;
        int extractFileSize = 0;
        

        fseek(file, 0, SEEK_SET);
        fread(&MBR, sizeof(struct MBRStruct), 1, file);//reads in the mbr into the MBR struct


        //For TESTING purposes
        // printf("MBR Flag: %u\n",MBR.flag);//should be 0x55AA
        // printf("Size of MBR: %li\n", sizeof(MBR));
        // printf("LBA begin: %u    \n", MBR.part1.LBABegin);//prints LBA begin and LBA length for each partition entry
        // printf("LBA length: %d    \n", MBR.part1.LBALength);


        unsigned int BPBOffset = MBR.part1.LBABegin * 512;
        unsigned int BPBOffsetNotTimes512 = MBR.part1.LBABegin;

        fseek(file, BPBOffset, SEEK_SET);//Offsets to the location of the BPB block in the first partition
        fread(&BPB, sizeof(BPB), 1, file);//reads 512 bytes into the BPB
        
        // More print statements for TESTING purposes
        // printf("size of BPB: %li\n", sizeof(BPB));
        // printf("signature: %x%x \n", BPB.signature[0] & 0xff, BPB.signature[1] &0xff);
        // printf("bytes per sec: %u\n", BPB.BPB_BytsPerSec);
        // printf("sectors per cluster: %u\n", BPB.BPB_SecPerClus);
        // printf("reserved sector count: %u\n", BPB.BPB_RsvdSecCnt);
        // printf("number of copies of the fat: %u\n", BPB.BPB_NumFATs);
        // printf("Root cluster: %u\n", BPB.BPB_RootClus);

        
        //root Directory stuff
        unsigned int firstClusterOfRootDirectory =  MBR.part1.LBABegin + BPB.BPB_RsvdSecCnt+(BPB.BPB_NumFATs*BPB.BPB_FATSz32);
        unsigned int secondClusterOfRootDirectory = MBR.part1.LBABegin + BPB.BPB_RsvdSecCnt+(BPB.BPB_NumFATs*BPB.BPB_FATSz32) + (0x1623 * BPB.BPB_SecPerClus); 

        unsigned int clustersLBAZero =  MBR.part1.LBABegin + BPB.BPB_RsvdSecCnt+(BPB.BPB_NumFATs*BPB.BPB_FATSz32) - (2*BPB.BPB_SecPerClus);
        


        //fat stuff
        unsigned int FirstFatLBAOffset = MBR.part1.LBABegin + BPB.BPB_RsvdSecCnt;//holds the location of the fat

        unsigned int *fatCluster = malloc(BPB.BPB_FATSz32*512);//for reading in the fat32
        fseek(file, FirstFatLBAOffset *512, SEEK_SET);
        fread(fatCluster, 512*BPB.BPB_FATSz32, 1, file);

        int rootDirectoryClusters[10];//there shouldn't be more than 10 clusters in the root directory
        int countOfRootDirectoryClusters = 1;
        int nextNumber = fatCluster[BPB.BPB_RootClus];//first should be 0x1625
        int x = 0;

        rootDirectoryClusters[x] = BPB.BPB_RootClus;//put the number in from the 2 index of the fatCluster because that is the initial cluster value
        x = 1;

        while(nextNumber <= 0x0ffffff8){//checks to see if the cluster is the end of the linked list type thing
            rootDirectoryClusters[x] = nextNumber;//puts each cluster into the rootDirectoryClusters 
            nextNumber = fatCluster[nextNumber];//sets the next number for the next loop

            x++;//increments x in order to put the right number into the array
        }


        struct DirectoryFile idk[512*16];

        int fileCount = 0;//for total statistics
        int byteCount = 0;//for total statistics

    //READDIR SECTION
    //This section of the code is slightly modified code from part A. It goes through each cluster in the root directory and
    //prints out each entry in the directory
        for(int index = 0; index<x; index++){

            fseek(file, (clustersLBAZero + (rootDirectoryClusters[index]*BPB.BPB_SecPerClus))*512, SEEK_SET);
            fread(&idk, 512*16, 1, file);
            int x = 3;

            //---------------------------------------------------------------------------------------
            struct DirectoryFile userFile;//directory file for reading in 32 byte lines of data from the directory
            struct LDIR longDirectoryFile;
            struct LDIR previousLNF[10];//holds the previous lnf entries for printing the long name
            char dateString[11];//string to hold the date
            char lnfFormString[150];//holds the final string for the lnf name entries
            
            
            int lnfCount = 0;

            bool isVolumeLine = false;

            unsigned char nameOfVolume[12];//name of the volume

            bool isSubDirectory = false;//bool that holds whether the while loop is in a loop with a subdirectory

            //fseek(file, 0, SEEK_SET);//skips to 128 bytes into the file
            int currentLineOfDirectory = 0;

            while(1){
                memcpy(&longDirectoryFile, &idk[currentLineOfDirectory], sizeof(longDirectoryFile));
                currentLineOfDirectory++;
                if((longDirectoryFile.LDIR_Attr & 0x10) == 0x10){
                    int x = 4;
                    isSubDirectory = true;
                    //how do I access the system lines???
                }

                if((longDirectoryFile.LDIR_Attr & 0x01) == 0x01){//test code...Delete this
                    int x = 3;//readOnly
                }

                if((longDirectoryFile.LDIR_Attr & 0x02) == 0x02){//test code...Delete this
                    int x = 3;//hidden
                    //If I add continue here then it will mess up my lfn entries for some reason :(      I'll take the L
                }

                if((longDirectoryFile.LDIR_Attr & 0x04) == 0x04){//test code...Delete this
                    int x = 3;//system/file is protected
                }

                if(longDirectoryFile.LDIR_Attr == 0x8){//test code...Delete this
                    int x = 3;//volumeID attribute/name is not the name of the file, its the name of the volume
                    //hits the breakpoint when the EECS line is being read in. I can store the name by using the first 11 bytes

                    memcpy(&userFile, &longDirectoryFile, sizeof(userFile));
                    for(int x = 0; x<11; x++){
                        nameOfVolume[x] = userFile.DIR_Name[x];//puts the DIR_Name into a bigger char that has space for a null terminator
                    }
                    nameOfVolume[11] = '\0';

                    if(DIR)
                        printf("Volume in drive X is %s\n\n", nameOfVolume);//prints the volume

                    continue;
                }

                if((longDirectoryFile.LDIR_Attr & 0x20) == 0x20){//test code...Delete this
                    int x = 3;
                }

                if((longDirectoryFile.LDIR_Attr & 0x0F) == 0x0F){//checks the 32 bytes to see if they're in lnf form

                    memcpy(&previousLNF[lnfCount], &longDirectoryFile, sizeof(userFile));//puts the lnf entry into the array so that they can be printed later
                    lnfCount++;//counts the number of lnf entries in the array

                    continue;//goes to read the next 32 bytes
                }
                else if((longDirectoryFile.LDIR_Ord & 0xFF) == 0x00){//end of file check
                    break;
                }

                
                char bigContatenation[100];
                for(int x = lnfCount-1; x>= 0; x--){//loops through the indexes of the previousLNF array which goes through
                    int j = 0;

                    char name1[6];//these char names hold the characters that are used to extract the names from the lnf
                    char name2[7];
                    char name3[3];

                    for (int i = 0; i<10; i++) {//this for loop puts the even values into the name1 array
                        if(i%2 == 0){
                            name1[j] = previousLNF[x].LDIR_Name[i];
                            j++;
                        }
                    }

                    j=0;
                    for (int i = 0; i<12; i++) {//this for loop puts the even values into the name2 array
                        if(i%2 == 0){
                            name2[j] = previousLNF[x].LDIR_Name2[i];
                            j++;
                        }
                    }
                    
                    j=0;
                    for (int i = 0; i<4; i++) {//this for loop puts the even values into the name3 array
                        if(i%2 == 0){
                            name3[j] = previousLNF[x].LDIR_Name3[i];
                            j++;
                        }
                    }

                    char contatenated[26];
                    contatenated[0] = '\0';

                    strcat(contatenated, name1);//these strcats combine the 3 names in a single 32 lnf struct
                    strcat(contatenated, name2);
                    strcat(contatenated, name3);

                    strcat(bigContatenation, contatenated);//this strcat combines the recently created string onto the big combined string
                }

                for(int x = 0; x<100; x++){//I was getting random '-1' values and this helps to get rid of them
                    if(bigContatenation[x] == -1){//I should've used unsigned char arrays but its working right now so I don't want to change it
                        bigContatenation[x] = '\0';
                    }
                }

                sprintf(lnfFormString, "%s\n", bigContatenation);//put the lfn into another char array so that it can be printed later

                for(int x = 0; x<100; x++){
                    bigContatenation[x] = '\0';//clear out the big concatentation string for cleanliness purposes
                }

                lnfCount = 0;//reset the lnf count after

                memcpy(&userFile, &longDirectoryFile, sizeof(userFile));//move 32 bytes into the userFile struct. An alternative to using the union

                //creationDate
                unsigned int day = userFile.DIR_WrtDate & 0x1F;//bits 0-4
                unsigned int month = (userFile.DIR_WrtDate >> 5) & 0xF;//bits 5-8
                unsigned int year = ((userFile.DIR_WrtDate >> 9) & 0x7f) + 1980;//bits 11-15
                sprintf(dateString, "%02d/%02d/%04d", month, day, year); // Format the date string
                if(DIR)
                    printf( "%s    ", dateString);

                //creationTime
                unsigned int seconds = (userFile.DIR_WrtTime >> 0x1F);//bits 0-4
                unsigned int minutes = (userFile.DIR_WrtTime >> 5) & 0x3F;//bits 5-10
                unsigned int hours = (userFile.DIR_WrtTime >> 11) & 0x1F;//bits 11-15
                
                char ampm[3] = "AM\0";
                if(hours>12){           //this if statement decides if the time is in the morning or afternoon
                    hours = hours-12;
                    strcpy(ampm, "PM\0");
                }
                if(DIR)
                    printf("%.2d:%.2d %s   ", hours, minutes, ampm);//prints the hours, minutes and either AM or PM

                //DIR
                if(isSubDirectory){//prints <DIR> if the entry is a subdirectory
                    if(DIR)
                        printf("<DIR>");
                }
                else{
                    if(DIR)
                        printf("%5s", " ");//else it just prints 5 spaces to keep the lines organized
                }
                
                //ClusterNumber

                unsigned short hi = *(unsigned short *)userFile.DIR_FstClusHI;//figured out that I was creating the cluster wrong ever since Lab A. This is the fixed version
                unsigned short lo = *(unsigned short *)userFile.DIR_FstClusLO;
                unsigned int cluster = ((unsigned int)hi << 16) | (unsigned int)lo;

                if(DIR)
                    printf("%8u  ", cluster);//prints the cluster number

                //FileSize
                setlocale(LC_ALL, "");//allows me to use the commas for the big numbers

                if(DIR)
                    printf("%'14d     ", userFile.DIR_FileSize);//prints the file's size with commas
                
                unsigned char standardString[13];
                bool isSpace = false;
                for(int x = 0; x<11; x++){
                        standardString[x] = userFile.DIR_Name[x];//puts the DIR_Name into a bigger char that has space for a null terminator
                        if(standardString[x] == ' '){
                            isSpace = true;
                        }
                    }
                
                
                if(isSubDirectory){//subdirectory doesn't have a file type after the dot so just keep it the same
                    standardString[10] = '\0';
                    isSubDirectory = false;
                }
                else{
                    standardString[11] = '\0';//adds a null terminator for printing reasons

                    if(isSpace == false){//if there are no spaces then shift chars 1 and add a period
                        char temp = standardString[7];
                        char temp1 = standardString[8];
                        char temp2 = standardString[9];
                        char temp3 = standardString[10];
                        standardString[8] = '.';
                        standardString[9] = temp1;
                        standardString[10] = temp2;
                        standardString[11] = temp3;
                        standardString[12] = '\0';
                    }
                    else if(isSpace == true){//if there are spaces in the string make sure to trim down the spaces and include a period

                        int spaceCount = 0;
                        int i, j;
                        for (i = 0; i<11; i++) {
                            if (standardString[i] == ' ') {
                                spaceCount++;
                            }
                        }

                        char temp1 = standardString[8];//shifts the chars based on the space count
                        char temp2 = standardString[9];
                        char temp3 = standardString[10];
                        standardString[8 - spaceCount] = '.';
                        standardString[9 - spaceCount] = temp1;
                        standardString[10 - spaceCount] = temp2;
                        standardString[11 - spaceCount] = temp3;
                        standardString[12 - spaceCount] = '\0';//puts in a null terminator in the last spot so that it prints correctly
                    }
                }
                
                if(DIR)
                    printf("%-15s      ", standardString);//print standard name string

                if(DIR)
                    printf("%-s", lnfFormString);//print lnf name string

                if(EXTRACT && strcmp(lnfFormString, extractFileName)==0){
                    //set the valuable information for extracting the file
                    extractIsFile = true;
                    extractFileStartingClusterNumber = cluster;
                    extractFileSize = userFile.DIR_FileSize;
                }
                

                //these two lines are the lines that gather data for the summary statistics
                fileCount++;
                byteCount += userFile.DIR_FileSize;
            }
        }

        if(DIR)
            printf("            %d File(s)  %'15d bytes\n", fileCount, byteCount);//Summary statistics

        //---------------------------------------------------------------------------------------

        //EXTRACT SECTION
        //If the user entered file is found in the READDIR section then it will create a file and write its clusters of data into a file
        //else if it doesn't recognize the file then it prints out "File not found"
        if(extractIsFile){
            printf("File Found\n");
            int numberForIndex = 0;
            while(1){//removes the \n from the filename
                if(extractFileName[numberForIndex] == '\n'){
                    extractFileName[numberForIndex] = '\0';
                    break;
                }
                numberForIndex++;
            }
            
            //Displays the files data for testing purposes
            //printf("filename: %s    Starting cluster numebr: %d     file size: %d\n", extractFileName, extractFileStartingClusterNumber, extractFileSize);

            FILE *createdFile;
            createdFile = fopen(extractFileName, "w");//Input string of the file to extract

            if(createdFile == NULL) {
                printf("Error opening file.\n");
                return 1;
            }

            int clusterSize = BPB.BPB_SecPerClus*512;
            int totalClusters = extractFileSize/clusterSize;

            int fileClusters[50000];//large number to hold all the clusters
            int countOfFileClusters = 1;
            int nextNumber = fatCluster[extractFileStartingClusterNumber];//first should be 0x1625
            int x = 0;

            fileClusters[x] = extractFileStartingClusterNumber;//put the number in from the 2 index of the fatCluster because that is the initial cluster value
            x = 1;



            while(nextNumber <= 0x0ffffff8){//checks to see if the cluster is the end of the linked list type thing
                fileClusters[x] = nextNumber;//puts each cluster into the fileClusters
                nextNumber = fatCluster[nextNumber];//sets the next number for the next loop
                x++;//increments x in order to put the right number into the array
            }


            char clusterToPutIntoFile[512*BPB.BPB_SecPerClus];//for every loop this chunk of data gets put into the file

            for(int index = 0; index<x; index++){//loops for each cluster

                fseek(file, (clustersLBAZero + (fileClusters[index]*BPB.BPB_SecPerClus))*512, SEEK_SET);//navigates to the cluster
                fread(&clusterToPutIntoFile, 512*BPB.BPB_SecPerClus, 1, file);//reads a cluster from the cluster number

                if(index == x-1){//if last cluster in the file, make sure to not over read the size of the file
                    int remainingSize = extractFileSize%(512*BPB.BPB_SecPerClus);
                    fseek(createdFile, BPB.BPB_SecPerClus*512 *index, SEEK_SET);//seeks to the index of the next open cluster in the file
                    fwrite(clusterToPutIntoFile, sizeof(unsigned char), remainingSize, createdFile);
                }
                else{//writes a cluster to the file
                    fseek(createdFile, BPB.BPB_SecPerClus*512 *index, SEEK_SET);//seeks to the index of the next open cluster in the file
                    fwrite(clusterToPutIntoFile, sizeof(unsigned char), sizeof(clusterToPutIntoFile), createdFile);//puts the next cluster inside of the 
                }
            }

            fclose(createdFile);//closes the file
        }
        else if(EXTRACT){//file not found
            printf("File not found\n");
        }
    }
}
