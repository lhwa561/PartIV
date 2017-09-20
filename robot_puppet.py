import bsocket as s
import ast
import time
import subprocess

def main():
	vList = [0.0,0.0,0.0,0.0,0.0,0.0]
	
	try:
		filename = '/home/pi/Desktop/TinyB/tinyb-master/build/examples/LOG/output1.txt'
		f = open(filename, 'r')		
	except IOError:
		print "Failed to open File"		
	pos = [0,0,0,0,0,0]
	
	pos[0] = 1.1379672656522386
	pos[1] = -0.1466464158362061
	pos[2] = 0.33148866816956896
	pos[3] = 1.1925085847761348
	pos[4] = -1.4491180513279016
	pos[5] = 2.049052809255673
	
	pos = [str(x) for x in pos]	
	out_pos = ' '.join(pos)
	
	print s.sendSocket("limb_moveto right_limb " + out_pos)
	
	with f:

		for line in f:
			print(line)
			i = 0;
			for word in line.split(' '):
				vList[i] = float(word)		
				i += 1
			print "hello"
			pos_1 = s.sendSocket("limb_getpose right_limb")
			#pos_2 = s.sendSocket("limb_getpose left_limb")
			print "lul"
			pos_1 = ast.literal_eval(pos_1)
			#pos_2 = ast.literal_eval(pos_2)
			
			#if (vList[0] == 0 and vList[1] == 0 and vList[2] == 0):	
			#	time.sleep(0.5)
			
			pos_1[0] += vList[0]
			pos_1[1] += vList[1]
			pos_1[2] += vList[2]
			#pos_1[3] = vList[3]
			#pos_1[4] = vList[4]
			#pos_1[5] = vList[5]
			print(vList[0],vList[1],vList[2])
			print(pos_1[0],pos_1[1],pos_1[2])
			#pos_2[2] -= 0.05
			
			#pos_1[1] -= 0.05
			#pos_2[1] -= 0.05
			  
			pos_1 = [str(x) for x in pos_1]
			#pos_2 = [str(x) for x in pos_2]
			
			out_pos_1 = ' '.join(pos_1)
			#out_pos_2 = ' '.join(pos_2)
			#print out_pos_1  
			print s.sendSocket("limb_moveto right_limb " + out_pos_1)
			#print s.sendSocket("limb_moveto left_limb " + out_pos_2)

def lul():
	while(1):
		pos_1 = s.sendSocket("limb_getpose right_limb")
		print pos_1
		time.sleep(1)

def main2():
	pos = [0,0,0,0,0,0]
	pos[0] = 1.1379672656522386
	pos[1] = -0.1466464158362061
	pos[2] = 0.33148866816956896
	pos[3] = 1.1925085847761348
	pos[4] = -1.4491180513279016
	pos[5] = 2.049052809255673

	pos = [str(x) for x in pos]	
	out_pos = ' '.join(pos)
	print s.sendSocket("limb_moveto right_limb " + out_pos)
	
	while(1):
		pos_1 = s.sendSocket("limb_getpose right_limb")
		pos_1 = ast.literal_eval(pos_1)
		pos_1[0] += 0
		pos_1[1] += 0
		pos_1[2] += 0.05
		pos_1 = [str(x) for x in pos_1]	
		out_pos_1 = ' '.join(pos_1)
		print s.sendSocket("limb_moveto right_limb " + out_pos_1)
		time.sleep(1)
			
def main3():
	vList = [0.0,0.0,0.0,0.0,0.0,0.0]
	pos = [0,0,0,0,0,0]
	pos[0] = 1.1379672656522386
	pos[1] = -0.1466464158362061
	pos[2] = 0.33148866816956896
	pos[3] = 1.1925085847761348
	pos[4] = -1.4491180513279016
	pos[5] = 2.049052809255673
	
	filename = '/home/pi/Desktop/TinyB/tinyb-master/build/examples/LOG/output1.txt'
	
	lastLineIndex = 0
	currentLineIndex = 0;
	
	pos = [str(x) for x in pos]	
	out_pos = ' '.join(pos)
	print s.sendSocket("limb_moveto right_limb " + out_pos)
	
	with open(filename, 'r') as f:
		while True:
			line = f.readline()
			if not line:
				break
			lastLineIndex = lastLineIndex + 1
#			i = 0
#			for word in line.split(' '):
#				vList[i] = float(word)		
#				i += 1
#			pos_1 = s.sendSocket("limb_getpose right_limb")
#			pos_1 = ast.literal_eval(pos_1)
#			pos_1[0] += vList[0]
#			pos_1[1] += vList[1]
#			pos_1[2] += vList[2]
#			print(vList[0],vList[1],vList[2])
#			pos_1 = [str(x) for x in pos_1]				
#			out_pos_1 = ' '.join(pos_1)
#			print s.sendSocket("limb_moveto right_limb " + out_pos_1)
			
	while True:
		currentLineIndex = 0
		with open(filename, 'r') as f:
			while True:
				line = f.readline()
				if not line:
					break
				currentLineIndex = currentLineIndex + 1
		if currentLineIndex > lastLineIndex:
			with open(filename, 'r') as f:
				lines = f.readlines()
			line = lines[lastLineIndex]
			lastLineIndex = lastLineIndex + 1
			i = 0
			for word in line.split(' '):
				vList[i] = float(word)		
				i += 1
#			print "hello"
			pos_1 = s.sendSocket("limb_getpose right_limb")
#			print "lul"
			pos_1 = ast.literal_eval(pos_1)
			pos_1[0] += vList[0]
			pos_1[1] += vList[1]
			pos_1[2] += vList[2]
#			pos_1[3] = vList[3]
#			pos_1[4] = vList[4]
#			pos_1[5] = vList[5]
			print(vList[0],vList[1],vList[2])
			#print(pos_1[0],pos_1[1],pos_1[2])
			pos_1 = [str(x) for x in pos_1]
			out_pos_1 = ' '.join(pos_1)
			print s.sendSocket("limb_moveto right_limb " + out_pos_1)
				
main3()
