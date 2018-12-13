#ifndef SHADER_RAY_H
#define SHADER_RAY_H

struct Ray
{
	vec3 origin;
	vec3 dir;
};

Ray GenerateRay(vec3 origin, vec3 dir)
{
	Ray r;
	r.origin = origin;
	r.dir = dir;
	return r;
}

#endif
