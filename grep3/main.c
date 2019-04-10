
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
#include <string.h>
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
int    wait(int *);
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
int linenum = 20;
char    line[70];
char    *linp    = line;
char    *mktemp(char *);
char    tmpXXXXX[50] = "/tmp/eXXXXX";
char *getblock(unsigned int atl, int iof);
char *getline_(unsigned int tl);
char *place(char *sp, char *l1, char *l2);
void add(int i);
int advance(char *lp, char *ep);
int append(int (*f)(void), unsigned int *a);
void blkio(int b, char *buf, ssize_t (*iofcn)(int, void*, size_t));
void blkio4write(int b, char *buf, ssize_t (*iofcn)(int, const void*, size_t));
void commands(void);
void compile(int eof);
int compsub(void);
void dosub(void);
void error(char *s);
int execute(unsigned int *addr);
void exfile(void);
void filename(const char *s);
int getchr(void);
int getcopy(void);
int getfile(void);
void grepreadfile(char const *);
int getnum(void);
int getsub(void);
int gety(void);
void global(int k);
void init(void);
unsigned int *address(void);
void newline(void);
void nonzero(void);
void onintr(int n);
void print(void);
void printcommand(void);
void print_(int);
void printline_(char *,int);
void putchr_(int ac);
void putd(void);
int putline(void);
void puts_(char *sp);
void quit(int n);
void reverse(unsigned int *a1, unsigned int *a2);
void setwide(void);
void setnoaddr(void);
void squeeze(int i);

jmp_buf    savej;
#define SIZE 10000
char grepsearchbuf[SIZE];

#define BUFSIZE 100
char buf[BUFSIZE];
int bufp = 0;
int getch_(void) {
    char c = (bufp > 0) ? buf[--bufp] : getchar();
    lastc = c & 0177;
    if (lastc == '\n') {  // uncomment if you want to see the chars
        printf("getch(): newline\n");
    } else {
        printf("getch_(): %c\n", lastc);
    }
    return lastc;
}
void ungetch_(int c) { if (bufp >= BUFSIZE) { printf("ungetch: overflow\n");} else { buf[bufp++] = c; } }

void search(const char* re) {
    char buf[GBSIZE];
    snprintf(buf, sizeof(buf), "/%s\n", re);  // / and \n very important
    printf("g%s\n", buf);
    const char* p = buf + strlen(buf) - 1;
    while (p >= buf) { ungetch_(*p--); }
    global(1);
}
int main(int argc, char *argv[]) {
    zero = (unsigned *)malloc(nlall*sizeof(unsigned));
    if (argc != 3) { fprintf(stderr,"error:only accepting three arguments\n"); return 1; }
    else {
        printf("searching for RE: \'%s\'... in file(s)/directory(ies): \'%s\'...\n", argv[1], argv[2]);
        strcpy(grepsearchbuf, argv[1]);//say argv[1] is a string like what
        grepreadfile(argv[2]);
        search(argv[1]); printf("\nquitting...\n");  exit(1);
    } return 0;
}

void grepreadfile(const char *v) { setnoaddr();
    if (vflag && fchange) { fchange = 0;}
    filename(v); printf("%s\n",file); init(); addr2 = zero;
    if ((io = open(file, 0)) < 0) { lastc = '\n'; error(file); }
    setwide();squeeze(0);ninbuf = 0; append(getfile, addr2);exfile();
}

void filename(const char * name) { strcpy(file, name); strcpy(savedfile, name); }

void print_(int n) { int i = 0;
    while(n != (linenum-1) && genbuf[i] != '\0') {
        if (genbuf[i] == '\n') { ++n;}
        if (n == (linenum -1)) { ++i;
            while(genbuf[i] != '\n') { printf("%c", genbuf[i]); ++i; }
        } ++i;
    } printf("\n");
}

