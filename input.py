#===================================================================
#    FileName: play_audio.py
#      Author: Yin Mingming
#       Email: ymingming@gmail.com
#     WebSite: http://www.????.com
#  CreateTime: 2010.01.01
#===================================================================
import time
import numpy as np
import pyaudio
from Queue import Queue
import guiqwt.pyplot as plt 

def main(fs=96000,tw=1.0):
    frames = 65536 
    ns = int(float(fs)*float(tw))

    idx = ns/frames+1
    queue = Queue(idx)

    pa = pyaudio.PyAudio()
    stream = pa.open(format=pyaudio.paFloat32,channels = 1,rate =int(fs),input = True,frames_per_buffer=frames)

    print 'total:',idx
    fp = open('wave.bin','wb')
    for k in xrange(idx):
        data = stream.read(frames)
        fp.write(data)
        #audio_data = np.fromstring(data,dtype=np.float32)
        #queue.put(audio_data)
        print k
        
    stream.close() 
    pa.terminate()
    fp.close() 

if __name__ == '__main__':
    main(fs=192000,tw=10.0)
