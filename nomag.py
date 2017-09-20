import math
import time

# 3-tuples (x, y, z) for accel, gyro
ax, ay, az = accel                  # Units G (but later normalised)
gx, gy, gz = (radians(x) for x in gyro) # Units deg/s

q1, q2, q3, q4 = (q[x] for x in range(4))   # short name local variable for readability
# Auxiliary variables to avoid repeated arithmetic
_2q1 = 2 * q1
_2q2 = 2 * q2
_2q3 = 2 * q3
_2q4 = 2 * q4
_4q1 = 4 * q1
_4q2 = 4 * q2
_4q3 = 4 * q3
_8q2 = 8 * q2
_8q3 = 8 * q3
q1q1 = q1 * q1
q2q2 = q2 * q2
q3q3 = q3 * q3
q4q4 = q4 * q4

# Normalise accelerometer measurement
norm = sqrt(ax * ax + ay * ay + az * az)
if (norm == 0):
	return # handle NaN
norm = 1 / norm        # use reciprocal for division
ax *= norm
ay *= norm
az *= norm

# Gradient decent algorithm corrective step
s1 = _4q1 * q3q3 + _2q3 * ax + _4q1 * q2q2 - _2q2 * ay
s2 = _4q2 * q4q4 - _2q4 * ax + 4 * q1q1 * q2 - _2q1 * ay - _4q2 + _8q2 * q2q2 + _8q2 * q3q3 + _4q2 * az
s3 = 4 * q1q1 * q3 + _2q1 * ax + _4q3 * q4q4 - _2q4 * ay - _4q3 + _8q3 * q2q2 + _8q3 * q3q3 + _4q3 * az
s4 = 4 * q2q2 * q4 - _2q2 * ax + 4 * q3q3 * q4 - _2q3 * ay
norm = 1 / sqrt(s1 * s1 + s2 * s2 + s3 * s3 + s4 * s4)    # normalise step magnitude
s1 *= norm
s2 *= norm
s3 *= norm
s4 *= norm

# Compute rate of change of quaternion
qDot1 = 0.5 * (-q2 * gx - q3 * gy - q4 * gz) - self.beta * s1
qDot2 = 0.5 * (q1 * gx + q3 * gz - q4 * gy) - self.beta * s2
qDot3 = 0.5 * (q1 * gy - q2 * gz + q4 * gx) - self.beta * s3
qDot4 = 0.5 * (q1 * gz + q2 * gy - q3 * gx) - self.beta * s4

# Integrate to yield quaternion
deltat = elapsed_micros(self.start_time) / 1000000
self.start_time = time.ticks_us()
q1 += qDot1 * deltat
q2 += qDot2 * deltat
q3 += qDot3 * deltat
q4 += qDot4 * deltat
norm = 1 / sqrt(q1 * q1 + q2 * q2 + q3 * q3 + q4 * q4)    # normalise quaternion
self.q = q1 * norm, q2 * norm, q3 * norm, q4 * norm
self.yaw = 0
self.pitch = degrees(-asin(2.0 * (self.q[1] * self.q[3] - self.q[0] * self.q[2])))
self.roll = degrees(atan2(2.0 * (self.q[0] * self.q[1] + self.q[2] * self.q[3]),
	self.q[0] * self.q[0] - self.q[1] * self.q[1] - self.q[2] * self.q[2] + self.q[3] * self.q[3]))