unsigned int *address(void) { int sign; unsigned int *a, *b;int opcnt, nextopand; int c;
    nextopand = -1; sign = 1; opcnt = 0; a = dot;
    do { do c = getchr(); while (c==' ' || c=='\t');
        if ('0'<=c && c<='9') { peekc = c;
            if (!opcnt) { a = zero; } a += sign*getnum();
        } else switch (c) {
            case '$': a = dol; /* fall through */
            case '.': if (opcnt) { error(Q); } break;
            case '\'': c = getchr();
                if (opcnt || c<'a' || 'z'<c) { error(Q); } a = zero;
                do a++; while (a<=dol && names[c-'a']!=(*a&~01)); break;
            case '?': sign = -sign; /* fall through */
            case '/':
                compile(c);b = a;
                for (;;) {a += sign;
                    if (a<=zero) { a = dol; } if (a>dol) { a = zero; }
                    if (execute(a)) { break; } if (a==b) { error(Q); }
                } break;
            default:
                if (nextopand == opcnt) { a += sign;
                    if (a<zero || dol<a) { continue; }       /* error(Q); */
                }
                if (c!='+' && c!='-' && c!='^') { peekc = c; if (opcnt==0) { a = 0; } return (a); }
                sign = 1; if (c!='+') { sign = -sign; }
                nextopand = ++opcnt; continue;
        } sign = 1;opcnt++;
    } while (zero<=a && a<=dol);
    error(Q); /*NOTREACHED*/ return 0;
}

int getnum(void) { int r, c; r = 0;
    while ((c=getchr())>='0' && c<='9') { r = r*10 + c - '0'; }
    peekc = c;return (r);
}

void setwide(void) { if (!given) { addr1 = zero + (dol>zero); addr2 = dol; } }

void setnoaddr(void) { if (given) { error(Q); } }

void nonzero(void) { squeeze(1); }

void squeeze(int i) { if (addr1<zero+i || addr2>dol || addr1>addr2) { error(Q); } }

void newline(void) { int c;
    if ((c = getchr()) == '\n' || c == EOF) { return; }
    if (c=='p' || c=='l' || c=='n') { pflag++;
        if (c=='l'){ listf++; } else if (c=='n') { listn++; }
        if ((c=getchr())=='\n') { return; }
    } error(Q);
}
void exfile(void) {  close(io);  io = -1;  if (vflag) {  putd();  puts(" characters read");  }  }

void error(char *s) { int c;wrapp = 0;listf = 0;listn = 0;
    putchr_('?');puts(s);count = 0; lseek(0, (long)0, 2); pflag = 0;
    if (globp) { lastc = '\n'; } globp = 0; peekc = lastc;
    if(lastc) { while ((c = getchr()) != '\n' && c != EOF); }
    if (io > 0) { close(io); io = -1; }
    longjmp(savej, 1);
}
char *ptrofglobbuf = grepsearchbuf;

int getchr(void) {  char c;
    if ((lastc=peekc)) { peekc = 0;  return(lastc); }
    if (globp) { if ((lastc = *globp++) != 0)  { return(lastc); }  globp = 0;  return(EOF); }
    if ((c = getch_()) < 0) { return(lastc = EOF); }  return (lastc = c & 0177);
}
void print(void) {unsigned int *a1;
    nonzero(); a1 = addr1;
    do {
        if (listn) { count = a1-zero; putd(); putchr_('\t'); }
        puts(getline_(*a1++));
    } while (a1 <= addr2); dot = addr2; listf = 0; listn = 0; pflag = 0;
}

int getfile(void) { int c;char *lp, *fp; lp = linebuf; fp = nextip;
    do { if (--ninbuf < 0) {
            if ((ninbuf = (int)read(io, genbuf, LBSIZE)-1) < 0) {
                if (lp>linebuf) { puts("'\\n' appended"); *genbuf = '\n'; }
                else return(EOF); } fp = genbuf;
            while(fp < &genbuf[ninbuf]) { if (*fp++ & 0200) { break; } } fp = genbuf;
        } c = *fp++; if (c=='\0') { continue; }
        if (c&0200 || lp >= &linebuf[LBSIZE]) { lastc = '\n'; error(Q); } *lp++ = c; count++;
    } while (c != '\n');
    *--lp = 0; nextip = fp;return(0);
}

int append(int (*f)(void), unsigned int *a) { unsigned int *a1 = NULL;unsigned int *a2 = NULL;unsigned int *rdot = NULL; int nline, tl;
    nline = 0;dot = a;
    while ((*f)() == 0) {
        if ((dol-zero)+1 >= nlall) {
            unsigned *ozero = zero; nlall += 1024; dot += zero - ozero; dol += zero - ozero;
        }
        tl = putline(); nline++;a1 = ++dol;a2 = a1+1; rdot = ++dot;
        while (a1 > rdot) { *--a2 = *--a1; } *rdot = tl;
    } return(nline);
}

void quit(int n) { if (vflag && fchange && dol!=zero) { fchange = 0;error(Q); } unlink(tfname);exit(0); }

