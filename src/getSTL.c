#include "tree_openGL.h"
#include "getSTL.h"
// See some important compilation notes in this header!

static void renderSTL(STL_Triangle *t_array, long numTriangles);


/**************************************************/

void drawSTL(DrawEnum renderFlag, node a, enum LayoutChoice layout)

// NEED TO THINK ABOUT WHETHER THIS NEEDS TO BE MODIFIED A'LA getXYZ, IN CASES WHERE THE MID OF EACH AXIS IS NOT
// CLOSE TO 0. getXYZ HAS MORE CAREFUL TREATMENT OF THIS.


// Draw the taxon name, and possibly a translucent box behind the name, and possibly an image above the name
// Current coord is at a's ancestor, pointing with z-axis along branch between a and anc(a). Have to move distance +zr along z axis
// to draw label at node a.

			{
			extern node gCurrPickedNode;
			int style = 0; // 0 = 3d and font; 1 = bitmap, rasterized
	extern GLfloat lt_yellow[],green[],earth_blue[],blue[],black[],red[],white[],brown[],lt_green[];
	extern float objTheta[3];
			area A;
			float offset,x,y,z,thetaName,displayNumber, darkness, distance;
			extern float gTermOffset;
			extern double gRadiusOfOriginalCircle;
			extern enum LayoutChoice gLayoutChoice;
			extern int gLabelFace;
			extern plane gP;
			extern float theta[3];
			int i,len;
			if (!a->label) return;
			
			if (!a->data2) return; //no STL image at this node

			A=(area)(a->data);
			STL_Array * STL_array;
			STL_array = (STL_Array *)(a->data2);
			GLfloat params[4];
			int labelLen;
			char *p, labelBuf[128];




glPushMatrix();  

// change to local coords centered at the node and in right orientation
//glTranslatef(0.0,0.0,(A->zr)+gTermOffset);
switch (layout)
	{
	case cone:
	case hemisphere:
	case solid_cone:
		//glRotatef(-A->theta,A->cross_prodx,A->cross_prody,0.0); // puts the labels in one plane and orientation
		//glRotatef(-90,1.0,0.0,0.0); // ...and then moves them to face the viewer. Note that the order 
		//glRotatef(-theta[2]-gP->Heading,0.0,0.0,1.0); // ...and then moves them to face the viewer. Note that the order 
			  // of this and the next call is important! (for some reason) 
		//glRotatef(-theta[0]+gP->Pitch,1.0,0.0,0.0); // to ensure that the labels keep the same orientation toward the viewer
		break;
	case circle:
		glRotatef(-A->theta,A->cross_prodx,A->cross_prody,0.0); // puts the labels in one plane and orientation
		glRotatef(+toDegs(A->theta_center),0.0,0.0,1.0); // ...rotate names oriented along radii
		break;
	}


// make adjustments for flipping labels that are on the left side of circle only
if (layout == circle)
	{
	thetaName = toDegs(A->theta_center) /*+ theta[2]*/;
	if (thetaName > 360.0) thetaName -=360.0;
	if (thetaName < 0.0) thetaName +=360.0;
	if (thetaName > 90 && thetaName < 270)  
		// Do the following corrections for labels on left side of screen only
		  {
			glTranslatef(+labelLen,0.0,0.0);
			glRotatef(180,0.0,0.0,1.0);
		  }

	}
//#define BOXW 7.0
float widthX, widthY,widthZ, zoffset,xoffset,yoffset,scalefactor,maxW;
widthX = STL_array->xmax - STL_array->xmin;
widthY = STL_array->ymax - STL_array->ymin;
widthZ = STL_array->zmax - STL_array->zmin;
maxW = MAX(widthX,widthY);
maxW = MAX(widthZ,maxW);
scalefactor = BOXW/maxW;
zoffset = scalefactor* (STL_array->zmax+STL_array->zmin)/2;
xoffset = scalefactor* (STL_array->xmax+STL_array->xmin)/2;
yoffset = scalefactor* (STL_array->ymax+STL_array->ymin)/2;



glPushMatrix();

glTranslatef(0,+zoffset,0);
glTranslatef(-xoffset,-yoffset,-zoffset); //center the 3d scene on the node
//glTranslatef(0,-BOXW/2,0);	// and elevate it above the node just enough
glTranslatef(0,0,+BOXW/2);	// and elevate it above the node just enough

if (a == gCurrPickedNode)
	{
	glRotatef(objTheta[2],0,0,1);
	glRotatef(objTheta[0],1,0,0);
	glRotatef(objTheta[1],0,1,0);
	}


glScalef(scalefactor,scalefactor,scalefactor); // make sure to do uniform scaling here -- otherwise distorts image!
glEnable(GL_RESCALE_NORMAL); // VITAL!! scaling changes the normals; use this to correct, but only works if uniform scaling in every direction, else use GL_NORMALIZE, which is more expensive

if (renderFlag==RENDERING)
	renderSTL(STL_array->firstTriangle,STL_array->numTriangles);

glPopMatrix();


glTranslatef(0,0,+BOXW/2);	// and elevate it above the node just enough
drawTranslucentBox3d(renderFlag,BOXW,BOXW, BOXW, 0.1);

glPopMatrix();
return;
}






