#include "Render.h"

#include <sstream>
#include <iostream>
#include <cmath>

#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>

#include "MyOGL.h"

#include "Camera.h"
#include "Light.h"
#include "Primitives.h"

#include "GUItextRectangle.h"

using namespace std;

bool textureMode = true;
bool lightMode = true;
bool AlphaBlending = false;

//����� ��� ��������� ������
class CustomCamera : public Camera
{
public:
	//��������� ������
	double camDist;
	//���� �������� ������
	double fi1, fi2;


	//������� ������ �� ���������
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//������� ������� ������, ������ �� ����� ��������, ���������� �������
	void SetUpCamera()
	{
		//�������� �� ������� ������ ������
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//������� ��������� ������
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //������� ������ ������


//����� ��� ��������� �����
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//��������� ������� �����
		pos = Vector3(1, 1, 3);
	}


	//������ ����� � ����� ��� ���������� �����, ���������� �������
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);


		glColor3d(0.9, 0.8, 0);
		Sphere s;
		s.pos = pos;
		s.scale = s.scale * 0.08;
		s.Show();

		if (OpenGL::isKeyPressed('G'))
		{
			glColor3d(0, 0, 0);
			//����� �� ��������� ����� �� ����������
			glBegin(GL_LINES);
			glVertex3d(pos.X(), pos.Y(), pos.Z());
			glVertex3d(pos.X(), pos.Y(), 0);
			glEnd();

			//������ ���������
			Circle c;
			c.pos.setCoords(pos.X(), pos.Y(), 0);
			c.scale = c.scale * 1.5;
			c.Show();
		}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 0 };
		GLfloat spec[] = { .7, .7, .7, 0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// ��������� ��������� �����
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// �������������� ����������� �����
		// ������� ��������� (���������� ����)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// ��������� ������������ �����
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// ��������� ���������� ������������ �����
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //������� �������� �����




//������ ���������� ����
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//������ ���� ������ ��� ������� ����� ������ ����
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}


	//������� ���� �� ���������, � ����� ��� ����
	if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k * r.direction.X() + r.origin.X();
		y = k * r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}

	if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02 * dy);
	}


}

void mouseWheelEvent(OpenGL* ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;

}

void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 'L')
	{
		lightMode = !lightMode;
	}

	if (key == 'T')
	{
		textureMode = !textureMode;
	}

	if (key == 'R')
	{
		camera.fi1 = 1;
		camera.fi2 = 1;
		camera.camDist = 15;

		light.pos = Vector3(1, 1, 3);
	}

	if (key == 'F')
	{
		light.pos = camera.pos;
	}

	if (key == 'A')
	{
		AlphaBlending = !AlphaBlending;

	}

}

void keyUpEvent(OpenGL* ogl, int key)
{

}



GLuint texId;

