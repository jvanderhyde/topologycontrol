//
//  TopologyChecker2D.java
//  
//
//  Created by James Vanderhyde on 3/7/06.
//
// Each pixel x has 8 neighbors, laid out like this:
//
//     0  1  2
//     3  x  4
//     5  6  7
//
// Therefore the state of the neighbors can be represented in an unsigend byte.
//  Each neighboring pixel is associated with exactly one lower-dimensional face of x,
//  but which faces are marked depends on the connectivity.
//  We're looking at the connected components of the graph of the marked faces.

import java.io.*;

public class TopologyChecker2D
{
    protected boolean dataPrecomputed;
    protected byte[] data;
    
    public void printCase(int connectivity,int state,int[] next,int[] prev,int numComps)
    {
	int p=0;
	while (p<3)
	{
	    if (checkPixel(state,p++)) System.out.print(" x ");
	    else System.out.print(" . ");
	}
	System.out.println();
	if (checkPixel(state,p++)) System.out.print(" x ");
	else System.out.print(" . ");
	System.out.print(" "+numComps+" ");
	if (checkPixel(state,p++)) System.out.print(" x ");
	else System.out.print(" . ");
	System.out.println();
	while (p<8)
	{
	    if (checkPixel(state,p++)) System.out.print(" x ");
	    else System.out.print(" . ");
	}
	System.out.println();
	System.out.println();
    }
    
    public boolean checkPixel(int state,int pix)
    {
	return ((state & (1<<pix)) >= 1);
    }

    public boolean calcMark(int connectivity,int state,int cube,int[] next,int[] prev)
    {
	if (connectivity==1)
	{
	    //We mark the cube if any of its incident pixels (except x) are marked.
	    if ((cube==1) || (cube==3) || (cube==4) || (cube==6))
	    {
		return checkPixel(state,cube);
	    }
	    else
	    {
		return checkPixel(state,prev[cube]) || checkPixel(state,cube) || checkPixel(state,next[cube]);
	    }
	}
	else if (connectivity==2)
	{
	    //We mark the cube if all of its incident pixels (except x) are marked.
	    if ((cube==1) || (cube==3) || (cube==4) || (cube==6))
	    {
		return checkPixel(state,cube);
	    }
	    else
	    {
		return checkPixel(state,prev[cube]) && checkPixel(state,cube) && checkPixel(state,next[cube]);
	    }
	}
	else return false;
    }
    
    public void precomputeData(int connectivity)
    {
	data=new byte[32];  //2^8/3 = 2^5 = 32
	int[] next = {1,2,4,0,7,3,5,6};
	int[] prev = {3,0,1,5,2,6,7,4};
	int state;
	for (state=0; state<256; state++)
	{
	    int n,pix=0;
	    boolean mark=calcMark(connectivity,state,pix,next,prev);
	    int numComponents=0;
	    for (n=0; n<8; n++)
	    {
		pix=next[pix];
		if (calcMark(connectivity,state,pix,next,prev) != mark)
		{
		    numComponents++;
		    mark = !mark;
		}
	    }
	    setData(state,(numComponents != 2));
	    printCase(connectivity,state,next,prev,numComponents);
	}
	dataPrecomputed=true;
    }

    protected void setData(int neighborStates,boolean changesTopo)
    {
        int index=neighborStates;
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
    
    public void saveData(int connectivity)
    {
        switch (connectivity)
        {
            case 1:
                saveData("topoinfo2D_vertex");
                break;
            case 2:
                saveData("topoinfo2D_face");
                break;
            default:
                System.err.println("Connectivity type not supported.");
                System.err.println("No file written.");
        }
    }
    
    public void saveData(String filename)
    {
        System.out.print("Saving...");
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
    
    public static void main(String[] args)
    {
        TopologyChecker2D topocheck=new TopologyChecker2D();
        int conn=1;
        topocheck.precomputeData(conn);
        //topocheck.printCases();
        topocheck.saveData(conn);
    }
    
}
