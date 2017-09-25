#include "RayTracer.h"
#include <cstdio>

RayTracer::RayTracer(Model *objModel): device(rtcNewDevice(NULL)), obj_model(objModel)
{
	//assert(0);

	/* create new Embree device */
	//device = rtcNewDevice(cfg);
	error_handler(nullptr, rtcDeviceGetError(device));

	/* set error handler */
	rtcDeviceSetErrorFunction2(device, error_handler, nullptr);

	/* Initialize Scene */
	scene = rtcDeviceNewScene(device, RTC_SCENE_STATIC, RTC_INTERSECT1);
	/* add ground plane */
	addModleToScene(scene);

	/* commit changes to scene */
	rtcCommit(scene);
}


RayTracer::~RayTracer()
{
	rtcDeleteScene(scene);
	rtcDeleteDevice(device);
}

bool RayTracer::IntersectScene(const glm::vec3 & origin, const glm::vec3 & dir)
{
	//RTCRay ray;
	constexpr float dirScale = 0.0001f;
	ray.org[0] = origin.x + dir.x * dirScale;
	ray.org[1] = origin.y + dir.y * dirScale;
	ray.org[2] = origin.z + dir.z * dirScale;
	ray.dir[0] = dir.x;
	ray.dir[1] = dir.y;
	ray.dir[2] = dir.z;
	ray.tnear = 0.0f;
	ray.tfar = FLT_MAX;
	ray.instID = RTC_INVALID_GEOMETRY_ID;
	ray.geomID = RTC_INVALID_GEOMETRY_ID;
	ray.primID = RTC_INVALID_GEOMETRY_ID;
	ray.mask = 0xFFFFFFFF;
	ray.time = 0.0f;
	rtcOccluded(scene, ray);
	//rtcIntersect();
	//rtcOccluded();
	// geomID = 0 => isBlocked
	bool isBlocked = false;
	if (ray.geomID == 0) isBlocked = true;
	//if (ray.dir[0] < 0) isBlocked = true;

	return isBlocked;
}

unsigned int RayTracer::addModleToScene(RTCScene scene_i)
{
	size_t numVertices = obj_model->GetModelVertexSize();
	size_t numTriangles = obj_model->GetModelIndicesSize() / 3;
	/* create a triangulated plane with 2 triangles and 4 vertices */
	unsigned int mesh = rtcNewTriangleMesh(scene_i, RTC_GEOMETRY_STATIC, numTriangles, numVertices);

	/* set vertices */
	Vertex* vertices = (Vertex*)rtcMapBuffer(scene_i, mesh, RTC_VERTEX_BUFFER);
	for(size_t i = 0; i < numVertices; ++i)
	{
		const glm::vec3 &vertex = obj_model->GetCurrentVertexPosition(i); // 换成引用会不会快
		vertices[i].x = vertex.x; vertices[i].y = vertex.y; vertices[i].z = vertex.z;
	}
	//vertices[0].x = -10; vertices[0].y = -2; vertices[0].z = -10;
	//vertices[1].x = -10; vertices[1].y = -2; vertices[1].z = +10;
	//vertices[2].x = +10; vertices[2].y = -2; vertices[2].z = -10;
	//vertices[3].x = +10; vertices[3].y = -2; vertices[3].z = +10;
	rtcUnmapBuffer(scene_i, mesh, RTC_VERTEX_BUFFER);

	/* set triangles */
	Triangle* triangles = (Triangle*)rtcMapBuffer(scene_i, mesh, RTC_INDEX_BUFFER);
	for(size_t i = 0; i < numTriangles; ++i)
	{
		triangles[i].v0 = obj_model->GetCurrentIndexValue(i * 3);
		triangles[i].v1 = obj_model->GetCurrentIndexValue(i * 3 + 1);
		triangles[i].v2 = obj_model->GetCurrentIndexValue(i * 3 + 2);
	}
	//triangles[0].v0 = 0; triangles[0].v1 = 2; triangles[0].v2 = 1;
	//triangles[1].v0 = 1; triangles[1].v1 = 2; triangles[1].v2 = 3;
	rtcUnmapBuffer(scene_i, mesh, RTC_INDEX_BUFFER);

	return mesh;
}

/* error reporting function */
void error_handler(void* userPtr, const RTCError code, const char* str)
{
	if (code == RTC_NO_ERROR)
		return;

	printf("Embree: ");
	switch (code) {
	case RTC_UNKNOWN_ERROR: printf("RTC_UNKNOWN_ERROR"); break;
	case RTC_INVALID_ARGUMENT: printf("RTC_INVALID_ARGUMENT"); break;
	case RTC_INVALID_OPERATION: printf("RTC_INVALID_OPERATION"); break;
	case RTC_OUT_OF_MEMORY: printf("RTC_OUT_OF_MEMORY"); break;
	case RTC_UNSUPPORTED_CPU: printf("RTC_UNSUPPORTED_CPU"); break;
	case RTC_CANCELLED: printf("RTC_CANCELLED"); break;
	default: printf("invalid error code"); break;
	}
	if (str) {
		printf(" (");
		while (*str) putchar(*str++);
		printf(")\n");
	}
	exit(1);
}