#include "input.h"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>
#include <windows.h>
#include <glm/gtx/intersect.hpp>

/*									Help Functions									*/
glm::vec3 vec4ToVec3(glm::vec4 vec) {
	return glm::vec3(vec.x, vec.y, vec.z);
}

float distance(glm::vec3 v1, glm::vec4 v2) {
	float x = (v1.x - v2.x);
	float y = (v1.y - v2.y);
	float z = (v1.z - v2.z);
	return sqrt(x * x + y * y + z * z);
}

//object - <type, <position/normal, color>>
glm::vec4 getObjectPosition(pair<char, pair<glm::vec4, glm::vec4>> obj) {
	return obj.second.first;
}
glm::vec4 getObjectColor(pair<char, pair<glm::vec4, glm::vec4>> obj) {
	return obj.second.second;
}
char getObjectType(pair<char, pair<glm::vec4, glm::vec4>> obj) {
	return obj.first;
}
//light - <isDirectional, <direction, <intensity, position>>>
glm::vec4 getLightdirection(pair<bool, pair<glm::vec4, pair<glm::vec4, glm::vec4>>> light) {
	return light.second.first;
}
glm::vec4 getLightIntensity(pair<bool, pair<glm::vec4, pair<glm::vec4, glm::vec4>>> light) {
	return light.second.second.first;
}
glm::vec4 getLightPosition(pair<bool, pair<glm::vec4, pair<glm::vec4, glm::vec4>>> light) {
	return light.second.second.second;
}
bool isDirectional(pair<bool, pair<glm::vec4, pair<glm::vec4, glm::vec4>>> light) {
	return light.first;
}
glm::vec3 handle_spotlight(glm::vec3 hit, glm::vec3 position, glm::vec3 rayDirectionNormal, float cos_value) {
	glm::vec3 spotlightRayNormal = glm::normalize(hit - position);
	float cosValueForLight = glm::dot(spotlightRayNormal, rayDirectionNormal);
	if (cosValueForLight < cos_value) return glm::vec3(0.0f);
	else return spotlightRayNormal;
}
uint32_t vectorToInt(glm::vec4 v) {
	unsigned int r = v.x * 255.0;
	unsigned int g = v.y * 255.0;
	unsigned int b = v.z * 255.0;
	unsigned int a = v.w * 255.0;
	g = g << 8;
	b = b << 16;
	a = a << 24;
	return r + g + b + a;
}
glm::vec3 input::getVecNormal(bool isPlane, glm::vec3 hitCoord, int hitIndex) {
	if (!isPlane) return glm::normalize(hitCoord - vec4ToVec3(getObjectPosition(spheres[hitIndex])));
	return glm::normalize(vec4ToVec3(getObjectPosition(planes[hitIndex])));
}

