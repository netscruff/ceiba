#pragma once
#ifndef CALLBACK
#define CALLBACK
#endif
//#include "layout.h"

enum BranchCurvy {straight, nurbs};
enum BranchStyle {line, tube}; 

#define NUM_CONTROL_POINT_PARAMS 4

struct _BranchAttributeStruct
	{
	enum BranchStyle		branchStyle;
	enum BranchCurvy		branchCurvy;
	float					lineWidth;
	GLfloat *				color;
	short					corners;
	short					attenuate;
	float					attenuateFactor;
	float					lod_cutoff;
	float					ctlpointParms[NUM_CONTROL_POINT_PARAMS]; // 4x4 matrix of four control points = 2 end points + two "control" points
	};

typedef  struct _BranchAttributeStruct * BranchAttributeObj;

void drawBranchObject(node a, BranchAttributeObj BranchAttributes, float r1, float r2);
BranchAttributeObj branchAttributeObjAlloc(void);

void branchAttributeObjInit(
	BranchAttributeObj		branchAttributeObject,
	enum BranchStyle		branchStyle,
	enum BranchCurvy		branchCurvy,
	float					lineWidth,
	GLfloat *				color,
	short					corners,
	short					attenuate,
	float					attenuateFactor,
	float					lod_cutoff,
	float					ctlpointParms[] 
	);
void CALLBACK beginCallback(GLenum whichType);
void CALLBACK endCallback(void);
void CALLBACK vertexCallback(GLfloat *vertex);
void CALLBACK ErrorCallback(GLenum which);
void nurbsInit(void);
void drawArcBranch(double theta1, double r1, double z1, double theta2, double r2, double z2,  float radius, BranchAttributeObj branchAttributes);

struct oglTreeStruct; // need a forward reference here; otherwise have a circular reference with layout.h
void drawCornersArcBranch(node a, struct oglTreeStruct * T, float radius);
float setColorWithAttenuationLineSegment(float x1, float y1, float z1, float x2, float y2, float z2, GLfloat color[], float attenuateFactor);
