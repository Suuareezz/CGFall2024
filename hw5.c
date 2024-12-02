// architectural_system.c
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#ifdef USEGLEW
#include <GL/glew.h>
#endif
#define GL_GLEXT_PROTOTYPES
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include <math.h>

// Building parameters
float buildingWidth = 0;
float buildingLength = 0;
float buildingHeight = 0;
int numFloors = 0;
float floorHeight = 3.0f;
float windowWidth = 1.2f;
float windowHeight = 1.8f;
float windowSpacing = 3.0f;
float roofHeight = 2.0f;
bool showFrontWall = true;
bool showBackWall = true;
bool showLeftWall = true;
bool showRightWall = true;
bool showAllWalls = true;

// Camera parameters
float cameraDistance = 50.0f;
float cameraAngleX = 0.0f;
float cameraAngleY = 0.0f;

// Materials
typedef struct {
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float shininess;
} Material;

Material materials[] = {
    // Concrete (lighter gray)
    {{0.6f, 0.6f, 0.6f, 1.0f}, 
     {0.8f, 0.8f, 0.8f, 1.0f}, 
     {0.2f, 0.2f, 0.2f, 1.0f}, 
     10.0f},
    // Brick (more vibrant)
    {{0.45f, 0.25f, 0.15f, 1.0f}, 
     {0.75f, 0.35f, 0.25f, 1.0f}, 
     {0.1f, 0.1f, 0.1f, 1.0f}, 
     5.0f},
    // Glass
    {{0.2f, 0.3f, 0.4f, 0.6f}, 
     {0.4f, 0.5f, 0.6f, 0.6f}, 
     {0.9f, 0.9f, 0.9f, 1.0f}, 
     96.0f},
    // Roof material (darker, more visible)
    {{0.3f, 0.3f, 0.4f, 1.0f}, 
     {0.4f, 0.4f, 0.5f, 1.0f}, 
     {0.2f, 0.2f, 0.2f, 1.0f}, 
     15.0f}
};

int currentMaterial = 0;
bool showWindows = true;
bool showRoof = true;

// Mouse interaction
bool mouseLeftDown = false;
bool mouseRightDown = false;
int mouseX = 0, mouseY = 0;

void applyMaterial(Material* mat) {
    glMaterialfv(GL_FRONT, GL_AMBIENT, mat->ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat->diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat->specular);
    glMaterialf(GL_FRONT, GL_SHININESS, mat->shininess);
}

void drawWindow() {
    applyMaterial(&materials[2]); // Glass material
    glBegin(GL_QUADS);
    glVertex3f(-windowWidth/2, 0, 0.01f);
    glVertex3f(windowWidth/2, 0, 0.01f);
    glVertex3f(windowWidth/2, windowHeight, 0.01f);
    glVertex3f(-windowWidth/2, windowHeight, 0.01f);
    glEnd();
}

void drawRoof() {
    if (!showRoof) return;
    
    applyMaterial(&materials[3]); // Use roof-specific material
    
    glBegin(GL_TRIANGLES);
    // Front face
    glNormal3f(0.0f, 0.5f, 1.0f);
    glVertex3f(-buildingWidth/2, 0, buildingLength/2);
    glVertex3f(buildingWidth/2, 0, buildingLength/2);
    glVertex3f(0, roofHeight, 0);
    
    // Back face
    glNormal3f(0.0f, 0.5f, -1.0f);
    glVertex3f(-buildingWidth/2, 0, -buildingLength/2);
    glVertex3f(buildingWidth/2, 0, -buildingLength/2);
    glVertex3f(0, roofHeight, 0);
    
    // Left face
    glNormal3f(-1.0f, 0.5f, 0.0f);
    glVertex3f(-buildingWidth/2, 0, -buildingLength/2);
    glVertex3f(-buildingWidth/2, 0, buildingLength/2);
    glVertex3f(0, roofHeight, 0);
    
    // Right face
    glNormal3f(1.0f, 0.5f, 0.0f);
    glVertex3f(buildingWidth/2, 0, -buildingLength/2);
    glVertex3f(buildingWidth/2, 0, buildingLength/2);
    glVertex3f(0, roofHeight, 0);
    glEnd();
}

