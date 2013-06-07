#include "tree_openGL.h"
#include "getXYZ.h"
// See some important compilation notes in this header!

static void renderXYZ(XYZ_Array *xyz_ar, float scalefactor, float xm, float ym,float zm);

	GLUquadricObj *qobja;

/**************************************************/

void drawXYZ(DrawEnum renderFlag, node a, enum LayoutChoice layout)

// Draw the taxon name, and possibly a translucent box behind the name, and possibly an image above the name
// Current coord is at a's ancestor, pointing with z-axis along branch between a and anc(a). Have to move distance +zr along z axis
// to draw label at node a.

			{
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
			XYZ_Array * XYZ_array;
			XYZ_array = (XYZ_Array *)(a->data2);
			GLfloat params[4];
			int labelLen;
			char *p, labelBuf[128];




	qobja = gluNewQuadric();
	gluQuadricDrawStyle(qobja,GLU_FILL);
	gluQuadricNormals(qobja,GLU_SMOOTH);



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


		//glRotatef(-theta[2],0.0,0.0,1.0); // ...derotates the tree to face the viewer. Note that the order of this and the next matters; have to reverse the order of the viewing rotation in display()
		//glRotatef(-theta[0],1.0,0.0,0.0); //  




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
// #define BOXW 10.0
float widthX, widthY,widthZ, xmid,ymid,zmid,scalefactor,maxW;
widthX = XYZ_array->xmax - XYZ_array->xmin;
widthY = XYZ_array->ymax - XYZ_array->ymin;
widthZ = XYZ_array->zmax - XYZ_array->zmin;
maxW = MAX(widthX,widthY);
maxW = MAX(widthZ,maxW);
scalefactor = BOXW/maxW;
//zoffset = scalefactor* (XYZ_array->zmax+XYZ_array->zmin)/2;
//xoffset = scalefactor* (XYZ_array->xmax+XYZ_array->xmin)/2;
//yoffset = scalefactor* (XYZ_array->ymax+XYZ_array->ymin)/2;

zmid = (XYZ_array->zmax+XYZ_array->zmin)/2;
xmid = (XYZ_array->xmax+XYZ_array->xmin)/2;
ymid = (XYZ_array->ymax+XYZ_array->ymin)/2;


glPushMatrix();

glTranslatef(0,-BOXW/2,0);

glScalef(scalefactor,scalefactor,scalefactor); // make sure to do uniform scaling here -- otherwise distorts image!
glEnable(GL_RESCALE_NORMAL); // VITAL!! scaling changes the normals; use this to correct, but only works if uniform scaling in every direction, else use GL_NORMALIZE, which is more expensive



//glTranslatef(-xmid,-ymid,-zmid); //center the 3d scene on the node


glRotatef(objTheta[0],1,0,0);
glRotatef(objTheta[1],0,1,0);



if (renderFlag==RENDERING)
	renderXYZ(XYZ_array,scalefactor,xmid,ymid,zmid);



glPopMatrix();

glTranslatef(0,-BOXW/2,0);
//glRotatef(-90,1,0,0);
//drawTranslucentBox(BOXW,BOXW, 0, 0.5);
drawTranslucentBox3d(renderFlag,BOXW,BOXW, BOXW, 0.1);

glPopMatrix();
return;
}




#define atom_radius 0.2

