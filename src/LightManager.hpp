#ifndef _HPP_ANIMUS_LIGHTMANAGER_HPP_
#define _HPP_ANIMUS_LIGHTMANAGER_HPP_

#include <vector>
#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3

class LightManager
{

public:

	LightManager() : m_numPointLights(0) {}

	void addPointLight(glm::vec3 pos, glm::vec3 diffColor, glm::vec3 specColor, float intensity, bool uiCollapse)
	{
	    m_pointLightsPositions.push_back(pos);
	    m_pointLightsDiffuseColors.push_back(diffColor);
	    m_pointLightsSpecularColors.push_back(specColor);
	    m_pointLightsIntensities.push_back(intensity);
	    m_pointLightCollapses.push_back(uiCollapse);
	    m_numPointLights++;
	}

	void removePointLight(size_t i)
	{
		m_pointLightsPositions.erase(m_pointLightsPositions.begin() + i);
        m_pointLightsDiffuseColors.erase(m_pointLightsDiffuseColors.begin() + i);
        m_pointLightsSpecularColors.erase(m_pointLightsSpecularColors.begin() + i);
        m_pointLightsIntensities.erase(m_pointLightsIntensities.begin() + i);
        m_pointLightCollapses.erase(m_pointLightCollapses.begin() + i);
        m_numPointLights--;
	}

	void setPosition(size_t i, glm::vec3 pos) { m_pointLightsPositions[i] = pos; }
	void setDiffuse(size_t i, glm::vec3 col) { m_pointLightsDiffuseColors[i] = col; }
	void setSpec(size_t i, glm::vec3 col) { m_pointLightsSpecularColors[i] = col; }
	void setIntensity(size_t i, float intensity) { m_pointLightsIntensities[i] = intensity; }
	void setCollapse(size_t i, bool col) { m_pointLightCollapses[i] = col; }

	glm::vec3 getPosition(size_t i) const { return m_pointLightsPositions[i]; }
	glm::vec3 getDiffuse(size_t i) const { return m_pointLightsDiffuseColors[i]; }
	glm::vec3 getSpec(size_t i) const { return m_pointLightsSpecularColors[i]; }
	float getIntensity(size_t i) const { return m_pointLightsIntensities[i]; }
	bool getCollapse(size_t i) const { return m_pointLightCollapses[i]; }

	unsigned int getNumPointLight() const { return m_numPointLights; }


private:
	std::vector<glm::vec3> m_pointLightsPositions;
    std::vector<glm::vec3> m_pointLightsDiffuseColors;
    std::vector<glm::vec3> m_pointLightsSpecularColors;
    std::vector<float> m_pointLightsIntensities;
    std::vector<bool> m_pointLightCollapses;

    unsigned int m_numPointLights;

};

#endif