enum   ObjectType {FAN, IMAGE, LABEL};
	// FANs and fanlets are treated as FANs
struct PickableObject {
		OglTree tree; 
		node nd; 
		float height; 
		enum ObjectType objType;
		}; 


void initPickableObject(struct PickableObject *p, enum ObjectType objType, OglTree t, node a, float height);

void colorAndRegisterObject(enum ObjectType objType, OglTree t, node a, float y);
void handleDoubleClick(int x, int y);
void handleSingleClick(int x, int y);

