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

#include "Vectors.h"
#include "CubeMesh.h"
#include "Gun.h"
#include <math.h>

Gun::Gun() {}
void Gun::draw() {
	glPushMatrix();
		// position gun to be closer to scene and slightly up
		glTranslatef(0.0, -2.0, trajectoryStart);
		// rotate up/down and left/right
		glTranslatef(gunX, gunY, 0.0);
		// rotate gun so it looks like arm streched out swiveling
		glRotatef(theta, 0.0, 1.0, 0.0);

		// rotate gun to be facing towards booth
		glRotatef(90.0, 0.0, 1.0, 0.0);
		glMaterialfv(GL_FRONT, GL_AMBIENT, gun_ambient);
		glMaterialfv(GL_FRONT, GL_SPECULAR, gun_specular);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, gun_diffuse);
		glMaterialfv(GL_FRONT, GL_SHININESS, gun_shininess);
		glPushMatrix();
			// handle 
			glScalef(1.0f, 1.5f, 1.0f);
			glutSolidCube(1.0f);
		glPopMatrix();
		glPushMatrix();
			// position barrel to be on top of handle
			glTranslatef(1.0f, 1.0f, 0.0f);
			glScalef(3.0f, 1.0f, 1.0f);
			glutSolidCube(1.0f);
		glPopMatrix();
		// bullet 
		glMaterialfv(GL_FRONT, GL_AMBIENT, bullet_ambient);
		glMaterialfv(GL_FRONT, GL_SPECULAR, bullet_specular);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, bullet_diffuse);
		glMaterialfv(GL_FRONT, GL_SHININESS, bullet_shininess);
		glPushMatrix();
			// position bullet to be in front of barrel and slightly up
			glTranslatef(3.0f, 1.0f, 0.0f);
			glutSolidSphere(0.5f, 50, 50);
		glPopMatrix();
	glPopMatrix();

	// bullet that actually gets shot 
	glPushMatrix();
		glMaterialfv(GL_FRONT, GL_AMBIENT, bullet_ambient);
		glMaterialfv(GL_FRONT, GL_SPECULAR, bullet_specular);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, bullet_diffuse);
		glMaterialfv(GL_FRONT, GL_SHININESS, bullet_shininess);
		glTranslatef(0.0, -2.0, trajectoryStart);
		glTranslatef(bulletX, bulletY, 0.0);
		glRotatef(bulletAngle, 0.0, 1.0, 0.0);
		glRotatef(90.0, 0.0, 1.0, 0.0);
		glTranslatef(3.0f, 1.0f, 0.0f);
		glTranslatef(trajectory, 0.0f, 0.0f);
		glutSolidSphere(0.5f, 50, 50);

		// Matrix to store ModelView Matrix 
		float mv[16];
		// Get the ModelView Matrix
		glGetFloatv(GL_MODELVIEW_MATRIX, mv);

		// Get x, y, and z coordinates respectively from the matrix so the bullet's position is stored as a value always in world coordinates
		bulletWorldX = mv[12];
		bulletWorldY = mv[13];
		bulletWorldZ = mv[14];

	glPopMatrix();
}

void Gun::shoot() {
	inMotion = true;
	bulletX = gunX;
	bulletY = gunY;
	bulletAngle = theta;
}

void Gun::moveBullet() {
	if (!inMotion) return;

	if (trajectory > maxDistance) {
		inMotion = false;
		trajectory = 0.0f;

		// reset bullet position to be with gun
		bulletX = gunX;
		bulletY = gunY;
		bulletAngle = theta;
		return;
	}
	trajectory += trajectoryIncrease;
}

void Gun::moveGun(float x, float y) {
	if (gunX + x <= upperX && gunX + x >= lowerX) {
		gunX += x;
		// gunX value is within interval (-3.0, 3.0) and max angles are between -30 degrees and 30 degrees
		theta = gunX * -10.0f;
	}

	if (gunY + y <= upperY && gunY + y >= lowerY) {
		gunY += y;
	}

	// if the bullet is not in motion, should move with the gun
	if (!inMotion) {
		bulletX = gunX;
		bulletY = gunY;
		bulletAngle = theta;
	}
}

void Gun::drawLaser(GLuint laserShader) {
	if (laserShader == 0) return;

	// use laser shaders
	glUseProgram(laserShader);

	glPushMatrix();
	glEnable(GL_POINT_SMOOTH);
	glPointSize(10.0f);

		// apply same translations as in gun
		glTranslatef(0.0, -2.0, trajectoryStart);
		glTranslatef(gunX, gunY, 0.0);
		glRotatef(theta, 0.0, 1.0, 0.0);
		glRotatef(90.0, 0.0, 1.0, 0.0);

		// move laser to where dot should be
		glTranslatef(20.0f, 1.0f, 0.0f); 


		glBegin(GL_POINTS);
			glVertex3f(0.0f, 0.0f, 0.0f);
		glEnd();

	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	// reset
	glUseProgram(0);
}