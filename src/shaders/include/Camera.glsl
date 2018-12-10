#ifndef SHADER_CAMERA_H
#define SHADER_CAMERA_H

#include "Ray.glsl"
#include "UniformDataLayouts.glsl"

Ray GenerateRayFromCamera(Camera camera)
{
	const vec2 uv = vec2(gl_LaunchIDNV.xy) / vec2(gl_LaunchSizeNV.xy - 1);
    const vec3 direction = camera.topLeftCorner.xyz + (uv.x * camera.horizontalEnd.xyz) + (uv.y * camera.verticalEnd.xyz) - camera.origin.xyz;
    
    Ray r;
    r.origin = camera.origin.xyz;
    r.dir = direction;
    return r;
}

#endif
