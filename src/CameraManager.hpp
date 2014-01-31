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
	size_t m_currentCameraIndex;
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

	void switchTo(unsigned int id)
	{
		bool validID = false;
		for(size_t t = 0; t < m_cameras.size(); t++)
		{
			if(id == m_cameras[t].id)
			{
				validID = true;
				m_currentCameraIndex = t;
				break;
			}
		}
		if(validID)
		{
			std::cout << "Switch to camera " << id << std::endl;
		}
		else
		{
			std::cout << "Impossible to switch to the camera " << id << " because its doesn't exist." << std::endl;
		}
	}

	void zoom(float factor)
	{
		Camera& c = m_cameras[m_currentCameraIndex];
	    c.radius += factor * c.radius ;
	    if (c.radius < 0.1)
	    {
	        c.radius = 10.f;
	        c.o = c.eye + glm::normalize(c.o - c.eye) * c.radius;
	    }
	    camera_compute(c);
	}

	void turn(float phi, float theta)
	{
		Camera& c = m_cameras[m_currentCameraIndex];
		c.theta += 1.f * theta;
	    c.phi   -= 1.f * phi;
	    if (c.phi >= (2 * M_PI) - 0.1 )
	        c.phi = 0.00001;
	    else if (c.phi <= 0 )
	        c.phi = 2 * M_PI - 0.1;
	    camera_compute(c);
	}

	void pan(float x, float y)
	{
		Camera& c = m_cameras[m_currentCameraIndex];
		glm::vec3 up(0.f, c.phi < M_PI ?1.f:-1.f, 0.f);
		glm::vec3 fwd = glm::normalize(c.o - c.eye);
		glm::vec3 side = glm::normalize(glm::cross(fwd, up));
		c.up = glm::normalize(glm::cross(side, fwd));
		c.o[0] += up[0] * y * c.radius * 2;
		c.o[1] += up[1] * y * c.radius * 2;
		c.o[2] += up[2] * y * c.radius * 2;
		c.o[0] -= side[0] * x * c.radius * 2;
		c.o[1] -= side[1] * x * c.radius * 2;
		c.o[2] -= side[2] * x * c.radius * 2;       
		camera_compute(c);
	}

	glm::vec3 getEye() const
	{
		return m_cameras[m_currentCameraIndex].eye;
	}

	glm::vec3 getOrigin() const
	{
		return m_cameras[m_currentCameraIndex].o;
	}

	glm::vec3 getUp() const
	{
		return m_cameras[m_currentCameraIndex].up;
	}




};

#endif