char *getline_(unsigned int tl) { char *bp, *lp;int nl;
    lp = linebuf; bp = getblock(tl, READ);nl = nleft; tl &= ~((BLKSIZE/2)-1);
    while ((*lp++ = *bp++)) { if (--nl == 0) { bp = getblock(tl+=(BLKSIZE/2), READ); nl = nleft; } } return(linebuf);
}

int putline(void) { char *bp, *lp;int nl; unsigned int tl;
    fchange = 1;lp = linebuf;tl = tline;
    bp = getblock(tl, WRITE); nl = nleft; tl &= ~((BLKSIZE/2)-1);
    while ((*bp = *lp++)) {
        if (*bp++ == '\n') { *--bp = 0; linebp = lp; break; }
        if (--nl == 0) { bp = getblock(tl+=(BLKSIZE/2), WRITE); nl = nleft; }
    } nl = tline; tline += (((lp-linebuf)+03)>>1)&077776; return(nl);
}

char *getblock(unsigned int atl, int iof) { int bno, off;
    bno = (atl/(BLKSIZE/2)); off = (atl<<1) & (BLKSIZE-1) & ~03;
    if (bno >= NBLK) { lastc = '\n'; error(T); } nleft = BLKSIZE - off;
    if (bno==iblock) { ichanged |= iof; return(ibuff+off); }
    oblock = bno; return(obuff+off);
}

void init(void) { int *markp;
    close(tfile); tline = 2;
    for (markp = names; markp < &names[26]; ) { *markp++ = 0; }
    subnewa = 0; anymarks = 0; iblock = -1; oblock = -1; ichanged = 0;
    close(creat(tfname, 0600)); tfile = open(tfname, 2); dot = dol = zero;
}

void global(int k) { char *gp; int c; unsigned int *a1; char globuf[GBSIZE];
    printf("entering global\n");
    if (globp) { error(Q); } setwide(); squeeze(dol>zero);
    if ((c=getchr())=='\n') { error(Q); } compile(c); gp = globuf;
    printf("global: just after returning from compile, but before while\n");
    printf("c is: %c\n", (char)c);
    while ((c = getchr()) != '\n') {
        if (c==EOF) { error(Q); }
        if (c=='\\') { c = getchr(); if (c!='\n') { *gp++ = '\\'; } } *gp++ = c;
        if (gp >= &globuf[GBSIZE-2]) { error(Q); }
    } if (gp == globuf) { *gp++ = 'p'; } *gp++ = '\n';*gp++ = 0;
    for (a1=zero; a1<=dol; a1++) { *a1 &= ~01; if (a1>=addr1 && a1<=addr2 && execute(a1)==k) { *a1 |= 01; } }
    for (a1=zero; a1<=dol; a1++) {
        if (*a1 & 01) { *a1 &= ~01;dot = a1;globp = globuf;newline(); printcommand(); a1 = zero; }
    }
}

void compile(int eof) { int c;char *ep;char *lastep;char bracket[NBRA], *bracketp; int cclcnt; ep = expbuf; bracketp = bracket;
    if ((c = getchr()) == '\n') { peekc = c; c = eof; }
    if (c == eof) { if (*ep==0) { error(Q); } return; } nbra = 0;
    if (c=='^') { c = getchr(); *ep++ = CCIRC; } peekc = c; lastep = 0;
    for (;;) {
        if (ep >= &expbuf[ESIZE]) { goto cerror; } //-------------------------------------why does it give me this warning
        c = getchr();
        if (c == '\n') { peekc = c; c = eof; }
        if (c==eof) { if (bracketp != bracket){ expbuf[0] = 0; nbra = 0; error(Q); } *ep++ = CEOF; return; }
        if (c!='*') { lastep = ep; }
        switch (c) {
            case '\\': if ((c = getchr())=='(') { if (nbra >= NBRA) { expbuf[0] = 0; nbra = 0; error(Q); }
                    *bracketp++ = nbra;*ep++ = CBRA; *ep++ = nbra++; continue; }
                if (c == ')') { if (bracketp <= bracket) { expbuf[0] = 0; nbra = 0; error(Q); }
                    *ep++ = CKET;*ep++ = *--bracketp; continue; }
                if (c>='1' && c<'1'+NBRA) { *ep++ = CBACK; *ep++ = c-'1'; continue; }*ep++ = CCHR;
                if (c=='\n') { expbuf[0] = 0; nbra = 0; error(Q); } *ep++ = c; continue;
                
            case '.': *ep++ = CDOT; continue;
            case '\n': expbuf[0] = 0; nbra = 0; error(Q);
            case '*': if (lastep==0 || *lastep==CBRA || *lastep==CKET) { *ep++ = CCHR; *ep++ = c; } *lastep |= STAR; continue;
            case '$': if ((peekc=getchr()) != eof && peekc!='\n') { *ep++ = CCHR; *ep++ = c; } *ep++ = CDOL;continue;
            case '[': *ep++ = CCL; *ep++ = 0; cclcnt = 1;
                if ((c=getchr()) == '^') { c = getchr(); ep[-2] = NCCL; }
                do { if (c=='\n') { expbuf[0] = 0; nbra = 0; error(Q); }
                    if (c=='-' && ep[-1]!=0) { if ((c=getchr())==']') { *ep++ = '-'; cclcnt++; break;}
                        while (ep[-1]<c) { *ep = ep[-1]+1; ep++; cclcnt++;
                            if (ep>=&expbuf[ESIZE]) { expbuf[0] = 0; nbra = 0; error(Q); } }
                    } *ep++ = c; cclcnt++;
                    if (ep >= &expbuf[ESIZE]) { expbuf[0] = 0; nbra = 0; error(Q); }
                } while ((c = getchr()) != ']'); lastep[1] = cclcnt; continue;
            default: *ep++ = CCHR; *ep++ = c;
        }
    }
cerror: expbuf[0] = 0; nbra = 0; error(Q);
}

