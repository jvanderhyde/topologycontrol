//TopologyChecker, a class for storing all the voxel neighborhood possibilities
//James Vanderhyde, 16 July 2002

import java.io.*;

public class TopologyChecker
{
    protected boolean dataPrecomputed;
    protected byte[] data;
    boolean[] marks;
    int[][] cases;
    static final int casesDim=14;

    public TopologyChecker()
    {
        dataPrecomputed=false;
        data=null;
        marks=new boolean[26];
        cases=new int[casesDim][casesDim];
    }

    public void loadData()
    {
        loadData(VoxelData.FACE_CONN);
    }

    public void loadData(int connectivity)
    {
        switch (connectivity)
        {
            case VoxelData.VERTEX_CONN:
                loadData("topoinfo_vertex");
                break;
            case VoxelData.FACE_CONN:
                loadData("topoinfo_face");
                break;
            default:
                System.err.println("Connectivity type not supported.");
                loadData("topoinfo_face");
        }
    }
    
    public void loadData(String filename)
    {
        DataInputStream in;
        try
        {
            in=new DataInputStream(new BufferedInputStream(new FileInputStream(filename)));
        }
        catch (FileNotFoundException e)
        {
            System.err.println(e);
            return;
        }

        System.out.print("Topology data loading...");
        data=new byte[8*1024*1024];	// 2^26/8 = 2^23 = 8*1024*1024
        try
        {
            in.readFully(data);
        }
        catch (IOException e)
        {
            System.out.println();
            System.err.println(e);
            data=null;
            return;
        }

        System.out.println("done.");
        dataPrecomputed=true;
    }

    public boolean removalChangesTopology(Voxel v,VoxelData vd) /*fast*/
    {
        if (dataPrecomputed)
            return checkData(v,vd);
        else
        {
            int ncc=numConnectedComponents(v,vd);
            //System.out.println("Voxel "+v.coordsToString()+" has "+ncc+" connected components");
            return (ncc > 2);
       }
    }

    protected boolean checkData(Voxel v,VoxelData vd) /*fast*/
    {
        int i,index=0;
        getMarksInOrder(v,vd);
        for (i=0; i<25; i++)
        {
            if (marks[i]) index |= 1;
            index <<= 1;
        }
        if (marks[i]) index |= 1;
        byte b=data[index >> 3];
        int o=1 << (index & 0x07);
        return ((b & o)==o);
    }

    protected void getMarksInOrder(Voxel v,VoxelData vd) /*fast*/
    {
        int x=v.getX(),y=v.getY(),z=v.getZ();
        final int markFlag=VoxelData.OUTSIDE;
        marks[0]=vd.getFlag(x-1,y-1,z-1,markFlag);
        marks[1]=vd.getFlag(x,y-1,z-1,markFlag);
        marks[2]=vd.getFlag(x+1,y-1,z-1,markFlag);
        marks[3]=vd.getFlag(x-1,y,z-1,markFlag);
        marks[4]=vd.getFlag(x,y,z-1,markFlag);
        marks[5]=vd.getFlag(x+1,y,z-1,markFlag);
        marks[6]=vd.getFlag(x-1,y+1,z-1,markFlag);
        marks[7]=vd.getFlag(x,y+1,z-1,markFlag);
        marks[8]=vd.getFlag(x+1,y+1,z-1,markFlag);
        marks[9]=vd.getFlag(x-1,y-1,z,markFlag);
        marks[10]=vd.getFlag(x,y-1,z,markFlag);
        marks[11]=vd.getFlag(x+1,y-1,z,markFlag);
        marks[12]=vd.getFlag(x-1,y,z,markFlag);
        //marks[0]=vd.getFlag(x,y,z,markFlag); //I'm not my own neighbor
        marks[13]=vd.getFlag(x+1,y,z,markFlag);
        marks[14]=vd.getFlag(x-1,y+1,z,markFlag);
        marks[15]=vd.getFlag(x,y+1,z,markFlag);
        marks[16]=vd.getFlag(x+1,y+1,z,markFlag);
        marks[17]=vd.getFlag(x-1,y-1,z+1,markFlag);
        marks[18]=vd.getFlag(x,y-1,z+1,markFlag);
        marks[19]=vd.getFlag(x+1,y-1,z+1,markFlag);
        marks[20]=vd.getFlag(x-1,y,z+1,markFlag);
        marks[21]=vd.getFlag(x,y,z+1,markFlag);
        marks[22]=vd.getFlag(x+1,y,z+1,markFlag);
        marks[23]=vd.getFlag(x-1,y+1,z+1,markFlag);
        marks[24]=vd.getFlag(x,y+1,z+1,markFlag);
        marks[25]=vd.getFlag(x+1,y+1,z+1,markFlag);
    }

