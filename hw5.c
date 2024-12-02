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

// Texture IDs
GLuint brickTexture;
GLuint concreteTexture;
GLuint windowTexture1;
GLuint windowTexture2;
GLuint marbleTexture;
GLuint ledgeTexture;

typedef enum {
    WINDOW_STANDARD,
    WINDOW_ARCHED,
    WINDOW_DIVIDED,
    WINDOW_CIRCULAR
} WindowStyle;

WindowStyle currentWindowStyle = WINDOW_STANDARD;

// Shadow mapping
GLuint shadowMapTexture;
GLuint shadowMapFBO;
const int SHADOW_MAP_SIZE = 2048;

// Enhanced lighting parameters
typedef struct {
    float position[4];
    float ambient[4];
    float diffuse[4];
    float specular[4];
    float direction[3];
    float cutoff;
} Light;

Light mainLight = {
    .position = {100.0f, 100.0f, 100.0f, 1.0f},
    .ambient = {0.2f, 0.2f, 0.2f, 1.0f},
    .diffuse = {1.0f, 1.0f, 1.0f, 1.0f},
    .specular = {1.0f, 1.0f, 1.0f, 1.0f},
    .direction = {-1.0f, -1.0f, -1.0f},
    .cutoff = 45.0f
};
// Add new parameters for stairs
typedef struct {
    float width;      // Width of staircase
    float depth;      // Depth of each step
    float height;     // Height of each step
    float totalRun;   // Total horizontal distance
    int numSteps;     // Number of steps
} Staircase;

Staircase stairs = {
    .width = 2.0f,    // 2 meters wide
    .depth = 0.3f,    // 30cm deep steps
    .height = 0.1667f,// Standard step height (floor height / num steps)
    .totalRun = 3.0f, // 3 meters total run
    .numSteps = 18    // Number of steps per floor
};

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

// Shader for shadow mapping
const char* shadowVertexShader = 
    "#version 330\n"
    "uniform mat4 lightSpaceMatrix;\n"
    "layout(location = 0) in vec3 position;\n"
    "void main() {\n"
    "    gl_Position = lightSpaceMatrix * vec4(position, 1.0);\n"
    "}\n";

const char* shadowFragmentShader =
    "#version 330\n"
    "void main() {\n"
    "    // Fragment depth is automatically written\n"
    "}\n";

int currentMaterial = 0;
bool showWindows = true;
bool showRoof = true;

// Mouse interaction
bool mouseLeftDown = false;
bool mouseRightDown = false;
int mouseX = 0, mouseY = 0;

bool advancedLighting = false;
bool shadowsEnabled = false;

// Add light toggle function
void toggleAdvancedLighting() {
    advancedLighting = !advancedLighting;
    
    if (advancedLighting) {
        // Enhanced lighting settings
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);  // Additional light source
        
        // Main light (sun-like)
        GLfloat lightPos[] = {100.0f, 100.0f, 100.0f, 1.0f};
        GLfloat lightAmbient[] = {0.3f, 0.3f, 0.3f, 1.0f};
        GLfloat lightDiffuse[] = {1.0f, 1.0f, 0.9f, 1.0f};  // Slightly warm
        GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
        
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
        glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
        
        // Secondary light (fill light)
        GLfloat light1Pos[] = {-50.0f, 50.0f, -50.0f, 1.0f};
        GLfloat light1Ambient[] = {0.1f, 0.1f, 0.1f, 1.0f};
        GLfloat light1Diffuse[] = {0.4f, 0.4f, 0.5f, 1.0f};  // Slightly cool
        GLfloat light1Specular[] = {0.3f, 0.3f, 0.3f, 1.0f};
        
        glLightfv(GL_LIGHT1, GL_POSITION, light1Pos);
        glLightfv(GL_LIGHT1, GL_AMBIENT, light1Ambient);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, light1Diffuse);
        glLightfv(GL_LIGHT1, GL_SPECULAR, light1Specular);
        
        // Enable shadows
        shadowsEnabled = true;
        
        // Material properties for better lighting
        GLfloat matAmbient[] = {0.7f, 0.7f, 0.7f, 1.0f};
        GLfloat matDiffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
        GLfloat matSpecular[] = {0.5f, 0.5f, 0.5f, 1.0f};
        GLfloat matShininess[] = {50.0f};
        
        glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
        glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
        glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
        glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);
        
    } else {
        // Basic lighting settings
        glDisable(GL_LIGHT1);
        
        // Reset to basic light
        GLfloat basicLight[] = {0.7f, 0.7f, 0.7f, 1.0f};
        GLfloat basicPos[] = {1.0f, 1.0f, 1.0f, 0.0f};
        
        glLightfv(GL_LIGHT0, GL_POSITION, basicPos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, basicLight);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, basicLight);
        
        // Disable shadows
        shadowsEnabled = false;
        
        // Reset material properties
        GLfloat basicMaterial[] = {0.8f, 0.8f, 0.8f, 1.0f};
        glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, basicMaterial);
    }
}

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

