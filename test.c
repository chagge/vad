#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "wb_vad.h"

int main(int argc,char* argv[])   
{      
	if ( argc != 3 ){
		printf("usage:./vad in-file out-file\n");
		return -1;
	}

	char* out_file = argv[2];
	int rc = 1;
	int size;
	char *buffer;
	int i;
	int frames = FRAME_LEN;

	/*for vad*/
	float indata[FRAME_LEN];   
	short outdata[FRAME_LEN];   
	VadVars *vadstate;                     
	char name[128] = {0}; 
	int temp, vad;
	int recording = 0;
	int count = 0;
	FILE *fp = NULL;
	FILE *fp_all = NULL;
	/*XXX:2 bytes/sample, 2 channels*/
	size = frames * 4; 
	buffer = (char*)malloc(size);

	/*vad init*/
	wb_vad_init(&(vadstate));
	sprintf(name, "%s.pcm", out_file);
	fp_all = fopen(name, "wb");

	int in_fd = open(argv[1], O_RDONLY, 0);
	/*wav header 44 byte*/
	lseek(in_fd, 4, SEEK_SET);

	while (rc > 0) {
		rc = read(in_fd, buffer, frames);

		for(i = 0; i< frames; i++) {
			indata[i]=0;
			temp = 0;
			memcpy(&temp,buffer+4*i,2);
			indata[i]=(float)temp;
			outdata[i]=temp;
			if (indata[i] > 65535/2)
				indata[i] = indata[i]-65536;
		}
		vad = wb_vad(vadstate, indata);
		/*check the vad flag, 1: noise, other: silent */
		if(vad == 1)
			recording = 1;
		else
			recording = 0;
		if (recording ==1 && fp == NULL) {
			sprintf(name,"%s.%d.pcm", out_file, count);
			fp=fopen(name, "wb");   
		}
		if (recording == 0 && fp != NULL) { 
			fclose(fp);
			fp = NULL;
			count++;
		}
		if (fp != NULL && recording == 1)
			fwrite(outdata, 2, FRAME_LEN, fp);
		if (fp_all != NULL)
			fwrite(outdata, 2, FRAME_LEN, fp_all);
	}
	close(in_fd);
	fcloseall();
}