int execute(unsigned int *addr) { char *p1, *p2; int c;
    for (c=0; c<NBRA; c++) { braslist[c] = 0; braelist[c] = 0; }
    p2 = expbuf; p1 = getline_(*addr);
    if (*p2==CCIRC) { loc1 = p1; return(advance(p1, p2+1)); }
    /* fast check for first character */
    if (*p2==CCHR) { c = p2[1];
        do { if (*p1!=c) { continue; } if (advance(p1, p2)) { loc1 = p1; return(1); }
        } while (*p1++); return(0);
    }
    do { if (advance(p1, p2)) { loc1 = p1; return(1); } } while (*p1++); return(0);
}

int advance(char *lp, char *ep) {
    for (;;) switch (*ep++) {
        case CCHR: if (*ep++ == *lp++) { continue; } return(0);
        case CEOF: loc2 = lp; return(1);
        default: error(Q);
    }
}

void putd(void) {  printf("%ld characters read\n", count); }

void putchr_(int ac) {  char *lp = linp;  int c = ac;
    if (listf) {
        if (c == '\n') { if (linp != line && linp[-1]==' ') {  *lp++ = '\\';  *lp++ = 'n';  }
        } else {
            if (col > (72 - 4 - 2)) {  col = 8;  *lp++ = '\\';  *lp++ = '\n';  *lp++ = '\t';  }  col++;
            if (c=='\b' || c=='\t' || c=='\\') {
                *lp++ = '\\';  if (c=='\b') { c = 'b'; }  else if (c=='\t') { c = 't'; }  col++;
            } else if (c<' ' || c=='\177') {
                *lp++ = '\\';  *lp++ =  (c>>6) +'0';  *lp++ = ((c >> 3) & 07) + '0';  c = (c & 07) + '0';  col += 3;
            }
        }
    }  *lp++ = c;
    if (c == '\n' || lp>=&line[64]) { linp = line;  write(oflag ? 2 : 1, line, lp - line); return; }  linp = lp;
}
void printcommand(void) {  int c;  char lastsep;
    for (;;) {  unsigned int* a1;
        if (pflag) { pflag = 0;  addr1 = addr2 = dot;  print(); }  c = '\n';
        for (addr1 = 0;;) {  lastsep = c;  a1 = address();  c = getchr();
            if (c != ',' && c != ';') { break; }  if (lastsep==',') { error(Q); }
            if (a1==0) {  a1 = zero+1;  if (a1 > dol) { a1--; }  }  addr1 = a1;  if (c == ';') { dot = a1; }
        }
        if (lastsep != '\n' && a1 == 0) { a1 = dol; }
        if ((addr2 = a1)==0) { given = 0;  addr2 = dot;  } else { given = 1; }  if (addr1==0) { addr1 = addr2; }
        switch(c) {
            case 'p':  case 'P':  newline();  print();  continue;
            case EOF:  default:  return;
        }
    }
}
