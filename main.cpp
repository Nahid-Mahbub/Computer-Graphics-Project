#define GL_SILENCE_DEPRECATION
#ifdef __APPLE_CC__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <cstring>  // for memset

// ─────────────────────────────────────────────────────────────────────────────
//  LIGHTING / ANIMATION TOGGLES  (unchanged)
// ─────────────────────────────────────────────────────────────────────────────
GLboolean redFlag = true, switchOne = false, switchTwo = false,
          switchLamp = false,
          amb1 = true,  diff1 = true,  spec1 = true,
          amb2 = true,  diff2 = true,  spec2 = true,
          amb3 = true,  diff3 = true,  spec3 = true;
GLboolean fanOn    = false;
GLfloat   fanAngle = 0.0f;

// ─────────────────────────────────────────────────────────────────────────────
//  WINDOW
// ─────────────────────────────────────────────────────────────────────────────
int windowWidth  = 1024;
int windowHeight = 768;

// ─────────────────────────────────────────────────────────────────────────────
//  FPS CAMERA  ← replaces old fixed eye/ref variables
// ─────────────────────────────────────────────────────────────────────────────
float camX = 3.0f, camY = 1.7f, camZ = 8.0f;

// yaw = 0  → facing –Z (into the room)
// yaw = 90 → facing +X
float yaw   =   0.0f;
float pitch =   0.0f;

const float MOUSE_SENSITIVITY = 0.15f;   // degrees per pixel
const float MOVE_SPEED        = 4.0f;    // world units per second

// Room bounds for basic collision (walls defined in room())
const float ROOM_XMIN = -1.0f,  ROOM_XMAX = 8.2f;
const float ROOM_ZMIN =  0.6f,  ROOM_ZMAX = 11.0f;
const float COLLISION_MARGIN = 0.25f;

// Key state array for smooth, frame-rate-independent movement
bool keys[256];

// Mouse centering helpers
int  centerX, centerY;
bool warpGuard = false;   // suppress the callback fired by glutWarpPointer

// Delta-time
int lastFrameTime = 0;

// ─────────────────────────────────────────────────────────────────────────────
//  PENDULUM STATE  (unchanged)
// ─────────────────────────────────────────────────────────────────────────────
double theta = 180.0, y = 1.36, z = 7.97888;

// ─────────────────────────────────────────────────────────────────────────────
//  GEOMETRY DATA  (unchanged)
// ─────────────────────────────────────────────────────────────────────────────
static GLfloat v_cube[8][3] =
{
    {0.0, 0.0, 0.0}, //0
    {0.0, 0.0, 3.0}, //1
    {3.0, 0.0, 3.0}, //2
    {3.0, 0.0, 0.0}, //3
    {0.0, 3.0, 0.0}, //4
    {0.0, 3.0, 3.0}, //5
    {3.0, 3.0, 3.0}, //6
    {3.0, 3.0, 0.0}  //7
};

static GLubyte quadIndices[6][4] =
{
    {0, 1, 2, 3}, //bottom
    {4, 5, 6, 7}, //top
    {5, 1, 2, 6}, //front
    {0, 4, 7, 3}, //back
    {2, 3, 7, 6}, //right
    {1, 5, 4, 0}  //left
};

static GLfloat v_trapezoid[8][3] =
{
    {0.0, 0.0, 0.0}, //0
    {0.0, 0.0, 3.0}, //1
    {3.0, 0.0, 3.0}, //2
    {3.0, 0.0, 0.0}, //3
    {0.5, 3.0, 0.5}, //4
    {0.5, 3.0, 2.5}, //5
    {2.5, 3.0, 2.5}, //6
    {2.5, 3.0, 0.5}  //7
};

static GLubyte TquadIndices[6][4] =
{
    {0, 1, 2, 3}, //bottom
    {4, 5, 6, 7}, //top
    {5, 1, 2, 6}, //front
    {0, 4, 7, 3}, //back
    {2, 3, 7, 6}, //right
    {1, 5, 4, 0}  //left
};

static GLfloat v_pyramid[5][3] =
{
    {0.0, 0.0, 0.0},
    {0.0, 0.0, 2.0},
    {2.0, 0.0, 2.0},
    {2.0, 0.0, 0.0},
    {1.0, 4.0, 1.0}
};

static GLubyte p_Indices[4][3] =
{
    {4, 1, 2},
    {4, 2, 3},
    {4, 3, 0},
    {4, 0, 1}
};

static GLubyte PquadIndices[1][4] =
{
    {0, 3, 2, 1}
};

// ─────────────────────────────────────────────────────────────────────────────
//  HELPERS
// ─────────────────────────────────────────────────────────────────────────────
static void getNormal3p(
    GLfloat x1,GLfloat y1,GLfloat z1,
    GLfloat x2,GLfloat y2,GLfloat z2,
    GLfloat x3,GLfloat y3,GLfloat z3)
{
    GLfloat Ux=x2-x1, Uy=y2-y1, Uz=z2-z1;
    GLfloat Vx=x3-x1, Vy=y3-y1, Vz=z3-z1;
    glNormal3f(Uy*Vz-Uz*Vy, Uz*Vx-Ux*Vz, Ux*Vy-Uy*Vx);
}

// ─────────────────────────────────────────────────────────────────────────────
//  PRIMITIVE DRAW FUNCTIONS  (all unchanged from original)
// ─────────────────────────────────────────────────────────────────────────────
void drawCube()
{
    glBegin(GL_QUADS);
    for (GLint i = 0; i < 6; i++)
    {
        getNormal3p(v_cube[quadIndices[i][0]][0], v_cube[quadIndices[i][0]][1], v_cube[quadIndices[i][0]][2],
                    v_cube[quadIndices[i][1]][0], v_cube[quadIndices[i][1]][1], v_cube[quadIndices[i][1]][2],
                    v_cube[quadIndices[i][2]][0], v_cube[quadIndices[i][2]][1], v_cube[quadIndices[i][2]][2]);
        glVertex3fv(&v_cube[quadIndices[i][0]][0]);
        glVertex3fv(&v_cube[quadIndices[i][1]][0]);
        glVertex3fv(&v_cube[quadIndices[i][2]][0]);
        glVertex3fv(&v_cube[quadIndices[i][3]][0]);
    }
    glEnd();
}

