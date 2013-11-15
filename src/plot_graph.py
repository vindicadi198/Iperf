import sys
import matplotlib.pyplot as plt

def throughput():
	y=[]
	print 'hello'
	readfile = open('throughput.txt','r')
	
	for line in readfile:
		line=line.strip()
		y.append(float(line))
	
	plt.plot(y)	
	plt.ylabel('Throughput')
	plt.show()
	
def tcp_log():
	print '1) State'
	print '2) LastDataSent'
	print '3) LastDataRecv'
	print '4) SNDCWND'
	print '5) SNDSTHRESH'
	print '6) RCVSTHRESH'
	print '7) RTT'
	print '8) RTTVAR'
	print '9) UNACK'
	print '10) SACKED'
	print '11) LOST'
	print '12) RETRANS'
	print '13) FACKS'
	j=raw_input('>')
	j=int(j)
	x=[]
	y=[]
	a=[]

	i=-1
	readfile = open('rttgraph.txt','r')

	for line in readfile:
		line=line.strip()
		x.append(float(line))
	
	x=x[0:]
	print 'size of x '+str(len(x))
	readfile.close()
	
	logfile = open('log','r')
	
	for line in logfile:
		i = i+1
		if i!=0 :
			a =line.split()
			y.append(int(a[j-1]))
	print 'size of y '+str(len(y))		
	plt.plot(x,y)
	if j==1 :
		plt.title('STATE vs TIME')
		plt.xlabel('state')
	if j==2 :
		plt.title('LastDataSent vs TIME')
		plt.xlabel('LastDataSent')
	if j==3 :
		plt.title('LastDataRecv vs TIME')
		plt.xlabel('LastDataRecv')		
	if j==4 :
		plt.title('SNDCWND vs TIME')
		plt.xlabel('SNDCWND')
	if j==5 :
		plt.title('SNDSTHRESH vs TIME')
		plt.xlabel('SNDSTHRESH')
	if j==6 :
		plt.title('RCVSTHRESH vs TIME')
		plt.xlabel('RCVSTHRESH')
	if j==7 :
		plt.title('RTT vs TIME')
		plt.xlabel('RTT')
	if j==8 :
		plt.title('RTTVAR vs TIME')
		plt.xlabel('rttvar')
	if j==9 :
		plt.title('UNACK vs TIME')
		plt.xlabel('UNACK')
	if j==10 :
		plt.title('SACKED vs TIME')
		plt.xlabel('SACKED')
	if j==11 :
		plt.title('LOST vs TIME')
		plt.xlabel('LOST')
	if j==12 :
		plt.title('RETRANS vs TIME')
		plt.xlabel('RETRANS')
	if j==13 :
		plt.title('FACKS vs TIME')
		plt.xlabel('FACKS')				
		
				
	plt.xlabel('TIME')
	plt.show()		




def main():
	print '1) throughput'
	print '2) tcp log'
	k=raw_input('>')
	print 'k is'+k
	if int(k)==1:
		throughput()
	else:
		tcp_log()

if __name__ == '__main__':
	main()