static void renderXYZ(XYZ_Array *xyz_ar,float scalefactor,float xmid,float ymid, float zmid)
{

//	extern GLUquadricObj *qobj;
			extern float objTheta[3];
	extern GLfloat lt_yellow[],green[],earth_blue[],lt_blue[],black[],grey[],lt_red[],white[],brown[],lt_green[],my_brown[];
	GLfloat my_bone[4]={ .4, .4, .3};
	GLfloat grey2[4]={ .2, .2, .2};
	GLfloat *thisColor;
	thisColor=my_bone;
	int i,j,ix;
	XYZ_atom *atom_array,atom1, atom2;
	XYZ_bond *bond_array;
	long numAtoms;
	int numBonds;
	numBonds = xyz_ar->numBonds;
	atom_array = xyz_ar->firstAtom;
	bond_array = xyz_ar->firstBond;
	numAtoms =   xyz_ar->numAtoms;
	XYZ_atom *currAtom;
	XYZ_bond *currBond;
   	//GLfloat mat_specular_dull[] = { 0.1,0.1,0.1, 1.0 };
   	GLfloat mat_specular_dull[] = { 0,0,0, 1.0 };
   	GLfloat mat_shininess[] = { 0.0 };
	//glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, my_bone);
   	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular_dull);
   	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess); // KEEP!
	//glColor4fv(my_brown);
	glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	
	currAtom = atom_array;
	
	glPushMatrix();
	for (i=0;i<numAtoms;i++)
		{
		
		//glScalef(scalefactor,scalefactor,scalefactor); // make sure to do uniform scaling here -- otherwise distorts image!
		//glEnable(GL_RESCALE_NORMAL); // VITAL!! scaling changes the normals; use this to correct, but only works if 
		glTranslatef(-xmid,-ymid,-zmid);
		glTranslatef(currAtom->x,currAtom->y,currAtom->z);	


GLfloat H_color[4]={1,1,1,1},
		C_color[4]={144/255.,144/255.,144/255.,1},
		N_color[4]={48/255.,80/255.,248/255.,1},
		O_color[4]={255/255.,13/255.,13/255.,1},
		S_color[4]={255/255.,255/255.,48/255.,1};
		switch (currAtom->label)
			{
			case 'C': thisColor=C_color; break;
			case 'H': thisColor=H_color; break;
			case 'N': thisColor=N_color; break;
			case 'O': thisColor=O_color; break;
			case 'S': thisColor=S_color; break;
			
			}
		glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, thisColor);

		gluSphere(qobja,atom_radius,7,7);
		glTranslatef(+xmid,+ymid,+zmid);
		glTranslatef(-currAtom->x,-currAtom->y,-currAtom->z);	


		currAtom++;
		}
	glPopMatrix();

	//glScalef(scalefactor,scalefactor,scalefactor); 
	//glEnable(GL_RESCALE_NORMAL); // VITAL!! scaling changes the normals; use this to correct, but only works if 
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, grey2);

	currBond = bond_array;
	for (ix=0;ix<numBonds;ix++)
		{
		i = (bond_array[ix]).i - 1;
		j = (bond_array[ix]).j - 1;  // format uses 1-off array indexing
		atom1 = atom_array[i];
		atom2 = atom_array[j];
		drawCylinder(atom1.x-xmid,atom1.y-ymid,atom1.z-zmid,atom2.x-xmid,atom2.y-ymid,atom2.z-zmid,atom_radius/1.5,atom_radius/1.5); // can't use gltranslate as above, because of how drawCylinder works
		
		currBond++;
		}
// DO ALL THE SUBTRACTIONS WITH A TRANSLATE COMMAND

	glDisable(GL_LIGHT0);
	glDisable(GL_LIGHTING);



}




XYZ_Array * getXYZ(char * fileName)

/*

Read the XYZ file, store its data

*/

{
	XYZ_Array * xyz_ar;
	XYZ_atom *atom_ar, *currAtom;
	char dummy;
	unsigned long numAtoms;
	float xmin=1e10,ymin=1e10,zmin=1e10,xmax=-1e10,ymax=-1e10,zmax=-1e10;

	XYZ_atom atom;
	FILE* in_file = fopen(fileName,"r");
	if (in_file == NULL)
		{
		fprintf(stderr,"Filename %s not found\n",fileName);
		return NULL;
		}
	int items_read=0;

	fscanf(in_file,"%li\n",&numAtoms);
printf ("Atoms = %li\n",numAtoms);
	fscanf(in_file,"\n");
	
	xyz_ar = (XYZ_Array *)malloc(sizeof(XYZ_Array));
	atom_ar = (XYZ_atom*)malloc(numAtoms*sizeof(XYZ_atom));	

	if (!atom_ar)
		{
		fprintf(stderr,"Failed to allocate memory in getXYZ\n");
		exit(1);
		}


	currAtom = atom_ar;
	while (fscanf(in_file,"%s %f %f %f",&(currAtom->label),&(currAtom->x),&(currAtom->y),&(currAtom->z)) != EOF)
		{
		items_read++;
printf("label=%c\n",currAtom->label);

		if (currAtom->x < xmin ) xmin = currAtom->x;
		if (currAtom->y < ymin ) ymin = currAtom->y;
		if (currAtom->z < zmin ) zmin = currAtom->z;
		if (currAtom->x > xmax ) xmax = currAtom->x;
		if (currAtom->y > ymax ) ymax = currAtom->y;
		if (currAtom->z > zmax ) zmax = currAtom->z;
		currAtom++;
		}
	if (ferror(in_file))
		{
		fprintf(stderr,"?Read error, file %s record %d\n",fileName,items_read+1);exit(1);
		}
	printf("Finished; %d elements read from file %s\n",items_read, fileName);
	
	xyz_ar -> firstAtom = atom_ar;
	xyz_ar ->numAtoms = items_read;
	xyz_ar ->xmin = xmin;
	xyz_ar ->ymin = ymin;
	xyz_ar ->zmin = zmin;
	xyz_ar ->xmax = xmax;
	xyz_ar ->ymax = ymax;
	xyz_ar ->zmax = zmax;

printf("Minima and maxima in XYZ file: %f %f %f %f %f %f\n",xmin,xmax,ymin,ymax,zmin,zmax);

	return xyz_ar;
}	