void drawCube1(GLfloat difX, GLfloat difY, GLfloat difZ,
               GLfloat ambX=0, GLfloat ambY=0, GLfloat ambZ=0, GLfloat shine=50)
{
    GLfloat no_mat[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_ambient[] = { ambX, ambY, ambZ, 1.0 };
    GLfloat mat_diffuse[] = { difX, difY, difZ, 1.0 };
    GLfloat mat_specular[]= { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[]= { shine };

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION,  no_mat);

    glBegin(GL_QUADS);
    for (GLint i = 0; i < 6; i++)
    {
        getNormal3p(v_cube[quadIndices[i][0]][0], v_cube[quadIndices[i][0]][1], v_cube[quadIndices[i][0]][2],
                    v_cube[quadIndices[i][1]][0], v_cube[quadIndices[i][1]][1], v_cube[quadIndices[i][1]][2],
                    v_cube[quadIndices[i][2]][0], v_cube[quadIndices[i][2]][1], v_cube[quadIndices[i][2]][2]);
        glVertex3fv(&v_cube[quadIndices[i][0]][0]);
        glVertex3fv(&v_cube[quadIndices[i][1]][0]);
        glVertex3fv(&v_cube[quadIndices[i][2]][0]);
        glVertex3fv(&v_cube[quadIndices[i][3]][0]);
    }
    glEnd();
}

void drawTrapezoid(GLfloat difX, GLfloat difY, GLfloat difZ,
                   GLfloat ambX, GLfloat ambY, GLfloat ambZ, GLfloat shine=50)
{
    GLfloat no_mat[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_ambient[] = { ambX, ambY, ambZ, 1.0 };
    GLfloat mat_diffuse[] = { difX, difY, difZ, 1.0 };
    GLfloat mat_specular[]= { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_emission[]= { difX, difY, difZ, 0.0 };
    GLfloat mat_shininess[]= { shine };

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    if (switchLamp)
        glMaterialfv(GL_FRONT, GL_EMISSION, mat_emission);
    else
        glMaterialfv(GL_FRONT, GL_EMISSION, no_mat);

    glBegin(GL_QUADS);
    for (GLint i = 0; i < 6; i++)
    {
        getNormal3p(v_trapezoid[TquadIndices[i][0]][0], v_trapezoid[TquadIndices[i][0]][1], v_trapezoid[TquadIndices[i][0]][2],
                    v_trapezoid[TquadIndices[i][1]][0], v_trapezoid[TquadIndices[i][1]][1], v_trapezoid[TquadIndices[i][1]][2],
                    v_trapezoid[TquadIndices[i][2]][0], v_trapezoid[TquadIndices[i][2]][1], v_trapezoid[TquadIndices[i][2]][2]);
        glVertex3fv(&v_trapezoid[TquadIndices[i][0]][0]);
        glVertex3fv(&v_trapezoid[TquadIndices[i][1]][0]);
        glVertex3fv(&v_trapezoid[TquadIndices[i][2]][0]);
        glVertex3fv(&v_trapezoid[TquadIndices[i][3]][0]);
    }
    glEnd();
}

void drawpyramid(GLfloat difX, GLfloat difY, GLfloat difZ,
                 GLfloat ambX, GLfloat ambY, GLfloat ambZ, GLfloat shine)
{
    GLfloat no_mat[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_ambient[] = { ambX, ambY, ambZ, 1.0 };
    GLfloat mat_diffuse[] = { difX, difY, difZ, 1.0 };
    GLfloat mat_specular[]= { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[]= { shine };

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glBegin(GL_TRIANGLES);
    for (GLint i = 0; i < 4; i++)
    {
        getNormal3p(v_pyramid[p_Indices[i][0]][0], v_pyramid[p_Indices[i][0]][1], v_pyramid[p_Indices[i][0]][2],
                    v_pyramid[p_Indices[i][1]][0], v_pyramid[p_Indices[i][1]][1], v_pyramid[p_Indices[i][1]][2],
                    v_pyramid[p_Indices[i][2]][0], v_pyramid[p_Indices[i][2]][1], v_pyramid[p_Indices[i][2]][2]);
        glVertex3fv(&v_pyramid[p_Indices[i][0]][0]);
        glVertex3fv(&v_pyramid[p_Indices[i][1]][0]);
        glVertex3fv(&v_pyramid[p_Indices[i][2]][0]);
    }
    glEnd();

    glBegin(GL_QUADS);
    for (GLint i = 0; i < 1; i++)
    {
        getNormal3p(v_pyramid[PquadIndices[i][0]][0], v_pyramid[PquadIndices[i][0]][1], v_pyramid[PquadIndices[i][0]][2],
                    v_pyramid[PquadIndices[i][1]][0], v_pyramid[PquadIndices[i][1]][1], v_pyramid[PquadIndices[i][1]][2],
                    v_pyramid[PquadIndices[i][2]][0], v_pyramid[PquadIndices[i][2]][1], v_pyramid[PquadIndices[i][2]][2]);
        glVertex3fv(&v_pyramid[PquadIndices[i][0]][0]);
        glVertex3fv(&v_pyramid[PquadIndices[i][1]][0]);
        glVertex3fv(&v_pyramid[PquadIndices[i][2]][0]);
        glVertex3fv(&v_pyramid[PquadIndices[i][3]][0]);
    }
    glEnd();
}

void polygon(GLfloat difX, GLfloat difY, GLfloat difZ,
             GLfloat ambX, GLfloat ambY, GLfloat ambZ, GLfloat shine)
{
    GLfloat no_mat[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_ambient[] = { ambX, ambY, ambZ, 1.0 };
    GLfloat mat_diffuse[] = { difX, difY, difZ, 1.0 };
    GLfloat mat_specular[]= { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[]= { shine };

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glBegin(GL_POLYGON);
    glVertex2f(0,0);
    glVertex2f(6,0);
    glVertex2f(5.8f,1);
    glVertex2f(5.2f,2);
    glVertex2f(5, 2.2f);
    glVertex2f(4, 2.8f);
    glVertex2f(3,3);
    glVertex2f(2, 2.8f);
    glVertex2f(1, 2.2f);
    glVertex2f(0.8f, 2);
    glVertex2f(0.2f,1);
    glEnd();
}

void polygonLine(GLfloat difX, GLfloat difY, GLfloat difZ,
                 GLfloat ambX, GLfloat ambY, GLfloat ambZ, GLfloat shine)
{
    GLfloat no_mat[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_ambient[] = { ambX, ambY, ambZ, 1.0 };
    GLfloat mat_diffuse[] = { difX, difY, difZ, 1.0 };
    GLfloat mat_specular[]= { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[]= { shine };

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    glBegin(GL_LINE_STRIP);
    glVertex2f(6,0);
    glVertex2f(5.8f,1);
    glVertex2f(5.2f,2);
    glVertex2f(5, 2.2f);
    glVertex2f(4, 2.8f);
    glVertex2f(3,3);
    glVertex2f(2, 2.8f);
    glVertex2f(1, 2.2f);
    glVertex2f(0.8f, 2);
    glVertex2f(0.2f,1);
    glVertex2f(0,0);
    glEnd();
}

void drawSphere(GLfloat difX, GLfloat difY, GLfloat difZ,
                GLfloat ambX, GLfloat ambY, GLfloat ambZ, GLfloat shine=90)
{
    GLfloat no_mat[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_ambient[] = { ambX, ambY, ambZ, 1.0 };
    GLfloat mat_diffuse[] = { difX, difY, difZ, 1.0 };
    GLfloat mat_specular[]= { 1.0, 1.0, 1.0, 1.0 };
    GLfloat mat_shininess[]= { shine };

    glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glutSolidSphere(3.0, 20, 16);
}

// Helper to set material safely
void setMaterial(const GLfloat* amb, const GLfloat* diff, const GLfloat* spec, GLfloat shininess = 30.0f)
{
    glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
    glMaterialfv(GL_FRONT, GL_SHININESS, &shininess);
}

// Draw a single blade at given rotation
void drawBlade(float angle)
{
    glPushMatrix();
    glRotatef(angle, 0, 1, 0);
    glScalef(3.0f, 0.1f, 0.45f);
    glutSolidCube(0.8f);
    glPopMatrix();
}

void drawFan()
{
    const float x = 1.5f, y = 4.0f, z = 7.5f;

    // ---------------- Downrod ----------------
    glPushMatrix();
    glTranslatef(x, 4.5f, z);

    GLfloat rod_amb[]  = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat rod_diff[] = {0.7f, 0.7f, 0.7f, 1.0f};
    GLfloat rod_spec[] = {0.3f, 0.3f, 0.3f, 1.0f};

    setMaterial(rod_amb, rod_diff, rod_spec);

    glScalef(0.08f, 0.5f, 0.08f);
    glutSolidCube(1.0f);

    glPopMatrix();

    // ---------------- Motor ----------------
    glPushMatrix();
    glTranslatef(x, 4.0f, z);

    GLfloat motor_amb[]  = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat motor_diff[] = {0.6f, 0.6f, 0.6f, 1.0f};
    GLfloat motor_spec[] = {0.2f, 0.2f, 0.2f, 1.0f};

    setMaterial(motor_amb, motor_diff, motor_spec);

    glScalef(0.6f, 0.3f, 0.6f);
    glutSolidCube(1.0f);

    glPopMatrix();

    // ---------------- Blades ----------------
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(fanAngle, 0, 1, 0);

    GLfloat blade_amb[]  = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat blade_diff[] = {0.9f, 0.9f, 0.9f, 1.0f};
    GLfloat blade_spec[] = {0.1f, 0.1f, 0.1f, 1.0f};

    setMaterial(blade_amb, blade_diff, blade_spec, 10.0f);

    drawBlade(0);
    drawBlade(90);
    drawBlade(180);
    drawBlade(270);

    glPopMatrix();

    // ---------------- Hub ----------------
    glPushMatrix();
    glTranslatef(x, y, z);

    GLfloat hub_amb[]  = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat hub_diff[] = {0.5f, 0.5f, 0.5f, 1.0f};
    GLfloat hub_spec[] = {0.1f, 0.1f, 0.1f, 1.0f};

    setMaterial(hub_amb, hub_diff, hub_spec, 5.0f);

    glutSolidSphere(0.28f, 20, 20);

    glPopMatrix();
}

bool acOn = false;
float acFlow = 0.0f;

void drawAC(float x, float y, float z, float rotY = 0.0f)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);

    auto setMat = [](float dr, float dg, float db,
                     float ar, float ag, float ab,
                     float shine = 40.0f,
                     float er = 0.0f, float eg = 0.0f, float eb = 0.0f)
    {
        GLfloat diff[] = {dr, dg, db, 1.0f};
        GLfloat amb[]  = {ar, ag, ab, 1.0f};
        GLfloat spec[] = {0.6f, 0.6f, 0.6f, 1.0f};
        GLfloat sh[]   = {shine};
        GLfloat emi[]  = {er, eg, eb, 1.0f};

        glMaterialfv(GL_FRONT, GL_DIFFUSE, diff);
        glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
        glMaterialfv(GL_FRONT, GL_SPECULAR, spec);
        glMaterialfv(GL_FRONT, GL_SHININESS, sh);
        glMaterialfv(GL_FRONT, GL_EMISSION, emi);
    };

    // ───────────────── BODY ─────────────────
    setMat(0.93f,0.93f,0.91f, 0.46f,0.46f,0.45f, 60.0f);
    glPushMatrix();
        glScalef(1.0f, 0.35f, 0.18f);
        drawCube1(0.93f,0.93f,0.91f, 0.46f,0.46f,0.45f, 60.0f);
    glPopMatrix();

    // ───────────────── FRONT PANEL ─────────────────
    setMat(0.80f,0.82f,0.85f, 0.40f,0.41f,0.42f, 30.0f);
    glPushMatrix();
        glTranslatef(0.04f, 0.22f, 0.175f);
        glScalef(0.30f, 0.08f, 0.001f);
        drawCube1(0.80f,0.82f,0.85f, 0.40f,0.41f,0.42f, 30.0f);
    glPopMatrix();

    // ───────────────── SLATS ─────────────────
    setMat(0.75f,0.78f,0.80f, 0.37f,0.39f,0.40f, 20.0f);

    for(int s = 0; s < 7; s++)
    {
        glPushMatrix();
            glTranslatef(0.05f, 0.03f + s * 0.035f, 0.176f);
            glScalef(0.30f, 0.008f, 0.001f);
            drawCube1(0.75f,0.78f,0.80f, 0.37f,0.39f,0.40f, 20.0f);
        glPopMatrix();
    }

    // ───────────────── AIR OUTLET ─────────────────
    setMat(0.70f,0.72f,0.74f, 0.35f,0.36f,0.37f, 15.0f);

    glPushMatrix();
        glTranslatef(0.05f, 0.01f, 0.172f);
        glRotatef(acOn ? -30.0f : -10.0f, 1.0f, 0.0f, 0.0f);
        glScalef(0.30f, 0.012f, 0.025f);
        drawCube1(0.70f,0.72f,0.74f, 0.35f,0.36f,0.37f, 15.0f);
    glPopMatrix();

    // ───────────────── BRAND STRIP ─────────────────
    setMat(0.20f,0.20f,0.20f, 0.10f,0.10f,0.10f, 10.0f);

    glPushMatrix();
        glTranslatef(0.05f, 0.30f, 0.177f);
        glScalef(0.17f, 0.025f, 0.001f);
        drawCube1(0.20f,0.20f,0.20f, 0.10f,0.10f,0.10f, 10.0f);
    glPopMatrix();

    // ───────────────── POWER LED ─────────────────
    glPushMatrix();
        glTranslatef(0.85f, 0.30f, 0.178f);
        glScalef(0.02f, 0.02f, 0.01f);

        if(acOn)
            setMat(0.0f,1.0f,0.0f, 0.0f,0.5f,0.0f, 80.0f, 0.0f,1.0f,0.0f);
        else
            setMat(0.0f,0.25f,0.0f, 0.0f,0.12f,0.0f, 10.0f);

        drawCube1(0.0f,
                  acOn ? 1.0f : 0.25f,
                  0.0f,
                  0.0f,
                  acOn ? 0.5f : 0.12f,
                  0.0f,
                  80.0f);
    glPopMatrix();

    // ───────────────── SIDE VENT ─────────────────
    setMat(0.85f,0.85f,0.83f, 0.42f,0.42f,0.41f, 20.0f);

    glPushMatrix();
        glTranslatef(0.0f, 0.05f, 0.03f);
        glScalef(0.001f, 0.25f, 0.12f);
        drawCube1(0.85f,0.85f,0.83f, 0.42f,0.42f,0.41f, 20.0f);
    glPopMatrix();

    glPushMatrix();
        glTranslatef(0.998f, 0.05f, 0.03f);
        glScalef(0.001f, 0.25f, 0.12f);
        drawCube1(0.85f,0.85f,0.83f, 0.42f,0.42f,0.41f, 20.0f);
    glPopMatrix();

    // ───────────────── AIR FLOW (ONLY IF ON) ─────────────────
    if(acOn)
    {
        glDisable(GL_LIGHTING);
        glColor3f(0.7f, 0.9f, 1.0f);
        glLineWidth(1.5f);

        for(int i = 0; i < 6; i++)
        {
            float offset = fmod(acFlow + i * 0.2f, 1.0f);

            glBegin(GL_LINES);
                glVertex3f(-0.4f + i * 0.15f, -0.2f - offset * 0.2f, 0.3f + offset);
                glVertex3f(-0.4f + i * 0.15f, -0.2f - offset * 0.2f, 0.6f + offset);
            glEnd();
        }

        glLineWidth(1.0f);
        glEnable(GL_LIGHTING);
    }

    glPopMatrix();
}

void drawSofa(float x, float y, float z, float rotY = 0.0f)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glRotatef(rotY, 0.0f, 1.0f, 0.0f);

    // ── Sofa colour palette ─────────────────────────────────────────────────
    // Frame / structure  – deep charcoal fabric
    const float FD[3] = { 0.22f, 0.20f, 0.20f };
    const float FA[3] = { 0.11f, 0.10f, 0.10f };
    // Cushion / upholstery – warm medium grey
    const float CD[3] = { 0.48f, 0.44f, 0.40f };
    const float CA[3] = { 0.24f, 0.22f, 0.20f };
    // Cushion accent (slightly lighter)
    const float CAc[3]= { 0.58f, 0.53f, 0.48f };
    const float CAcA[3]={ 0.29f, 0.26f, 0.24f };
    // Leg colour – dark walnut
    const float LD[3] = { 0.25f, 0.14f, 0.05f };
    const float LA[3] = { 0.12f, 0.07f, 0.02f };

    // ── 1. Seat base platform ───────────────────────────────────────────────
    //    2.1 wide × 0.28 tall × 0.9 deep
    glPushMatrix();
    glTranslatef(0.0f, 0.20f, 0.0f);
    glScalef(2.1f / 3.0f, 0.28f / 3.0f, 0.9f / 3.0f);
    drawCube1(FD[0],FD[1],FD[2],  FA[0],FA[1],FA[2],  30.0f);
    glPopMatrix();

    // ── 2. Back-rest ────────────────────────────────────────────────────────
    //    2.1 wide × 0.55 tall × 0.16 deep, sits at rear of seat
    glPushMatrix();
    glTranslatef(0.0f, 0.34f, 0.0f);
    glScalef(2.1f / 3.0f, 0.55f / 3.0f, 0.16f / 3.0f);
    drawCube1(FD[0],FD[1],FD[2],  FA[0],FA[1],FA[2],  30.0f);
    glPopMatrix();

    // ── 3. Left armrest ─────────────────────────────────────────────────────
    glPushMatrix();
    glTranslatef(0.0f, 0.20f, 0.0f);
    glScalef(0.16f / 3.0f, 0.42f / 3.0f, 0.9f / 3.0f);
    drawCube1(FD[0],FD[1],FD[2],  FA[0],FA[1],FA[2],  30.0f);
    glPopMatrix();

    // Armrest top pad (left)
    glPushMatrix();
    glTranslatef(0.0f, 0.46f, 0.0f);
    glScalef(0.16f / 3.0f, 0.08f / 3.0f, 0.9f / 3.0f);
    drawCube1(CD[0],CD[1],CD[2],  CA[0],CA[1],CA[2],  25.0f);
    glPopMatrix();

    // ── 4. Right armrest ────────────────────────────────────────────────────
    glPushMatrix();
    glTranslatef(1.94f, 0.20f, 0.0f);
    glScalef(0.16f / 3.0f, 0.42f / 3.0f, 0.9f / 3.0f);
    drawCube1(FD[0],FD[1],FD[2],  FA[0],FA[1],FA[2],  30.0f);
    glPopMatrix();

    // Armrest top pad (right)
    glPushMatrix();
    glTranslatef(1.94f, 0.46f, 0.0f);
    glScalef(0.16f / 3.0f, 0.08f / 3.0f, 0.9f / 3.0f);
    drawCube1(CD[0],CD[1],CD[2],  CA[0],CA[1],CA[2],  25.0f);
    glPopMatrix();

    // ── 5. Three seat cushions ───────────────────────────────────────────────
    //    Each ~0.56 wide × 0.14 tall × 0.72 deep
    //    Starting after left arm (x=0.16) with 0.03 gaps
    for (int s = 0; s < 3; ++s)
    {
        float cx = 0.16f + s * (0.59f);          // 0.56 wide + 0.03 gap
        glPushMatrix();
        glTranslatef(cx, 0.34f, 0.09f);
        glScalef(0.56f / 3.0f, 0.14f / 3.0f, 0.72f / 3.0f);
        drawCube1(CD[0],CD[1],CD[2],  CA[0],CA[1],CA[2],  20.0f);
        glPopMatrix();

        // Cushion piping / seam line on top (slightly lighter strip)
        glPushMatrix();
        glTranslatef(cx + 0.04f, 0.385f, 0.09f);
        glScalef(0.48f / 3.0f, 0.005f / 3.0f, 0.72f / 3.0f);
        drawCube1(CAc[0],CAc[1],CAc[2],  CAcA[0],CAcA[1],CAcA[2],  15.0f);
        glPopMatrix();
    }

    // ── 6. Three backrest cushions ──────────────────────────────────────────
    //    0.56 wide × 0.40 tall × 0.10 deep, in front of back-rest
    for (int s = 0; s < 3; ++s)
    {
        float cx = 0.16f + s * 0.59f;
        glPushMatrix();
        glTranslatef(cx, 0.38f, 0.0f);
        glScalef(0.56f / 3.0f, 0.40f / 3.0f, 0.10f / 3.0f);
        drawCube1(CD[0],CD[1],CD[2],  CA[0],CA[1],CA[2],  20.0f);
        glPopMatrix();

        // Cushion centre dividing seam (horizontal)
        glPushMatrix();
        glTranslatef(cx + 0.04f, 0.555f, 0.032f);
        glScalef(0.48f / 3.0f, 0.005f / 3.0f, 0.10f / 3.0f);
        drawCube1(CAc[0],CAc[1],CAc[2],  CAcA[0],CAcA[1],CAcA[2],  15.0f);
        glPopMatrix();
    }

    // ── 7. Four legs (tapered walnut blocks) ───────────────────────────────
    struct LegPos { float lx, lz; };
    LegPos legs[4] = {
        { 0.10f, 0.07f },   // front-left
        { 1.92f, 0.07f },   // front-right
        { 0.10f, 0.76f },   // back-left
        { 1.92f, 0.76f }    // back-right
    };
    for (int l = 0; l < 4; ++l)
    {
        glPushMatrix();
        glTranslatef(legs[l].lx, 0.0f, legs[l].lz);
        glScalef(0.08f / 3.0f, 0.22f / 3.0f, 0.08f / 3.0f);
        drawCube1(LD[0],LD[1],LD[2],  LA[0],LA[1],LA[2],  60.0f);
        glPopMatrix();
    }

    // ── 8. Decorative throw pillow (on left seat, tossed at an angle) ──────
    glPushMatrix();
    glTranslatef(0.20f, 0.43f, 0.20f);
    glRotatef(15.0f, 0.0f, 1.0f, 0.0f);   // slight yaw
    glRotatef(12.0f, 0.0f, 0.0f, 1.0f);   // leaning
    glScalef(0.26f / 3.0f, 0.26f / 3.0f, 0.08f / 3.0f);
    // Rust / terracotta accent colour
    drawCube1(0.72f,0.26f,0.10f,  0.36f,0.13f,0.05f,  20.0f);
    glPopMatrix();

    glPopMatrix(); // end Sofa
}

void cupboard(float x, float y, float z)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(0.5, 1, 0.5);
    drawCube1(0.5,0.2,0.2,  0.25, 0.1, 0.1);
    glPopMatrix();


    glPushMatrix();
    glTranslatef(x, y + 1, z + 1.5);
    glScalef(0.5, 0.01, 0.0001);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, y + 0.5, z + 1.5);
    glScalef(0.5, 0.01, 0.0001);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, y, z + 1.5);
    glScalef(0.5, 0.01, 0.0001);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 1.5, y, z + 1.5);
    glScalef(0.01, 1, 0.0001);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 0.75, y + 1, z + 1.5);
    glScalef(0.01,0.67,0.0001);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, y, z + 1.5);
    glScalef(0.01, 1, 0.0001);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();


    // handles
    glPushMatrix();
    glTranslatef(x + 1, y + 1.4, z + 1.5);
    glScalef(0.02,0.18,0.01);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 1.02, y + 1.9, z + 1.51);
    glScalef(0.02,0.02,0.01);
    drawSphere(0.2,0.1,0.1, 0.1,0.05,0.05, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 0.5, y + 1.4, z + 1.5);
    glScalef(0.02,0.18,0.01);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 0.52, y + 1.9, z + 1.51);
    glScalef(0.02,0.02,0.01);
    drawSphere(0.2,0.1,0.1, 0.1,0.05,0.05, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 0.5, y + 0.7, z + 1.5);
    glScalef(0.16,0.02,0.01);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 0.5, y + 0.25, z + 1.5);
    glScalef(0.16,0.02,0.01);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();
}

