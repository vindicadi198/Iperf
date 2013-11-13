import sys
import matplotlib.pyplot as plt

if len(sys.argv)<2:
	print 'command requires argument'
	exit()

opt=sys.argv[1]
opt = opt[1:]
opt=int(opt)

if opt > 12 or opt < 0 :
	print 'give valid options in range [0,12]'
	exit()
	
x=[]
y=[]
a=[]

i=-1
readfile = open('rttgraph.txt','r')

for line in readfile:
	x.append(int(line))
	
x=x[2:]
	
readfile.close()

logfile = open('log','r')

for line in logfile:
	i = i+1
	if i!=0 and i!=1 and i!=2 and i!=3 :
		a =line.split(' ')
		y.append(int(a[opt]))

plt.plot(x,y)
plt.title('RTT vs TIME')
plt.xlabel('rtt')
plt.xlabel('time')

plt.show()
