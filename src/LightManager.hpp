#ifndef _HPP_ANIMUS_LIGHTMANAGER_HPP_
#define _HPP_ANIMUS_LIGHTMANAGER_HPP_

#include <vector>
#include "glm/glm.hpp"
#include "glm/vec3.hpp" // glm::vec3

class LightManager
{

struct PointLight
{
	glm::vec3 pos;
	glm::vec3 diffcolor;
	glm::vec3 speccolor;
	float intensity;
	bool guicollapse;
	PointLight(glm::vec3 pos, glm::vec3 dif, glm::vec3 spec, float inte, bool coll) : pos(pos), diffcolor(dif), speccolor(spec), intensity(inte), guicollapse(coll) {}
};

struct DirLight
{
	glm::vec3 dir;
	glm::vec3 diffcolor;
	glm::vec3 speccolor;
	float intensity;
	bool guicollapse;
	DirLight(glm::vec3 dir, glm::vec3 dif, glm::vec3 spec, float inte, bool coll) : dir(dir), diffcolor(dif), speccolor(spec), intensity(inte), guicollapse(coll) {}
};

struct SpotLight
{
	glm::vec3 pos;
	glm::vec3 dir;
	glm::vec3 diffcolor;
	glm::vec3 speccolor;
	float intensity;
	float extangle;
	float intangle;
	bool guicollapse;
	SpotLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 dif, glm::vec3 spec, float intensity, float extA, float intA, float inte, bool coll)
		: pos(pos), dir(dir), diffcolor(dif), speccolor(spec), intensity(inte), extangle(extA), intangle(intA), guicollapse(coll) {}
};

public:

	LightManager() {}

	void addPointLight(glm::vec3 pos, glm::vec3 diffColor, glm::vec3 specColor, float intensity)
	{
		PointLight p(pos, diffColor, specColor, intensity, true);
		m_pointLights.push_back(p);
	}

	void removePointLight(size_t i)
	{
		m_pointLights.erase(m_pointLights.begin() + i);
	}

	unsigned int getNumPointLight() const { return m_pointLights.size(); }

	void setPLPosition(size_t i, glm::vec3 pos) { m_pointLights[i].pos = pos; }
	void setPLDiffuse(size_t i, glm::vec3 col) { m_pointLights[i].diffcolor = col; }
	void setPLSpec(size_t i, glm::vec3 col) { m_pointLights[i].speccolor = col; }
	void setPLIntensity(size_t i, float intensity) { m_pointLights[i].intensity = intensity; }
	void setPLCollapse(size_t i, bool col) { m_pointLights[i].guicollapse = col; }

	glm::vec3 getPLPosition(size_t i) const { return m_pointLights[i].pos; }
	glm::vec3 getPLDiffuse(size_t i) const { return m_pointLights[i].diffcolor; }
	glm::vec3 getPLSpec(size_t i) const { return m_pointLights[i].speccolor; }
	float getPLIntensity(size_t i) const { return m_pointLights[i].intensity; }
	bool getPLCollapse(size_t i) const { return m_pointLights[i].guicollapse; }

	void addDirLight(glm::vec3 dir, glm::vec3 diffColor, glm::vec3 specColor, float intensity)
	{
		DirLight d(dir, diffColor, specColor, intensity, true);
		m_dirLights.push_back(d);
	}

	void removeDirLight(size_t i)
	{
		m_dirLights.erase(m_dirLights.begin() + i);
	}

	unsigned int getNumDirLight() const { return m_dirLights.size(); }

	void setDLDirection(size_t i, glm::vec3 dir) { m_dirLights[i].dir = dir; }
	void setDLDiffuse(size_t i, glm::vec3 col) { m_dirLights[i].diffcolor = col; }
	void setDLSpec(size_t i, glm::vec3 col) { m_dirLights[i].speccolor = col; }
	void setDLIntensity(size_t i, float intensity) { m_dirLights[i].intensity = intensity; }
	void setDLCollapse(size_t i, bool col) { m_dirLights[i].guicollapse = col; }

	glm::vec3 getDLDirection(size_t i) const { return m_dirLights[i].dir; }
	glm::vec3 getDLDiffuse(size_t i) const { return m_dirLights[i].diffcolor; }
	glm::vec3 getDLSpec(size_t i) const { return m_dirLights[i].speccolor; }
	float getDLIntensity(size_t i) const { return m_dirLights[i].intensity; }
	bool getDLCollapse(size_t i) const { return m_dirLights[i].guicollapse; }

	void addSpotLight(glm::vec3 pos, glm::vec3 dir, glm::vec3 dif, glm::vec3 spec, float extA, float intA, float intensity)
	{
		SpotLight s(pos, dir, dif, spec, intensity, extA, intA, intensity, true);
		m_spotLight.push_back(s);
	}

	void removeSpotLight(size_t i)
	{
		m_spotLight.erase(m_spotLight.begin() + i);
	}

	unsigned int getNumSpotLight() const { return m_spotLight.size(); }

	void setSPLPosition(size_t i, glm::vec3 pos) { m_spotLight[i].pos = pos; }
	void setSPLDirection(size_t i, glm::vec3 dir) { m_spotLight[i].dir = dir; }
	void setSPLDiffuse(size_t i, glm::vec3 col) { m_spotLight[i].diffcolor = col; }
	void setSPLSpec(size_t i, glm::vec3 col) { m_spotLight[i].speccolor = col; }
	void setSPLIntensity(size_t i, float intensity) { m_spotLight[i].intensity = intensity; }
	void setSPLExternalAngle(size_t i, float angle) { m_spotLight[i].extangle = angle; }
	void setSPLInternalAngle(size_t i, float angle) { m_spotLight[i].intangle = angle; }
	void setSPLCollapse(size_t i, bool col) { m_spotLight[i].guicollapse = col; }

	glm::vec3 getSPLPosition(size_t i) const { return m_spotLight[i].pos; }
	glm::vec3 getSPLDirection(size_t i) const { return m_spotLight[i].dir; }
	glm::vec3 getSPLDiffuse(size_t i) const { return m_spotLight[i].diffcolor; }
	glm::vec3 getSPLSpec(size_t i) const { return m_spotLight[i].speccolor; }
	float getSPLIntensity(size_t i) const { return m_spotLight[i].intensity; }
	float getSPLExternalAngle(size_t i) const { return m_spotLight[i].extangle; }
	float getSPLInternalAngle(size_t i) const { return m_spotLight[i].intangle; }
	bool getSPLCollapse(size_t i) const { return m_spotLight[i].guicollapse; }


private:
	std::vector<PointLight> m_pointLights;
	std::vector<DirLight> m_dirLights;
	std::vector<SpotLight> m_spotLight;
};

#endif