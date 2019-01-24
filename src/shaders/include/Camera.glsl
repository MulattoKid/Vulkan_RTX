#ifndef SHADER_CAMERA_H
#define SHADER_CAMERA_H

#include "Ray.glsl"
#include "DataLayouts.glsl"

Ray GenerateRayFromCamera(CameraShader camera)
{	
    Ray r;
    r.origin = camera.origin.xyz;	
	
	const vec2 uv = vec2(gl_LaunchIDNV.xy) / vec2(gl_LaunchSizeNV.xy - 1);
    r.dir = normalize(camera.topLeftCorner.xyz + (uv.x * camera.horizontalEnd.xyz) + (uv.y * camera.verticalEnd.xyz) - r.origin);
    
    return r;
}

#endif
