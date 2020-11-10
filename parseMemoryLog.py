
count=0
dataDict={}
dlist=[]
dlastList={}
for line in open(r'C:\Logs\frame2TCP\memory.log'):
    if line[0]=='#':
        dataDict[count]=dlist
        count+=1
        dlist={}
        dlastList=[]
        dlist["cams"]=count
        continue
    
    v = line.split(',')
    v2 = {}
    
    if v[-2][0]=='=':
        v2['start main()'] = v
        dlastList.append('start main()')
        dlist['start main()']=v
    else:
        v2[v[-2]]= v
        dlastList.append(v[-2])
        dlist[v[-2]]=v
    
dataDict[count]=dlist    
count+=1


f=open('out.csv','w')
print(dlastList)
for a in dlastList:    
    for i in range(1,count):
        if a in dataDict[i]:
            print("{}".format(dataDict[i][a][1]),end=',',file=f)
        else:
            print('        ',end=',',file=f)
    print(a+",",file=f)

for i in range(8+1):
    print("=sum({0}1:{0}{1})".format(chr(97+i),len(dlastList)),end=',',file=f)
print("totalMem,",file=f)

f.close()