void drawIndoorPlant(float plantX, float drawerTopY, float plantZ)
{
    GLUquadric* quad = gluNewQuadric();

    GLfloat pot_amb[]  = {0.3f,0.15f,0.05f,1.0f};
    GLfloat pot_diff[] = {0.6f,0.35f,0.2f,1.0f};
    GLfloat pot_spec[] = {0.1f,0.1f,0.1f,1.0f};
    GLfloat pot_emi[]  = {0.0f,0.0f,0.0f,1.0f};
    GLfloat shine[]    = {30.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT,   pot_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   pot_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  pot_spec);
    glMaterialfv(GL_FRONT, GL_EMISSION,  pot_emi);
    glMaterialfv(GL_FRONT, GL_SHININESS, shine);

    glPushMatrix(); glTranslatef(plantX, drawerTopY, plantZ); glRotatef(-90,1,0,0); gluCylinder(quad,0.22,0.25,0.3,32,32); glPopMatrix();

    GLfloat soil_amb[]  = {0.1f,0.05f,0.02f,1.0f};
    GLfloat soil_diff[] = {0.2f,0.1f,0.05f,1.0f};
    GLfloat soil_spec[] = {0.05f,0.05f,0.05f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT,  soil_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  soil_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, soil_spec);
    glMaterialfv(GL_FRONT, GL_EMISSION, pot_emi);
    glPushMatrix(); glTranslatef(plantX, drawerTopY+0.28f, plantZ); glRotatef(-90,1,0,0); gluDisk(quad,0.0,0.22,32,1); glPopMatrix();

    GLfloat trunk_amb[]  = {0.2f,0.1f,0.05f,1.0f};
    GLfloat trunk_diff[] = {0.4f,0.25f,0.1f,1.0f};
    GLfloat trunk_spec[] = {0.1f,0.1f,0.1f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT,  trunk_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  trunk_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, trunk_spec);
    glMaterialfv(GL_FRONT, GL_EMISSION, pot_emi);
    glPushMatrix(); glTranslatef(plantX, drawerTopY+0.3f, plantZ); glRotatef(-90,1,0,0); gluCylinder(quad,0.05,0.04,0.4,16,16); glPopMatrix();

    GLfloat leaf_amb[]  = {0.0f,0.3f,0.0f,1.0f};
    GLfloat leaf_diff[] = {0.0f,0.8f,0.0f,1.0f};
    GLfloat leaf_spec[] = {0.1f,0.1f,0.1f,1.0f};
    glMaterialfv(GL_FRONT, GL_AMBIENT,  leaf_amb);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,  leaf_diff);
    glMaterialfv(GL_FRONT, GL_SPECULAR, leaf_spec);
    glMaterialfv(GL_FRONT, GL_EMISSION, pot_emi);

    srand(42);
    for (int i = 0; i < 120; i++)
    {
        float rx = ((rand()%100)/100.0f - 0.5f)*0.8f;
        float ry = ((rand()%100)/100.0f)*0.8f;
        float rz = ((rand()%100)/100.0f - 0.5f)*0.8f;
        if (sqrt(rx*rx+rz*rz) > 0.4f) continue;
        glPushMatrix();
        glTranslatef(plantX+rx, drawerTopY+0.5f+ry, plantZ+rz);
        float gv = 0.5f+(rand()%50)/100.0f;
        GLfloat vd[] = {0.0f, gv, 0.0f, 1.0f};
        glMaterialfv(GL_FRONT, GL_DIFFUSE, vd);
        glutSolidSphere(0.08f, 10, 10);
        glPopMatrix();
    }
    glMaterialfv(GL_FRONT, GL_DIFFUSE, leaf_diff);
    gluDeleteQuadric(quad);
}

