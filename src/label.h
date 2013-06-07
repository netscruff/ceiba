enum TextStyle {bitmap, stroke};
enum Alignment {left, center};
enum Orientation {face_viewer_horiz, face_viewer_radial};
#define TRUNC_LABEL_N 25 // make sure this is > 3 because I will insert "..." at end of it.

struct _LabelAttributeStruct
	{
	enum TextStyle	textStyle;
	float			scaleSize;
	short			attenuate;
	float			attenuateFactor;
	float			lod_cutoff;
	enum Alignment	alignment;
	short			suppress;
	GLfloat *		labelColor;
	float			lineWidth;
	enum Orientation orientation;
	short			flipLeftSide;
	float			strokeTerminalOffset;
	void *			font; // one of the GLUT fonts
	short			diagonalLine;
	};

typedef  struct _LabelAttributeStruct * LabelAttributeObj;
struct oglTreeStruct; // need a forward reference here for following prototype
void drawLabelObject(struct oglTreeStruct *T, node a, LabelAttributeObj labelAttributes, float x, float y, float z);
LabelAttributeObj labelAttributeObjAlloc(void);

void labelAttributeObjInit(
	LabelAttributeObj L,
	enum TextStyle	textStyle,
	float			scaleSize,
	short			attenuate,
	float			attenuateFactor,
	float			lod_cutoff,
	enum Alignment	alignment,
	short			suppress,
	GLfloat *		labelColor,
	float			lineWidth,
	enum Orientation orientation,
	short			flipLeftSide,
	float			strokeTerminalOffset,
	void*			font,
	short			diagonalLine
	);

LabelAttributeObj labelAttributeObjInitByCopy(
	LabelAttributeObj L
	);


void truncateLabel(char *label, char *truncatedLabel, int maxChars);
void drawDiagBitmap(int width, GLubyte * bitmap);
GLubyte * initBitmapDiagLine(int width);
