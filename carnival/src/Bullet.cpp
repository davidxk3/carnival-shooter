#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <ctime>

#define GLEW_STATIC
#include <GL/glew.h>
#ifdef _WIN32
#include <GL/wglew.h> // For wglSwapInterval
#endif

#define FREEGLUT_STATIC
#include <GL/freeglut.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "Bullet.h"

Bullet::Bullet() {}

void Bullet::draw() {
	glPushMatrix();
	// bullet 
	glMaterialfv(GL_FRONT, GL_AMBIENT, bullet_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, bullet_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, bullet_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, bullet_shininess);

	glTranslatef(bulletX, bulletY, bulletZ);
	glutSolidSphere(0.5f, 50, 50);
	glPopMatrix();
}


void Bullet::moveBullet() {
	// if it's passed the max distance, reset the bullet to its initial position, ex. destroy the object
	if (trajectory > maxDistance) {
	}
	else {
		trajectory += trajectoryIncrease;
		bulletZ -= trajectoryIncrease;
	}
}