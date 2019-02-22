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
	phi = 2 * np.pi * u0;
	theta = np.arccos(u1);
	points_x.append(np.sin(theta) * np.cos(phi))
	points_y.append(np.sin(theta) * np.sin(phi))
	points_z.append(u1)
ax.scatter(points_x, points_y, points_z, marker=".")

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
ax.scatter(points_x, points_y, points_z, marker=".")


plt.show()
