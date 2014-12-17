#import guiqwt.pyplot as plt
import matplotlib.pyplot as plt
import numpy as np

fs = 32000 

fp=open('1414348409.32000.S32','rb')
buf=fp.read()

wave = np.fromstring(buf,dtype=np.int32)

wave -= wave.mean()

wave1 = wave[::2]
wave1 -= wave1.mean()
wave1 = wave1[4096:4096*2]
t1 =np.arange(len(wave1))/float(fs)

wave2 = wave[1::2]
t2 =np.arange(len(wave2))/float(fs)
wave2 -= wave2.mean()
wave2 = wave2[4096:4096*2]

plt.figure()
plt.subplot(211)
plt.plot(t1,wave1)
plt.subplot(212)
plt.psd(wave1,Fs=fs,NFFT=4096)

plt.figure()
plt.subplot(211)
plt.plot(wave2)
plt.subplot(212)
plt.psd(wave2,Fs=fs,NFFT=4096)

plt.show()


