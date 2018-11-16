//Vector3D.h
//James Vanderhyde, 16 October 2004

class Vector3D
{
public:
	float x,y,z;
	Vector3D();
	Vector3D(float xx,float yy,float zz);
	friend ostream& operator<< (ostream& out, const Vector3D& p);
};

float dot(Vector3D p1,Vector3D p2);
Vector3D cross(Vector3D p1,Vector3D p2);
Vector3D plus(Vector3D p1,Vector3D p2);
Vector3D minus(Vector3D p1,Vector3D p2);
Vector3D times(Vector3D p,double c);
float length(Vector3D p);
Vector3D normalized(Vector3D p);
void print(Vector3D p);
