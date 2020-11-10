

import os
import shutil
mainCtrlCod='''
        char alog[1024];
		sprintf_s(alog, "=============NbRec:%d======NbPGM:%d=========", Config->getNbRec(),Config->getNbPGM());
		SHOWMEMORYINFOA(alog);
'''
def makeMainCtrl(fpath,startFunc):
    bStart=False
    count=0
    markCOunt=0
    bakFile=fpath+'bak'
    shutil.move(fpath,bakFile)

    fdest=open(fpath,'w')    
    for line in open(bakFile):
        if not bStart :
            if line.find(startFunc) >0:
                bStart=True
            fdest.write(line)
            continue

        if markCOunt==0 and line.find('{')>=0:
            line+=mainCtrlCod
            markCOunt+=1
        
        fdest.write(line)
    
    fdest.close()
    os.remove(bakFile)



memcacheCode='''
    //concurrency::parallel_invoke(
	//	[&] { m_frameFullSize.initialize(nbMaxFrame, m_closeToLiveMaxSize, defaultFrameSize, m_nbMaxTGAGroup); },
	//	[&] { m_frame480.initialize(m_nbMaxFrame480, m_closeToLiveMaxSize480, defaultFrameSize480); },
	//	[&] { m_matrixAudio.initialize(m_nbMaxMatrixAudio, m_closeToLiveMaxSize, defaultAudioSize); },
	//	[&] { m_pcmAudio.initialize(m_nbMaxPCMAudio, m_closeToLiveMaxSizePCMAudio, defaultAudioSize); }
	//);
SHOWMEMORYINFO;

char log[1024];
sprintf_s(log, "full-nbMaxFrame=%d, m_closeToLiveMaxSize=%d, defaultFrameSize=%d, m_nbMaxTGAGroup=%d", nbMaxFrame, m_closeToLiveMaxSize, defaultFrameSize, m_nbMaxTGAGroup);
SHOWMEMORYINFOA_force(log);

sprintf_s(log, "m_frame480-m_nbMaxFrame480=%d, m_closeToLiveMaxSize480=%d, defaultFrameSize480=%d", m_nbMaxFrame480, m_closeToLiveMaxSize480, defaultFrameSize480);
SHOWMEMORYINFOA_force(log);

sprintf_s(log, "m_matrixAudio-m_nbMaxMatrixAudio=%d, m_closeToLiveMaxSize=%d, defaultAudioSize=%d", m_nbMaxMatrixAudio, m_closeToLiveMaxSize, defaultAudioSize);
SHOWMEMORYINFOA_force(log);

sprintf_s(log, "m_pcmAudio-m_nbMaxPCMAudio=%d, m_closeToLiveMaxSizePCMAudio=%d, defaultAudioSize=%d", m_nbMaxPCMAudio, m_closeToLiveMaxSizePCMAudio, defaultAudioSize);
SHOWMEMORYINFOA_force(log);

	 m_frameFullSize.initialize(nbMaxFrame, m_closeToLiveMaxSize, defaultFrameSize, m_nbMaxTGAGroup); SHOWMEMORYINFOA("after m_frameFullSize");
	 m_frame480.initialize(m_nbMaxFrame480, m_closeToLiveMaxSize480, defaultFrameSize480); SHOWMEMORYINFOA("after m_frame480");
	 m_matrixAudio.initialize(m_nbMaxMatrixAudio, m_closeToLiveMaxSize, defaultAudioSize); SHOWMEMORYINFOA("after m_matrixAudio");
	 m_pcmAudio.initialize(m_nbMaxPCMAudio, m_closeToLiveMaxSizePCMAudio, defaultAudioSize); SHOWMEMORYINFOA("after m_pcmAudio");
	
'''


def makeMemcache(fpath,startFunc='concurrency::parallel_invoke'):
    bStart=False
    count=0
    markCOunt=0
    bakFile=fpath+'bak'
    shutil.move(fpath,bakFile)

    fdest=open(fpath,'w')    
    for line in open(bakFile):
        if not bStart :
            if line.find(startFunc) >0:
                bStart=True
                line = memcacheCode
            fdest.write(line)
            continue

        if bStart and markCOunt<6:
            markCOunt+=1
            continue
        
        fdest.write(line)
    
    fdest.close()
    os.remove(bakFile)

def makeCode(fpath,startFunc):
    bStart=False
    count=0
    markCOunt=0
    bakFile=fpath+'bak'
    shutil.move(fpath,bakFile)
    fdest=open(fpath,'w')
    ifStart=False
    for line in open(bakFile):
        if not bStart :
            if startFunc in line:
                bStart=True
            fdest.write(line)
            continue
        if 'if' in line:
            ifStart=True

        if '{' in line:
            markCOunt+=1
            ifStart=False
        if '}' in line:
            markCOunt-=1

        if line.strip().endswith(';'):
            if not ifStart:
                line = line[:-1]+ 'SHOWMEMORYINFOA(R"('+line[:-1].strip()+')");\n'
            ifStart = False
        fdest.write(line)
        count+=1
        if markCOunt==0:
            bStart=False    
    fdest.close()
    os.remove(bakFile)
    print(markCOunt)


baseDir =r'E:\WorkSpace\backend\\' 

flist=[
    (r'frameToTCP\gestTrain.cpp','GestTrain::start()'),
    (r'Lib.Core\frameCache.cpp','int FrameCache::start('),
]

dirname = os.getcwd()
os.chdir(baseDir)
os.system(r'git restore Lib.Memcache\Memcache.cpp')
os.system(r'git restore Lib.Logger\LogWriter.h')
os.system(r'git restore Lib.Logger\LogWriter.cpp')
os.system(r'git restore inc\profiler_config.h')
os.system(r'git restore frameToTCP\MainCtrl.h')

os.system(r'git restore Lib.Core\frameCache.cpp')
os.system(r'git restore frameToTCP/gestTrain.cpp')

for a in flist:
    os.system(r'git restore {}'.format(a[0]))



os.chdir(dirname)

mainCtrlH=baseDir+r'frameToTCP\MainCtrl.h'
markFunc='void Start(bool _showDebugScreen)'
makeMainCtrl(mainCtrlH,markFunc)

makeMemcache(baseDir+r'Lib.Memcache\Memcache.cpp')

log_H=baseDir+r'Lib.Logger\LogWriter.h'
log_cpp=baseDir+r'Lib.Logger\LogWriter.cpp'
config_h=baseDir+r'inc\profiler_config.h'
    
shutil.copyfile('profiler_config.h',config_h)
shutil.copyfile('LogWriter.h',log_H)
shutil.copyfile('LogWriter.cpp',log_cpp)


for a in flist:    
    makeCode(baseDir+a[0],a[1])
