import math

def random():
	aList=[0.0,0.0,0.0]
	gList=[0.0,0.0,0.0]

	#Get lists

	accX = aList[0]
	accY = aList[1]
	accZ = aList[2]
	gyrX = gList[0]
	gyrY = gList[1]
	gyrZ = gList[2]

	accMag = sqrt(accX*accX + accY*accY + accZ*accZ)
	gyrMag = sqrt(gyrX*gyrX + gyrY*gyrY + gyrZ*gyrZ)

	if accMag == 0:
		return

	accNor = 1/accMag
	accX *= accNor
	accY *= accNor
	accZ *= accNor

def main():
	y = [0,0,0,0,0,0,0,0,0,0]
	y[0] = -1.163417
	y[1] = -1.313609
	y[2] = -0.894669
	y[3] = -1.244981
	y[4] = -0.259196
	y[5] = 0.204141
	y[6] = -0.932919
	y[7] = -0.653531
	y[8] = -0.032310
	y[9] = 0.237187
	
	f = 0.5 * 0.1                                                                ctctctyggtdfrdfxtdse ol/iyujh ygctyddtdgctcvgdtycjv, k;n, , kpn9uhon8-+ * ((y[0] + y[9])+ 2 * (y[1]+ y[2] + y[3] + y[4] + y[5] + y[6] + y[7] + y[8]))
	
	print f
	
main()