static void renderSTL(STL_Triangle *t_array, long numTriangles)
{

	GLfloat my_bone[4]={ .4, .4, .3};
	long i;
	STL_Triangle *currTriangle;
   	GLfloat mat_specular_dull[] = { 0.1,0.1,0.1, 1.0 };
   	GLfloat mat_shininess[] = { 0.0 };
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, my_bone);
   	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_dull);
   	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess); // KEEP!
	//glColor4fv(my_brown);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glBegin(GL_TRIANGLES);
	currTriangle = t_array;
	for (i=0;i<numTriangles;i++)
		{
		
		glNormal3fv( &(currTriangle->n1) );
		glVertex3fv( &(currTriangle->x1) );
		glVertex3fv( &(currTriangle->x2) );
		glVertex3fv( &(currTriangle->x3) ); // use the same normal for all three vertices
		
		
		//printf ("%f %f %f\n",currTriangle->n1,currTriangle->n2,currTriangle->n3);
		currTriangle++;
		}
	glEnd();

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);



}




STL_Array * getSTL(char * fileName)

/*

Read the STL file, store its data, number of triangles, and bounding box values. Note to save time
the bounding box is calculated from just one of the three vertices, so it is possible that the other two
vertices might stick out of the box slightly.

*/

{
	STL_Array * stl_ar;
	STL_Triangle *triangle_ar;
	STL_Triangle *currTriangle;
	char header[80];
	unsigned int numTriangles;
	float xmin=1e10,ymin=1e10,zmin=1e10,xmax=-1e10,ymax=-1e10,zmax=-1e10;

	STL_Triangle triangle;
	FILE* in_file = fopen(fileName,"r");
	if (in_file == NULL)
		{
		fprintf(stderr,"Filename %s not found\n",fileName);
		return NULL;
		}
	int items_read=0;
	fread((char*) &header, 80*sizeof(char),1,in_file);
	//printf ("Header = %s\n",&header[0]);
	fread((char*) &numTriangles, sizeof(unsigned int),1,in_file);
	//printf ("NumTriangles = %i\n",numTriangles);	

	stl_ar = (STL_Array *)malloc(sizeof(STL_Array));
	triangle_ar = (STL_Triangle*)malloc(numTriangles*sizeof(STL_Triangle));	
	if (!triangle_ar)
		{
		fprintf(stderr,"Failed to allocate memory in getSTL\n");
		exit(1);
		}


	currTriangle = triangle_ar;
	while (fread((char*) currTriangle,sizeof(STL_Triangle),1,in_file) == 1)
		{
		items_read++;

		if (currTriangle->x1 < xmin ) xmin = currTriangle->x1;
		if (currTriangle->y1 < ymin ) ymin = currTriangle->y1;
		if (currTriangle->z1 < zmin ) zmin = currTriangle->z1;
		if (currTriangle->x1 > xmax ) xmax = currTriangle->x1;
		if (currTriangle->y1 > ymax ) ymax = currTriangle->y1;
		if (currTriangle->z1 > zmax ) zmax = currTriangle->z1;
		currTriangle++;

		}
	if (ferror(in_file))
		{
		fprintf(stderr,"?Read error, file %s record %d\n",fileName,items_read+1);exit(1);
		}
	printf("Finished; %d elements read from file %s\n",items_read, fileName);
	
	stl_ar -> firstTriangle = triangle_ar;
	stl_ar ->numTriangles = items_read;
	stl_ar ->xmin = xmin;
	stl_ar ->ymin = ymin;
	stl_ar ->zmin = zmin;
	stl_ar ->xmax = xmax;
	stl_ar ->ymax = ymax;
	stl_ar ->zmax = zmax;

printf("Minima and maxima in STL file: %f %f %f %f %f %f\n",xmin,xmax,ymin,ymax,zmin,zmax);

	return stl_ar;
}	
		
		
		
