from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

u = np.linspace(0, 2 * np.pi, 100)
v = np.linspace(0, 0.5 * np.pi, 100)
x = np.outer(np.cos(u), np.sin(v))
y = np.outer(np.sin(u), np.sin(v))
z = np.outer(np.ones(np.size(u)), np.cos(v))
ax.plot_surface(x, y, z, color='w', alpha=0.4)

# Initial implementation
points_x = []
points_y = []
points_z = []
numOcclusionSteps = 5;
totalOcclusionSamples = numOcclusionSteps * numOcclusionSteps;
for i in range(0, totalOcclusionSamples + 1):
	u0 = float(i) / float(numOcclusionSteps)
	u1 = float(i) / float(totalOcclusionSamples)
	phi = 2.0 * np.pi * u0;
	theta = np.arccos(u1);
	points_x.append(np.sin(theta) * np.cos(phi))
	points_y.append(np.sin(theta) * np.sin(phi))
	points_z.append(u1)
#ax.scatter(points_x, points_y, points_z, marker=".")

# Snail shell
'''points_x = []
points_y = []
points_z = []
for i in range(0, totalOcclusionSamples + 1):
	u0 = float(i) / float(totalOcclusionSamples)
	u1 = float(i) / float(totalOcclusionSamples)
	phi = 2 * np.pi * u0;
	theta = np.arccos(u1);
	points_x.append(np.sin(theta) * np.cos(phi))
	points_y.append(np.sin(theta) * np.sin(phi))
	points_z.append(u1)
ax.scatter(points_x, points_y, points_z, marker=".")'''

# Locked theta, but z-component has 'Sine' component
points_x = []
points_y = []
points_z = []
for i in range(0, totalOcclusionSamples + 1):
	u0 = float(i) / float(numOcclusionSteps)
	u1 = float(i) / float(totalOcclusionSamples)
	phi = 2 * np.pi * u0;
	theta = np.arccos(u1);
	x = np.sin(theta) * np.cos(phi)
	y = np.sin(theta) * np.sin(phi)
	z = u1 + (np.sin(phi * 2 * np.pi) * 0.1)
	length = x**2 + y**2 + z**2
	x = x / length
	y = y / length
	z = z / length
	points_x.append(x)
	points_y.append(y)
	points_z.append(z)
#ax.scatter(points_x, points_y, points_z, marker=".")

#Cosine
points_x = []
points_y = []
points_z = []
numOcclusionSteps = 7;
totalOcclusionSamples = numOcclusionSteps * numOcclusionSteps;
for y in range(0,numOcclusionSteps+1):
	for x in range(0,numOcclusionSteps+1):
		idx = y * numOcclusionSteps + x;
		u0 = float(idx) / float(numOcclusionSteps)
		u1 = float(idx) / float(totalOcclusionSamples)
		phi = 2.0 * np.pi * u0
		theta = np.sqrt(u1)
		points_x.append(theta * np.cos(phi))
		points_y.append(theta * np.sin(phi))
		points_z.append(np.sqrt(1.0 - u1))
ax.scatter(points_x, points_y, points_z, marker=".", color='b')

# Blue noise data
us = [[0.281250, 0.893555],
	[0.125000, 0.616211],
	[0.726562, 0.806641],
	[0.378906, 0.380859],
	[0.367188, 0.957031],
	[0.572266, 0.280273],
	[0.772461, 0.512695],
	[0.080078, 0.168945],
	[0.105469, 0.861328],
	[0.375977, 0.178711],
	[0.668945, 0.005859],
	[0.587891, 0.540039],
	[0.397461, 0.661133],
	[0.756836, 0.230469],
	[0.533203, 0.814453],
	[0.949219, 0.063477],
	[0.078125, 0.399414],
	[0.244141, 0.454102],
	[0.848633, 0.140625],
	[0.895508, 0.789062],
	[0.228516, 0.147461],
	[0.963867, 0.529297],
	[0.115234, 0.025391],
	[0.238281, 0.778320],
	[0.739258, 0.673828],
	[0.808594, 0.928711],
	[0.748047, 0.377930],
	[0.977539, 0.923828],
	[0.368164, 0.805664],
	[0.501953, 0.122070],
	[0.548828, 0.964844],
	[0.597656, 0.666016],
	[0.473633, 0.458984],
	[0.130859, 0.734375],
	[0.179688, 0.312500],
	[0.366211, 0.500977],
	[0.945312, 0.653320],
	[0.229492, 0.945312],
	[0.248047, 0.626953],
	[0.979492, 0.253906],
	[0.329102, 0.072266],
	[0.637695, 0.124023],
	[0.474609, 0.735352],
	[0.623047, 0.404297],
	[0.464844, 0.302734],
	[0.287109, 0.240234],
	[0.835938, 0.635742],
	[0.006836, 0.731445],
	[0.085938, 0.518555],
	[0.766602, 0.057617],
	[0.973633, 0.833984],
	[0.481445, 0.889648],
	[0.468750, 0.028320],
	[0.025391, 0.006836],
	[0.300781, 0.884766],
	[0.083984, 0.275391],
	[0.488281, 0.611328],
	[0.198242, 0.555664],
	[0.567383, 0.188477],
	[0.670898, 0.566406],
	[0.955078, 0.424805],
	[0.876953, 0.234375],
	[0.228516, 0.041992],
	[0.735352, 0.141602]]
points_x = []
points_y = []
points_z = []
for y in range(0,numOcclusionSteps+1):
	for x in range(0,numOcclusionSteps+1):
		idx = y * (numOcclusionSteps+1) + x;
		u0 = us[idx][0]
		u1 = us[idx][1]
		phi = 2.0 * np.pi * u0
		theta = np.sqrt(u1)
		points_x.append(theta * np.cos(phi))
		points_y.append(theta * np.sin(phi))
		points_z.append(np.sqrt(1.0 - u1))
ax.scatter(points_x, points_y, points_z, marker=".", color='g')

plt.show()