void room()
{
    // right wall
    glPushMatrix(); glTranslatef(-1.5,-1,.5);  glScalef(5,2,0.1); drawCube1(1,0.8,0.7, 0.5,0.4,0.35); glPopMatrix();
    // left wall
    glPushMatrix(); glTranslatef(-4.5,-1,0);   glScalef(1,2,5);   drawCube1(1,0.8,0.7, 0.5,0.4,0.35); glPopMatrix();
    // right-side wall
    glPushMatrix(); glTranslatef(8,-1,0);      glScalef(0.2,2,5); drawCube1(1,0.8,0.7, 0.5,0.4,0.35); glPopMatrix();
    // back wall (correct opposite side)
    glPushMatrix();
    glTranslatef(-1.5, -1, 14.5);   // positive Z (far end of room)
    glScalef(5, 2, 0.1);
    drawCube1(1,0.8,0.7, 0.5,0.4,0.35);
    glPopMatrix();
    // ceiling
    glPushMatrix(); glTranslatef(-2,5.1,0);    glScalef(5,0.1,7); drawCube1(1.0,0.9,0.8, 0.5,0.45,0.4); glPopMatrix();

    // Tiled floor
    glPushMatrix();
    glScalef(5, 0.1, 7);
    glTranslatef(-1, -5, 0);
    GLfloat y_top = 3.0f;
    int tilesX=30, tilesZ=30;
    GLfloat stepX=3.0f/tilesX, stepZ=3.0f/tilesZ;
    GLfloat colorA_diff[] = {0.78f,0.57f,0.34f,1.0f}, colorA_amb[] = {0.39f,0.28f,0.17f,1.0f};
    GLfloat colorB_diff[] = {0.55f,0.35f,0.18f,1.0f}, colorB_amb[] = {0.27f,0.17f,0.09f,1.0f};
    GLfloat specular[]    = {0.2f,0.2f,0.2f,1.0f};
    GLfloat shininess[]   = {30.0f};
    for (int i=0;i<tilesX;++i) {
        for (int j=0;j<tilesZ;++j) {
            GLfloat x0=i*stepX, x1=x0+stepX, z0=j*stepZ, z1=z0+stepZ;
            if ((i+j)%2==0) { glMaterialfv(GL_FRONT,GL_AMBIENT,colorA_amb); glMaterialfv(GL_FRONT,GL_DIFFUSE,colorA_diff); }
            else             { glMaterialfv(GL_FRONT,GL_AMBIENT,colorB_amb); glMaterialfv(GL_FRONT,GL_DIFFUSE,colorB_diff); }
            glMaterialfv(GL_FRONT,GL_SPECULAR,specular);
            glMaterialfv(GL_FRONT,GL_SHININESS,shininess);
            glBegin(GL_QUADS);
            glNormal3f(0,1,0);
            glVertex3f(x0,y_top,z0); glVertex3f(x1,y_top,z0);
            glVertex3f(x1,y_top,z1); glVertex3f(x0,y_top,z1);
            glEnd();
        }
    }
    glPopMatrix();
}