    protected int numConnectedComponents(Voxel v,VoxelData vd) /*somewhat fast*/
    {
        ElementaryCube[] boundary;
        ElementaryCube current;
        ElementaryCube[] curNeighbors;
        Stack dfs;
        int c,n;
        int numComponents,numMarkedComponents,numUnmarkedComponents;
        boolean marked;

        dfs=new Stack();
        boundary=ElementaryCube.buildGraph(vd,v.getX(),v.getY(),v.getZ());
        for (c=0; c<boundary.length; c++)
            boundary[c].setLabel(0);
        numComponents=0;
        numMarkedComponents=0;
        numUnmarkedComponents=0;
        for (c=0; c<boundary.length; c++)
        {
            if (boundary[c].getLabel()<1)
            {
                numComponents++;
                marked=boundary[c].getMark();
                if (marked) numMarkedComponents++;
                else numUnmarkedComponents++;
                dfs.push(boundary[c]);
                while (!dfs.isEmpty())
                {
                    current=(ElementaryCube)dfs.pop();
                    current.setLabel(numComponents);
                    curNeighbors=current.getNeighbors();
                    for (n=0; n<curNeighbors.length; n++)
                    {
                        if ((curNeighbors[n].getLabel()<1) &&
                            (curNeighbors[n].getMark()==marked))
                        {
                            dfs.push(curNeighbors[n]);
                        }
                    }
                }
            }
        }
        cases[numMarkedComponents][numUnmarkedComponents]++;
        return numComponents;
    }

    public void precomputeData(int connectivity)
    {
        float[][][] one=new float[1][1][1];
        one[0][0][0]=1f;
        for (int i=0; i<casesDim; i++) for (int j=0; j<casesDim; j++) cases[i][j]=0;
        VoxelData vd=new VoxelData(one,connectivity);
        Voxel v=vd.getVoxel(0,0,0);
        Voxel[] neighbors=getNeighborsInOrder(v,vd);
        data=new byte[8*1024*1024];	// 2^26/8 = 2^23 = 8*1024*1024
        precomputeDataRecursion(0,false,v,vd,neighbors);
        precomputeDataRecursion(0,true,v,vd,neighbors);
        dataPrecomputed=true;
    }

    protected void precomputeDataRecursion(int n,boolean mark,Voxel v,VoxelData vd,Voxel[] neighbors)
    {
        vd.setMark(neighbors[n],mark);
        if (n<25)
        {
            precomputeDataRecursion(n+1,false,v,vd,neighbors);
            precomputeDataRecursion(n+1,true,v,vd,neighbors);
        }
        else
        {
            setData(neighbors,(numConnectedComponents(v,vd) != 2));
        }
        if (n==8) System.out.print(".");
        if (n==3) System.out.println();
        if (n==1) System.out.println();
    }

    protected void setData(Voxel[] neighbors,boolean changesTopo)
    {
        int i,index=0;
        for (i=0; i<25; i++)
        {
            if (neighbors[i].getMark()) index |= 1;
            index <<= 1;
        }
        if (neighbors[i].getMark()) index |= 1;
        byte b=data[index >> 3];
        int o=index & 0x07;
        if (changesTopo)
            data[index >> 3]=setBit(b,o);
        else
            data[index >> 3]=clearBit(b,o);
    }

