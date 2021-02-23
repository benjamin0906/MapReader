/*
 * MapReader.c
 *
 *  Created on: 2021. febr. 23.
 *      Author: Benjamin
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct
{
    char *Name;
    int Addr;
    int Size;
} dtDataSeg;

typedef enum
{
    Read_TextData,
    Read_RoData,
    Read_BssData
} dtReadType;

int StrEq(char *str1, char *str2)
{
    int ret = 0;
    while(str1[ret] == str2[ret]) ret++;
    return ret;
}

int CountStr(char *Str, char *TestStr)
{
    int ret = 0;
    int TestLen = strlen(TestStr);
    while(*(Str+TestLen) != 0)
    {
        if(StrEq(Str, TestStr) == TestLen)
        {
            ret++;
        }
        Str++;
    }
    return ret;
}

int GetName(char *Str, char *Sect, char **Buff)
{
    int ret = 0;
    int SectLen = strlen(Sect);
    while((*(Str+SectLen) != 0) && (StrEq(Str, Sect) != SectLen))
    {
        ret++;
        Str++;
    }
    if(StrEq(Str, Sect) == SectLen)
    {
        Str+= SectLen;
        int Length = 0;

        /* Determine the length of the name of function */
        while((Str[Length] != 0) && ((Str[Length] != ' ') && (Str[Length] != '\n') && (Str[Length] != '\r'))) Length++;

        if(Buff != 0)
        {
            /* Allocate the space for the function name */
            *Buff = calloc(Length+1, sizeof(char));

            /* Copy the function name */
            memcpy(*Buff, Str, Length);

            /* Closing zero */
            (*Buff)[Length] = 0;
        }
        ret += Length;
    }
    return ret;
}

int HexStrToInt(char *Str)
{
    int ret = 0;
    while(*Str != 0)
    {
        ret <<= 4;
        if((*Str >= '0') && (*Str <= '9')) ret |= *Str-'0';
        else if((*Str >= 'A') && (*Str <= 'F')) ret |= *Str-55;
        else if((*Str >= 'a') && (*Str <= 'f')) ret |= *Str-87;
        Str++;
    }
    return ret;
}

int ReadMap(dtDataSeg **Array, char* Str, dtReadType DataSeg)
{
    int looper;
    char *Temp;
    int EntryLength = 0;
    char *DefSect = 0;
    char DefSectBss[] = ".bss.";
    char DefSectData[] = ".rodata.";
    char DefSectText[] = ".text.";
    char HexStr[] = "0x";

    switch(DataSeg)
    {
        case Read_TextData:
            DefSect = DefSectText;
            break;
        case Read_RoData:
            DefSect = DefSectData;
            break;
        case Read_BssData:
            DefSect = DefSectBss;
            break;
    }


    EntryLength = CountStr(Str, DefSect);
    printf("Text entry number: %d\n", EntryLength);

    *Array = calloc(EntryLength, sizeof(dtDataSeg));

    for(looper = 0; looper < EntryLength; looper++, Temp = 0)
    {
        Str += GetName(Str, DefSect, &(*Array)[looper].Name);

        /* Get the Address value into the Temp */
        Str += GetName(Str, HexStr, &Temp);

        /* Get the intiger value from the Temp */
        (*Array)[looper].Addr = HexStrToInt(Temp);

        /* deallocate the allocated array */
        free(Temp);

        /* Get the size value into the Temp */
        Str += GetName(Str, HexStr, &Temp);

        /* Get the intiger value from the Temp */
        (*Array)[looper].Size = HexStrToInt(Temp);

        /* deallocate the allocated array */
        free(Temp);
    }
    return EntryLength;
}

void PrintFixLenString(char *Str, int Length)
{
    int looper;
    for(looper = 0; looper < Length; looper++)
    {
        if(*Str != 0)
        {
            putchar(*Str);
            Str++;
        }
        else putchar(' ');
    }
}

void MaxSorting(dtDataSeg *DataSeg, int Length)
{
    int looper;
    for(looper = 0; looper < Length; looper++)
    {
        int i;
        int Max = looper;
        for(i = looper; i<Length; i++)
        {
            if(DataSeg[Max].Size < DataSeg[i].Size)
            {
                Max = i;
            }
        }
        dtDataSeg Temp = DataSeg[looper];
        DataSeg[looper] = DataSeg[Max];
        DataSeg[Max] = Temp;
    }
}

int main(int argv, char **argc)
{
    char FileName[] = "map.map";

    /* Open file */
    FILE *f = fopen(FileName, "r");

    if(f != 0)
    {
        dtDataSeg *TextArray;
        dtDataSeg *RodataArray;
        dtDataSeg *BssArray;
        int TextArrayLength;
        int RodataArrayLength;
        int BssArrayLength;
        char *File;
        int FileLength;

        /* Get file length */
        fseek(f, 0, SEEK_END);
        FileLength = ftell(f);
        fseek(f, 0, SEEK_SET);

        /* Allocate space for the file and read it in. */
        File = calloc(FileLength, sizeof(char));
        fread(File, sizeof(char), FileLength, f);

        /* Get the function informations from the read file */
        TextArrayLength = ReadMap(&TextArray, File, Read_TextData);
        RodataArrayLength = ReadMap(&RodataArray, File, Read_RoData);
        BssArrayLength = ReadMap(&BssArray, File, Read_BssData);

        /* Sort the function informations according to the size */
        MaxSorting(TextArray, TextArrayLength);
        MaxSorting(RodataArray, RodataArrayLength);
        MaxSorting(BssArray, BssArrayLength);

        printf("\nText Data\n");
        for(int looper = 0; looper < TextArrayLength; looper++)
        {
            if(TextArray[looper].Addr != 0)
            {
                PrintFixLenString(TextArray[looper].Name,30);
                printf(" 0x%08x, %d bytes\n", TextArray[looper].Addr, TextArray[looper].Size);
            }
        }

        printf("\nRodata Data\n");
        for(int looper = 0; looper < RodataArrayLength; looper++)
        {
            if(RodataArray[looper].Addr != 0)
            {
                PrintFixLenString(RodataArray[looper].Name,30);
                printf(" 0x%08x, %d bytes\n", RodataArray[looper].Addr, RodataArray[looper].Size);
            }
        }

        printf("\nBss Data\n");
        for(int looper = 0; looper < BssArrayLength; looper++)
        {
            if(BssArray[looper].Addr != 0)
            {
                PrintFixLenString(BssArray[looper].Name,30);
                printf(" 0x%08x, %d bytes\n", BssArray[looper].Addr, BssArray[looper].Size);
            }
        }

        for(int i = 0; i< TextArrayLength; i++)
        {
            free(TextArray[i].Name);
        }
        for(int i = 0; i< RodataArrayLength; i++)
        {
            free(RodataArray[i].Name);
        }
        for(int i = 0; i< BssArrayLength; i++)
        {
            free(BssArray[i].Name);
        }
        free(TextArray);
        free(RodataArray);
        free(BssArray);
        free(File);
    }
    else printf("Failed to open the file");


    fclose(f);
    return 0;
}

