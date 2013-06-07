#define IMAGES_PER_LEAF 6
#define PPM_TESTING 0

struct ppmstruct
	{
	int width;
	int height;
	GLubyte *image;
	GLuint texName; //clunky to put this here, but its a way to store the GL texture id 
	};
typedef         struct ppmstruct * ppm_image;

//typedef ppm_image ImageArray[IMAGES_PER_LEAF]; // image_array is an array of pointers to ppm_images

int nodeHasImages(node a);

ppm_image getPPM(char* filename);
void textureBox(node n);
void initTexByNode(node n);
void drawPPM(DrawEnum render, OglTree T, node a, float boxW);
void renderPPM(ppm_image ppm, int face, float boxwidth);