void drawFloor(float y) {
    applyMaterial(&materials[currentMaterial]);
    
    glPushMatrix();
    glTranslatef(0, y, 0);
    
    glBegin(GL_QUADS);
    // Draw walls based on toggles
    if (showFrontWall && showAllWalls) {
        // Front wall
        glNormal3f(0, 0, 1);
        glVertex3f(-buildingWidth/2, 0, buildingLength/2);
        glVertex3f(buildingWidth/2, 0, buildingLength/2);
        glVertex3f(buildingWidth/2, floorHeight, buildingLength/2);
        glVertex3f(-buildingWidth/2, floorHeight, buildingLength/2);
    }
    
    if (showBackWall && showAllWalls) {
        // Back wall
        glNormal3f(0, 0, -1);
        glVertex3f(-buildingWidth/2, 0, -buildingLength/2);
        glVertex3f(buildingWidth/2, 0, -buildingLength/2);
        glVertex3f(buildingWidth/2, floorHeight, -buildingLength/2);
        glVertex3f(-buildingWidth/2, floorHeight, -buildingLength/2);
    }
    
    if (showLeftWall && showAllWalls) {
        // Left wall
        glNormal3f(-1, 0, 0);
        glVertex3f(-buildingWidth/2, 0, -buildingLength/2);
        glVertex3f(-buildingWidth/2, 0, buildingLength/2);
        glVertex3f(-buildingWidth/2, floorHeight, buildingLength/2);
        glVertex3f(-buildingWidth/2, floorHeight, -buildingLength/2);
    }
    
    if (showRightWall && showAllWalls) {
        // Right wall
        glNormal3f(1, 0, 0);
        glVertex3f(buildingWidth/2, 0, -buildingLength/2);
        glVertex3f(buildingWidth/2, 0, buildingLength/2);
        glVertex3f(buildingWidth/2, floorHeight, buildingLength/2);
        glVertex3f(buildingWidth/2, floorHeight, -buildingLength/2);
    }
    
    // Floor and ceiling are always shown
    glNormal3f(0, -1, 0);
    glVertex3f(-buildingWidth/2, 0, -buildingLength/2);
    glVertex3f(buildingWidth/2, 0, -buildingLength/2);
    glVertex3f(buildingWidth/2, 0, buildingLength/2);
    glVertex3f(-buildingWidth/2, 0, buildingLength/2);
    
    glNormal3f(0, 1, 0);
    glVertex3f(-buildingWidth/2, floorHeight, -buildingLength/2);
    glVertex3f(buildingWidth/2, floorHeight, -buildingLength/2);
    glVertex3f(buildingWidth/2, floorHeight, buildingLength/2);
    glVertex3f(-buildingWidth/2, floorHeight, buildingLength/2);
    glEnd();
    
    // Draw windows if walls are visible
    if (showWindows) {
        float xStart = -buildingWidth/2 + windowSpacing;
        float xEnd = buildingWidth/2 - windowSpacing;
        float windowY = (floorHeight - windowHeight)/2;
        
        if (showFrontWall && showAllWalls) {
            // Front windows
            for (float x = xStart; x <= xEnd; x += windowSpacing + windowWidth) {
                glPushMatrix();
                glTranslatef(x, windowY, buildingLength/2);
                drawWindow();
                glPopMatrix();
            }
        }
        
        if (showBackWall && showAllWalls) {
            // Back windows
            for (float x = xStart; x <= xEnd; x += windowSpacing + windowWidth) {
                glPushMatrix();
                glTranslatef(x, windowY, -buildingLength/2);
                glRotatef(180, 0, 1, 0);
                drawWindow();
                glPopMatrix();
            }
        }
        
        float zStart = -buildingLength/2 + windowSpacing;
        float zEnd = buildingLength/2 - windowSpacing;
        
        if (showLeftWall && showAllWalls) {
            // Left side windows
            for (float z = zStart; z <= zEnd; z += windowSpacing + windowWidth) {
                glPushMatrix();
                glTranslatef(-buildingWidth/2, windowY, z);
                glRotatef(90, 0, 1, 0);
                drawWindow();
                glPopMatrix();
            }
        }
        
        if (showRightWall && showAllWalls) {
            // Right side windows
            for (float z = zStart; z <= zEnd; z += windowSpacing + windowWidth) {
                glPushMatrix();
                glTranslatef(buildingWidth/2, windowY, z);
                glRotatef(-90, 0, 1, 0);
                drawWindow();
                glPopMatrix();
            }
        }
    }
    
    glPopMatrix();
}

