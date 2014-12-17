#include <alsa/asoundlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>
#include <zmq.h>
#include "ltpro.h"

#define _FRAMES_ (8192)
#define _FRAMES_2 (2*_FRAMES_)

unsigned microseconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec*1000000 + ts.tv_nsec/1000;
}

int get_width()
{
	int width;
	char buffer[80] = {0};
	FILE *fp;
	fp=popen("stty size | cut -d\" \" -f2","r");
	fgets(buffer,sizeof(buffer),fp);
	pclose(fp);
	width = atoi(buffer);
	return width;
}

int main(int argc,char* argv[])
{
    unsigned int output = 0;
    char zmq_port[1024];

	progress_t bar;
	progress_init(&bar,"",get_width()-7,PROGRESS_BGC_STYLE);

    if(argc < 2)
    {
        printf("usage:\n%s fs tw [ZMQ_PUB_PORT]\n",argv[0]);
        return 0;
    }

    unsigned int fs = atoi(argv[1]);
    float tw = atof(argv[2]);
    if(tw < 0.0)
    {
        printf("argv[1] < 0.0, error!\n");
        return -1;
    }

    if(argc >3)
    {
        output=1;
        sprintf(zmq_port,"tcp://*:%s",argv[3]);
    }

    int buf_s32[_FRAMES_2];

    unsigned int ns = (unsigned int)((float)fs*tw);
    unsigned int idx = ns/_FRAMES_;
    if(ns%_FRAMES_ > 0) idx += 1;

    void *ctx = NULL;
    void *socket = NULL;
    zmq_msg_t msg;

    if(output==1)
    {
        ctx = zmq_init(1);
        if(ctx==NULL)return -1;
        socket = zmq_socket(ctx,ZMQ_PUB);
        if(socket==NULL)return -1;
        zmq_bind(socket,zmq_port);
        printf("%s\n",zmq_port);
    }

    FILE *fp = NULL;
    if(output==0)
    {
        char ofp[4096];
        sprintf(ofp,"%d_%d.int32",time(NULL),fs);

        fp = fopen(ofp,"wb");
        if(fp==NULL)
        {
            printf("open %s failed.\n");
            return -1;
        }
        else
        {
            printf("%s\n",ofp);
        }
    }

    snd_pcm_t *handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames = _FRAMES_;
    int i,rc,dir;
    unsigned k; 
    unsigned frame_size_byte = sizeof(buf_s32);
    float frame_t = (float)frames/(float)fs;

    rc = snd_pcm_open(&handle,"default",SND_PCM_STREAM_CAPTURE,0);
    if(rc < 0)
    {
        printf("unable to open pcm device:%s\n",snd_strerror(rc));
        goto EXIT;
    }

    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(handle,params);
    snd_pcm_hw_params_set_access(handle,params,SND_PCM_ACCESS_RW_INTERLEAVED);
    //    snd_pcm_hw_params_set_format(handle,params,SND_PCM_FORMAT_FLOAT);
    snd_pcm_hw_params_set_format(handle,params,SND_PCM_FORMAT_S32_LE);
    snd_pcm_hw_params_set_channels(handle,params,2);
    snd_pcm_hw_params_set_rate_near(handle,params,&fs,0);

    //snd_pcm_hw_params_set_period_size_near(handle,params,&frames,&dir);

    rc = snd_pcm_hw_params(handle,params);
    if(rc < 0)
    {
        printf(" unable to set hw params:%s\n",snd_strerror(rc));
        goto EXIT;
    }

    snd_pcm_hw_params_free(params);

    rc= snd_pcm_prepare(handle);
    if(rc < 0)
    {
        printf(" unable to prepare audio to use:%s\n",snd_strerror(rc));
        goto EXIT;
    }

    for(k=0;k<idx;k++)
    {

        if(rc = snd_pcm_readi(handle,buf_s32,frames) != frames)
        {
            printf(" unable to read:%s\n");
            goto EXIT;
        }
        else
        {
            if(output==0)
            {
                fwrite(buf_s32,2*sizeof(int),frames,fp);
            }
            if(output==1)
            {
                rc = zmq_msg_init_size(&msg,frame_size_byte);
                memcpy(zmq_msg_data(&msg),buf_s32,frame_size_byte);
                zmq_send(socket,&msg,0);
                zmq_msg_close(&msg);
            }
//            printf("%8.1fs/%8.1fs\n",(k+1)*frame_t,tw);
			progress_show(&bar,(float)(k+1.0f)/(float)idx);
        }
    }
	printf("\n");
EXIT:
    snd_pcm_close(handle);
    if(output==0)
    {
        if(fp)fclose(fp);
    }
    if(output==1)
    {
        zmq_close(socket);
        zmq_term(ctx);
    }
	progress_destroy(&bar);
    exit(0);
}
