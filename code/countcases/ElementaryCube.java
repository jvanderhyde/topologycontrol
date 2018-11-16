//ElementaryCube--an abstract class for vertices, edges, faces, etc.
//James Vanderhyde, 10 Jun 2002

public abstract class ElementaryCube
{
    protected boolean mark;	//indicates whether "cube" is incident to a marked voxel
    protected int label;
    protected Voxel owner;
    protected Voxel[] voxelNeighbors;

    protected ElementaryCube()
    {
        mark=false;
        label=0;
    }
    
    public abstract ElementaryCube[] getNeighbors();

    public boolean getMark()
    {
        return mark;
    }

    public void setMark(boolean p_mark)
    {
        mark=p_mark;
    }

    public int getLabel()
    {
        return label;
    }

    public void setLabel(int p_label)
    {
        label=p_label;
    }

    public void calcMark1()
    {
        //We mark the cube if all of its incident voxels (except owner) are marked.
        //In other words, the mark is false if any of its incident voxels are not marked.
        mark=true;
        for (int i=0; i<voxelNeighbors.length; i++)
            if (!voxelNeighbors[i].isMarked())
                mark=false;
    }

    public void calcMark3()
    {
        //We mark the cube if any of its incident voxels (except owner) are marked.
        //In other words, the mark is false if all of its incident voxels are not marked.
        mark=false;
        for (int i=0; i<voxelNeighbors.length; i++)
            if (voxelNeighbors[i].isMarked())
                mark=true;
    }

