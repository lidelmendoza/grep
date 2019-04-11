#ifndef grep_h
#define grep_h
#include <stdio.h>
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
#define    READ    0
#define    WRITE    1
#define SIZE 10000
#define BUFSIZE 100
int getch_(void);
void ungetch_(int c);
void search(const char* re);
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
void putchr_(int ac);
void putd(void);
int putline(void);
void puts_(char *sp);
void quit(int n);
void reverse(unsigned int *a1, unsigned int *a2);
void setwide(void);
void setnoaddr(void);
void squeeze(int i);
#endif /* grep_h */