float doorAngle = 0.0f;
void door(float x, float y, float z, float s)
{
    glPushMatrix();

    glTranslatef(x, y, z);
    glRotatef(90.0f, 0, 1, 0);

    glTranslatef(-0.25f * s, 0.0f, 0.0f);
    glRotatef(doorAngle, 0, 1, 0);
    glTranslatef(0.25f * s, 0.0f, 0.0f);

    // =========================
    // 🚪 MAIN DOOR BASE
    // =========================
    glPushMatrix();
    glScalef(0.5f * s, 1.2f * s, 0.15f * s);
    drawCube1(0.55, 0.30, 0.10, 0.30, 0.15, 0.05);
    glPopMatrix();

    // =========================
    // 🚪 VERTICAL WOOD PLANKS (REAL DOOR LOOK)
    // =========================
    for (int i = -2; i <= 2; i++)
    {
        glPushMatrix();
        glTranslatef(i * 0.08f * s, 0.0f, 0.07f * s);
        glScalef(0.01f * s, 1.15f * s, 0.01f * s);
        drawCube1(0.45, 0.25, 0.08, 0.20, 0.12, 0.04);
        glPopMatrix();
    }

    // =========================
    // 🚪 FRAME BORDER
    // =========================
    glPushMatrix();
    glScalef(0.52f * s, 1.25f * s, 0.02f * s);
    drawCube1(0.35, 0.18, 0.06, 0.20, 0.10, 0.03);
    glPopMatrix();

    // =========================
    // 🚪 HANDLE
    // =========================
    glPushMatrix();
    glTranslatef(0.18f * s, 0.0f, 0.10f * s);
    glScalef(0.04f * s, 0.08f * s, 0.04f * s);
    drawCube1(0.1, 0.1, 0.1, 0.05, 0.05, 0.05);
    glPopMatrix();

    glPopMatrix();
}

void openDoor()
{
    if (doorAngle < 90.0f)
        doorAngle += 20.0f;
}

void closeDoor()
{
    if (doorAngle > 0.0f)
        doorAngle -= 20.0f;
}

void bed(float x, float y, float z)
{
    glPushMatrix();

    // Move entire bed
    glTranslatef(x, y, z);

    glPushMatrix();
    glScalef(0.1,0.5,0.9);
    glTranslatef(-2,-0.5,6.2);
    drawCube1(0.5,0.2,0.2, 0.25,0.1,0.1);
    glPopMatrix();

    glPushMatrix();
    glScalef(1,0.2,0.9);
    glTranslatef(0,-0.5,6.2);
    drawCube1(0.824,0.706,0.549, 0.412,0.353,0.2745);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5,0.5,6);
    glRotatef(20,0,0,1);
    glScalef(0.1,0.15,0.28);
    drawCube1(0.627,0.322,0.176, 0.3135,0.161,0.088);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.5,0.5,7.2);
    glRotatef(22,0,0,1);
    glScalef(0.1,0.15,0.28);
    drawCube1(0.627,0.322,0.176, 0.3135,0.161,0.088);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.4,0.45,5.5);
    glScalef(0.5,0.05,0.95);
    drawCube1(0.627,0.322,0.176, 0.3135,0.161,0.088);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(1.4,-0.3,8.16);
    glScalef(0.5,0.25,0.05);
    drawCube1(0.627,0.322,0.176, 0.3135,0.161,0.088);
    glPopMatrix();

    glPopMatrix();
}

void bedsideDrawer(float x, float y, float z, float scaleX, float scaleY, float scaleZ) {

    // Main body
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(0.12 * scaleX, 0.2 * scaleY, 0.23 * scaleZ);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    // Front panel
    glPushMatrix();
    glTranslatef(x + 0.38 * scaleX, y + 0.1 * scaleY, z + 0.1 * scaleZ);
    glScalef(0.0001 * scaleX, 0.11 * scaleY, 0.18 * scaleZ);
    drawCube1(0.3,0.2,0.2, 0.15,0.1,0.1);
    glPopMatrix();

    // Handle (sphere)
    glPushMatrix();
    glTranslatef(x + 0.4 * scaleX, y + 0.25 * scaleY, z + 0.35 * scaleZ);
    glScalef(0.01 * scaleX, 0.02 * scaleY, 0.02 * scaleZ);
    drawSphere(0.3,0.1,0.0, 0.15,0.05,0.0);
    glPopMatrix();
}

void lamp(float x, float y, float z, float s)
{
    // Base
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(0.07 * s, 0.02 * s, 0.07 * s);
    drawCube1(0,0,1, 0,0,0.5);
    glPopMatrix();

    // Stand
    glPushMatrix();
    glTranslatef(x + 0.1 * s, y - 0.15 * s, z + 0.1 * s);
    glScalef(0.01 * s, 0.2 * s, 0.01 * s);
    drawCube1(1,0,0, 0.5,0,0);
    glPopMatrix();

    // Shade
    glPushMatrix();
    glTranslatef(x, y + 0.4 * s, z);
    glScalef(0.08 * s, 0.09 * s, 0.08 * s);
    drawTrapezoid(0,0,0.545, 0,0,0.2725);
    glPopMatrix();
}

void LinkinParkPoster()
{
    glPushMatrix(); glTranslatef(-1,1.4,4.6);   glScalef(0.0001,.65,.8);   drawCube1(0,0,0, 0,0,0, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.9,2.1,5.5); glScalef(0.0001,.02,.25);  drawCube1(1,1,1, 1,1,1, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.9,2.1,6.2); glRotatef(-14,1,0,0); glScalef(0.0001,.28,.02); drawCube1(1,1,1, 1,1,1, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.9,1.8,6);   glRotatef(-14,1,0,0); glScalef(0.0001,.29,.02); drawCube1(1,1,1, 1,1,1, 10); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.9,2.1,5.5); glRotatef(23,1,0,0);  glScalef(0.0001,.25,.02); drawCube1(1,1,1, 1,1,1, 10); glPopMatrix();
}

