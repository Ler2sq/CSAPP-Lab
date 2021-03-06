#include "cachelab.h"
#include <math.h>
#include <getopt.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
typedef int i32;
typedef unsigned int u32;
typedef long long i64;
typedef unsigned long long u64;
#define MAXLINE 1024
#define Err(str) printf("\033[31m" \
						str 	   \
						"\n\033[0m");\
						exit(1)
#define Ver(str) printf("\033[35m"\
						str  	  \
						"\n\033[0m")
#define DBG(str) printf("\033[31m" \
						str 	   \
						"\n\033[0m")
#define __TODO__ void
void TODO(char *s)
{
	if( s ) printf("\033[31mTODO: %s\033[0m", s);
	else printf("\033[31mTODO: IMPLEMENT HERE%s\033[0m", s);
}
bool IsVer = false;// Is verbosity
i32 tmp_i32;
i32 hit_cnt = 0;
i32 mis_cnt = 0;
i32 evi_cnt = 0;
typedef struct{
	i32 lru;
	i32 tag;
	bool inuse;
}Cache;
Cache **cache_listp;
enum {HIT,MISS,EVICT};
char *tf = NULL;// trace file
void parseArgs(i32 argc, char **argv);
i32 accessCache(u64 addr);
/*
Usage: ./csim-ref [-hv] -s <s> -E <E> -b <b> -t <tracefile>
• -h: Optional help flag that prints usage info
• -v: Optional verbose flag that displays trace info
• -s <s>: Number of set index bits (S = 2s
is the number of sets)
• -E <E>: Associativity (number of lines per set)
• -b <b>: Number of block bits (B = 2b
is the block size)
• -t <tracefile>: Name of the valgrind trace to replay

*/
void Usage(char *arg){
	printf("\033[34mUsage: %s [-hv] -s <s> -E <E> -b <b> -t <tracefile>\033[m\n",arg);
	puts("-h: Optional help flag that prints usage info\n"
		 "-v: Optional verbose flag that displays trace info\n"
		 "-s <s>: Number of set index bits (S = 2^s is the number of sets)\n"
		 "-E <E>: Associativity (number of lines per set)\n"
		 "-b <b>: Number of block bits (B = 2^b is the block size)\n"
		 "-t <tracefile>: Name of the valgrind trace to replay");
	puts("Example: User> ./csim -s 1 -E 1 -b 1 -t traces/yi.trace");
}
void initCache();
void freeCache();
void verbose(){};
void exec();
void t(){}
/*
 * S represent number of set (2^s)
 * E represent number of block per set
 * b represent number of bytes per block
*/
i32 s,S,E,b;
i32 main(i32 argc, char *argv[])
{
	parseArgs(argc,argv);
	initCache();
	exec();
	printSummary(hit_cnt,mis_cnt,evi_cnt);
	freeCache();
    return 0;
}
void printVer(i32 sta,char op ,u64 addr ,i32 len){
	if(IsVer){
		switch(sta)
		{
			case HIT:
				printf("\033[35m%c %llx,%d hit\n\033[0m",op,addr,len);
				break;
			case MISS:
				printf("\033[35m%c %llx,%d miss\n\033[0m",op,addr,len);
				break;
			case EVICT:
				printf("\033[35m%c %llx,%d miss and evict\n\033[0m",op,addr,len);
				break;
			default:
				Err("Shouldn't reach here");
		}
	}
}
void exec(){
	char buf[MAXLINE];
	FILE *trace_fp = fopen(tf,"r");
	if(!trace_fp){
		Err("file open failed!");
	}
	while( fgets(buf,sizeof(buf),trace_fp) != NULL ){
		u64 addr;
		u32 len;
		i32 idx = 0;
		char op;
		// char tmpbuf[MAXLINE];
		while(buf[idx] == ' ') idx++;
		if( buf[idx] == 'L' || buf[idx] == 'S' || buf[idx] == 'M')
		{
			sscanf(&buf[idx],"%c %llx,%u",&op,&addr,&len);
			switch(op)
			{
				case 'L':
					tmp_i32 = accessCache(addr);
					printVer(tmp_i32,op,addr,len);
					break;
				case 'M':
					// Don't need break, M = S + L, i.e hit++
					tmp_i32 = accessCache(addr);
					printVer(tmp_i32,op,addr,len);
				case 'S':
					tmp_i32 = accessCache(addr);
					printVer(tmp_i32,op,addr,len);
					break;
				default:
					Err("Shouldn't reach here");
			}
		}
	}
}
/*
 * cache -> | tag | set_idx | blk_addr |
 */