//����������� ����� ������ ��������
void initRender(OpenGL* ogl)
{
	//��������� �������

	//4 ����� �� �������� �������
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//��������� ������ ��������� �������
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//�������� ��������
	glEnable(GL_TEXTURE_2D);


	//������ ����������� ���������  (R G B)
	RGBTRIPLE* texarray;

	//������ ��������, (������*������*4      4, ���������   ����, �� ������� ������������ �� 4 ����� �� ������� �������� - R G B A)
	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("texture.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//���������� �� ��� ��������
	glGenTextures(1, &texId);
	//������ ��������, ��� ��� ����� ����������� � ���������, ����� ����������� �� ����� ��
	glBindTexture(GL_TEXTURE_2D, texId);

	//��������� �������� � �����������, � ���������� ��� ������  ��� �� �����
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//�������� ������
	free(texCharArray);
	free(texarray);

	//������� ����
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


	//������ � ���� ����������� � "������"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// ������������ �������� : �� ����� ����� ����� 1
	glEnable(GL_NORMALIZE);

	// ���������� ������������� ��� �����
	glEnable(GL_LINE_SMOOTH);


	//   ������ ��������� ���������
	//  �������� GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  ������� � ���������� �������� ���������(�� ���������), 
	//                1 - ������� � ���������� �������������� ������� ��������       
	//                �������������� ������� � ���������� ��������� ����������.    
	//  �������� GL_LIGHT_MODEL_AMBIENT - ������ ������� ���������, 
	//                �� ��������� �� ���������
	// �� ��������� (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}

void calculateNormal(double p1[], double p2[], double p3[]) {
	double u1 = p2[0] - p1[0];
	double u2 = p2[1] - p1[1];
	double u3 = p2[2] - p1[2];

	double v1 = p3[0] - p1[0];
	double v2 = p3[1] - p1[1];
	double v3 = p3[2] - p1[2];

	double normalX = u2 * v3 - u3 * v2;
	double normalY = u3 * v1 - u1 * v3;
	double normalZ = u1 * v2 - u2 * v1;

	double length = sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ);

	normalX /= length;
	normalY /= length;
	normalZ /= length;

	glNormal3d(normalX, normalY, normalZ);
}

void figure() {
	double A[] = { 0, 8, 0 };
	double B[] = { 7,9,0 };
	double C[] = { 6,13,0 };
	double D[] = { 9,10,0 };
	double E[] = { 16,13,0 };
	double F[] = { 13,6,0 };
	double G[] = { 8,7,0 };
	double H[] = { 4,0,0 };

	double A2[] = { 0, 8, 5 };
	double B2[] = { 7,9,5 };
	double C2[] = { 6,13,5 };
	double D2[] = { 9,10,5 };
	double E2[] = { 16,13,5 };
	double F2[] = { 13,6,5 };
	double G2[] = { 8,7,5 };
	double H2[] = { 4,0,5 };


	glBindTexture(GL_TEXTURE_2D, texId);

	////������ ��������� ������
	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glColor4d(0.1, 0.1, 0.1, 1);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(G);

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glVertex3dv(A);
	glVertex3dv(H);
	glVertex3dv(G);

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glVertex3dv(G);
	glVertex3dv(F);
	glVertex3dv(D);

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(B);

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glVertex3dv(B);
	glVertex3dv(G);
	glVertex3dv(D);

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, -1);
	glVertex3dv(D);
	glVertex3dv(E);
	glVertex3dv(F);

	glEnd();

	glBegin(GL_QUADS);
	glColor4d(0.7, 0.7, 0.7, 1);
	calculateNormal(A2, B2, B);
	glVertex3dv(A);
	glVertex3dv(B);
	glVertex3dv(B2);
	glVertex3dv(A2);

	glBegin(GL_QUADS);
	glColor4d(0.7, 0.7, 0.7, 1);
	calculateNormal(B2, C2, C);
	glVertex3dv(B);
	glVertex3dv(C);
	glVertex3dv(C2);
	glVertex3dv(B2);

	glBegin(GL_QUADS);
	glColor4d(0.7, 0.7, 0.7, 1);
	calculateNormal(C2, D2, D);
	glVertex3dv(C);
	glVertex3dv(D);
	glVertex3dv(D2);
	glVertex3dv(C2);

	glBegin(GL_QUADS);
	glColor4d(0.7, 0.7, 0.7, 1);
	calculateNormal(D2, E2, E);
	glVertex3dv(D);
	glVertex3dv(E);
	glVertex3dv(E2);
	glVertex3dv(D2);

	glBegin(GL_QUADS);
	glColor4d(0.7, 0.7, 0.7, 1);
	calculateNormal(E2, F2, F);
	glVertex3dv(E);
	glVertex3dv(F);
	glVertex3dv(F2);
	glVertex3dv(E2);

	glBegin(GL_QUADS);
	glColor4d(0.7, 0.7, 0.7, 1);
	calculateNormal(F2, G2, G);
	glVertex3dv(F);
	glVertex3dv(G);
	glVertex3dv(G2);
	glVertex3dv(F2);

	glBegin(GL_QUADS);
	glColor4d(0.7, 0.7, 0.7, 1);
	calculateNormal(G2, H2, H);
	glVertex3dv(G);
	glVertex3dv(H);
	glVertex3dv(H2);
	glVertex3dv(G2);

	glBegin(GL_QUADS);
	glColor4d(0.7, 0.7, 0.7, 1);
	calculateNormal(H2, A2, A);
	glVertex3dv(H);
	glVertex3dv(A);
	glVertex3dv(A2);
	glVertex3dv(H2);

	glEnd();
	////====================================
	////������� ��������� ������
	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glColor4d(0.2, 0.2, 0.2, 0.8);
	glTexCoord2d(0, 8/20);
	glVertex3dv(A2);
	glTexCoord2d(7 / 20, 9/20);
	glVertex3dv(B2);
	glTexCoord2d(8/20, 7/20);
	glVertex3dv(G2);

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glTexCoord2d(7 / 20, 9 / 20);
	glVertex3dv(B2);
	glTexCoord2d(13/20, 6/20);
	glVertex3dv(C2);
	glTexCoord2d(9/20, 0.5);
	glVertex3dv(D2);

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glTexCoord2d(8 / 20, 7 / 20);
	glVertex3dv(G2);
	glTexCoord2d(7 / 20, 9 / 20);
	glVertex3dv(B2);
	glTexCoord2d(9 / 20, 0.5);
	glVertex3dv(D2);

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glTexCoord2d(8 / 20, 7 / 20);
	glVertex3dv(G2);
	glTexCoord2d(9 / 20, 0.5);
	glVertex3dv(D2);
	glTexCoord2d(12/20, 6/20);
	glVertex3dv(F2);

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glTexCoord2d(9 / 20, 0.5);
	glVertex3dv(D2);
	glTexCoord2d(16/20, 13/20);
	glVertex3dv(E2);
	glTexCoord2d(12 / 20, 6 / 20);
	glVertex3dv(F2);

	glBegin(GL_TRIANGLES);
	glNormal3d(0, 0, 1);
	glTexCoord2d(0, 8 / 20);
	glVertex3dv(A2);
	glTexCoord2d(8 / 20, 7 / 20);
	glVertex3dv(G2);
	glTexCoord2d(4/20, 0);
	glVertex3dv(H2);
	glEnd();
	//=========================
	//������� ����� ������
	

	glBegin(GL_LINES);
	glColor3d(1, 1, 0);
	glVertex3d(0, 4, 5);
	glVertex3d(7, 0, 5);

	glBegin(GL_LINES);
	glColor3d(0, 1, 0);
	glVertex3d(0, 4, 5);
	glVertex3d(3, 11, 5);

	glEnd();

	glBegin(GL_LINES);
	double Z = 0;
	Z = sqrt((A2[0] - B2[0] * (A2[0] - B2[0]) + (A2[1] - B2[1]) * (A2[1] - B2[1])));
	Z = Z / 2;
	double Point[] = { (A2[0] + B2[0]) / 2.0,(A2[1] + B2[1]) / 2.0, 0 };
	glEnd();

	for (int i = 150; i < 328; i += 2) {
		double x = Point[0] + Z * cos(i * (3.14 / 180.0));
		double y = Point[1] + Z * sin(i * (3.14 / 180.0));
		double x2 = Point[0] + Z * cos((i + 5) * (3.14 / 180.0));
		double y2 = Point[1] + Z * sin((i + 5) * (3.14 / 180.0));

		//������ ���������
		glBegin(GL_TRIANGLES);
		glColor4d(0.1, 0.1, 0.1, 1);
		glNormal3d(0, 0, -1);
		glVertex3d(Point[0], Point[1], 0);
		glVertex3d(x, y, 0);
		glVertex3d(x2, y2, 0);
		glEnd();

		//������� ���������
		glBegin(GL_TRIANGLES);
		glColor4d(0.2, 0.2, 0.2, 0.2);
		glNormal3d(0, 0, 1);
		glTexCoord2d(Point[0] / 20, Point[1] / 20);
		glVertex3d(Point[0], Point[1], 5);
		glTexCoord2d(x / 20, y / 20);
		glVertex3d(x, y, 5);
		glTexCoord2d(x2 / 20, y2 / 20);
		glVertex3d(x2, y2, 5);
		glEnd();

		double A10[] = { x, y, 0 };
		double B10[] = { x2, y2, 0 };
		double C10[] = { x2, y2,5 };
		double D10[] = { x, y, 5 };

		//������� �����
		glBegin(GL_QUADS);
		glColor4d(0.7, 0.7, 0.7, 1);
		calculateNormal(A10, B10, C10);
		glVertex3dv(A10);
		glVertex3dv(B10);
		glVertex3dv(C10);
		glVertex3dv(D10);
		glEnd();
	}
}