/*									Parse file									*/
input::input(char* textFile)
{
	//reference https://stackoverflow.com/questions/7868936/read-file-line-by-line-using-ifstream-in-c
	std::ifstream infile(textFile);
	std::string line;
	vector<pair<char, glm::vec4>> objects;
	vector<glm::vec4> lightsTemp;
	vector<glm::vec4> colorsTemp;
	vector<glm::vec4> colorIntensity;
	vector<glm::vec4> position;
	while (std::getline(infile, line))
	{
		std::istringstream iss(line);
		char mode;
		float x, y, z, w;
		if (!(iss >> mode >> x >> y >> z >> w)) { break; }

		glm::vec4 vec(x, y, z, w);
		switch (mode) {
		case 'e': {
			eye = vec;
			break;
		}
		case 'a': {
			ambLight = vec;
			break;
		}
		case 'd': {
			lightsTemp.push_back(vec);
			break;
		}
		case 'p': {
			position.push_back(vec);
			break;
		}
		case 'i': {
			colorIntensity.push_back(vec);
			break;
		}
		case 'o': {
			objects.push_back({ mode,vec });
			break;
		}
		case 'r': {
			objects.push_back({ mode,vec });
			break;
		}
		case 't': {
			objects.push_back({ mode,vec });
			break;
		}
		case 'c': {
			colorsTemp.push_back(vec);
			break;
		}
		}
	}

	int counter = 0;
	for (unsigned int i = 0; i < lightsTemp.size(); i++) {
		if (lightsTemp[i].w == 0) {
			//directional - <true, <direction, intensity>>
			lights.push_back({ true, {lightsTemp[i], {colorIntensity[i], glm::vec4(0.0f)}} });
		}
		else {
			//spotlight - <false, <direction, <intensity, position>>>
			lights.push_back({ false, {lightsTemp[i], {colorIntensity[i], position[counter]}} });
			counter++;
		}
	}
	for (unsigned int i = 0; i < objects.size(); i++) {
		if (objects[i].second.w > 0) spheres.push_back({ objects[i].first,{ objects[i].second, colorsTemp[i] } });
		else planes.push_back({ objects[i].first, {objects[i].second, colorsTemp[i] } });
	}
}
/*								   Render picture									*/
unsigned char* input::getPicture(int DISPLAY_WIDTH, int DISPLAY_HEIGHT)
{
	//FILE* file = fopen("../coord.txt", "w+");
	uint32_t* data = new uint32_t[DISPLAY_WIDTH * DISPLAY_HEIGHT];
	for (int y = 0; y < DISPLAY_HEIGHT; y++) {
		for (int x = 0; x < DISPLAY_WIDTH; x++) {
			glm::vec2 coord = { ((float)x) / ((float)DISPLAY_WIDTH), ((float)y) / ((float)DISPLAY_HEIGHT) };
			coord = coord * 2.0f - 1.0f;
			data[x + y * DISPLAY_WIDTH] = perPixel(glm::vec3(coord.x, -coord.y, 0.0f));
			//fprintf(file, "%d, %d, %d, %d, ", (int)(((unsigned char*)data[x + y * DISPLAY_WIDTH])[0])
			//,(int)(((unsigned char*)data[x + y * DISPLAY_WIDTH])[1]), (int)(((unsigned char*)data[x + y * DISPLAY_WIDTH])[2]),
			//(int)(((unsigned char*)data[x + y * DISPLAY_WIDTH])[3]));
		}
	}
	return (unsigned char*)data;
}

uint32_t input::perPixel(glm::vec3 rayDirection) {
	int hitIndex = -1;
	bool isPlane = false;
	bool isDark = false;
	glm::vec3 rayPosition = glm::vec3(eye.x, eye.y, eye.z);
	glm::vec3 hitCoord = getHitCoords(rayPosition, rayDirection - rayPosition, hitIndex, isPlane, isDark);
	glm::vec4 colorForPixel = getFinalColor(rayPosition, rayDirection, hitCoord, hitIndex, isPlane, isDark, 0);
	return vectorToInt(colorForPixel);
}

glm::vec3 input::getHitCoords(glm::vec3 rayPosition, glm::vec3 rayDirection, int& hitIndex, bool& isPlane, bool& isDark) {
	int hitPrev = hitIndex;
	bool isPlanePrev = isPlane;
	float hitDistance = -1;
	glm::vec3 hitCoord(0.0f);

	if (rayDirection != glm::vec3(0.0f)) rayDirection = glm::normalize(rayDirection);
	for (unsigned int i = 0; i < planes.size(); i++) {
		if (!(hitPrev != -1 && getObjectType(planes[i]) == 't')) {
			if (!(isPlanePrev && hitPrev == i)) {
				glm::vec3 val = calculatePlane(rayPosition, rayDirection, getObjectPosition(planes[i]));
				if (val != glm::vec3(0.0f))
					if (hitDistance == -1 || distance(val, eye) < hitDistance) {
						hitCoord = val;
						hitDistance = distance(val, eye);
						isPlane = true;
						hitIndex = i;
						if (getObjectType(planes[i]) == 'o')
							if (abs(int(2.0 * (val.x + 10))) % 2 == abs(int(2.0 * (val.y + 10))) % 2) isDark = true;
							else isDark = false;
					}
			}
		}
	}
	for (unsigned int i = 0; i < spheres.size(); i++) {
		if (!(!isPlanePrev && hitPrev == i)) {
			if (!(hitPrev != -1 && getObjectType(spheres[i]) == 't')) {
				glm::vec3 val = calculateSphere(getObjectPosition(spheres[i]), rayPosition, rayDirection);
				if (val != glm::vec3(0.0f))
					if (hitDistance == -1 || distance(val, eye) < hitDistance) {
						hitCoord = val;
						hitDistance = distance(val, eye);
						isPlane = false;
						isDark = false;
						hitIndex = i;
					}
			}
		}
	}
	return hitCoord;
}