// Function to load a texture
GLuint loadTexture(const char* filename) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    // For this example, we'll create procedural textures
    unsigned char* data = malloc(256 * 256 * 3);
    // Fill with pattern based on filename
    // In real implementation, load actual texture files
    free(data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    return textureID;
}

void initShadowMap() {
    // Create FBO for shadow mapping
    glGenFramebuffers(1, &shadowMapFBO);
    glGenTextures(1, &shadowMapTexture);
    glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 
                 SHADOW_MAP_SIZE, SHADOW_MAP_SIZE, 0, 
                 GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
                          GL_TEXTURE_2D, shadowMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void drawFloorDivider(float y) {
    glPushMatrix();
    glTranslatef(0, y, 0);
    
    // Enable texture
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, ledgeTexture);
    
    float ledgeHeight = 0.3f;
    float ledgeDepth = 0.4f;
    
    glBegin(GL_QUADS);
    // Top
    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 0);
    glVertex3f(-buildingWidth/2 - ledgeDepth, 0, buildingLength/2 + ledgeDepth);
    glTexCoord2f(1, 0);
    glVertex3f(buildingWidth/2 + ledgeDepth, 0, buildingLength/2 + ledgeDepth);
    glTexCoord2f(1, 1);
    glVertex3f(buildingWidth/2 + ledgeDepth, 0, -buildingLength/2 - ledgeDepth);
    glTexCoord2f(0, 1);
    glVertex3f(-buildingWidth/2 - ledgeDepth, 0, -buildingLength/2 - ledgeDepth);
    
    // Front
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 0);
    glVertex3f(-buildingWidth/2 - ledgeDepth, -ledgeHeight, buildingLength/2 + ledgeDepth);
    glTexCoord2f(1, 0);
    glVertex3f(buildingWidth/2 + ledgeDepth, -ledgeHeight, buildingLength/2 + ledgeDepth);
    glTexCoord2f(1, 1);
    glVertex3f(buildingWidth/2 + ledgeDepth, 0, buildingLength/2 + ledgeDepth);
    glTexCoord2f(0, 1);
    glVertex3f(-buildingWidth/2 - ledgeDepth, 0, buildingLength/2 + ledgeDepth);
    glEnd();
    
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

