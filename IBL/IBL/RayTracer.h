#pragma once


//#include "embree2/rtcore_ray.h"
//#include "ray.h"
//#include "application.h"
//#include "camera.h"
//#include "scene.h"
//#include "scene_device.h"
#include <model.h>

#include <embree2/rtcore.h>
#include <embree2/rtcore_ray.h>
#include <embree2/rtcore_scene.h>

#include <glm/glm.hpp>



class RayTracer
{
	/* vertex and triangle layout */
	struct Vertex { float x, y, z, r; }; // FIXME: rename to Vertex4f
	struct Triangle { int v0, v1, v2; };

	friend void error_handler(void* userPtr, const RTCError code, const char* str);
public:
	explicit RayTracer(Model* objModel);
	~RayTracer();

	bool IntersectScene(const glm::vec3 &origin, const glm::vec3 &dir);

private:
	// Embree Part
	RTCRay ray;
	RTCDevice device;
	RTCScene scene = nullptr;
	Model* obj_model;

	unsigned int addModleToScene(RTCScene);
};

void error_handler(void* userPtr, const RTCError code, const char* str = nullptr);
