#ifndef _HPP_ANIMUS_CAMERAMANAGER_HPP_
#define _HPP_ANIMUS_CAMERAMANAGER_HPP_

#include <vector>

class CameraManager
{

	struct Camera
	{
	    float radius;
	    float theta;
	    float phi;
	    glm::vec3 o;
	    glm::vec3 eye;
	    glm::vec3 up;
	    unsigned int id;
	};

	std::vector<Camera> m_cameras;
	unsigned int m_nbCameras;

	void camera_compute(Camera & c)
	{
	    c.eye.x = cos(c.theta) * sin(c.phi) * c.radius + c.o.x;   
	    c.eye.y = cos(c.phi) * c.radius + c.o.y ;
	    c.eye.z = sin(c.theta) * sin(c.phi) * c.radius + c.o.z;   
	    c.up = glm::vec3(0.f, c.phi < M_PI ?1.f:-1.f, 0.f);
	}

	void camera_defaults(Camera & c)
	{
	    c.phi = 3.14/2.f;
	    c.theta = 3.14/2.f;
	    c.radius = 10.f;
	    camera_compute(c);
	}

public:

	CameraManager() : m_nbCameras(0) {}

	unsigned int createCamera()
	{
		Camera c;
		camera_defaults(c);
		c.id = m_nbCameras;
		m_cameras.push_back(c);
		m_nbCameras++;
		return c.id;
	}

	void removeCamera(unsigned int id)
	{
		for(size_t t = 0; t < m_cameras.size(); t++)
		{
			if(m_cameras[t].id == id)
			{
				m_cameras.erase(m_cameras.begin() + t);
			}
		}
	}


};

#endif