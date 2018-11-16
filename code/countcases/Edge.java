//Edge--a class for a voxel edge
//James Vanderhyde, 10 Jun 2002

public class Edge extends ElementaryCube
{
    protected Face[] faces;
    protected Vertex[] verts;

    public Edge(Voxel v)
    {
        super();
        owner=v;
    }

    public void setFaces(Face f1,Face f2)
    {
        faces=new Face[2];
        faces[0]=f1;
        faces[1]=f2;
    }

    public void setVertices(Vertex v1,Vertex v2)
    {
        verts=new Vertex[2];
        verts[0]=v1;
        verts[1]=v2;
    }

    public void setVoxelNeighbors(Voxel v1,Voxel v2,Voxel v3)
    {
        voxelNeighbors=new Voxel[3];
        voxelNeighbors[0]=v1;
        voxelNeighbors[1]=v2;
        voxelNeighbors[2]=v3;
    }

    public ElementaryCube[] getNeighbors()
    {
        int i;
        ElementaryCube[] r=new ElementaryCube[4];
        for (i=0; i<2; i++) r[i]=faces[i];
        for (i=0; i<2; i++) r[i+2]=verts[i];
        return r;
    }

}