    public static ElementaryCube[] buildGraph(VoxelData vd,int x,int y,int z)
    {
        int i;
        Face[] faces=new Face[6];
        Edge[] edges=new Edge[12];
        Vertex[] verts=new Vertex[8];
        Voxel owner=vd.getVoxel(x,y,z),
            vx0y0z0=vd.getVoxel(x-1,y-1,z-1),
            vx0y0z1=vd.getVoxel(x-1,y-1,z+0),
            vx0y0z2=vd.getVoxel(x-1,y-1,z+1),
            vx0y1z0=vd.getVoxel(x-1,y+0,z-1),
            vx0y1z1=vd.getVoxel(x-1,y+0,z+0),
            vx0y1z2=vd.getVoxel(x-1,y+0,z+1),
            vx0y2z0=vd.getVoxel(x-1,y+1,z-1),
            vx0y2z1=vd.getVoxel(x-1,y+1,z+0),
            vx0y2z2=vd.getVoxel(x-1,y+1,z+1),
            vx1y0z0=vd.getVoxel(x+0,y-1,z-1),
            vx1y0z1=vd.getVoxel(x+0,y-1,z+0),
            vx1y0z2=vd.getVoxel(x+0,y-1,z+1),
            vx1y1z0=vd.getVoxel(x+0,y+0,z-1),
            vx1y1z1=vd.getVoxel(x+0,y+0,z+0),
            vx1y1z2=vd.getVoxel(x+0,y+0,z+1),
            vx1y2z0=vd.getVoxel(x+0,y+1,z-1),
            vx1y2z1=vd.getVoxel(x+0,y+1,z+0),
            vx1y2z2=vd.getVoxel(x+0,y+1,z+1),
            vx2y0z0=vd.getVoxel(x+1,y-1,z-1),
            vx2y0z1=vd.getVoxel(x+1,y-1,z+0),
            vx2y0z2=vd.getVoxel(x+1,y-1,z+1),
            vx2y1z0=vd.getVoxel(x+1,y+0,z-1),
            vx2y1z1=vd.getVoxel(x+1,y+0,z+0),
            vx2y1z2=vd.getVoxel(x+1,y+0,z+1),
            vx2y2z0=vd.getVoxel(x+1,y+1,z-1),
            vx2y2z1=vd.getVoxel(x+1,y+1,z+0),
            vx2y2z2=vd.getVoxel(x+1,y+1,z+1);

        for (i=0; i<6; i++) faces[i]=new Face(owner);
        for (i=0; i<12; i++) edges[i]=new Edge(owner);
        for (i=0; i<8; i++) verts[i]=new Vertex(owner);

        faces[0].setEdges(edges[0],edges[1],edges[2],edges[3]);
        faces[1].setEdges(edges[0],edges[4],edges[8],edges[5]);
        faces[2].setEdges(edges[1],edges[5],edges[9],edges[6]);
        faces[3].setEdges(edges[3],edges[7],edges[11],edges[4]);
        faces[4].setEdges(edges[2],edges[6],edges[10],edges[7]);
        faces[5].setEdges(edges[11],edges[10],edges[9],edges[8]);

        faces[0].setVertices(verts[0],verts[1],verts[2],verts[3]);
        faces[1].setVertices(verts[0],verts[4],verts[5],verts[1]);
        faces[2].setVertices(verts[1],verts[5],verts[6],verts[2]);
        faces[3].setVertices(verts[0],verts[3],verts[7],verts[4]);
        faces[4].setVertices(verts[3],verts[2],verts[6],verts[7]);
        faces[5].setVertices(verts[7],verts[6],verts[5],verts[4]);

        faces[0].setVoxelNeighbors(vx1y0z1);
        faces[1].setVoxelNeighbors(vx2y1z1);
        faces[2].setVoxelNeighbors(vx1y1z2);
        faces[3].setVoxelNeighbors(vx1y1z0);
        faces[4].setVoxelNeighbors(vx0y1z1);
        faces[5].setVoxelNeighbors(vx1y2z1);

        edges[0].setFaces(faces[0],faces[1]);
        edges[1].setFaces(faces[0],faces[2]);
        edges[2].setFaces(faces[0],faces[4]);
        edges[3].setFaces(faces[0],faces[3]);
        edges[4].setFaces(faces[1],faces[3]);
        edges[5].setFaces(faces[1],faces[2]);
        edges[6].setFaces(faces[2],faces[4]);
        edges[7].setFaces(faces[3],faces[4]);
        edges[8].setFaces(faces[1],faces[5]);
        edges[9].setFaces(faces[2],faces[5]);
        edges[10].setFaces(faces[4],faces[5]);
        edges[11].setFaces(faces[3],faces[5]);

        edges[0].setVertices(verts[0],verts[1]);
        edges[1].setVertices(verts[1],verts[2]);
        edges[2].setVertices(verts[2],verts[3]);
        edges[3].setVertices(verts[3],verts[0]);
        edges[4].setVertices(verts[0],verts[4]);
        edges[5].setVertices(verts[1],verts[5]);
        edges[6].setVertices(verts[2],verts[6]);
        edges[7].setVertices(verts[3],verts[7]);
        edges[8].setVertices(verts[4],verts[5]);
        edges[9].setVertices(verts[5],verts[6]);
        edges[10].setVertices(verts[6],verts[7]);
        edges[11].setVertices(verts[7],verts[4]);

        edges[0].setVoxelNeighbors(vx2y1z1,vx2y0z1,vx1y0z1);
        edges[1].setVoxelNeighbors(vx1y1z2,vx1y0z2,vx1y0z1);
        edges[2].setVoxelNeighbors(vx0y1z1,vx0y0z1,vx1y0z1);
        edges[3].setVoxelNeighbors(vx1y1z0,vx1y0z0,vx1y0z1);
        edges[4].setVoxelNeighbors(vx1y1z0,vx2y1z0,vx2y1z1);
        edges[5].setVoxelNeighbors(vx2y1z1,vx2y1z2,vx1y1z2);
        edges[6].setVoxelNeighbors(vx1y1z2,vx0y1z2,vx0y1z1);
        edges[7].setVoxelNeighbors(vx0y1z1,vx0y1z0,vx1y1z0);
        edges[8].setVoxelNeighbors(vx2y1z1,vx2y2z1,vx1y2z1);
        edges[9].setVoxelNeighbors(vx1y1z2,vx1y2z2,vx1y2z1);
        edges[10].setVoxelNeighbors(vx0y1z1,vx0y2z1,vx1y2z1);
        edges[11].setVoxelNeighbors(vx1y1z0,vx1y2z0,vx1y2z1);

        verts[0].setEdges(edges[0],edges[3],edges[4]);
        verts[1].setEdges(edges[1],edges[0],edges[5]);
        verts[2].setEdges(edges[2],edges[1],edges[6]);
        verts[3].setEdges(edges[3],edges[2],edges[7]);
        verts[4].setEdges(edges[4],edges[11],edges[8]);
        verts[5].setEdges(edges[9],edges[5],edges[8]);
        verts[6].setEdges(edges[10],edges[6],edges[9]);
        verts[7].setEdges(edges[7],edges[10],edges[11]);

        verts[0].setFaces(faces[0],faces[3],faces[1]);
        verts[1].setFaces(faces[0],faces[1],faces[2]);
        verts[2].setFaces(faces[0],faces[2],faces[4]);
        verts[3].setFaces(faces[0],faces[4],faces[3]);
        verts[4].setFaces(faces[1],faces[3],faces[5]);
        verts[5].setFaces(faces[2],faces[1],faces[5]);
        verts[6].setFaces(faces[4],faces[2],faces[5]);
        verts[7].setFaces(faces[3],faces[4],faces[5]);

        verts[0].setVoxelNeighbors(vx2y0z0,vx2y0z1,vx2y1z0,vx2y1z1,vx1y0z0,vx1y0z1,vx1y1z0);
        verts[1].setVoxelNeighbors(vx2y0z1,vx2y0z2,vx2y1z1,vx2y1z2,vx1y0z1,vx1y0z2,vx1y1z2);
        verts[2].setVoxelNeighbors(vx0y0z1,vx0y0z2,vx0y1z1,vx0y1z2,vx1y0z1,vx1y0z2,vx1y1z2);
        verts[3].setVoxelNeighbors(vx0y0z0,vx0y0z1,vx0y1z0,vx0y1z1,vx1y0z0,vx1y0z1,vx1y1z0);
        verts[4].setVoxelNeighbors(vx2y1z0,vx2y1z1,vx2y2z0,vx2y2z1,vx1y1z0,vx1y2z1,vx1y2z0);
        verts[5].setVoxelNeighbors(vx2y1z1,vx2y1z2,vx2y2z1,vx2y2z2,vx1y2z1,vx1y1z2,vx1y2z2);
        verts[6].setVoxelNeighbors(vx0y1z1,vx0y1z2,vx0y2z1,vx0y2z2,vx1y2z1,vx1y1z2,vx1y2z2);
        verts[7].setVoxelNeighbors(vx0y1z0,vx0y1z1,vx0y2z0,vx0y2z1,vx1y1z0,vx1y2z1,vx1y2z0);


        switch (vd.getConnectivity())
        {
            case VoxelData.VERTEX_CONN:
                for (i=0; i<6; i++) faces[i].calcMark1();
                for (i=0; i<12; i++) edges[i].calcMark1();
                for (i=0; i<8; i++) verts[i].calcMark1();
                break;
            case VoxelData.FACE_CONN:
                for (i=0; i<6; i++) faces[i].calcMark3();
                for (i=0; i<12; i++) edges[i].calcMark3();
                for (i=0; i<8; i++) verts[i].calcMark3();
                break;
            default:
                System.err.println("Connectivity type not supported.");
                for (i=0; i<6; i++) faces[i].calcMark3();
                for (i=0; i<12; i++) edges[i].calcMark3();
                for (i=0; i<8; i++) verts[i].calcMark3();
        }

        ElementaryCube[] r=new ElementaryCube[6+12+8];
        for (i=0; i<6; i++) r[i]=faces[i];
        for (i=0; i<12; i++) r[i+6]=edges[i];
        for (i=0; i<8; i++) r[i+6+12]=verts[i];

        return r;
    }
}
