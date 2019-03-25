
/*
 * Editor
 */
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <aio.h>
#include <fcntl.h>
#include <stdlib.h>

/*  our program is simple: all I need to know is what to look for and where to go argv[1] and argv[2] will tell us this */

/* make BLKSIZE and LBSIZE 512 for smaller machines */
#define    BLKSIZE    4096
#define    NBLK    2047

#define    FNSIZE    128
#define    LBSIZE    4096
#define    ESIZE    256
#define    GBSIZE    256
#define    NBRA    5
#define    KSIZE    9

#define    CBRA    1
#define    CCHR    2
#define    CDOT    4
#define    CCL    6
#define    NCCL    8
#define    CDOL    10
#define    CEOF    11
#define    CKET    12
#define    CBACK    14
#define    CCIRC    15

#define    STAR    01

char    Q[]    = "";
char    T[]    = "TMP";
#define    READ    0
#define    WRITE    1

int    peekc;
int    lastc;
char    savedfile[FNSIZE];
char    file[FNSIZE];
char    linebuf[LBSIZE];
char    rhsbuf[LBSIZE/2];
char    expbuf[ESIZE+4];
int    given;
unsigned int    *addr1, *addr2;
unsigned int    *dot, *dol, *zero;
char    genbuf[LBSIZE];
long    count;
char    *nextip;
char    *linebp;
int    ninbuf;
int    io;
int    pflag;
//long    lseek(int, long, int);
//int    open(char *, int);
//int    creat(char *, int);
//int    read(int, char*, int);
//int    write(int, char*, int);
//int    close_(int);
//int    fork(void);
//int    execl(char *, ...);
//int    exit(int);
int    wait(int *);
//int    unlink(char *);


int    vflag    = 1;
int    oflag;
int    listf;
int    listn;
int    col;
char    *globp;
int    tfile    = -1;
int    tline;
char    *tfname;
char    *loc1;
char    *loc2;
char    ibuff[BLKSIZE];
int    iblock    = -1;
char    obuff[BLKSIZE];
int    oblock    = -1;
int    ichanged;
int    nleft;
char    WRERR[]    = "WRITE ERROR";
int    names[26];
int    anymarks;
char    *braslist[NBRA];
char    *braelist[NBRA];
int    nbra;
int    subnewa;
int    subolda;
int    fchange;
int    wrapp;
int    bpagesize = 20;
unsigned nlall = 128;

char    *mktemp(char *);
char    tmpXXXXX[50] = "/tmp/eXXXXX";
//char    *malloc_(int);
//char    *realloc_(char *, int);

char *getblock(unsigned int atl, int iof);
char *getline_(unsigned int tl);
char *place(char *sp, char *l1, char *l2);
void add(int i);
int advance(char *lp, char *ep);
int append(int (*f)(void), unsigned int *a);
//int backref_(int i, char *lp);
//void blkio(int b, char *buf, int (*iofcn)(int, void*, int));
void blkio(int b, char *buf, ssize_t (*iofcn)(int, void*, size_t));
void blkio4write(int b, char *buf, ssize_t (*iofcn)(int, const void*, size_t));
//void callunix(void);
//int cclass(char *set, int c, int af);
void commands(void);
void compile(int eof);
int compsub(void);
void dosub(void);
void error(char *s);
int execute(unsigned int *addr);
void exfile(void);
void filename(int num);

//void gdelete(void);
int getchr(void);
int getcopy(void);
int getfile(void);
void grepreadfile(char **);
int getnum(void);
int getsub(void);
//int gettty(void);
int gety(void);
void global(int k);
void init(void);
unsigned int *address(void);
//void join(void);
//void move_(int cflag);
void newline(void);
void nonzero(void);
//void onhup(int n);
void onintr(int n);
void print(void);
void putchr(int ac);
void putd(void);
//void putfile(void);
int putline(void);
void puts_(char *sp);
void quit(int n);
//void rdelete(unsigned int *ad1, unsigned int *ad2);
void reverse(unsigned int *a1, unsigned int *a2);
void setwide(void);
void setnoaddr(void);
void squeeze(int i);
//void substitute(int inglob);

jmp_buf    savej;
char grepsearchbuf[1000];

