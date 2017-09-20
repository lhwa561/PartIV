import time
import robot_puppet.py


def readData():
	List[0.0,0.0,0.0,0.0,0.0,0.0]
	try:
		filename = '/home/pi/Desktop/TinyB/tinyb-master/build/examples/LOG/output1.txt'
		f = open(filename, 'r')		
	except IOError:
		print "Failed to open File"
		
	with f:
		i = 0;
		c = 'u';
		for line in f:
			print(line)
			for word in line.split(','):
				List[i] = float(word)