glm::vec4 input::getFinalColor(glm::vec3 rayPosition, glm::vec3 rayDirection, glm::vec3 hitCoord, int hitIndex, bool isPlane, bool isDark, int depth) {
	bool isPlane2 = isPlane, isDark2 = isDark;
	int hitIndex2 = hitIndex;
	glm::vec3 phongModel = glm::vec3(0, 0, 0);
	pair<char, pair<glm::vec4, glm::vec4>> obj;
	if (isPlane) obj = planes[hitIndex];
	else if (hitIndex != -1) obj = spheres[hitIndex];
	else return glm::vec4(0.0f);

	if (getObjectType(obj) == 'o') {
		phongModel = handle_object(hitCoord, phongModel, vec4ToVec3(getObjectColor(obj)), getObjectColor(obj).w, hitIndex2, isPlane2, isDark2);
	}
	else if (getObjectType(obj) == 'r') {
		phongModel = handle_reflective(hitCoord, rayDirection, phongModel, vec4ToVec3(getObjectColor(obj)), getObjectColor(obj).w, hitIndex2, isPlane2, isDark2, depth);
	}
	else if (getObjectType(obj) == 't') {
		phongModel = handle_transparent(hitCoord, rayDirection, phongModel, vec4ToVec3(getObjectColor(obj)), getObjectColor(obj).w, hitIndex2, isPlane2, isDark2, depth);
	}
	if (isDark)
		return glm::vec4(phongModel.r, phongModel.g, phongModel.b, 0.0) * 0.5f;
	else return glm::vec4(phongModel.r, phongModel.g, phongModel.b, 0.0);
}
/*									Intersections									*/
glm::vec3 input::calculatePlane(glm::vec3 rayOrigin, glm::vec3 rayDirection, glm::vec4 plane) {
	glm::vec3 planeNormal(plane.x, plane.y, plane.z);
	float d = plane.w;
	if (glm::dot(rayDirection, planeNormal) != 0) {
		float ans = -(glm::dot(planeNormal, rayOrigin) + d) / glm::dot(planeNormal, rayDirection);
		if (ans >= 0) {
			glm::vec3 res = rayOrigin + rayDirection * ans;
			return res;
		}
	}
	return glm::vec3(0.0f);
}

glm::vec3 input::calculateSphere(glm::vec4 sphere, glm::vec3 rayOrigin, glm::vec3 rayDirection) {
	float discriminant;
	glm::vec3 center = rayOrigin - glm::vec3(sphere.x, sphere.y, sphere.z);

	float a = 1;
	float b = 2.0f * glm::dot(rayDirection, center);
	float c = glm::dot(center, center) - sphere.w * sphere.w;

	discriminant = b * b - 4.0f * a * c;
	if (discriminant >= 0.0f) {
		float delta = sqrt(discriminant);
		float ans1 = (-b + delta) / (2 * a);
		float ans2 = (-b - delta) / (2 * a);
		discriminant = glm::min(ans1, ans2);
		return rayOrigin + rayDirection * discriminant;
	}
	return glm::vec3(0.0f);
}
/*									Handle Sphere/Plane									*/
glm::vec3 input::handle_object(glm::vec3 hitCoord, glm::vec3 phongModel, glm::vec3 color, float shiny, int hitIndex, bool isPlane, bool isDark) {
	phongModel = color * glm::vec3(ambLight.x, ambLight.y, ambLight.z);
	for (int i = 0; i < lights.size(); i++) {
		glm::vec3 colorForDiffuse = getDiffuseColor(hitCoord, vec4ToVec3(getLightdirection(lights[i])), vec4ToVec3(getLightPosition(lights[i])), vec4ToVec3(getLightIntensity(lights[i])), color, isDirectional(lights[i]), hitIndex, isPlane, isDark, getLightPosition(lights[i]).w);
		colorForDiffuse = glm::max(colorForDiffuse, glm::vec3(0, 0, 0));
		glm::vec3 colorForSpecular = getSpecularColor(hitCoord, vec4ToVec3(getLightdirection(lights[i])), vec4ToVec3(getLightPosition(lights[i])), vec4ToVec3(getLightIntensity(lights[i])), color, isDirectional(lights[i]), hitIndex, isPlane, isDark, getLightPosition(lights[i]).w, shiny);
		colorForSpecular = glm::max(colorForSpecular, glm::vec3(0, 0, 0));
		if (hasShadow(hitCoord, vec4ToVec3(getLightdirection(lights[i])), vec4ToVec3(getLightPosition(lights[i])), vec4ToVec3(getLightIntensity(lights[i])), color, isDirectional(lights[i]), hitIndex, isPlane, isDark, getLightPosition(lights[i]).w, shiny))
			phongModel += colorForDiffuse + colorForSpecular;
	}
	return phongModel = glm::min(phongModel, glm::vec3(1.0f, 1.0f, 1.0f));
}

glm::vec3 input::handle_reflective(glm::vec3 hitCoord, glm::vec3 rayDirection, glm::vec3 phongModel, glm::vec3 color, float shiny, int hitIndex, bool isPlane, bool isDark, int depth) {
	if (depth == 5) {
		return glm::vec3(0.f, 0.f, 0.f);
	}
	glm::vec3 normalForObj = getVecNormal(isPlane, hitCoord, hitIndex);
	glm::vec3 reflection_direction = rayDirection - 2.0f * normalForObj * glm::dot(rayDirection, normalForObj);
	glm::vec3 hitCoord2 = getHitCoords(hitCoord, reflection_direction, hitIndex, isPlane, isDark);
	if (hitIndex == -1) {
		return glm::vec3(0.f, 0.f, 0.f);
	}
	glm::vec4 colorOfReflection = getFinalColor(hitCoord, reflection_direction, hitCoord2, hitIndex, isPlane, isDark, depth + 1);
	phongModel = glm::vec3(colorOfReflection.r, colorOfReflection.g, colorOfReflection.b);
	return glm::min(phongModel, glm::vec3(1.0, 1.0, 1.0));
}

glm::vec3 input::handle_transparent(glm::vec3 hitCoord, glm::vec3 rayDirection, glm::vec3 phongModel, glm::vec3 color, float shiny, int hitIndex, bool isPlane, bool isDark, int depth) {
	if (depth == 5) {
		return glm::vec3(0.f, 0.f, 0.f);
	}
	glm::vec4 colorOfTransparency = glm::vec4(0.f, 0.f, 0.f, 0.f);
	if (isPlane) {
		glm::vec3 hitCoord2 = getHitCoords(hitCoord, rayDirection, hitIndex, isPlane, isDark);
		if (hitIndex == -1) {
			return glm::vec3(0.f, 0.f, 0.f);
		}
		colorOfTransparency = getFinalColor(hitCoord, rayDirection, hitCoord2, hitIndex, isPlane, isDark, depth + 1);
	}
	else {
		// Using Snell's law
		glm::vec3 normalForObj = getVecNormal(isPlane, hitCoord, hitIndex);
		float pi = 3.14159265f;
		float cosFirst = glm::dot(normalForObj, -rayDirection);
		float thetaFirst = acos(cosFirst) * (180.0f / pi);
		float snellFraction = (1.0f / 1.5f);
		float sinFirst = sin(thetaFirst * (pi / 180.0f));
		float sinSecond = snellFraction * sinFirst;
		float thetaSecond = asin(sinSecond) * (180.0f / pi);
		float cosSecond = cos(thetaSecond * (pi / 180.0f));

		glm::vec3 rayDirectionToIn = (snellFraction * cosFirst - cosSecond) * normalForObj - snellFraction * (-rayDirection);
		rayDirectionToIn = glm::normalize(rayDirectionToIn);
		glm::vec3 hitCoord2 = getHitCoords(hitCoord, rayDirection, hitIndex, isPlane, isDark);

		if (hitIndex != -1) {
			colorOfTransparency = getFinalColor(hitCoord, rayDirectionToIn, hitCoord2, hitIndex, isPlane, isDark, depth + 1);
		}
		else {
			glm::vec3 theSecondHitPoint = calculateSphere(getObjectPosition(spheres[hitIndex]), hitCoord, rayDirectionToIn);

			cosFirst = glm::dot(-glm::normalize(theSecondHitPoint), -rayDirectionToIn);
			thetaFirst = acos(cosFirst) * (180.0f / pi);
			snellFraction = (1.5f / 1.0f);
			sinFirst = sin(thetaFirst * (pi / 180.0f));
			sinSecond = snellFraction * sinFirst;
			thetaSecond = asin(sinSecond) * (180.0f / pi);
			cosSecond = cos(thetaSecond * (pi / 180.0f));

			glm::vec3 rayDirectionToOut = (snellFraction * cosFirst - cosSecond) * -normalForObj - snellFraction * (-rayDirectionToIn);
			rayDirectionToOut = glm::normalize(rayDirectionToOut);

			glm::vec3 hitCoord2 = getHitCoords(theSecondHitPoint, rayDirectionToOut, hitIndex, isPlane, isDark);

			if (hitIndex == -1) {
				return glm::vec3(0.f, 0.f, 0.f);
			}
			colorOfTransparency = getFinalColor(theSecondHitPoint, rayDirectionToOut, hitCoord2, hitIndex, isPlane, isDark, depth + 1);
		}
	}

	phongModel = glm::vec3(colorOfTransparency.r, colorOfTransparency.g, colorOfTransparency.b);
	phongModel = glm::min(phongModel, glm::vec3(1.0, 1.0, 1.0));
	return phongModel;
}
/*									Handle Lights									*/
glm::vec3 input::getDiffuseColor(glm::vec3 hit, glm::vec3 direction, glm::vec3 position, glm::vec3 lightColor, glm::vec3 color, bool isDirectional, int hitIndex, bool isPlane, bool isDark, float cos_value) {
	float factorForObj = 1.0f;
	if (isPlane) {
		factorForObj = -1.0f;
	}
	glm::vec3 rayDirectionNormal = factorForObj * glm::normalize(direction);
	if (!isDirectional) {
		rayDirectionNormal = handle_spotlight(hit, position, factorForObj * rayDirectionNormal, cos_value) * factorForObj;
		if (rayDirectionNormal == glm::vec3(0.0f))
			return rayDirectionNormal;
	}

	glm::vec3 normalForObj = getVecNormal(isPlane, hit, hitIndex);
	float cosValForHit = glm::dot(normalForObj, -rayDirectionNormal);

	glm::vec3 colorForDiffuse = color * cosValForHit * lightColor;
	return colorForDiffuse;
}

glm::vec3 input::getSpecularColor(glm::vec3 hit, glm::vec3 direction, glm::vec3 position, glm::vec3 lightColor, glm::vec3 color, bool isDirectional, int hitIndex, bool isPlane, bool isDark, float cos_value, float shiny) {
	glm::vec3 rayDirectionNormal = direction;
	if (direction != glm::vec3(0.0f))
		rayDirectionNormal = glm::normalize(direction);

	if (!isDirectional) {
		rayDirectionNormal = handle_spotlight(hit, position, rayDirectionNormal, cos_value);
		if (rayDirectionNormal == glm::vec3(0.0f))
			return rayDirectionNormal;
	}

	glm::vec3 normalForObj = getVecNormal(isPlane, hit, hitIndex);
	glm::vec3 reflectedRayLight = rayDirectionNormal - 2.0f * normalForObj * glm::dot(rayDirectionNormal, normalForObj);
	glm::vec3 rayToVeye = glm::normalize(position - hit);

	float cosValForHit = glm::dot(rayToVeye, reflectedRayLight);
	cosValForHit = glm::max(0.0f, cosValForHit);
	cosValForHit = pow(cosValForHit, shiny);

	glm::vec3 resultColor = 0.7f * cosValForHit * lightColor;
	return resultColor;
}

bool input::hasShadow(glm::vec3 hit, glm::vec3 direction, glm::vec3 position, glm::vec3 lightColor, glm::vec3 color, bool isDirectional, int hitIndex, bool isPlane, bool isDark, float cos_value, float shiny) {
	glm::vec3 rayDirectionNormal = direction;
	if (direction != glm::vec3(0.0f))
		rayDirectionNormal = glm::normalize(direction);

	float minDis = INFINITY;

	if (!isDirectional) {
		rayDirectionNormal = handle_spotlight(hit, position, rayDirectionNormal, cos_value);
		if (rayDirectionNormal == glm::vec3(0.0f))
			return false;
		else minDis = -(glm::dot(hit, position)) / std::abs(glm::dot(-rayDirectionNormal, position));
	}

	int hitIndex2 = hitIndex;
	bool isPlane2 = isPlane;
	glm::vec3 hitCoord = getHitCoords(hit, -rayDirectionNormal, hitIndex2, isPlane2, isDark);
	if (hitCoord != glm::vec3(0.0f) && hitIndex2 != -1 && distance(hitCoord, glm::vec4(hit.x, hit.y, hit.z, 0.0f)) < minDis) {
		return false;
	}
	return true;
}