//typedef void    (*SIG_TYP)(int);
//SIG_TYP    oldhup;
//SIG_TYP    oldquit;
/* these two are not in ansi, but we need them */
#define    SIGHUP    1    /* hangup */
#define    SIGQUIT    3    /* quit (ASCII FS) */
#include <string.h>

int main(int argc, char *argv[]) {
    char *p1, *p2;
    
    if (argc != 3) { printf("Too many arguments");}
    //    printf("searching for RE: \'%s\'... in file(s)/directory(ies): \'%s\'...\n", argv[1], argv[2]);
    // strcpy(grepsearchbuf, argv[1]);
    //grepreadfile(&argv[2]);
    if (argc>1) {
        p1 = *argv;
        p2 = savedfile;
        while ((*p2++ = *p1++))
            if (p2 >= &savedfile[sizeof(savedfile)])
                p2--;
        globp = "r";
    }
    zero = (unsigned *)malloc(nlall*sizeof(unsigned));
    tfname = mktemp(tmpXXXXX);
    init();
    //  printf("searching for RE: \'%s\'... in file(s)/directory(ies): \'%s\'...\n", argv[1], argv[2]);
    setjmp(savej);
    commands();
    
    
    quit(0);
    return 0;
}

void commands(void) {
    unsigned int *a1;
    int c;
    char lastsep;
    
    for (;;) {
        if (pflag) {
            pflag = 0;
            addr1 = addr2 = dot;
            print(); ///where I am
        }
        c = '\n';
        for (addr1 = 0;;) {
            lastsep = c; //laststep = '\n'
            a1 = address(); //going to get directory?
            c = getchr();
            if (c!=',' && c!=';')
                break;
            if (lastsep==',')
                error(Q);
            if (a1==0) {
                a1 = zero+1;
                if (a1>dol)
                    a1--;
            }
            addr1 = a1;
            if (c==';')
                dot = a1;
        }
        if (lastsep!='\n' && a1==0)
            a1 = dol;
        if ((addr2=a1)==0) {
            given = 0;
            addr2 = dot;
        }
        else
            given = 1;
        if (addr1==0)
            addr1 = addr2;
        switch(c) {
                
            case 'E':
                fchange = 0;
                c = 'e';
            case 'e':
                setnoaddr();
                if (vflag && fchange) {
                    fchange = 0;
                    error(Q);
                }
                filename(c);
                init();
                addr2 = zero;
                goto caseread;
                
            case 'g':
                global(1);
                continue;
                
            case 'l':
                listf++;
            case 'p':
            case 'P':
                newline();
                print();
                continue;
                
            case 'Q':
                fchange = 0;
            case 'q':
                setnoaddr();
                newline();
                quit(0);
                
            caseread:
                if ((io = open(file, 0)) < 0) {
                    lastc = '\n';
                    error(file);
                }
                setwide();
                squeeze(0);
                ninbuf = 0;
                c = zero != dol;
                append(getfile, addr2);
                exfile();
                fchange = c;
                continue;
                
            case EOF:
                return;
                
        }
        error(Q);
    }
}

void print(void) {
    unsigned int *a1;
    
    nonzero(); //next
    a1 = addr1;
    do {
        if (listn) {
            count = a1-zero;
            putd();
            putchr('\t');
        }
        puts(getline_(*a1++));
    } while (a1 <= addr2);
    dot = addr2;
    listf = 0;
    listn = 0;
    pflag = 0;
}

unsigned int *
address(void) {
    int sign;
    unsigned int *a, *b;
    int opcnt, nextopand;
    int c;
    
    nextopand = -1;
    sign = 1;
    opcnt = 0;
    a = dot;
    do {
        do c = getchr(); while (c==' ' || c=='\t');
        if ('0'<=c && c<='9') {
            peekc = c;
            if (!opcnt)
                a = zero;
            a += sign*getnum();
        } else switch (c) {
            case '$':
                a = dol;
                /* fall through */
            case '.':
                if (opcnt)
                    error(Q);
                break;
            case '\'':
                c = getchr();
                if (opcnt || c<'a' || 'z'<c)
                    error(Q);
                a = zero;
                do a++; while (a<=dol && names[c-'a']!=(*a&~01));
                break;
            case '?':
                sign = -sign;
                /* fall through */
            case '/':
                compile(c);
                b = a;
                for (;;) {
                    a += sign;
                    if (a<=zero)
                        a = dol;
                    if (a>dol)
                        a = zero;
                    if (execute(a))
                        break;
                    if (a==b)
                        error(Q);
                }
                break;
            default:
                if (nextopand == opcnt) {
                    a += sign;
                    if (a<zero || dol<a)
                        continue;       /* error(Q); */
                }
                if (c!='+' && c!='-' && c!='^') {
                    peekc = c;
                    if (opcnt==0)
                        a = 0;
                    return (a);
                }
                sign = 1;
                if (c!='+')
                    sign = -sign;
                nextopand = ++opcnt;
                continue;
        }
        sign = 1;
        opcnt++;
    } while (zero<=a && a<=dol);
    error(Q);
    /*NOTREACHED*/
    return 0;
}

