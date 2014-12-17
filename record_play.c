#include <alsa/asoundlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <math.h>
#include <time.h>

#define _FRAMES_ (16384)
#define _FRAMES_2 (2*_FRAMES_)

unsigned microseconds(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    return ts.tv_sec*1000000 + ts.tv_nsec/1000;
}

int main(int argc,char* argv[])
{
	if(argc < 3)
	{
		printf("usage:\n\t%s [fs] [tw]\n",argv[0]);
		return 0;
	}

    unsigned int fs = atoi(argv[1]);
	float tw = atof(argv[2]);
	
	if(tw < 0.0)
	{
		printf("argv[1] < 0.0, error!\n");
		return -1;
	}

    unsigned int ns = (unsigned int)((float)fs*tw);
	unsigned int idx = ns/_FRAMES_+1;

    int buf_s32[_FRAMES_2];

    snd_pcm_t *record_handle, *play_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames = _FRAMES_;
    int i,rc,dir;
    unsigned k; 
	float frame_t = (float)frames/(float)fs;

    rc = snd_pcm_open(&record_handle,"default",SND_PCM_STREAM_CAPTURE,0);
    if(rc < 0)
    {
        printf("unable to open pcm device:%s\n",snd_strerror(rc));
        goto EXIT;
    }

    rc = snd_pcm_open(&play_handle,"default",SND_PCM_STREAM_PLAYBACK,0);
    if(rc < 0)
    {
        printf("unable to open pcm device:%s\n",snd_strerror(rc));
        goto EXIT;
    }

    snd_pcm_hw_params_malloc(&params);
    snd_pcm_hw_params_any(record_handle,params);
    snd_pcm_hw_params_set_access(record_handle,params,SND_PCM_ACCESS_RW_INTERLEAVED);
//    snd_pcm_hw_params_set_format(record_handle,params,SND_PCM_FORMAT_FLOAT);
    snd_pcm_hw_params_set_format(record_handle,params,SND_PCM_FORMAT_S32_LE);
    snd_pcm_hw_params_set_channels(record_handle,params,2);
    snd_pcm_hw_params_set_rate_near(record_handle,params,&fs,0);

    //snd_pcm_hw_params_set_period_size_near(record_handle,params,&frames,&dir);

    rc = snd_pcm_hw_params(record_handle,params);
    if(rc < 0)
    {
        printf(" unable to set hw params:%s\n",snd_strerror(rc));
        goto EXIT;
    }
    
    snd_pcm_hw_params_free(params);
    
    rc= snd_pcm_prepare(record_handle);
    if(rc < 0)
    {
        printf(" unable to prepare audio to use:%s\n",snd_strerror(rc));
        goto EXIT;
    }

    for(k=0;k<idx;k++)
    {
	   	if(rc = snd_pcm_readi(record_handle,buf_s32,frames) != frames)
        {
			printf(" unable to read:%s\n");
			goto EXIT;
        }
		else
		{
			if(-EPIPE == snd_pcm_writei(play_handle,buf_s32,frames))
			{
				printf("underrun\n");
				rc = snd_pcm_prepare(play_handle);
				if(rc < 0)
				{
					printf(" unable to prepare audio to use:%s\n",snd_strerror(rc));
				}
			}
		}
   }
EXIT:
    snd_pcm_close(record_handle);
    snd_pcm_close(play_handle);
    exit(0);
}
