//Jared Huberman CPS 330
#include <stdio.h>
#include <stdlib.h>
static unsigned short n, z, p;
void setcc(short b, short i){
    if(b < 0){
        n = 1;
        z = 0;
        p = 0;
    }
    else if(b == 0){
        n = 0;
        z = 1;
        p = 0;
    }
    else{
        n = 0;
        z = 0;
        p = 1;
    }
}
int main(int argc, char *argv[])
{
    short r[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned short pc = 0, ir;
    short mem[65536];
    unsigned short opcode, dr, sr, sr1, baser, sr2, imm9,
        offset6, bit5, bit11, nm, zm, pm, temp;
    short trapvect8, pcoffset9, pcoffset11, imm5;

    FILE *infile;
    FILE *outfile;
    infile = fopen(argv[1], "rb");
    outfile = fopen("a1test.e", "wb");
    printf("Jared Huberman %s, %s\n\n", argv[0], argv[1]);
    if(!infile){
        printf("Cannot open input file %s\n", argv[1]);
        exit(1);
    }

    char c;
    fread(&c, 1, 1, infile);
    fread(mem, 1, sizeof(mem), infile);

    while(1){
        ir = mem[pc++];
        opcode = ir >> 12;
        nm = (ir & 0x0800) >> 11;
        zm = (ir & 0x0400) >> 10;
        pm = (ir & 0x0200) >> 9;
        pcoffset9 = ir << 7;
        pcoffset9 = pcoffset9 >> 7;
        pcoffset11 = ir << 5;
        pcoffset11 = pcoffset11 >> 5;
        imm5 = ir << 11;
        imm5 = imm5 >> 11;
        offset6 = ir << 10;
        offset6 = offset6 >> 10;
        trapvect8 = ir & 0xff;
        dr = sr = (ir & 0x0e00) >> 9;
        sr1 = baser = (ir & 0x01c0) >> 6;
        sr2 = (ir & 0x0007);
        bit5 = nm;
        bit11 = (ir & 0x0020) >> 5;
        switch (opcode){
            case 0:     //br
                if(n & nm || z & zm || p & pm)
                    pc = pc + pcoffset9;
                break;
            case 1:     //add
                if (bit11)
                    r[dr] = r[sr1] + imm5;
                else
                    r[dr] = r[sr1] + r[sr2];
                setcc(r[dr], dr);
                break;
            case 2:     //ld
                r[dr] = mem[pc + pcoffset9];
                setcc(r[dr], dr);
                break;
            case 3:     //st
                mem[pc + pcoffset9] = r[sr];
                break;
            case 4:     //jsr/jsrr
                temp = pc;
                if(bit5 == 0)
                    pc = r[baser];
                else
                    pc = pc + pcoffset11;
                r[7] = temp;
                break;
            case 5:     //and
                if(bit11 == 0)
                    r[dr] = r[sr1] & r[sr2];
                else
                    r[dr] = r[sr1] & imm5;
                setcc(r[dr], dr);
                break;
            case 6:     //ldr
                r[dr] = mem[r[baser] + offset6];
                setcc(r[dr], dr);
                break;
            case 7:     //str
                mem[r[baser] + offset6] = r[sr];
                break;
            case 9:     //not
                r[dr] = ~r[sr1];
                setcc(r[dr], dr);
                break;
            case 10:    //ldi
                r[dr] = mem[mem[pc + pcoffset9]];
                setcc(r[dr], dr);
                break;
            case 11:    //sti
                mem[mem[pc + pcoffset9]] = r[sr];
                break;
            case 12:    //jmp/ret
                pc = r[baser];
                break;
            case 14:    //lea
                r[dr] = pc + pcoffset9;
                setcc(r[dr], dr);
                break;
            case 15:    //trap
                if(trapvect8 == 0x25)   //halt
                    exit(0);
                else
                if(trapvect8 == 0x26){   //nl
                    printf("\n");
                    fputs("\n", outfile);}
                else
                if(trapvect8 == 0x27){   //dout
                    printf("%d", r[0]);
                    fprintf(outfile, "%d", r[0]);}
                break;
        }
    }
}