void drawWindowStyle(WindowStyle style) {
    switch(style) {
        case WINDOW_ARCHED:
            // Draw arched window
            glBegin(GL_TRIANGLE_FAN);
            // Center of arch
            glVertex3f(0, windowHeight - windowWidth/2, 0);
            // Draw arch
            for(float angle = 0; angle <= 180; angle += 10) {
                float x = windowWidth/2 * cos(angle * M_PI / 180.0f);
                float y = windowHeight - windowWidth/2 + 
                         windowWidth/2 * sin(angle * M_PI / 180.0f);
                glVertex3f(x, y, 0);
            }
            glEnd();
            break;
            
        case WINDOW_DIVIDED:
            // Draw divided pane window
            glBegin(GL_QUADS);
            for(int i = 0; i < 2; i++) {
                for(int j = 0; j < 3; j++) {
                    float x1 = -windowWidth/2 + i * windowWidth/2;
                    float x2 = -windowWidth/2 + (i+1) * windowWidth/2;
                    float y1 = j * windowHeight/3;
                    float y2 = (j+1) * windowHeight/3;
                    
                    glVertex3f(x1, y1, 0);
                    glVertex3f(x2, y1, 0);
                    glVertex3f(x2, y2, 0);
                    glVertex3f(x1, y2, 0);
                }
            }
            glEnd();
            break;
            
        case WINDOW_CIRCULAR:
            // Draw circular window
            glBegin(GL_TRIANGLE_FAN);
            glVertex3f(0, windowHeight/2, 0);
            for(float angle = 0; angle <= 360; angle += 10) {
                float x = windowWidth/2 * cos(angle * M_PI / 180.0f);
                float y = windowHeight/2 + windowWidth/2 * sin(angle * M_PI / 180.0f);
                glVertex3f(x, y, 0);
            }
            glEnd();
            break;
            
        default:
            // Standard rectangular window
            drawWindow();
            break;
    }
}

void drawSteps() {
    applyMaterial(&materials[0]); // Use concrete material for stairs
    
    float stepDepth = stairs.totalRun / stairs.numSteps;
    
    glBegin(GL_QUADS);
    for(int i = 0; i < stairs.numSteps; i++) {
        float x1 = -stairs.width/2;
        float x2 = stairs.width/2;
        float y1 = i * stairs.height;
        float y2 = (i + 1) * stairs.height;
        float z1 = i * stepDepth;
        float z2 = (i + 1) * stepDepth;
        
        // Top of step
        glNormal3f(0, 1, 0);
        glVertex3f(x1, y2, z1);
        glVertex3f(x2, y2, z1);
        glVertex3f(x2, y2, z2);
        glVertex3f(x1, y2, z2);
        
        // Front of step
        glNormal3f(0, 0, 1);
        glVertex3f(x1, y1, z2);
        glVertex3f(x2, y1, z2);
        glVertex3f(x2, y2, z2);
        glVertex3f(x1, y2, z2);
        
        // Sides of step
        glNormal3f(1, 0, 0);
        glVertex3f(x2, y1, z1);
        glVertex3f(x2, y1, z2);
        glVertex3f(x2, y2, z2);
        glVertex3f(x2, y2, z1);
        
        glNormal3f(-1, 0, 0);
        glVertex3f(x1, y1, z1);
        glVertex3f(x1, y1, z2);
        glVertex3f(x1, y2, z2);
        glVertex3f(x1, y2, z1);
    }
    glEnd();
}


void drawStaircase(float y) {
    glPushMatrix();
    
    // Position stairs in the building
    glTranslatef(buildingWidth/4, y, 0); // Place stairs on the right side
    drawSteps();
    
    // Draw landing platform
    glBegin(GL_QUADS);
    glNormal3f(0, 1, 0);
    glVertex3f(-stairs.width/2, floorHeight, stairs.totalRun);
    glVertex3f(stairs.width/2, floorHeight, stairs.totalRun);
    glVertex3f(stairs.width/2, floorHeight, stairs.totalRun + stairs.width);
    glVertex3f(-stairs.width/2, floorHeight, stairs.totalRun + stairs.width);
    glEnd();
    
    glPopMatrix();
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
    // Draw staircase for all floors except the top floor
    if (y < (numFloors - 1) * floorHeight) {
        drawStaircase(y);
    }
    
    // Add stair opening in floor above
    if (y > 0) {  // Don't cut hole in ground floor
        glPushMatrix();
        glTranslatef(buildingWidth/4, y, 0);
        
        // Cut opening in floor for stairwell
        // This is done by drawing a slightly darker section
        glColor3f(0.3f, 0.3f, 0.3f);
        glBegin(GL_QUADS);
        glNormal3f(0, -1, 0);
        glVertex3f(-stairs.width/2 - 0.3f, 0, 0);
        glVertex3f(stairs.width/2 + 0.3f, 0, 0);
        glVertex3f(stairs.width/2 + 0.3f, 0, stairs.totalRun + stairs.width);
        glVertex3f(-stairs.width/2 - 0.3f, 0, stairs.totalRun + stairs.width);
        glEnd();
        
        glPopMatrix();
    }
    
    glPopMatrix();
}