STL_Array * getSTLt(char * fileName)

/*

Read the STL file, store its data, number of triangles, and bounding box values. Note to save time
the bounding box is calculated from just one of the three vertices, so it is possible that the other two
vertices might stick out of the box slightly.

*/

{
	STL_Array * stl_ar;
	STL_Triangle *triangle_ar;
	STL_Triangle *currTriangle;
	char header[80];
	unsigned int numTriangles, index;
	float f1,f2,f3, xmin=1e10,ymin=1e10,zmin=1e10,xmax=-1e10,ymax=-1e10,zmax=-1e10;
	unsigned long maxTriangles = 10000;
	STL_Triangle triangle;
	FILE* in_file = fopen(fileName,"r");
	if (in_file == NULL)
		{
		fprintf(stderr,"Filename %s not found\n",fileName);
		return NULL;
		}
	int items_read=0;

	stl_ar = (STL_Array *)malloc(sizeof(STL_Array));
	triangle_ar = (STL_Triangle*)malloc(maxTriangles*sizeof(STL_Triangle));	
	if (!triangle_ar)
		{
		fprintf(stderr,"Failed to allocate memory in getSTL\n");
		exit(1);
		}


	currTriangle = triangle_ar;
	index = 0;
	
	while (fscanf(in_file,"%f %f %f",&f1, &f2, &f3) != EOF)
		{
		
		switch (index)
			{
			case 0:
				currTriangle->n1=f1;
				currTriangle->n2=f2;
				currTriangle->n3=f3;
				break;
			case 1:
				currTriangle->x1=f1;
				currTriangle->y1=f2;
				currTriangle->z1=f3;
				break;
			case 2:
				currTriangle->x2=f1;
				currTriangle->y2=f2;
				currTriangle->z2=f3;
				break;
			case 3:
				currTriangle->x3=f1;
				currTriangle->y3=f2;
				currTriangle->z3=f3;
				break;
			}
		++index;
		if (index == 4) 
			{
			index = 0; // reset to reloop
			if (currTriangle->x1 < xmin ) xmin = currTriangle->x1;
			if (currTriangle->y1 < ymin ) ymin = currTriangle->y1;
			if (currTriangle->z1 < zmin ) zmin = currTriangle->z1;
			if (currTriangle->x1 > xmax ) xmax = currTriangle->x1;
			if (currTriangle->y1 > ymax ) ymax = currTriangle->y1;
			if (currTriangle->z1 > zmax ) zmax = currTriangle->z1;
			currTriangle++;
			items_read++;
			}
		}
	if (ferror(in_file))
		{
		fprintf(stderr,"?Read error, file %s record %d\n",fileName,items_read+1);exit(1);
		}
	printf("Finished; %d elements read from file %s\n",items_read, fileName);
	
	stl_ar -> firstTriangle = triangle_ar;
	stl_ar ->numTriangles = items_read;
	stl_ar ->xmin = xmin;
	stl_ar ->ymin = ymin;
	stl_ar ->zmin = zmin;
	stl_ar ->xmax = xmax;
	stl_ar ->ymax = ymax;
	stl_ar ->zmax = zmax;

printf("Minima and maxima in STLt file: %f %f %f %f %f %f\n",xmin,xmax,ymin,ymax,zmin,zmax);

	return stl_ar;
}	

