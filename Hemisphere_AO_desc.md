# Description of Hemispherical Ambient Occlusion Technique

## Data
The first thing that is needed is the scene intersection data for the current pixel. This must include the world position and the normal of the itnersection. Secondly, an array of all the spherical lights in the scene must be available.

## Inital color calculations
The algorithm is designed so that any desired color calculation can be performed and have AO applied to it later. This means that two things are needed from this stage:

1) vec3 representing the color of the current pixel when. This calculation should **not** include any type of shadows, i.e. hard, soft, AO, SSAO.

2) An *int* indicating whether or not the current intersection point is ever visible from the center of any light. I.e.
```
int neverVisible = 1
for each light
	if (point is visible from lights center)
		neverVisible = 0
```

## AO calculation
If a intersection point never is visble from any light('s center), AO should be calculated.

For the given intersection point, take *n* samples around the hemisphere given the intersection normal. For each sample, record if it intersects any geometry (e.g. ```numHitsAO++```). After all samples have been taken, find the percentage of samples that hit something, e.g.:
```
float occlusion = float(numHitsAO) / float(totalSamples);
```

This value can be post-processed, e.g. blurred (note that this will require separate render passes etc.).

**Note:** as only pixels that are never visible from any of the lights have AO calculated, more lights in varying positions will result in the shadows becoming smaller. This issue *must* be dealt with before the algorithm can be called **"finished"**.

## Combine AO with color
To combine the AO value into the color image, simply multiply the value with the color value, e.g.:
```
vec3 finalColor = color * occlusion;
```

## Things to check out
### Soft shadows
- https://www.nvidia.com/object/doc_shadows.html#ambi
- https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch11.html
- http://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf
