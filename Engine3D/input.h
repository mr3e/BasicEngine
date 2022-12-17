#pragma once
#include <vector>
#include "glm/glm.hpp"
using std::vector;
using std::pair;
class input {
public:
	input(char* textFile);
	unsigned char* getPicture(int DISPLAY_WIDTH, int DISPLAY_HEIGHT);
private:
	glm::vec3 getHitCoords(glm::vec3 rayPosition, glm::vec3 rayDirection, int& hitIndex, bool& isPlane, bool& isDark);
	glm::vec4 getFinalColor(glm::vec3 rayPosition, glm::vec3 rayDirection, glm::vec3 hitCoord, int hitIndex, bool isPlane, bool isDark, int depth);
	uint32_t perPixel(glm::vec3 coord);
	glm::vec3 getDiffuseColor(glm::vec3 hit, glm::vec3 direction, glm::vec3 position, glm::vec3 lightColor, glm::vec3 color, bool isDirectional, int hitIndex, bool isPlane, bool isDark, float cos_value);
	glm::vec3 getSpecularColor(glm::vec3 hit, glm::vec3 direction, glm::vec3 position, glm::vec3 lightColor, glm::vec3 color, bool isDirectional, int hitIndex, bool isPlane, bool isDark, float cos_value, float shiny);
	bool hasShadow(glm::vec3 hit, glm::vec3 direction, glm::vec3 position, glm::vec3 lightColor, glm::vec3 color, bool isDirectional, int hitIndex, bool isPlane, bool isDark, float cos_value, float shiny);
	glm::vec3 calculatePlane(glm::vec3 rayDirection, glm::vec3 rayOrigin, glm::vec4 plane);
	glm::vec3 calculateSphere(glm::vec4 sphere, glm::vec3 rayOrigin, glm::vec3 rayDirection);
	glm::vec3 handle_object(glm::vec3 hitCoord, glm::vec3 phong_model_color, glm::vec3 color, float shiny, int hitIndex, bool isPlane, bool isDark);
	glm::vec3 handle_reflective(glm::vec3 hitCoord, glm::vec3 rayDirection, glm::vec3 phong_model_color, glm::vec3 color, float shiny, int hitIndex, bool isPlane, bool isDark, int depth);
	glm::vec3 handle_transparent(glm::vec3 hitCoord, glm::vec3 rayDirection, glm::vec3 phong_model_color, glm::vec3 color, float shiny, int hitIndex, bool isPlane, bool isDark, int depth);
	glm::vec3 getVecNormal(bool isPlane, glm::vec3 hitCoord, int hitIndex);

	glm::vec4 eye;											//e
	glm::vec4 ambLight;										//a
	vector<pair<bool, pair<glm::vec4, pair<glm::vec4, glm::vec4>>>> lights;		//<isDirectional, <d,<i,p>>>
	vector<pair<char, pair<glm::vec4, glm::vec4>>> spheres;					//<type, <position/normal, color>>
	vector<pair<char, pair<glm::vec4, glm::vec4>>> planes;					//<type, <position/normal, color>>

};