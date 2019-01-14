#ifndef SHADER_SPHERE_H
#define SHADER_SPHERE_H

#define PI 3.1415926535897932f

float SphereArea(float radius)
{
	return 4.0f * PI * radius * radius;
}

float SphereIntersect(vec3 sphereCenter, float sphereRadius, vec3 rayOrigin, vec3 rayDir, float tMin, float tMax)
{
	//t²dot(rayDir, rayDir) + 2tdot(rayDir, rayOrigin - sphereCenter) + dot(rayOrigin - sphereCenter, rayOrigin - sphereCenter) - sphereRadius² = 0
	float a = dot(rayDir, rayDir);
	vec3 sphereCenterToRayOrigin = rayOrigin - sphereCenter;
	float b = 2.0f * dot(rayDir, sphereCenterToRayOrigin);
	float c = dot(sphereCenterToRayOrigin, sphereCenterToRayOrigin) - pow(sphereRadius, 2.0f);
	
	float discriminant = pow(b, 2.0f) - 4.0f * a * c;
	if (discriminant <= 0.0f)
	{
		return -1.0f;
	}
	
	float denomiator = 2.0f * a;
	float t2 = (-b - sqrt(discriminant)) / denomiator;
	if (t2 >= tMin && t2 <= tMax)
	{
		return t2;
	}
	
	float t1 = (-b + sqrt(discriminant)) / denomiator;
	if (t1 >= tMin && t1 <= tMax)
	{
		return t1;
	}
	
	return -1.0f;
}

#endif
