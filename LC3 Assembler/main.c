//Jared Huberman
//CPS 330
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
FILE *infile, *outfile;
char outfilename[100], linesave[100], buf[100], *p, *mnemonic, *o1,
 *o2, *o3, *label;
short pcoffset9, pcoffset11, imm5, imm9, offset6, trapvect8, macword;
unsigned short location_counter, address, dr, sr, sr1,baser, sr2, sr3;
int num, linenum;
char *symbol[1000];
int symadd[1000];

unsigned short getadd(char *ptr){
    int i;
    for(i = 0; i < 1000; i++){
        if(!strcmp(symbol[i], ptr)){
            return symadd[i];
        }
    }
    return -1;
}

void error(char *msg){
    printf("%s\n", msg);
    printf("At line %d:\n", linenum);
    printf("%s\n", buf[linenum]);
    exit(1);
}

int getreg(char *reg){
    if(reg[1] == '0') return 0;
    if(reg[1] == '1') return 1;
    if(reg[1] == '2') return 2;
    if(reg[1] == '3') return 3;
    if(reg[1] == '4') return 4;
    if(reg[1] == '5') return 5;
    if(reg[1] == '6') return 6;
    if(reg[1] == '7') return 7;
    return -1;
}

int main(int argc, char *argv[])
{
    if (argc == 2)
        printf("Jared Huberman %s, %s\n\n", argv[0], argv[1]);
    else{
        printf("Wrong number of command line args\n");
        exit(1);
    }
    infile = fopen(argv[1], "r");
    if(!infile){
        printf("Cannot open input file %s\n", argv[1]);
        exit(1);
    }
    strcpy(outfilename, argv[1]);
    p = strrchr(outfilename, '.');
    if(p)
        *p = '\0';
    strcat(outfilename, ".e");
    outfile = fopen(outfilename, "wb");
    if(!outfile){
        printf("Cannot open output file %s\n", outfilename);
        exit(1);
    }
    location_counter = linenum = 0;
    int stsize = 0;
    fwrite("C", 1, 1, outfile);

    //Pass 1
    printf("Starting Pass 1\n");
    while(fgets(buf, sizeof(buf), infile)){
        linenum++;  //get track of line number
        p = buf;
        while(isspace(*p)) p++;
        if(*p == '\0' || *p == ';') //if line all blank, go to next line
            continue;
        strcpy(linesave, buf);
        if(!isspace(buf[0])){
            symbol[stsize] = strdup(strtok(buf, " \r\n\t"));
            symadd[stsize++] = location_counter;
            mnemonic = strtok(NULL, " \r\n\t"); // tokenize mnemonic/directive
            o1 = strtok(NULL, " \r\n\t,"); // tokenize first operand
            o2 = strtok(NULL, " \r\n\t,"); // tokenize second operand
            o3 = strtok(NULL, " \r\n\t,"); // tokenize third operand
        }
        else{
            mnemonic = strtok(buf, " \r\n\t"); // tokenize mnemonic/directive
            o1 = strtok(NULL, " \r\n\t,"); // tokenize first operand
            o2 = strtok(NULL, " \r\n\t,"); // tokenize second operand
            o3 = strtok(NULL, " \r\n\r,"); // tokenize third operand
        }
        if(mnemonic){
            if(!strcmp(mnemonic, ".blkw")){
                location_counter += strtol(o1, '\0', 10);
            }
            else location_counter++;
        }
    }
        rewind(infile);

        //Pass 2
        printf("Starting Pass 2\n");
        location_counter = linenum = 0;
        while(fgets(buf, sizeof(buf), infile)){
            p = buf;
            while (isspace(*p)) p++;
            if (*p == '\0' || *p ==';') // if line all blank, go to next line
                continue;
            if(!isspace(buf[0])){
                label = strdup(strtok(buf, " \r\n\t"));
                mnemonic = strtok(NULL, " \r\n\t"); // tokenize mnemonic/directive
                o1 = strtok(NULL, " \r\n\t,"); // tokenize first operand
                o2 = strtok(NULL, " \r\n\t,"); // tokenize second operand
                o3 = strtok(NULL, " \r\n\t,"); // tokenize third operand
            }
            else{
                mnemonic = strtok(buf, " \r\n\t"); // tokenize mnemonic/directive
                o1 = strtok(NULL, " \r\n\t,"); // tokenize first operand
                o2 = strtok(NULL, " \r\n\t,"); // tokenize second operand
                o3 = strtok(NULL, " \r\n\r,"); // tokenize third operand
            }
            if(!mnemonic){
                continue;
            }
            if (mnemonic[0] == 'b' && mnemonic[1] == 'r'){
                macword = 0;
                if (strchr(mnemonic, 'n'))
                    macword = macword | 0x0800;
                if (strchr(mnemonic, 'z'))
                    macword = macword | 0x0400;
                if (strchr(mnemonic, 'p'))
                    macword = macword | 0x0200;
                address = getadd(o1);
                pcoffset9 = (address - location_counter - 1);
                if (pcoffset9 > 255 || pcoffset9 < -256)
                    error("pcoffset9 out of range");
                macword = macword | (pcoffset9 & 0x01ff);
                fwrite(&macword, 2, 1, outfile); // write out instruction
            }
            else
            if(!strcmp(mnemonic, "add")){
                macword = 0x1000;
                dr = getreg(o1) << 9;
                sr1 = getreg(o2) << 6;
                if(o3 && (*o3 == 'r')){
                    sr2 = getreg(o3);
                    macword = macword | dr | sr1 | sr2;
                    fwrite(&macword, 2, 1, outfile);
                }
                else{
                    if(o3 && sscanf(o3, "%d", &num) != 1) //converts o3 to int
                        error("Bad imm5");
                    if(num > 15 || num < -16)
                        error("imm5 out of range");
                    macword = macword | dr | sr1 | 0x0020 | (num & 0x1f);
                    fwrite(&macword, 2, 1, outfile);
                }
            }
            else
            if(!strcmp(mnemonic, "ld")){
                macword = 0x2000;
                dr = getreg(o1) << 9;
                pcoffset9 = (getadd(o2) - location_counter - 1);
                if(pcoffset9 > 255 || pcoffset9 < -256)
                    error("pcoffset9 out of range");
                macword = macword | dr | pcoffset9/* & 0x1ff)*/; //assemble
                fwrite(&macword, 2, 1, outfile); //write out instruction
            }
            else
            if(!strcmp(mnemonic, "st")){
                macword = 0x3000;
                dr = getreg(o1) << 9;
                pcoffset9 = (getadd(o2) - location_counter - 1);
                if(pcoffset9 > 255 || pcoffset9 < -256)
                    error("pcoffset9 out of range");
                macword = macword | dr | (pcoffset9 & 0x1ff);
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, "jsr") || !strcmp(mnemonic, "jsrr")){
                macword = 0x4000;
                if(!strcmp(mnemonic, "jsr")){
                    pcoffset11 = (getadd(o1) - location_counter - 1);
                    macword = macword | 0x0800 | pcoffset11;
                    fwrite(&macword, 2, 1, outfile);
                }
                else if(!strcmp(mnemonic, "jsrr")){
                    baser = (getreg(o1)) << 6;
                    macword = macword | baser;
                    fwrite(&macword, 2, 1, outfile);
                }
            }
            else
            if(!strcmp(mnemonic, "and")){
                macword = 0x5000;
                dr = getreg(o1) << 9;
                sr1 = getreg(o2) << 6;
                if(o3 && *o3 == 'r'){
                    sr2 = getreg(o3);
                    macword = macword | dr | sr1 | sr2;
                    fwrite(&macword, 2, 1, outfile);
                }
                else{
                    if(o3 && sscanf(o3, "%d", &num) != 1) //converts o3 to int
                        error("Bad imm5");
                    if(num > 15 || num < -16)
                        error("imm5 out of range");
                    macword = macword | dr | sr1 | 0x0020 | (num & 0x1f);
                    fwrite(&macword, 2, 1, outfile);
                }
            }
            else
            if(!strcmp(mnemonic, "ldr")){
                macword = 0x6000;
                dr = getreg(o1) << 9;
                baser = getreg(o2) << 6;
                offset6 = strtol(o3, '\0', 10);
                macword = macword | dr | baser | offset6;
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, "str")){
                macword = 0x7000;
                dr = getreg(o1) << 9;
                baser = (getreg(o2)) << 6;
                offset6 = strtol(o3, '\0', 10);
                macword = macword | dr | baser | offset6;
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, "push")){
                macword = 0x8000;
                sr = getreg(o1) << 9;
                macword = macword | sr;
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, "pop")){
                macword = 0x8000;
                dr = getreg(o1) << 9;
                macword = macword | dr | 1;
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, "not")){
                macword = 0x9000;
                dr = getreg(o1) << 9;
                sr1 = getreg(o2) << 6;
                macword = macword | dr | sr1 | 0x003f;
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, "ldi")){
                macword = 0xa000;
                dr = getreg(o1) << 9;
                pcoffset9 = (getadd(o2) - location_counter - 1);
                macword = macword | dr | (pcoffset9 & 0x1ff);
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, "sti")){
                macword = 0xb000;
                sr = getreg(o1) << 9;
                pcoffset9 = (getadd(o2) - location_counter - 1);
                macword = macword | sr | (pcoffset9 & 0x1ff);
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if (!strcmp(mnemonic, "jmp" )){
                macword = 0xc000; // get opcode for jmp
                baser = getreg(o1) << 6; // get reg number and position it
                macword = macword | baser; // combine opcode and reg number
                fwrite(&macword, 2, 1, outfile); // write out instruction
            }
            else
            if(!strcmp(mnemonic, "ret")){
                macword = 0xc1c0;
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, "lea")){
                macword = 0xe000;
                dr = getreg(o1) << 9;
                pcoffset9 = (getadd(o2) - location_counter - 1);
                macword = macword | dr | (pcoffset9 & 0x1ff);
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, "halt")){
                macword = 0xf025;
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, "nl")){
                macword = 0xf026;
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, "dout")){
                macword = 0xf027;
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, ".fill")){
                macword = strtol(o1, '\0', 10);
                fwrite(&macword, 2, 1, outfile);
            }
            else
            if(!strcmp(mnemonic, ".blkw")){
                macword = 0;
                int j = 0;
                while(j < strtol(o1, '\0', 10)){
                    fwrite(&macword, 2, 1, outfile);
                    location_counter++;
                    j++;
                }
                continue;
            }
            location_counter++;
        }
    return 0;
}