void wardrobe()
{
       //wardrobe
       glPushMatrix();
       glTranslatef(0,0,4);
       glScalef(0.12, 0.6, 0.4);
       drawCube1(0.3,0.1,0, 0.15,0.05,0);
       glPopMatrix();

       //wardrobe's 1st drawer
       glPushMatrix();
       glTranslatef(0.36,1.4,4.05);
       //glRotatef(22, 0,0,1);
       glScalef(0.0001, 0.11, 0.38);
       drawCube1(0.5,0.2,0.2, 0.25,0.1,0.1);
       glPopMatrix();

       //wardrobe's 2nd drawer
       glPushMatrix();
       glTranslatef(0.36,1,4.05);
       //glRotatef(22, 0,0,1);
       glScalef(0.0001, 0.11, 0.38);
    drawCube1(0.5,0.2,0.2, 0.25,0.1,0.1);
       glPopMatrix();

       //wardrobe's 3rd drawer
       glPushMatrix();
       glTranslatef(0.36,0.6,4.05);
       //glRotatef(22, 0,0,1);
       glScalef(0.0001, 0.11, 0.38);
    drawCube1(0.5,0.2,0.2, 0.25,0.1,0.1);
       glPopMatrix();

       //wardrobe's 4th drawer
       glPushMatrix();
       glTranslatef(0.36,0.2,4.05);
       //glRotatef(22, 0,0,1);
       glScalef(0.0001, 0.11, 0.38);
    drawCube1(0.5,0.2,0.2, 0.25,0.1,0.1);
       glPopMatrix();

       //wardrobe's 1st drawer handle
       glColor3f(0.3,0.1,0);
       glPushMatrix();
       glTranslatef(0.37,1.5,4.3);
       //glRotatef(22, 0,0,1);
       glScalef(0.01, 0.03, 0.2);
       drawCube1(0.3,0.1,0, 0.15,0.05,0.0);
       glPopMatrix();

       //wardrobe's 2nd drawer handle
       glColor3f(0.3,0.1,0);
       glPushMatrix();
       glTranslatef(0.37,1.1,4.3);
       //glRotatef(22, 0,0,1);
       glScalef(0.01, 0.03, 0.2);
    drawCube1(0.3,0.1,0, 0.15,0.05,0.0);
    glPopMatrix();

       //wardrobe's 3rd drawer handle
       glColor3f(0.3,0.1,0);
       glPushMatrix();
       glTranslatef(0.37,0.7,4.3);
       //glRotatef(22, 0,0,1);
       glScalef(0.01, 0.03, 0.2);
    drawCube1(0.3,0.1,0, 0.15,0.05,0.0);
    glPopMatrix();

       //wardrobe's 4th drawer handle
       glColor3f(0.3,0.1,0);
       glPushMatrix();
       glTranslatef(0.37,0.3,4.3);
       //glRotatef(22, 0,0,1);
       glScalef(0.01, 0.03, 0.2);
    drawCube1(0.3,0.1,0, 0.15,0.05,0.0);
    glPopMatrix();

}

void dressingTable(float x, float y, float z, float s)
{
    // legs
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(0.2 * s, 0.2 * s, 0.2 * s);
    drawCube1(0.545,0.271,0.075, 0.2725,0.1355,0.0375);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 1.1 * s, y, z);
    glScalef(0.2 * s, 0.2 * s, 0.2 * s);
    drawCube1(0.545,0.271,0.075, 0.2725,0.1355,0.0375);
    glPopMatrix();

    // table top
    glPushMatrix();
    glTranslatef(x, y + 0.6 * s, z);
    glScalef(0.57 * s, 0.1 * s, 0.2 * s);
    drawCube1(0.545,0.271,0.075, 0.2725,0.1355,0.0375);
    glPopMatrix();

    // drawer panels / inner structure
    glPushMatrix();
    glTranslatef(x, y + 0.6 * s, z + 0.6 * s);
    glScalef(0.57 * s, 0.01 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, y + 0.9 * s, z + 0.6 * s);
    glScalef(0.57 * s, 0.01 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 0.6 * s, y + 0.75 * s, z + 0.6 * s);
    glScalef(0.16 * s, 0.02 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 0.5 * s, y + 0.1 * s, z + 0.6 * s);
    glScalef(0.02 * s, 0.13 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 1.2 * s, y + 0.1 * s, z + 0.6 * s);
    glScalef(0.02 * s, 0.13 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    // mirrors
    glPushMatrix();
    glTranslatef(x + 0.3 * s, y + 0.9 * s, z + 0.1 * s);
    glScalef(0.36 * s, 0.5 * s, 0.0001 * s);
    drawCube1(0.690,0.878,0.902, 0.345,0.439,0.451, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, y + 0.9 * s, z + 0.1 * s);
    glScalef(0.1 * s, 0.48 * s, 0.0001 * s);
    drawCube1(0.690,0.878,0.902, 0.345,0.439,0.451, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 1.3 * s, y + 0.9 * s, z + 0.1 * s);
    glScalef(0.1 * s, 0.48 * s, 0.0001 * s);
    drawCube1(0.690,0.878,0.902, 0.345,0.439,0.451, 10);
    glPopMatrix();

    // mirror frame lines
    glPushMatrix();
    glTranslatef(x, y + 0.9 * s, z + 0.11 * s);
    glScalef(0.019 * s, 0.48 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 0.25 * s, y + 0.9 * s, z + 0.11 * s);
    glScalef(0.019 * s, 0.48 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, y + 0.9 * s, z + 0.11 * s);
    glScalef(0.55 * s, 0.019 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x, y + 2.3 * s, z + 0.11 * s);
    glScalef(0.1 * s, 0.019 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 1.3 * s, y + 2.3 * s, z + 0.11 * s);
    glScalef(0.1 * s, 0.019 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 1.3 * s, y + 0.9 * s, z + 0.11 * s);
    glScalef(0.019 * s, 0.48 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 1.5 * s, y + 0.9 * s, z + 0.11 * s);
    glScalef(0.019 * s, 0.48 * s, 0.0001 * s);
    drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05);
    glPopMatrix();

    // top decoration
    glPushMatrix();
    glTranslatef(x + 0.3 * s, y + 2.4 * s, z + 0.1 * s);
    glScalef(0.18 * s, 0.18 * s, 2 * s);
    polygon(0.690,0.878,0.902, 0.345,0.439,0.451, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(x + 0.3 * s, y + 2.4 * s, z + 0.11 * s);
    glScalef(0.18 * s, 0.18 * s, 1 * s);
    polygonLine(0.2,0.1,0.1, 0.1,0.05,0.05, 50);
    glPopMatrix();
}

void wallshelf(float x, float y, float z)
{
    glPushMatrix();

    // =========================
    // SHELVES
    // =========================
    glPushMatrix(); glTranslatef(x + 1.5, y + 2.7, z); glScalef(0.4,0.03,0.2); drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05); glPopMatrix();
    glPushMatrix(); glTranslatef(x + 1.0, y + 2.3, z); glScalef(0.4,0.03,0.2); drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05); glPopMatrix();
    glPushMatrix(); glTranslatef(x + 0.5, y + 1.9, z); glScalef(0.4,0.03,0.2); drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05); glPopMatrix();
    glPushMatrix(); glTranslatef(x + 1.0, y + 1.5, z); glScalef(0.4,0.03,0.2); drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05); glPopMatrix();
    glPushMatrix(); glTranslatef(x + 1.5, y + 1.1, z); glScalef(0.4,0.03,0.2); drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05); glPopMatrix();


    // =========================
    // SHOWPIECES
    // =========================
    glPushMatrix(); glTranslatef(x + 1.5, y + 1.2, z); glScalef(0.04,0.06,0.2); drawCube1(0.698,0.133,0.133, 0.349,0.0665,0.0665); glPopMatrix();
    glPushMatrix(); glTranslatef(x + 2.0, y + 1.2, z); glScalef(0.04,0.06,0.2); drawCube1(0.729,0.333,0.827, 0.3645,0.1665,0.4135); glPopMatrix();
    glPushMatrix(); glTranslatef(x + 2.5, y + 1.2, z); glScalef(0.04,0.06,0.2); drawCube1(0.098,0.098,0.439, 0.049,0.049,0.2195); glPopMatrix();

    glPushMatrix(); glTranslatef(x + 2.51, y + 1.35, z); glScalef(0.01,0.05,0.2); drawCube1(0.529,0.808,0.980, 0.2645,0.404,0.490); glPopMatrix();
    glPushMatrix(); glTranslatef(x + 2.5, y + 2.71, z); glScalef(0.05,0.16,0.01); drawCube1(0.502,0.502,0, 0.251,0.251,0); glPopMatrix();

    glPushMatrix(); glTranslatef(x + 1.8, y + 2.71, z); glScalef(0.16,0.1,0.01); drawCube1(0,0,0.9, 0,0,0.45); glPopMatrix();
    glPushMatrix(); glTranslatef(x + 1.3, y + 2.4, z); glScalef(0.16,0.08,0.01); drawCube1(0.416,0.353,0.804, 0.208,0.1765,0.402); glPopMatrix();

    glPushMatrix(); glTranslatef(x + 0.4, y + 1.9, z); glScalef(0.05,0.16,0.01); drawCube1(0.863,0.078,0.235, 0.4315,0.039,0.1175); glPopMatrix();
    glPushMatrix(); glTranslatef(x + 0.7, y + 1.9, z); glScalef(0.05,0.12,0.01); drawCube1(0.780,0.082,0.522, 0.39,0.041,0.261); glPopMatrix();
    glPushMatrix(); glTranslatef(x + 1.0, y + 1.9, z); glScalef(0.05,0.09,0.01); drawCube1(0.6,0.196,0.8, 0.3,0.098,0.4); glPopMatrix();


    // =========================
    // PYRAMIDS
    // =========================
    glPushMatrix(); glTranslatef(x + 1.8, y + 1.5, z); glScalef(0.2,0.1,0.2); drawpyramid(0.282,0.239,0.545, 0.141,0.1195,0.2725, 50); glPopMatrix();
    glPushMatrix(); glTranslatef(x + 1.4, y + 1.5, z); glScalef(0.15,0.1,0.2); drawpyramid(0.251,0.878,0.816, 0.1255,0.439,0.408, 50); glPopMatrix();

    glPopMatrix();
}

