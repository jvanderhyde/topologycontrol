//Vertex--a class for a voxel vertex
//James Vanderhyde, 10 Jun 2002

public class Vertex extends ElementaryCube
{
    protected Edge[] edges;
    protected Face[] faces;

    public Vertex(Voxel v)
    {
        super();
        owner=v;
    }

    public void setEdges(Edge e1,Edge e2,Edge e3)
    {
        edges=new Edge[3];
        edges[0]=e1;
        edges[1]=e2;
        edges[2]=e3;
    }

    public void setFaces(Face f1,Face f2,Face f3)
    {
        faces=new Face[3];
        faces[0]=f1;
        faces[1]=f2;
        faces[2]=f3;
    }

    public void setVoxelNeighbors(Voxel v1,Voxel v2,Voxel v3,Voxel v4,Voxel v5,Voxel v6,Voxel v7)
    {
        voxelNeighbors=new Voxel[7];
        voxelNeighbors[0]=v1;
        voxelNeighbors[1]=v2;
        voxelNeighbors[2]=v3;
        voxelNeighbors[3]=v4;
        voxelNeighbors[4]=v5;
        voxelNeighbors[5]=v6;
        voxelNeighbors[6]=v7;
    }

    public ElementaryCube[] getNeighbors()
    {
        int i;
        ElementaryCube r[]=new ElementaryCube[6];
        for (i=0; i<3; i++) r[i]=edges[i];
        for (i=0; i<3; i++) r[i+3]=faces[i];
        return r;
    }

}
