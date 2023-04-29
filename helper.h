#pragma pack(push, 1)
struct PartitionTableEntry //16 bytes
{  
    unsigned char bootFlag;  
    unsigned char CHSBegin[3];  
    unsigned char typeCode;  
    unsigned char CHSEnd[3];  
    unsigned int  LBABegin;  
    unsigned int  LBALength; 
};  
typedef struct PartitionTableEntry PartitionTableEntry;

struct MBRStruct 
{  
    unsigned char       bootCode[446];  //64+446+2 = 512
    PartitionTableEntry part1;  
    PartitionTableEntry part2;  
    PartitionTableEntry part3;  
    PartitionTableEntry part4;  
    unsigned short      flag; 
} MBR; 
typedef struct MBRStruct MBRStruct;

struct BPBStruct {     
    unsigned char  BS_jmpBoot[3];    //  Jump instruction to boot code     
    unsigned char  BS_OEMNane[8];    // 8-Character string (not null terminated)     11
    unsigned short BPB_BytsPerSec;   // Had BETTER be 512!     13
    unsigned char  BPB_SecPerClus;   // How many sectors make up a cluster?     14
    unsigned short BPB_RsvdSecCnt;   // # of reserved sectors at the beginning (including the BPB)?16     
    unsigned char  BPB_NumFATs;      // How many copies of the FAT are there?  (had better be 2)     17
    unsigned short BPB_RootEntCnt;   // ZERO for FAT32     19
    unsigned short BPB_TotSec16;     // ZERO for FAT32     21
    unsigned char  BPB_Media;        // SHOULD be 0xF8 for "fixed", but isn't critical for us     22
    unsigned short BPB_FATSz16;      // ZERO for FAT32     24
    unsigned short BPB_SecPerTrk;    // Don't care; we're using LBA; no CHS     26
    unsigned short BPB_NumHeads;     // Don't care; we're using LBA; no CHS     28
    unsigned int   BPB_HiddSec;      // Don't care ?     32
    unsigned int   BPB_TotSec32;     // Total Number of Sectors on the volume     36
    unsigned int   BPB_FATSz32;      // How many sectors long is ONE Copy of the FAT?     40
    unsigned short BPB_Flags;        // Flags (see document)     42
    unsigned short BPB_FSVer;        // Version of the File System     44
    unsigned int   BPB_RootClus;     // Cluster number where the root directory starts (should be 2)     46
    unsigned short BPB_FSInfo;       // What sector is the FSINFO struct located in?  Usually 1     48
    unsigned short BPB_BkBootSec;    // REALLY should be 6 – (sector # of the boot record backup)     50
    unsigned char  BPB_Reserved[12]; // Should be all zeroes -- reserved for future use     62
    unsigned char  BS_DrvNum;        // Drive number for int 13 access (ignore)     63
    unsigned char  BS_Reserved1;     // Reserved (should be 0)     64
    unsigned char  BS_BootSig;       // Boot Signature (must be 0x29)     65
    unsigned char  BS_VolID[4];      // Volume ID     69
    unsigned char  BS_VolLab[11];    // Volume Label     80
    unsigned char  BS_FilSysType[8]; // Must be "FAT32   "     88
    unsigned char  unused[420];      // Not used                508
    unsigned char  signature[2];     // MUST BE 0x55 AA         510
    } BPB; 
    typedef struct MBRStruct MBRStruct;

struct DirectoryFile{//A struct for putting the inputed 32 bytes from the fat32. Equates to 32 bytes
    unsigned char   DIR_Name[11];
    unsigned char   DIR_Attr[1];//attributes
    unsigned char   DIR_NTRes[1];//always 0
    unsigned char   DIR_CrtTimeTenth[1];//fractionalPortionOfFileCreation
    unsigned short  DIR_CrtTime;//fileCreationTime
    unsigned short  DIR_CrtDate;//fileCreationDate
    unsigned short  DIR_LstAccDate;//dateOfFileLastAccesed
    unsigned char   DIR_FstClusHI[2];//highOrderClusterNumberWhereTheFileStarts
    unsigned short  DIR_WrtTime;//lastWrittenToTime
    unsigned short  DIR_WrtDate;//lastWrittenToDate
    unsigned char   DIR_FstClusLO[2];//lowOrderClusterNumberWhereTheFileStarts
    unsigned int    DIR_FileSize;//fileSize
};

struct LDIR{//A struct for putting long directory entries into
    unsigned char   LDIR_Ord;
    unsigned char   LDIR_Name[10];//name1
    unsigned char   LDIR_Attr;//attributes
    unsigned char   LDIR_Type;
    unsigned char   LDIR_Chksum;
    unsigned char   LDIR_Name2[12];//name2
    unsigned short  LDIR_FstClusLO;
    unsigned char   LDIR_Name3[4];//name3
};


    #pragma pack(pop)


    void displaySector(unsigned char* sector) {     // Display the contents of sector[] as 16 rows of 32 bytes each.  Each row is shown as 16 bytes,     
    // a "-", and then 16 more bytes.  The left part of the display is in hex; the right part is in     
    // ASCII (if the character is printable; otherwise we display ".".     
       for (int i = 0; i < 16; i++)                      
    // for each of 16 rows     
    {                                                 
        //          
        for (int j = 0; j < 32; j++)                 
        // for each of 32 values per row          
        {                                            
            //               
            printf("%02X ", sector[i * 32 + j]);    // Display the byte in hex               
            if (j % 32 == 15) printf("- ");         // At the half-way point?  Show divider          
            }          printf(" ");                                       // Separator space between hex & ASCII          
            for (int j = 0; j < 32; j++)                       // For each of those same 32 bytes           
            {                                                  //               
            if (j == 16) printf(" ");                     // Halfway? Add a space for clarity               
            int c = (unsigned int)sector[i * 32 + j];     // What IS this char's ASCII value?               
            if   (c >= 32 && c <= 127) printf("%1c", c);  // IF printable, display it               
            else                       printf(".");       // Otherwise, display a "."          
            }                                                  //          
            printf("\n");                                // That’s the end of this row     
            } 
            }