int getnum(void) {
    int r, c;
    
    r = 0;
    while ((c=getchr())>='0' && c<='9')
        r = r*10 + c - '0';
    peekc = c;
    return (r);
}

void setwide(void) {
    if (!given) {
        addr1 = zero + (dol>zero);
        addr2 = dol;
    }
}

void setnoaddr(void) {
    if (given)
        error(Q);
}

void nonzero(void) {
    squeeze(1);
}

void squeeze(int i) {
    if (addr1<zero+i || addr2>dol || addr1>addr2)
        error(Q);
}

void newline(void) {
    int c;
    
    if ((c = getchr()) == '\n' || c == EOF)
        return;
    if (c=='p' || c=='l' || c=='n') {
        pflag++;
        if (c=='l')
            listf++;
        else if (c=='n')
            listn++;
        if ((c=getchr())=='\n')
            return;
    }
    error(Q);
}


void filename(int comm) {
    char *p1, *p2;
    int c;
    
    count = 0;
    c = getchr();
    if (c=='\n' || c==EOF) {
        p1 = savedfile;
        if (*p1==0 && comm!='f')
            error(Q);
        p2 = file;
        while ((*p2++ = *p1++))
            ;
        return;
    }
    if (c!=' ')
        error(Q);
    while ((c = getchr()) == ' ')
        ;
    if (c=='\n')
        error(Q);
    p1 = file;
    do {
        if (p1 >= &file[sizeof(file)-1] || c==' ' || c==EOF)
            error(Q);
        *p1++ = c;
    } while ((c = getchr()) != '\n');
    *p1++ = 0;
    if (savedfile[0]==0 || comm=='e' || comm=='f') {
        p1 = savedfile;
        p2 = file;
        while ((*p1++ = *p2++))
            ;
    }
}

void exfile(void) {
    close(io);
    io = -1;
    if (vflag) {
        putd();
        putchr('\n');
    }
}

void error(char *s) {
    int c;
    
    wrapp = 0;
    listf = 0;
    listn = 0;
    putchr('?');
    puts(s);
    count = 0;
    lseek(0, (long)0, 2);
    pflag = 0;
    if (globp)
        lastc = '\n';
    globp = 0;
    peekc = lastc;
    if(lastc)
        while ((c = getchr()) != '\n' && c != EOF)
            ;
    if (io > 0) {
        close(io);
        io = -1;
    }
    longjmp(savej, 1);
}

int getchr(void) {
    char c;
    if ((lastc=peekc)) {
        peekc = 0;
        return(lastc);
    }
    if (globp) {
        if ((lastc = *globp++) != 0)
            return(lastc);
        globp = 0;
        return(EOF);
    }
    if (read(0, &c, 1) <= 0)
        return(lastc = EOF);
    lastc = c&0177;
    return(lastc);
}

int getfile(void) {
    int c;
    char *lp, *fp;
    
    lp = linebuf;
    fp = nextip;
    do {
        if (--ninbuf < 0) {
            if ((ninbuf = (int)read(io, genbuf, LBSIZE)-1) < 0) {
                if (lp>linebuf) {
                    puts("'\\n' appended");
                    *genbuf = '\n';
                }
                else return(EOF);
            }
            fp = genbuf;
            while(fp < &genbuf[ninbuf]) {
                if (*fp++ & 0200)
                    break;
            }
            fp = genbuf;
        }
        c = *fp++;
        if (c=='\0')
            continue;
        if (c&0200 || lp >= &linebuf[LBSIZE]) {
            lastc = '\n';
            error(Q);
        }
        *lp++ = c;
        count++;
    } while (c != '\n');
    *--lp = 0;
    nextip = fp;
    return(0);
}