void Clock()
{
    glPushMatrix(); glTranslatef(-0.9,1.8,7.87); glScalef(0.08,0.25,0.1); drawCube1(0.545,0.271,0.075, 0.271,0.1335,0.0375, 50); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.83,1.9,7.9); glScalef(0.06,0.2,0.08); drawCube1(1.000,0.894,0.710, 1.000,0.894,0.710); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.65,2.18,8.01); glRotatef(45,1,0,0);  glScalef(0.0001,0.01,0.04); drawCube1(0,0,0, 0,0,0); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.65,2.18,8.01); glRotatef(90,1,0,0);  glScalef(0.0001,0.012,0.08); drawCube1(0,0,0, 0,0,0); glPopMatrix();

    // pendulum
    glPushMatrix(); glTranslatef(-0.7,2,8.1); glRotatef(theta,1,0,0); glScalef(0.0001,0.2,0.03); drawCube1(0.2,0.1,0.1, 0.1,0.05,0.05); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.72,1.42,z); glScalef(0.035,0.035,0.035); drawSphere(0.2,0.1,0.1, 0.1,0.05,0.05, 10); glPopMatrix();

    // top pyramid
    glPushMatrix(); glTranslatef(-0.9,2.5,7.81); glScalef(0.16,0.1,0.2); drawpyramid(0.5,0.2,0, 0.25,0.1,0, 50); glPopMatrix();
}

void window()
{
    glPushMatrix(); glTranslatef(-0.9,1,8.9);  glScalef(0.0001,.6,.3);    drawCube1(1.0,1.0,1.0, 0.05,0.05,0.05); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.9,1,8.9);  glScalef(0.04,0.6,0.0001); drawCube1(0.8,0.6,0.4, 0.4,0.3,0.2);   glPopMatrix();
    glPushMatrix(); glTranslatef(-0.9,1,9.8);  glScalef(0.04,0.6,0.0001); drawCube1(0.8,0.6,0.4, 0.4,0.3,0.2);   glPopMatrix();
    glPushMatrix(); glTranslatef(-0.7,2.7,8.9);glScalef(0.0001,0.05,0.4); drawCube1(0.7,0.6,0.5, 0.35,0.3,0.25); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.8,1.02,8.9);glScalef(0.0001,0.02,0.34); drawCube1(0.7,0.6,0.5, 0.35,0.3,0.25); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.87,2.1,8.9); glScalef(0.0001,0.02,0.3); drawCube1(0,0,0, 0,0,0, 5); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.87,1.6,8.9); glScalef(0.0001,0.02,0.3); drawCube1(0,0,0, 0,0,0, 5); glPopMatrix();
    glPushMatrix(); glTranslatef(-0.87,1,9.3);   glScalef(0.0001,0.6,0.02); drawCube1(0,0,0, 0,0,0, 5); glPopMatrix();
}

void lightBulb1()
{
    GLfloat no_mat[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_diffuse[] = { 1.000, 0.843, 0.000, 1.0 };
    GLfloat high_shininess[] = { 100.0 };
    GLfloat mat_emission[]   = { 1.0, 1.0, 1.0, 0.0 };
    glPushMatrix();
    glTranslatef(5, 5, 8); glScalef(0.2, 0.2, 0.2);
    glMaterialfv(GL_FRONT, GL_AMBIENT,   no_mat);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  no_mat);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION,  switchOne ? mat_emission : no_mat);
    glutSolidSphere(1.0, 16, 16);
    glPopMatrix();
}

