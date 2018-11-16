//Face--a class for a voxel face
//James Vanderhyde, 10 Jun 2002

public class Face extends ElementaryCube
{
    protected Vertex[] verts;
    protected Edge[] edges;

    public Face(Voxel v)
    {
        super();
        owner=v;
    }

    public void setVertices(Vertex v1,Vertex v2,Vertex v3,Vertex v4)
    {
        verts=new Vertex[4];
        verts[0]=v1;
        verts[1]=v2;
        verts[2]=v3;
        verts[3]=v4;
    }

    public void setEdges(Edge e1,Edge e2,Edge e3,Edge e4)
    {
        edges=new Edge[4];
        edges[0]=e1;
        edges[1]=e2;
        edges[2]=e3;
        edges[3]=e4;
    }

    public void setVoxelNeighbors(Voxel v1)
    {
        voxelNeighbors=new Voxel[1];
        voxelNeighbors[0]=v1;
    }
    
    public ElementaryCube[] getNeighbors()
    {
        int i;
        ElementaryCube r[]=new ElementaryCube[8];
        for (i=0; i<4; i++) r[i]=verts[i];
        for (i=0; i<4; i++) r[i+4]=edges[i];
        return r;
    }

}
