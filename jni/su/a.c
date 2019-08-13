#include <stdio.h>
#include <sys/ptrace.h>
#include <errno.h>
#include <memory.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>
/*
bionic/libc/bionic/system_properties.c

struct prop_info {
    unsigned volatile serial;
    char value[PROP_VALUE_MAX];
    char name[0];
};
*/

long find_match_pos(char* mapbuf,long maplen,char* keyname)
{
	long key_len=strlen(keyname);
	long i=0;
	
	
	for(i=0;i<(maplen-key_len);i++)
	{
		//find first 4 char match
		if( *(unsigned int *)keyname != *(unsigned int *)&mapbuf[i] )
		{
			i+= (sizeof(unsigned int)-1);
			continue;
		}
		
		//
		if( memcmp(&mapbuf[i] , keyname,key_len) )
		{
			
			continue;
		};
		
		if( mapbuf[i+key_len] == 0 )
		{
			
			/*
			find ok
			*/
			printf("find at %d\n",i);
			return i;
		}
		
		
	}

	return -1;
}



int set_new_val(unsigned long maps,unsigned long mape,char* keyname,char* value)
{
	 char *buffer;
	 int valuelen = strlen(value) ;
	int bret=0;
	        int rc;
			unsigned long         addr,test;
	long memlen=mape-maps;
	if(valuelen>=92||valuelen<=0 )
		return 0;
        buffer = (char *) malloc(memlen + 16);
        if (!buffer) {
                perror("malloc");
                return 1;
        }
        rc = ptrace(PTRACE_ATTACH, 1, 0, 0);
        if (rc < 0) {
                perror("ptrace");
                return rc;
        }
		//copy memory
        for (addr = maps; addr < mape; addr += 4) {
                test = ptrace(PTRACE_PEEKTEXT, 1, (void *) addr, 0);
                *((unsigned long *)(buffer + addr - maps)) = test;
        }
		long i=0;
		char line_text[17]={0};
		for(i=0;i<memlen;i++)
		{
			char data=buffer[i];
			if(!(i%16))
			{
				printf("0x%08x ",i);
			}
			printf("%02x ",data&0xff);
			if(!isprint(data))
			{
				data=46;
			}
			line_text[i%16]=data;
			if( (i%16) ==15 )
			{
				printf(" %s\n",line_text);
			}

		}
		long pos=find_match_pos(buffer,memlen,keyname);

		if(pos>92)
		{
			
			printf("find index at %d  %s \n",pos,&buffer[pos]);
			pos-=92;
			printf("value is   %s \n",&buffer[pos]);
			
			/*
			modify data
			*/
			int cptimes=(valuelen +3)/ 4;
			char * tmp=(char *) malloc( cptimes *4 );
			printf("__ %d ",cptimes);
			if(tmp)
			{
				strcpy(tmp,value);
				printf("__ %s  ",tmp);
				for(int i=0;i<cptimes;i++)
				{
					int v =ptrace(PTRACE_POKETEXT, 1, (void*)((maps + pos)+i*4),*(unsigned int *)(tmp+i*4));
					
					printf("_val_ %d  ",v);
					bret=1;
				}
				free(tmp);
			}
			
			
		}
		

        free(buffer);
        ptrace(PTRACE_DETACH, 1, 0, 0);
        return bret;
}

int main(int argc, char **argv) {

        unsigned long maps, mape, test, fake;
        FILE *fp;
        char line[512]={0};
       
		
		
		if(argc<=2)
		{
			printf("Usage:\n    modprop keyname value");
			return 1;
		}
		printf("NEW VERSION\n");
		char* keyname=argv[1];
		char* value=argv[2];
		
		int keylen=strlen(keyname);
		if(keylen >=64 || keylen<= 4)
		{
			printf("len too long (keyname or value)");
			return 1;
		}

        fp = fopen("/proc/1/maps", "r");
        if (!fp) 
		{
                perror("fopen");
                return 1;
        }
		
		char realline[1024]={0};
		while(fgets(line, sizeof(line), fp))
		{
			if(strstr(line,"/dev/__properties__"))
			{
				strcpy(realline,line);
				char* pos=realline;
				while(*pos!='-')
					pos++;
				if(*pos=='-')
				{
					*pos=0;
					pos++;
				}
				maps=strtoll(realline,NULL,16);
				mape=strtoll(pos,NULL,16);
				printf("%s == %s \n",realline,pos);
				printf("from %llx to %llx %d -- %d \n",mape,maps,sizeof(long),sizeof(unsigned long));
				if( set_new_val(maps,mape,keyname,value))
					break;;
			}
		}

        fclose(fp);

		return 0;
}