void lightBulb2()
{
    GLfloat no_mat[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat mat_diffuse[] = { 1.000, 0.843, 0.000, 1.0 };
    GLfloat high_shininess[] = { 100.0 };
    GLfloat mat_emission[]   = { 1.0, 1.0, 1.0, 1.0 };
    glPushMatrix();
    glTranslatef(0, 5, 8); glScalef(0.2, 0.2, 0.2);
    glMaterialfv(GL_FRONT, GL_AMBIENT,   no_mat);
    glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR,  no_mat);
    glMaterialfv(GL_FRONT, GL_SHININESS, high_shininess);
    glMaterialfv(GL_FRONT, GL_EMISSION,  switchTwo ? mat_emission : no_mat);
    glutSolidSphere(1.0, 16, 16);
    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────────────────────
//  LIGHTS  (unchanged)
// ─────────────────────────────────────────────────────────────────────────────
void lightOne()
{
    glPushMatrix();
    GLfloat no_light[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[]= { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[]= { 5.0, 5.0, 8.0, 1.0 };
    glLightfv(GL_LIGHT0, GL_AMBIENT,  amb1  ? light_ambient  : no_light);
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  diff1 ? light_diffuse  : no_light);
    glLightfv(GL_LIGHT0, GL_SPECULAR, spec1 ? light_specular : no_light);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glPopMatrix();
}

void lightTwo()
{
    glPushMatrix();
    GLfloat no_light[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light_ambient[] = { 0.05, 0.05, 0.05, 1.0 };
    GLfloat light_diffuse[] = { 0.1, 0.1, 0.09, 1.0 };
    GLfloat light_specular[]= { 0.05, 0.05, 0.05, 1.0 };
    GLfloat light_position[]= { 0.0, 5.0, 8.0, 1.0 };
    glLightfv(GL_LIGHT1, GL_AMBIENT,  amb2  ? light_ambient  : no_light);
    glLightfv(GL_LIGHT1, GL_DIFFUSE,  diff2 ? light_diffuse  : no_light);
    glLightfv(GL_LIGHT1, GL_SPECULAR, spec2 ? light_specular : no_light);
    glLightfv(GL_LIGHT1, GL_POSITION, light_position);
    glPopMatrix();
}

void lampLight()
{
    glPushMatrix();
    GLfloat no_light[]      = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[]= { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[]= { 0.7, 1.5, 9.0, 1.0 };
    glLightfv(GL_LIGHT2, GL_AMBIENT,  amb3  ? light_ambient  : no_light);
    glLightfv(GL_LIGHT2, GL_DIFFUSE,  diff3 ? light_diffuse  : no_light);
    glLightfv(GL_LIGHT2, GL_SPECULAR, spec3 ? light_specular : no_light);
    glLightfv(GL_LIGHT2, GL_POSITION, light_position);
    GLfloat spot_dir[] = { 0.3f, -1.0f, -0.8f };
    glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, spot_dir);
    glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 35.0f);
    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────────────────────
//  DISPLAY  ← updated with FPS camera
// ─────────────────────────────────────────────────────────────────────────────
void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // ── Projection ──────────────────────────────────────────────────────────
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = (float)windowWidth / (float)windowHeight;
    gluPerspective(70.0, aspect, 0.05, 200.0);

    // ── FPS Camera via gluLookAt ─────────────────────────────────────────────
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float radYaw   = yaw   * (float)M_PI / 180.0f;
    float radPitch = pitch * (float)M_PI / 180.0f;

    // Forward direction vector from yaw/pitch
    // Convention: yaw=0 → looking along -Z (into the room)
    float lookDirX = cosf(radPitch) * sinf(radYaw);
    float lookDirY = sinf(radPitch);
    float lookDirZ = cosf(radPitch) * (-cosf(radYaw));

    float lookAtX = camX + lookDirX;
    float lookAtY = camY + lookDirY;
    float lookAtZ = camZ + lookDirZ;

    gluLookAt(camX, camY, camZ,        // eye
              lookAtX, lookAtY, lookAtZ, // center
              0.0, 1.0, 0.0);           // up

    // ── Lighting ─────────────────────────────────────────────────────────────
    glEnable(GL_LIGHTING);
    lightOne();
    lightTwo();
    lampLight();

    // ── Scene objects (all unchanged) ─────────────────────────────────────────
    room();
    door(7.95f, -1.0f, 12.0f, 0.9f);
    bed(-0.5f, 0.0f, -1.0f);
    bedsideDrawer(-0.7, -0.1, 7.7, 1.0, 1.0, 1.0);
    lamp(-0.7, 0.5, 7.95, 1.0);
    drawIndoorPlant(3.1f, 0.0f, 1.85f);
    drawIndoorPlant(0.1f, 0.0f, 1.85f);
    drawIndoorPlant(-0.3f, 0.0f, 8.85f);
    LinkinParkPoster();
    wallshelf(-0.5 , 0, 0.8);
    //wardrobe();
    cupboard(4, 0, 1.0);
    dressingTable(5.9, -0.1, 1.0, 1.0);
    Clock();
    window();
    lightBulb1();
    lightBulb2();
    drawFan();
    drawAC(7.9f, 2.8f, 6.0f, 270.0f);
    drawSofa(0.5f, 0.0f, 12.5f, 90.0f);

    glDisable(GL_LIGHTING);

    // ── HUD: controls reminder (drawn in 2D over the scene) ──────────────────
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glColor3f(1.0f, 1.0f, 0.8f);

    // Crosshair
    int cx = windowWidth/2, cy = windowHeight/2;
    glBegin(GL_LINES);
    glVertex2i(cx-8, cy); glVertex2i(cx+8, cy);
    glVertex2i(cx, cy-8); glVertex2i(cx, cy+8);
    glEnd();

    glEnable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

    glutSwapBuffers();
}

// ─────────────────────────────────────────────────────────────────────────────
//  MOUSE MOTION  ← FPS free-look with cursor recentering
// ─────────────────────────────────────────────────────────────────────────────

bool mouseLocked = true;
void unlockMouse()
{
    mouseLocked = false;
}
void lockMouse()
{
    mouseLocked = true;
    warpGuard = true;
    glutWarpPointer(centerX, centerY);
}


void mouseMotion(int x, int y)
{
    if (!mouseLocked) return;

    if (warpGuard) {
        warpGuard = false;
        return;
    }

    int dx = x - centerX;
    int dy = y - centerY;

    yaw   += dx * MOUSE_SENSITIVITY;
    pitch -= dy * MOUSE_SENSITIVITY;

    if (pitch >  89.0f) pitch =  89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    if (yaw >= 360.0f) yaw -= 360.0f;
    if (yaw < 0.0f) yaw += 360.0f;

    warpGuard = true;
    glutWarpPointer(centerX, centerY);

    glutPostRedisplay();
}

// ─────────────────────────────────────────────────────────────────────────────
//  RESHAPE
// ─────────────────────────────────────────────────────────────────────────────
void reshape(int w, int h)
{
    if (h == 0) h = 1;
    windowWidth  = w;
    windowHeight = h;
    centerX = w / 2;
    centerY = h / 2;
    glViewport(0, 0, w, h);
    glutWarpPointer(centerX, centerY);
}

// ─────────────────────────────────────────────────────────────────────────────
//  KEYBOARD  ← WASD added, lights/fan/escape preserved
// ─────────────────────────────────────────────────────────────────────────────
void myKeyboardFunc(unsigned char key, int x, int y)
{
    keys[key] = true;  // mark key as held

    switch (key)
    {
        // Lights
        case '1':
            switchOne = !switchOne;
            if (switchOne) { amb1=diff1=spec1=true;  glEnable(GL_LIGHT0); }
            else           { amb1=diff1=spec1=false; glDisable(GL_LIGHT0); }
            break;

        case 'l': case 'L':
            switchTwo = !switchTwo;
            if (switchTwo) { amb2=diff2=spec2=true;  glEnable(GL_LIGHT1); }
            else           { amb2=diff2=spec2=false; glDisable(GL_LIGHT1); }
            break;

        case '2':
            switchLamp = !switchLamp;
            if (switchLamp) { amb3=diff3=spec3=true;  glEnable(GL_LIGHT2); }
            else            { amb3=diff3=spec3=false; glDisable(GL_LIGHT2); }
            break;

        // Fan
        case '3':
            fanOn = !fanOn;
            break;

        // 🚪 Door control added here
        case 'o':
            openDoor();
            break;

        case 'c':
            closeDoor();
            break;

        case '4':
            acOn = !acOn;
            break;

        case 'm':
            mouseLocked = !mouseLocked;

            if (mouseLocked)
                lockMouse();
            glutPostRedisplay();
            break;

        // ESC
        case 27:
            exit(0);
    }

    glutPostRedisplay();
}

void myKeyboardUpFunc(unsigned char key, int x, int y)
{
    keys[key] = false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  GAME LOOP  ← WASD movement + delta-time + existing animations
// ─────────────────────────────────────────────────────────────────────────────
void animate()
{
    // ── Delta time ────────────────────────────────────────────────────────────
    int now      = glutGet(GLUT_ELAPSED_TIME);
    float dt     = (now - lastFrameTime) / 1000.0f;
    lastFrameTime = now;
    if (dt > 0.1f) dt = 0.1f;   // clamp for safety (e.g. window minimised)

    // ── WASD movement (camera-relative, horizontal plane only) ────────────────
    float radYaw = yaw * (float)M_PI / 180.0f;

    // Forward direction projected onto XZ plane (sin/cos from our convention)
    float fwdX =  sinf(radYaw);
    float fwdZ = -cosf(radYaw);

    // Right direction: rotate forward 90° clockwise around Y
    // right = cross(forward, up) — see README comment above display()
    float rgtX =  cosf(radYaw);
    float rgtZ =  sinf(radYaw);

    float speed = MOVE_SPEED * dt;

    float nx = camX, nz = camZ;

    if (keys['w'] || keys['W']) { nx += fwdX * speed; nz += fwdZ * speed; }
    if (keys['s'] || keys['S']) { nx -= fwdX * speed; nz -= fwdZ * speed; }
    if (keys['d'] || keys['D']) { nx += rgtX * speed; nz += rgtZ * speed; }
    if (keys['a'] || keys['A']) { nx -= rgtX * speed; nz -= rgtZ * speed; }

    // ── Basic collision: keep inside room bounds ───────────────────────────────
    float m = COLLISION_MARGIN;
    if (nx < ROOM_XMIN + m) nx = ROOM_XMIN + m;
    if (nx > ROOM_XMAX - m) nx = ROOM_XMAX - m;
    if (nz < ROOM_ZMIN + m) nz = ROOM_ZMIN + m;
    if (nz > ROOM_ZMAX - m) nz = ROOM_ZMAX - m;

    camX = nx;
    camZ = nz;

    // ── Fan rotation ──────────────────────────────────────────────────────────
    if (fanOn) {
        fanAngle += 200.0f * dt;
        if (fanAngle >= 360.0f) fanAngle -= 360.0f;
    }

    // ── Pendulum (clock) ──────────────────────────────────────────────────────
    if (redFlag) {
        theta += 0.2;
        z     -= 0.002;
        if (theta >= 210.0) redFlag = false;
        if      (theta >= 196 && theta <= 210) y = 1.44;
        else if (theta >= 180 && theta <= 194) y = 1.42;
        else if (theta >= 164 && theta <= 178) y = 1.42;
    } else {
        theta -= 0.2;
        z     += 0.002;
        if (theta <= 150.0) redFlag = true;
        if      (theta >= 196 && theta <= 210) y = 1.44;
        else if (theta >= 180 && theta <= 194) y = 1.42;
        else if (theta >= 164 && theta <= 178) y = 1.42;
    }

    glutPostRedisplay();
}

// ─────────────────────────────────────────────────────────────────────────────
//  MAIN
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char** argv)
{
    glutInit(&argc, argv);

glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
glutInitWindowSize(windowWidth, windowHeight);
glutInitWindowPosition(100, 100);
glutCreateWindow("FPS Bedroom – WASD + Mouse Look");

std::cout << "FPS BEDROOM – Controls\n";
std::cout << "W/A/S/D : Move\n";
std::cout << "Mouse   : Look around\n";
std::cout << "1       : Toggle ceiling light 1\n";
std::cout << "L       : Toggle ceiling light 2\n";
std::cout << "2       : Toggle lamp light\n";
std::cout << "3       : Toggle ceiling fan\n";
std::cout << "ESC     : Quit\n";

glShadeModel(GL_SMOOTH);
glEnable(GL_DEPTH_TEST);
glEnable(GL_NORMALIZE);
glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

memset(keys, 0, sizeof(keys));

lastFrameTime = glutGet(GLUT_ELAPSED_TIME);

glutDisplayFunc(display);
glutReshapeFunc(reshape);
glutKeyboardFunc(myKeyboardFunc);
glutKeyboardUpFunc(myKeyboardUpFunc);
glutPassiveMotionFunc(mouseMotion);
glutMotionFunc(mouseMotion);
glutIdleFunc(animate);

glutSetCursor(GLUT_CURSOR_NONE);

centerX = windowWidth / 2;
centerY = windowHeight / 2;
glutWarpPointer(centerX, centerY);

glutMainLoop();

return 0;
}
