from matplotlib import pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import numpy as np

REGULAR_SAMPLE = False
VARYING_Z = False
COSINE_WEIGHTED = False
COSINE_WEIGHTED_BLUE_NOISE = False
COSINE_WEIGHTED_GOLDEN_RATIO = False
FIBONACCI_SPIRAL = True
numOcclusionSteps = 7;
totalOcclusionSamples = numOcclusionSteps * numOcclusionSteps;

fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
u = np.linspace(0, 2 * np.pi, 100)
v = np.linspace(0, 0.5 * np.pi, 100)
x = np.outer(np.cos(u), np.sin(v))
y = np.outer(np.sin(u), np.sin(v))
z = np.outer(np.ones(np.size(u)), np.cos(v))
ax.plot_surface(x, y, z, color='w', alpha=0.4)

if REGULAR_SAMPLE:
	points_x = []
	points_y = []
	points_z = []
	for i in range(0, totalOcclusionSamples + 1):
		u0 = float(i) / float(numOcclusionSteps)
		u1 = float(i) / float(totalOcclusionSamples)
		phi = 2.0 * np.pi * u0;
		theta = np.arccos(u1);
		points_x.append(np.sin(theta) * np.cos(phi))
		points_y.append(np.sin(theta) * np.sin(phi))
		points_z.append(u1)
	ax.scatter(points_x, points_y, points_z, marker=".")

if VARYING_Z:
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

if COSINE_WEIGHTED:
	points_x = []
	points_y = []
	points_z = []
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

if COSINE_WEIGHTED_BLUE_NOISE:
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
	
if COSINE_WEIGHTED_GOLDEN_RATIO:
	us = [[0.253906, 0.069336],
		[0.008789, 0.639648],
		[0.763672, 0.208984],
		[0.518555, 0.779297],
		[0.273438, 0.348633],
		[0.028320, 0.918945],
		[0.783203, 0.488281],
		[0.538086, 0.058594],
		[0.292969, 0.627930],
		[0.047852, 0.198242],
		[0.802734, 0.767578],
		[0.557617, 0.337891],
		[0.312500, 0.907227],
		[0.067383, 0.477539],
		[0.822266, 0.046875],
		[0.577148, 0.617188],
		[0.332031, 0.186523],
		[0.086914, 0.756836],
		[0.841797, 0.326172],
		[0.596680, 0.896484],
		[0.351562, 0.465820],
		[0.106445, 0.036133],
		[0.861328, 0.605469],
		[0.616211, 0.175781],
		[0.371094, 0.745117],
		[0.125977, 0.315430],
		[0.880859, 0.884766],
		[0.635742, 0.455078],
		[0.390625, 0.024414],
		[0.145508, 0.594727],
		[0.900391, 0.165039],
		[0.655273, 0.734375],
		[0.410156, 0.304688],
		[0.165039, 0.874023],
		[0.919922, 0.444336],
		[0.674805, 0.013672],
		[0.429688, 0.583984],
		[0.184570, 0.153320],
		[0.939453, 0.723633],
		[0.694336, 0.292969],
		[0.449219, 0.863281],
		[0.204102, 0.432617],
		[0.958984, 0.002930],
		[0.713867, 0.572266],
		[0.468750, 0.142578],
		[0.223633, 0.711914],
		[0.978516, 0.282227],
		[0.733398, 0.851562],
		[0.488281, 0.421875],
		[0.243164, 0.991211],
		[0.998047, 0.561523],
		[0.752930, 0.130859],
		[0.507812, 0.701172],
		[0.262695, 0.270508],
		[0.017578, 0.840820],
		[0.772461, 0.410156],
		[0.527344, 0.980469],
		[0.282227, 0.549805],
		[0.037109, 0.120117],
		[0.791992, 0.689453],
		[0.546875, 0.259766],
		[0.301758, 0.830078],
		[0.056641, 0.399414],
		[0.811523, 0.969727]]
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
	
if FIBONACCI_SPIRAL:
	us = [[0.088853, 2.399963],
		[0.154306, 4.799926],
		[0.199744, 7.199889],
		[0.236984, 9.599853],
		[0.269456, 11.999816],
		[0.298726, 14.399778],
		[0.325667, 16.799742],
		[0.350824, 19.199705],
		[0.374564, 21.599669],
		[0.397148, 23.999632],
		[0.418771, 26.399595],
		[0.439582, 28.799557],
		[0.459699, 31.199520],
		[0.479216, 33.599483],
		[0.498213, 35.999447],
		[0.516752, 38.399410],
		[0.534891, 40.799374],
		[0.552676, 43.199337],
		[0.570149, 45.599300],
		[0.587346, 47.999264],
		[0.604299, 50.399227],
		[0.621037, 52.799191],
		[0.637585, 55.199154],
		[0.653969, 57.599113],
		[0.670209, 59.999077],
		[0.686326, 62.399040],
		[0.702339, 64.799004],
		[0.718268, 67.198967],
		[0.734127, 69.598930],
		[0.749935, 71.998894],
		[0.765708, 74.398857],
		[0.781461, 76.798820],
		[0.797210, 79.198784],
		[0.812971, 81.598747],
		[0.828760, 83.998711],
		[0.844591, 86.398674],
		[0.860483, 88.798637],
		[0.876452, 91.198601],
		[0.892515, 93.598564],
		[0.908691, 95.998528],
		[0.925000, 98.398491],
		[0.941463, 100.798454],
		[0.958103, 103.198418],
		[0.974945, 105.598381],
		[0.992016, 107.998344],
		[1.009347, 110.398308],
		[1.026971, 112.798271],
		[1.044927, 115.198227],
		[1.063261, 117.598190],
		[1.082021, 119.998154],
		[1.101269, 122.398117],
		[1.121075, 124.798080],
		[1.141526, 127.198044],
		[1.162727, 129.598007],
		[1.184810, 131.997971],
		[1.207944, 134.397934],
		[1.232354, 136.797897],
		[1.258348, 139.197861],
		[1.286370, 141.597824],
		[1.317101, 143.997787],
		[1.351690, 146.397751],
		[1.392380, 148.797714],
		[1.444973, 151.197678],
		[1.570796, 153.597641]]
	points_x = []
	points_y = []
	points_z = []
	golden_angle = np.pi * (3.0 - np.sqrt(5.0))
	for y in range(0,numOcclusionSteps+1):
		for x in range(0,numOcclusionSteps+1):
			idx = y * (numOcclusionSteps+1) + x
			sin_theta = np.sin(us[idx][0])
			cos_theta = np.cos(us[idx][0])
			phi = golden_angle * (idx + 1.0)
			cos_phi = np.cos(us[idx][1])
			sin_phi = np.sin(us[idx][1])
			points_x.append(cos_phi * sin_theta)
			points_y.append(sin_phi * sin_theta)
			points_z.append(cos_theta)
	ax.scatter(points_x, points_y, points_z, marker=".", color='g')

plt.show()







