XYZ_Array * getXYZ_v2(char * fileName)

/*

Read a v3000 file, store its data on atoms and bonds...

*/

{
#define MAXFIELDS 10
#define SEPCHARS " \t\n"
#define MAX_LINE_LENGTH 80
	char * words[MAXFIELDS]; // array of pointers to substring tokens within original string
	char s[MAX_LINE_LENGTH+1];
	char *dummy,label;
	int ix,numtok,numAtoms,numBonds,i,j;
	int atomFlag=0,bondFlag=0;
	
	XYZ_Array * xyz_ar;
	XYZ_atom *atom_ar, *currAtom;
	XYZ_bond *bond_ar, *currBond;
	
	float x,y,z,xmin=1e10,ymin=1e10,zmin=1e10,xmax=-1e10,ymax=-1e10,zmax=-1e10;

	XYZ_atom atom;
	XYZ_bond bond;
	
	FILE* in_file = fopen(fileName,"r");
	if (in_file == NULL)
		{
		fprintf(stderr,"Filename %s not found\n",fileName);
		return NULL;
		}
	int items_read=0;

	xyz_ar = (XYZ_Array *)malloc(sizeof(XYZ_Array));



	while (fgets(&s[0],MAX_LINE_LENGTH,in_file))
		{
		if (strlen(s) <=1) 
			break;

		numtok = split(s,SEPCHARS,&words[0]);

		if (atomFlag && numtok == 8)
			{
			label=*(words[3]);
			x=strtod(words[4],&dummy);
			y=strtod(words[5],&dummy);
			z=strtod(words[6],&dummy);
			printf("Atom: %c %f %f %f\n",label,x,y,z);
			currAtom->label=label;
			currAtom->x = x;
			currAtom->y = y;
			currAtom->z = z;
			if (currAtom->x < xmin ) xmin = currAtom->x;
			if (currAtom->y < ymin ) ymin = currAtom->y;
			if (currAtom->z < zmin ) zmin = currAtom->z;
			if (currAtom->x > xmax ) xmax = currAtom->x;
			if (currAtom->y > ymax ) ymax = currAtom->y;
			if (currAtom->z > zmax ) zmax = currAtom->z;
			currAtom++;
			}
		if (bondFlag && numtok == 6)
			{
			i=strtod(words[4],&dummy);
			j=strtod(words[5],&dummy);
			currBond->i = i;
			currBond->j = j;
			printf("Bond: %i %i\n",i,j);
			currBond++;
			}
		if (numtok == 8 && strcmp(words[2],"COUNTS")==0) // "COUNT" : do a bunch of initializes here
			{
			numAtoms = strtod(words[3],&dummy);
			numBonds = strtod(words[4],&dummy);
			atom_ar = (XYZ_atom*)malloc(numAtoms*sizeof(XYZ_atom));	
			bond_ar = (XYZ_bond*)malloc(numBonds*sizeof(XYZ_bond));	
			currAtom = atom_ar;
			currBond = bond_ar;
			xyz_ar -> firstAtom = atom_ar;
			xyz_ar -> firstBond = bond_ar;
			xyz_ar ->numAtoms = numAtoms;
			xyz_ar ->numBonds = numBonds;
			
			printf("atoms = %i bonds = %i\n",numAtoms, numBonds);
			}
		if (numtok == 4 && strcmp(words[2],"BEGIN")==0 && strcmp(words[3],"ATOM")==0 )
			atomFlag=1;
		if (numtok == 4 && strcmp(words[2],"BEGIN")==0 && strcmp(words[3],"BOND")==0 )
			bondFlag=1;
		if (numtok == 4 && strcmp(words[2],"END")==0 && strcmp(words[3],"ATOM")==0 )
			atomFlag=0;
		if (numtok == 4 && strcmp(words[2],"END")==0 && strcmp(words[3],"BOND")==0 )
			bondFlag=0;
		}	


	xyz_ar ->xmin = xmin;
	xyz_ar ->ymin = ymin;
	xyz_ar ->zmin = zmin;
	xyz_ar ->xmax = xmax;
	xyz_ar ->ymax = ymax;
	xyz_ar ->zmax = zmax;

printf("Minima and maxima in XYZ file: %f %f %f %f %f %f\n",xmin,xmax,ymin,ymax,zmin,zmax);

	return xyz_ar;
}	
// 
// Splits a string s into tokens. An array of pointers must be provided up front.
// The array will hold pointers to the start of the tokens within s, which will
// have been modified by inserting nulls after each token.
//

int split(char *s, char* delimiters, char* words[])
	{
	int ix=0;
	words[ix++] = strtok(s,delimiters);
	while (words[ix-1] != NULL)
		words[ix++] = strtok(NULL,delimiters);
	return ix-1;
	}
		