void drawBuilding() {
    float y = 0;
    
    // Draw floors
    for(int floor = 0; floor < numFloors; floor++) {
        drawFloor(y);
        y += floorHeight;
    }
    
    // Draw roof
    glPushMatrix();
    glTranslatef(0, y, 0);
    drawRoof();
    glPopMatrix();
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();
    float camX = cameraDistance * sin(cameraAngleY) * cos(cameraAngleX);
    float camY = cameraDistance * sin(cameraAngleX) + (buildingHeight / 2.0f); // Center vertically on building
    float camZ = cameraDistance * cos(cameraAngleY) * cos(cameraAngleX);
    
    // Look at the center of the building
    gluLookAt(camX, camY, camZ,
              0, buildingHeight / 2.0f, 0,  // Look at center of building
              0, 1, 0);
    // Set camera position
    /* gluLookAt(cameraDistance * sin(cameraAngleY) * cos(cameraAngleX),
              cameraDistance * sin(cameraAngleX),
              cameraDistance * cos(cameraAngleY) * cos(cameraAngleX),
              0, buildingHeight/2, 0,
              0, 1, 0); */
    
    // Draw coordinate axes
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(10,0,0);
    glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,10,0);
    glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,10);
    glEnd();
    glEnable(GL_LIGHTING);
    
    drawBuilding();
    
    glutSwapBuffers();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0f, (float)w/h, 0.1f, 500.0f);
    glMatrixMode(GL_MODELVIEW);
}

void mouseFunc(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON) {
        mouseLeftDown = (state == GLUT_DOWN);
        mouseX = x;
        mouseY = y;
    }
    else if(button == GLUT_RIGHT_BUTTON) {
        mouseRightDown = (state == GLUT_DOWN);
        mouseX = x;
        mouseY = y;
    }
}

void motionFunc(int x, int y) {
    if(mouseLeftDown) {
        cameraAngleY += (x - mouseX) * 0.01f;
        cameraAngleX += (y - mouseY) * 0.01f;
        
        // Adjusted angle limits
        if(cameraAngleX > 1.2f) cameraAngleX = 1.2f;
        if(cameraAngleX < -1.2f) cameraAngleX = -1.2f;
    }
    else if(mouseRightDown) {
        cameraDistance += (y - mouseY) * 0.5f;
        // Adjusted zoom limits
        if(cameraDistance < 20.0f) cameraDistance = 20.0f;
        if(cameraDistance > 200.0f) cameraDistance = 200.0f;
    }
    
    mouseX = x;
    mouseY = y;
    glutPostRedisplay();
}


void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'm':
        case 'M':
            currentMaterial = (currentMaterial + 1) % 2;
            break;
        case 'w':
        case 'W':
            showWindows = !showWindows;
            break;
        case 'r':
        case 'R':
            showRoof = !showRoof;
            break;
        case 'a':
        case 'A':
            showAllWalls = !showAllWalls;
            break;
        case '1':
            showFrontWall = !showFrontWall;
            break;
        case '2':
            showBackWall = !showBackWall;
            break;
        case '3':
            showLeftWall = !showLeftWall;
            break;
        case '4':
            showRightWall = !showRightWall;
            break;
        case '+':
            if(numFloors < 20) {
                numFloors++;
                buildingHeight = numFloors * floorHeight;
            }
            break;
        case '-':
            if(numFloors > 1) {
                numFloors--;
                buildingHeight = numFloors * floorHeight;
            }
            break;
        case 27: // ESC key
            exit(0);
            break;
    }
    glutPostRedisplay();
}

void init() {
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    
    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    
    // Set light properties
    GLfloat lightPos[] = {1.0f, 1.0f, 1.0f, 0.0f};
    GLfloat lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
}

void getUserInput() {
    printf("Enter building width (meters): ");
    scanf("%f", &buildingWidth);
    
    printf("Enter building length (meters): ");
    scanf("%f", &buildingLength);
    
    printf("Enter number of floors (1-20): ");
    scanf("%d", &numFloors);
    if(numFloors < 1) numFloors = 1;
    if(numFloors > 20) numFloors = 20;
    
    buildingHeight = numFloors * floorHeight;
}

void printControls() {
    printf("\nControls:\n");
    printf("Left Mouse: Rotate camera\n");
    printf("Right Mouse: Zoom in/out\n");
    printf("M: Change material\n");
    printf("W: Toggle windows\n");
    printf("R: Toggle roof\n");
    printf("+: Add floor\n");
    printf("-: Remove floor\n");
    printf("ESC: Exit\n\n");
}

int main(int argc, char** argv) {
    getUserInput();
    printControls();
    
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Enhanced Building Generator");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouseFunc);
    glutMotionFunc(motionFunc);
    glutKeyboardFunc(keyboard);
    
    init();
    glutMainLoop();
    
    return 0;
}