int append(int (*f)(void), unsigned int *a) {
    unsigned int *a1, *a2, *rdot;
    int nline, tl;
    
    nline = 0;
    dot = a;
    while ((*f)() == 0) {
        if ((dol-zero)+1 >= nlall) {
            unsigned *ozero = zero;
            
            nlall += 1024;
            dot += zero - ozero;
            dol += zero - ozero;
        }
        tl = putline();
        nline++;
        a1 = ++dol;
        a2 = a1+1;
        rdot = ++dot;
        while (a1 > rdot)
            *--a2 = *--a1;
        *rdot = tl;
    }
    return(nline);
}

void quit(int n) {
    if (vflag && fchange && dol!=zero) {
        fchange = 0;
        error(Q);
    }
    unlink(tfname);
    exit(0);
}

char *
getline_(unsigned int tl) {
    char *bp, *lp;
    int nl;
    
    lp = linebuf;
    bp = getblock(tl, READ);
    nl = nleft;
    tl &= ~((BLKSIZE/2)-1);
    while ((*lp++ = *bp++))
        if (--nl == 0) {
            bp = getblock(tl+=(BLKSIZE/2), READ);
            nl = nleft;
        }
    return(linebuf);
}

int putline(void) {
    char *bp, *lp;
    int nl;
    unsigned int tl;
    
    fchange = 1;
    lp = linebuf;
    tl = tline;
    bp = getblock(tl, WRITE);
    nl = nleft;
    tl &= ~((BLKSIZE/2)-1);
    while ((*bp = *lp++)) {
        if (*bp++ == '\n') {
            *--bp = 0;
            linebp = lp;
            break;
        }
        if (--nl == 0) {
            bp = getblock(tl+=(BLKSIZE/2), WRITE);
            nl = nleft;
        }
    }
    nl = tline;
    tline += (((lp-linebuf)+03)>>1)&077776;
    return(nl);
}

char *
getblock(unsigned int atl, int iof) {
    int bno, off;
    
    bno = (atl/(BLKSIZE/2));
    off = (atl<<1) & (BLKSIZE-1) & ~03;
    if (bno >= NBLK) {
        lastc = '\n';
        error(T);
    }
    nleft = BLKSIZE - off;
    if (bno==iblock) {
        ichanged |= iof;
        return(ibuff+off);
    }
    
    oblock = bno;
    return(obuff+off);
}



void init(void) {
    int *markp;//pointer
    
    close(tfile);
    tline = 2;
    for (markp = names; markp < &names[26]; )
        *markp++ = 0;
    subnewa = 0;
    anymarks = 0;
    iblock = -1;
    oblock = -1;
    ichanged = 0;
    close(creat(tfname, 0600));
    tfile = open(tfname, 2);
    dot = dol = zero;
}

void global(int k) {
    char *gp;
    int c;
    unsigned int *a1;
    char globuf[GBSIZE];
    
    if (globp)
        error(Q);
    setwide();
    squeeze(dol>zero);
    if ((c=getchr())=='\n')
        error(Q);
    compile(c);
    gp = globuf;
    while ((c = getchr()) != '\n') {
        if (c==EOF)
            error(Q);
        if (c=='\\') {
            c = getchr();
            if (c!='\n')
                *gp++ = '\\';
        }
        *gp++ = c;
        if (gp >= &globuf[GBSIZE-2])
            error(Q);
    }
    if (gp == globuf)
        *gp++ = 'p';
    *gp++ = '\n';
    *gp++ = 0;
    for (a1=zero; a1<=dol; a1++) {
        *a1 &= ~01;
        if (a1>=addr1 && a1<=addr2 && execute(a1)==k)
            *a1 |= 01;
    }
    /*
     * Special case: g/.../d (avoid n^2 algorithm)
     */
    for (a1=zero; a1<=dol; a1++) {
        if (*a1 & 01) {
            *a1 &= ~01;
            dot = a1;
            globp = globuf;
            commands();
            a1 = zero;
        }
    }
}