i32 accessCache(u64 addr){
	u64 tag = addr >> (s + b);// get  tag
	i32 set_idx = (addr >> b) & ((1 << s) - 1);// get set number according to addr
	i32 evict = 0;// evict idx inside set
	i32 ept = -1;
	for (i32 i = 0; i < E ; i++)
	{
		/*
		 * hit condition then:
		 * 		update lru
		 * 		output msg "hit"
		 */
		if (cache_listp[set_idx][i].inuse)
		{
			if (cache_listp[set_idx][i].tag == tag)
			{
				// hit
				hit_cnt++;
				cache_listp[set_idx][i].lru = 1;
				// modify other cache's lru and return
				for(int j = i + 1; j < E ;j ++){
					cache_listp[set_idx][j].lru++;
				}
				return HIT;
			}

			cache_listp[set_idx][i].lru++;

			if(cache_listp[set_idx][i].lru 
				>= cache_listp[set_idx][evict].lru)
			{
				evict = i;
			}
		}
		else{
			ept = i;
		}
	}
	/* 
	 * miss condition
	 * if cache set empty, then put it in
	 * else eviction lru cache
	 */
	mis_cnt++;
	// ept dont equal to 1 means that have empty but dont find match cache
	if(ept != -1)
	{
		assert(set_idx >= 0 && ept >= 0);
		cache_listp[set_idx][ept].lru = 1;
		cache_listp[set_idx][ept].inuse = true;
		cache_listp[set_idx][ept].tag = tag;
		return MISS;
	}
	// full and miss , need to execute evict
	else
	{
		assert(set_idx >= 0 && evict >= 0);
		cache_listp[set_idx][evict].lru = 1;
		// cache_listp[set_idx][ept].inuse = true;
		cache_listp[set_idx][evict].tag = tag;
		evi_cnt++;
		return EVICT;
	}
}
void initCache(){
	cache_listp = (Cache **)malloc(sizeof(Cache *)*S);
	for (i32 _ = 0; _ < S ; _++)
		cache_listp[_] = (Cache *)malloc(sizeof(Cache)*E);
	// S*E block cache
	for( i32 _ = 0; _ < S; _++ )
	{
		for ( i32 __ = 0; __ < E; __++)
		{
			cache_listp[_][__].lru = 0;
			cache_listp[_][__].tag = 0;
			cache_listp[_][__].inuse = false;
		}
	}
}
void freeCache(){
	for( i32 _ = 0; _ < S ; _++)
	{	
		// printf("\033[31mDEBUG[+]:%d\n\033[0m",_);
		// printf("\033[31mDEBUG[+]:%p\n\033[0m",cache_listp[_]);
		free(cache_listp[_]);
	}
	free(cache_listp);
}
void parseArgs(i32 argc, char **argv){
    // printSummary(0, 0, 0);
	char opt;

	while( (opt = getopt(argc, argv, "hvs:E:b:t:")) != -1 )
	{
		// h and v don't have postposition argument
		switch(opt)
		{
			case 'h':
				Usage(argv[0]);
				exit(0);
			case 'v':
				IsVer = 1;
				break;
			case 's':
				s = atoi(optarg);
				S = pow(2,s);
				break;
			case 'E':
				E = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
				// B = pow(2,tmp_i32);
				break;
			case 't':
				tf = optarg;
				break;
			default:
				Usage(argv[0]);
				exit(0);
		}
	}
	if( !s || !S || !E || !b || !tf )
	{
		Usage(argv[0]);
		Err("Invalid arguments");
	}
}