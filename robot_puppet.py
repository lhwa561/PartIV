import bsocket as s
import ast
import time
import subprocess
		
def main():
	
	#Variable Initialisation
	vList = [0.0,0.0,0.0,0.0,0.0,0.0]
	pos = [0,0,0,0,0,0]
	pos[0] = 1.1379672656522386
	pos[1] = -0.1466464158362061
	pos[2] = 0.33148866816956896
	pos[3] = 1.1925085847761348
	pos[4] = -1.4491180513279016
	pos[5] = 2.049052809255673
	
	#The location of the log file used for this program (Created in TinyB 'program.cpp')
	filename = '/home/pi/Desktop/TinyB/tinyb-master/build/examples/LOG/output1.txt'
	
	lastLineIndex = 0
	currentLineIndex = 0;
	
	#Initial position update
	pos = [str(x) for x in pos]	
	out_pos = ' '.join(pos)
	print s.sendSocket("limb_moveto right_limb " + out_pos)
	
	scale = 1.2

	#Checks the length of the log file
	with open(filename, 'r') as f:
		while True:
			line = f.readline()
			if not line:
				break
			lastLineIndex = lastLineIndex + 1

	#Reads each line and executes the position update (NO EULERS IMPLEMENTED HERE)
	while True:
		currentLineIndex = 0
		
		#Opens the log file
		with open(filename, 'r') as f:
			while True:
				line = f.readline()
				if not line:
					break
				currentLineIndex = currentLineIndex + 1
		if currentLineIndex > lastLineIndex:
			#Opens the log file and gets next available values and adds them to the
			#current position of the robot
			with open(filename, 'r') as f:
				lines = f.readlines()
			line = lines[lastLineIndex]
			lastLineIndex = lastLineIndex + 1
			i = 0
			for word in line.split(' '):
				vList[i] = float(word)		
				i += 1
			pos_1 = s.sendSocket("limb_getpose right_limb")
			pos_1 = ast.literal_eval(pos_1)
			
			#Scales the movements by 20% if the movement is less than 5 cm for scaling
			if (vList[0] < 0.05 and vList[0] > -0.05):
				vList[0] *= scale
			if (vList[1] < 0.05 and vList[1] > -0.05):
				vList[1] *= scale
			if (vList[2] < 0.05 and vList[2] > -0.05):
				vList[2] *= scale
			
			pos_1[0] += vList[0]
			pos_1[1] += vList[1]
			pos_1[2] += vList[2]
#			pos_1[3] = vList[3]
#			pos_1[4] = vList[4]
#			pos_1[5] = vList[5]
			print(vList[0],vList[1],vList[2])
			pos_1 = [str(x) for x in pos_1]
			out_pos_1 = ' '.join(pos_1)
			
			#Sends the new position of the right arm to baxter to execute
			print s.sendSocket("limb_moveto right_limb " + out_pos_1)
				
main()
