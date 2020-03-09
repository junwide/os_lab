#include "cachelab.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#define true 1
#define false 0

typedef  char bool;
bool v_mode = false;
int s = 0 , E = 0, b = 0;
char *file="";
unsigned long times = 0;
unsigned hit_count = 0, miss_count = 0, eviction_count = 0; //center

typedef struct {
    bool valid;
    // bool  dirty;
    unsigned  long  tag;
    //data
    unsigned int last_use;

}Line ;


void getOpt(bool *v_mode, int *s, int *E, int *b, char **file, int argc, char **argv);
void access(unsigned long adder);

Line **cache = NULL;

int main(int argc, char **argv)
{
    //get option
    getOpt(&v_mode, &s, &E, &b, &file, argc, argv);
    // open the trace file
    FILE *trace_file = fopen(file, "r");
    if (trace_file)
    {
        int S = 1<<s;
        cache = malloc(sizeof(Line *) * S);
        if(cache)
        {
            for (size_t i = 0; i < S; i++)
            {
                cache[i] = malloc(sizeof(Line) * E);
                if (!cache) exit(EXIT_FAILURE);

            }
            for (size_t i = 0; i < S; i++)
            {
                for (size_t j = 0; j < E; j++)
                {
                    cache[i][j].last_use = 0;
                    cache[i][j].tag = 0;
                    cache[i][j].valid = 0;
                    //data
                }
            }
            unsigned long addr;
            char type;
            int m_size;
            char buffer[80];
            while (fgets(buffer, 80, trace_file))
            {
                sscanf(buffer, " %c %lx,%d", &type, &addr, &m_size);
                if(type == 'L' || type == 'S') access(addr);
                else if(type == 'M')
                {
                    access(addr);
                    access(addr);
                }
            }
            //free
            for (size_t i = 0; i < S; i++)
            {
                free(cache[i]);
                cache[i] = NULL;
            }
            free(cache);
            cache = NULL;

            // close the trace file
            fclose(trace_file);

            printSummary(hit_count, miss_count, eviction_count);
            return 0;
            }
           printSummary(hit_count, miss_count, eviction_count);
        }
        exit(EXIT_FAILURE);
        return 0;
}

void access(unsigned long adder)
{
    int group = (adder>> b)&((1 << s) - 1) ; //adder >> b get the center
    int tag = adder >> (b + s);
    // int bias = adder & ((1 << b) - 1);
    for (size_t i = 0; i < E; i++)
    {
        if (cache[group][i].tag == tag && cache[group][i].valid == 1) //hint
        {
            hit_count++;
            cache[group][i].last_use = ++times;
            return;
        }
        else if(cache[group][i].valid == 0) //miss and empty
        {
            cache[group][i].valid = 1;
            cache[group][i].last_use = ++times;
            cache[group][i].tag = tag;
            miss_count++;
            return;
        }
    }
    eviction_count++;
    miss_count++;
    unsigned long lru_time = -1UL;
    unsigned line_index=0;

    //LRU
    for (size_t i = 0; i < E; i++)
    {
        if (cache[group][i].last_use < lru_time)
        {
            lru_time=cache[group][i].last_use;
            line_index=i;
        }
    }

    cache[group][line_index].last_use = ++times;
    cache[group][line_index].tag=tag;

    printf("test\n");

}

void getOpt(bool *v_mode, int *s, int *E, int *b, char **file, int argc, char **argv)
{
    char opt;
    while ((opt = getopt(argc, argv, "v::s:E:b:t:ma")) != -1)
    {
        switch (opt)
        {
            case 'v':
                *v_mode = true;
                break;
            case 's':
                *s = atoi(optarg);
                break;
            case 'E':
                *E = atoi(optarg);
                break;
            case 'b':
                *b = atoi(optarg);
                break;
            case 't':
                *file = optarg;
                break;
            case '?':

                fprintf(stderr, "Usage: %s [-hv] -s <num> -E <num> -b <num> -t <file>\n\
Options:\n\
  -h         Print this help message.\n\
  -v         Optional verbose flag.\n\
  -s <num>   Number of set index bits.\n\
  -E <num>   Number of lines per set.\n\
  -b <num>   Number of block offset bits.\n\
  -t <file>  Trace file.\n\
\n\
Examples:\n\
  linux>  %s -s 4 -E 1 -b 4 -t traces/yi.trace\n\
  linux>  %s -v -s 8 -E 2 -b 4 -t traces/yi.trace\n",
                        argv[0], argv[0], argv[0]);
                exit(EXIT_FAILURE);

            default:
                break;
        }
    }
}