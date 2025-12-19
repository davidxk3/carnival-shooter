#include "Vectors.h"

class DuckTarget
{
private:
	float duckX = -8.0;
	float duckY = -0.5;
	float duckZ = -8.0;
	float targetWidth = 4.0;
	float targetLength = 3.0;
	float targetDepth = 1.0;
	
	bool turn1 = false;
	bool turn2 = false;
	bool leftToRight = true;
	bool rightToLeft = false;
	bool flipped = false;
	float flipAngle = 0;
	float spin = 0;

	// Material properties for drawing
	float mat_ambient[4] = { 0.957f, 0.74f, 0.047f, 1.0f };
	float mat_diffuse[4] = { 0.957f, 0.74f, 0.047f, 1.0f };
	float mat_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float mat_shininess[1] = { 100.0F };

	float beakmat_ambient[4] = { 0.878f, 0.129f, 0.153f, 1.0f };
	float beakmat_diffuse[4] = { 0.878f, 0.129f, 0.153f, 1.0f };
	float beakmat_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float beakmat_shininess[1] = { 100.0F };

	float bullseye_ambient[4] = { 0.851f, 0.118f, 0.161f, 1.0f };
	float bullseye_diffuse[4] = { 0.851f, 0.118f, 0.161f, 1.0f };
	float bullseye_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float bullseye_shininess[1] = { 100.0F };
	float bullseyeInner_ambient[4] = { 0.929f, 0.929f, 0.929f, 1.0f };
	float bullseyeInner_diffuse[4] = { 0.929f, 0.929f, 0.929f, 1.0f };
	float bullseyeInner_specular[4] = { 0.5f, 0.5f, 0.5f, 1.0f };
	float bullseyeInner_shininess[1] = { 100.0F };

	// target radius for hit detection
	const float targetRadius = 0.2 * targetWidth; 

	// world coords for center of target (duck)
	Vector3 targetWorldCoords = Vector3(0, 0, 0);

	GLuint shaderProgram = 0;

public:
	// make constructor to allow duck's position to be set
	DuckTarget(float x = -8.0f, bool flip = false);
	void DuckTarget::draw();
	void DuckTarget::animate(bool wave);
	void DuckTarget::flip();
	bool DuckTarget::hit(Vector3 bulletCoords); 

	// used for hit detection (world coords)
	Vector3 getWorldCoords() { return targetWorldCoords; }

	// used for shaders (problem 2)
	void getShaders(GLuint shaderProgram);
};



