enum FaceType {FACE_VIEWER, FACE_FIXED};

struct _dataObjStruct
	{
	enum ImageType	dataType;
	float			thetaObj[3];
	float			boxWidth;
	short			attenuate;
	float			attenuateFactor;
	float			lod_cutoff;
	enum FaceType orientation;
	};

typedef  struct _dataObjStruct * DataObj;

void drawDataObject(DataObj a, float x, float y, float z);
DataObj dataObjAlloc(void);

void dataObjInit(
	DataObj			L,
	enum ImageType	dataType,
	float			theta[3],	// orientation of data object
	float			boxWidth,	// size of bounding box
	short			attenuate,
	float			attenuateFactor,
	float			lod_cutoff,
	enum FaceType	faces	// faces viewer?
	);

DataObj dataObjInitByCopy(
	DataObj L
	);