void compile(int eof) {
    int c;
    char *ep;
    char *lastep;
    char bracket[NBRA], *bracketp;
    int cclcnt;
    
    ep = expbuf;
    bracketp = bracket;
    if ((c = getchr()) == '\n') {
        peekc = c;
        c = eof;
    }
    if (c == eof) {
        if (*ep==0)
            error(Q);
        return;
    }
    nbra = 0;
    if (c=='^') {
        c = getchr();
        *ep++ = CCIRC;
    }
    peekc = c;
    lastep = 0;
    for (;;) {
        if (ep >= &expbuf[ESIZE])
            goto cerror;
        c = getchr();
        if (c == '\n') {
            peekc = c;
            c = eof;
        }
        if (c==eof) {
            if (bracketp != bracket)
                goto cerror;
            *ep++ = CEOF;
            return;
        }
        if (c!='*')
            lastep = ep;
        switch (c) {
                
            case '\\':
                if ((c = getchr())=='(') {
                    if (nbra >= NBRA)
                        goto cerror;
                    *bracketp++ = nbra;
                    *ep++ = CBRA;
                    *ep++ = nbra++;
                    continue;
                }
                if (c == ')') {
                    if (bracketp <= bracket)
                        goto cerror;
                    *ep++ = CKET;
                    *ep++ = *--bracketp;
                    continue;
                }
                if (c>='1' && c<'1'+NBRA) {
                    *ep++ = CBACK;
                    *ep++ = c-'1';
                    continue;
                }
                *ep++ = CCHR;
                if (c=='\n')
                    goto cerror;
                *ep++ = c;
                continue;
                
            case '.':
                *ep++ = CDOT;
                continue;
                
            case '\n':
                goto cerror;
                
            case '*':
                if (lastep==0 || *lastep==CBRA || *lastep==CKET)
                    goto defchar;
                *lastep |= STAR;
                continue;
                
            case '$':
                if ((peekc=getchr()) != eof && peekc!='\n')
                    goto defchar;
                *ep++ = CDOL;
                continue;
                
            case '[':
                *ep++ = CCL;
                *ep++ = 0;
                cclcnt = 1;
                if ((c=getchr()) == '^') {
                    c = getchr();
                    ep[-2] = NCCL;
                }
                do {
                    if (c=='\n')
                        goto cerror;
                    if (c=='-' && ep[-1]!=0) {
                        if ((c=getchr())==']') {
                            *ep++ = '-';
                            cclcnt++;
                            break;
                        }
                        while (ep[-1]<c) {
                            *ep = ep[-1]+1;
                            ep++;
                            cclcnt++;
                            if (ep>=&expbuf[ESIZE])
                                goto cerror;
                        }
                    }
                    *ep++ = c;
                    cclcnt++;
                    if (ep >= &expbuf[ESIZE])
                        goto cerror;
                } while ((c = getchr()) != ']');
                lastep[1] = cclcnt;
                continue;
                
            defchar:
            default:
                *ep++ = CCHR;
                *ep++ = c;
        }
    }
cerror:
    expbuf[0] = 0;
    nbra = 0;
    error(Q);
}

int execute(unsigned int *addr) {
    char *p1, *p2;
    int c;
    
    for (c=0; c<NBRA; c++) {
        braslist[c] = 0;
        braelist[c] = 0;
    }
    p2 = expbuf;
    p1 = getline_(*addr);
    if (*p2==CCIRC) {
        loc1 = p1;
        return(advance(p1, p2+1));
    }
    /* fast check for first character */
    if (*p2==CCHR) {
        c = p2[1];
        do {
            if (*p1!=c)
                continue;
            if (advance(p1, p2)) {
                loc1 = p1;
                return(1);
            }
        } while (*p1++);
        return(0);
    }
    /* regular algorithm */
    do {
        if (advance(p1, p2)) {
            loc1 = p1;
            return(1);
        }
    } while (*p1++);
    return(0);
}

int advance(char *lp, char *ep) {
    
    for (;;) switch (*ep++) {
            
        case CCHR:
            if (*ep++ == *lp++)
                continue;
            return(0);
            
        case CEOF:
            loc2 = lp;
            return(1);
            
        default:
            error(Q);
    }
}


void putd(void) {
    int r;
    
    r = count%10;
    count /= 10;
    if (count)
        putd();
    putchr(r + '0');
}

char    line[70];
char    *linp    = line;

void putchr(int ac) {
    char *lp;
    int c;
    lp = linp;
    c = ac;
    *lp++ = c;
    if(c == '\n' || lp >= &line[64]) {
        linp = line;
        write(oflag?2:1, line, lp-line);
        return;
    }
    linp = lp;
}