// Add function to draw railings
void drawRailing(float length) {
    float railHeight = 0.9f;  // Standard railing height
    float postSpacing = 1.0f; // Space between posts
    
    glPushMatrix();
    
    // Draw main handrail
    glColor3f(0.4f, 0.4f, 0.4f);
    glBegin(GL_LINES);
    glVertex3f(0, railHeight, 0);
    glVertex3f(0, railHeight, length);
    glEnd();
    
    // Draw posts
    for(float z = 0; z <= length; z += postSpacing) {
        glBegin(GL_LINES);
        glVertex3f(0, 0, z);
        glVertex3f(0, railHeight, z);
        glEnd();
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
    // First handle shadow pass if enabled
    if (shadowsEnabled) {
        // First pass: Render from light's perspective (shadow map)
        glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, SHADOW_MAP_SIZE, SHADOW_MAP_SIZE);
        
        // Set up light's perspective matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(45.0f, 1.0f, 1.0f, 200.0f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(mainLight.position[0], mainLight.position[1], mainLight.position[2],
                  0.0f, 0.0f, 0.0f,
                  0.0f, 1.0f, 0.0f);
        
        // Render scene for shadow map
        drawBuilding();
        
        // Second pass: Regular rendering with shadows
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    // Normal rendering pass
    glViewport(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Set up camera view
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0f, (float)glutGet(GLUT_WINDOW_WIDTH)/glutGet(GLUT_WINDOW_HEIGHT), 0.1f, 500.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    // Calculate camera position
    float camX = cameraDistance * sin(cameraAngleY) * cos(cameraAngleX);
    float camY = cameraDistance * sin(cameraAngleX) + (buildingHeight / 2.0f);
    float camZ = cameraDistance * cos(cameraAngleY) * cos(cameraAngleX);
    
    // Set camera
    gluLookAt(camX, camY, camZ,
              0, buildingHeight / 2.0f, 0,
              0, 1, 0);
    
    // Draw coordinate axes
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    glColor3f(1,0,0); glVertex3f(0,0,0); glVertex3f(10,0,0);
    glColor3f(0,1,0); glVertex3f(0,0,0); glVertex3f(0,10,0);
    glColor3f(0,0,1); glVertex3f(0,0,0); glVertex3f(0,0,10);
    glEnd();
    
    // Enable lighting for the building
    if (advancedLighting) {
        glEnable(GL_LIGHTING);
        
        // Apply shadow texture if enabled
        if (shadowsEnabled) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE_ARB);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
        }
    }
    
    // Draw the building
    drawBuilding();
    
    // Single buffer swap at the end
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
        case 't':
        case 'T':
            // Cycle through window styles
            currentWindowStyle = (currentWindowStyle + 1) % 4;
            break;
            
        case 'l':
        case 'L':
            toggleAdvancedLighting();
            printf("Advanced Lighting: %s\n", advancedLighting ? "ON" : "OFF");
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

    brickTexture = loadTexture("brick.jpg");
    concreteTexture = loadTexture("concrete.jpg");
    windowTexture1 = loadTexture("window1.jpg");
    windowTexture2 = loadTexture("window2.jpg");
    marbleTexture = loadTexture("marble.jpg");
    ledgeTexture = loadTexture("ledge.jpg");
    
    // Set light properties
    GLfloat lightPos[] = {1.0f, 1.0f, 1.0f, 0.0f};
    GLfloat lightAmbient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat lightDiffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat lightSpecular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    initShadowMap();
    
    // Enhanced lighting setup
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, mainLight.position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, mainLight.ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, mainLight.diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, mainLight.specular);
    
    // Enable features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_NORMALIZE);
    glShadeModel(GL_SMOOTH);
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
    printf("T: Change window style\n");
    printf("L: Toggle advanced lighting\n");
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