    private byte setBit(byte b,int bit)
    {
        return (byte)(b | (1 << bit));
    }

    private byte clearBit(byte b,int bit)
    {
        return (byte)(b & ~(1 << bit));
    }

    protected Voxel[] getNeighborsInOrder(Voxel v,VoxelData vd)
    {
        int x=v.getX(),y=v.getY(),z=v.getZ();
        Voxel[] neighbors=new Voxel[26];
        neighbors[0]=vd.getVoxel(x-1,y-1,z-1);
        neighbors[1]=vd.getVoxel(x,y-1,z-1);
        neighbors[2]=vd.getVoxel(x+1,y-1,z-1);
        neighbors[3]=vd.getVoxel(x-1,y,z-1);
        neighbors[4]=vd.getVoxel(x,y,z-1);
        neighbors[5]=vd.getVoxel(x+1,y,z-1);
        neighbors[6]=vd.getVoxel(x-1,y+1,z-1);
        neighbors[7]=vd.getVoxel(x,y+1,z-1);
        neighbors[8]=vd.getVoxel(x+1,y+1,z-1);
        neighbors[9]=vd.getVoxel(x-1,y-1,z);
        neighbors[10]=vd.getVoxel(x,y-1,z);
        neighbors[11]=vd.getVoxel(x+1,y-1,z);
        neighbors[12]=vd.getVoxel(x-1,y,z);
        //neighbors[0]=vd.getVoxel(x,y,z); //I'm not my own neighbor
        neighbors[13]=vd.getVoxel(x+1,y,z);
        neighbors[14]=vd.getVoxel(x-1,y+1,z);
        neighbors[15]=vd.getVoxel(x,y+1,z);
        neighbors[16]=vd.getVoxel(x+1,y+1,z);
        neighbors[17]=vd.getVoxel(x-1,y-1,z+1);
        neighbors[18]=vd.getVoxel(x,y-1,z+1);
        neighbors[19]=vd.getVoxel(x+1,y-1,z+1);
        neighbors[20]=vd.getVoxel(x-1,y,z+1);
        neighbors[21]=vd.getVoxel(x,y,z+1);
        neighbors[22]=vd.getVoxel(x+1,y,z+1);
        neighbors[23]=vd.getVoxel(x-1,y+1,z+1);
        neighbors[24]=vd.getVoxel(x,y+1,z+1);
        neighbors[25]=vd.getVoxel(x+1,y+1,z+1);
        return neighbors;
    }

    public void saveData(int connectivity)
    {
        switch (connectivity)
        {
            case VoxelData.VERTEX_CONN:
                saveData("topoinfo_vertex");
                break;
            case VoxelData.FACE_CONN:
                saveData("topoinfo_face");
                break;
            default:
                System.err.println("Connectivity type not supported.");
                System.err.println("No file written.");
        }
    }

    public void saveData(String filename)
    {
        DataOutputStream out;
        try
        {
            out=new DataOutputStream(new BufferedOutputStream(new FileOutputStream(filename)));
        }
        catch (FileNotFoundException e)
        {
            System.err.println(e);
            System.err.println("No file written.");
            return;
        }

        try
        {
            out.write(data);
            out.close();
            System.out.println("Data written to file "+filename+" successfully.");
        }
        catch (IOException e)
        {
            System.err.println(e);
        }
    }

    public void printCases()
    {
        int i,j;
        for (i=0; i<casesDim; i++)
        {
            for (j=0; j<casesDim; j++)
            {
                System.out.print(" "+cases[i][j]);
            }
            System.out.println();
        }
    }

    public static void main(String[] args)
    {
        TopologyChecker topocheck=new TopologyChecker();
        int conn=VoxelData.VERTEX_CONN;
        topocheck.precomputeData(conn);
        topocheck.printCases();
        System.out.print("Saving...");
        topocheck.saveData(conn);
    }
    
}
