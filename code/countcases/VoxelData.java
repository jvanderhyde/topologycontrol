//VoxelData--a class for reading and storing voxel data
//James Vanderhyde, 15 May 2002

import java.io.*;

public class VoxelData
{
    protected int sizex,sizey,sizez;
    protected float[][][] values;
    protected byte[][][] flags;
    protected int connectivity;

    public static final int VERTEX_CONN=1,EDGE_CONN=2,FACE_CONN=3;
    public static final int OUTSIDE=1,BOUNDARY=2,TOPO_CHANGE=4;

    public VoxelData()
    {
        this(FACE_CONN);
    }

    public VoxelData(int connectivityType)
    {
        sizex=sizey=sizez=0;
        values=new float[sizex][sizey][sizez];
        flags=new byte[sizex][sizey][sizez];
        connectivity=connectivityType;
    }

    public VoxelData(float[][][] p_values,int connectivityType)
    {
        int x,y,z;
        sizex=sizey=sizez=0;
        sizex=p_values.length;
        if (sizex>0) sizey=p_values[0].length;
        if (sizey>0) sizez=p_values[0][0].length;
        initDataArrays();
        for (x=0; x<sizex; x++)
            for (y=0; y<sizey; y++)
                for (z=0; z<sizez; z++)
                    setUpDatum(p_values[x][y][z],x,y,z);
        setUpOutsideVoxels();
        connectivity=connectivityType;
    }

    public VoxelData(String filename,int connectivityType)
    {
        readFile(filename);
        setUpOutsideVoxels();
        connectivity=connectivityType;
    }

    public void readFile(String filename)
    {
        try
        {
            if (filename.endsWith(".dat"))
                readDATFile(filename);
            else if (filename.endsWith(".hdr"))
                readANALYZEFile(filename);
            else if (filename.endsWith(".img"))
                readANALYZEFile(filename);
            else if (filename.endsWith(".v"))
                readVFile(filename);

            else
                throw new IOException("Unsupported file format");
        }
        catch (IOException e)
        {
            System.err.println(e);
            System.exit(-1);
        }
        catch (NumberFormatException e)
        {
            System.err.println("Incorrect format in file "+filename);
            System.err.println(e);
            System.exit(-1);
        }
    }

    protected void initDataArrays()
    {
        values=new float[sizex+2][sizey+2][sizez+2];
        flags=new byte[sizex+2][sizey+2][sizez+2];
    }
    
    protected void setUpDatum(double p_val,int x,int y,int z)
    {
        setUpDatum(p_val,x,y,z,false);
    }
    
    protected void setUpDatum(double p_val,int x,int y,int z,boolean p_mark)
    {
        values[x+1][y+1][z+1]=(float)p_val;
        setFlag(x,y,z,OUTSIDE,p_mark);
        setFlag(x,y,z,BOUNDARY,false);
    }

    protected void setUpOutsideVoxels()
    {
        int x,y,z;

        //Set up voxels outside of data
        z=0;
        for (x=0; x<sizex+2; x++)
            for (y=0; y<sizey+1; y++)
                setUpDatum(0.0,x-1,y-1,z-1,true);
        y=sizey+1;
        for (x=0; x<sizex+2; x++)
            for (z=0; z<sizez+1; z++)
                setUpDatum(0.0,x-1,y-1,z-1,true);
        z=sizez+1;
        for (x=0; x<sizex+2; x++)
            for (y=1; y<sizey+2; y++)
                setUpDatum(0.0,x-1,y-1,z-1,true);
        y=0;
        for (x=0; x<sizex+2; x++)
            for (z=1; z<sizez+2; z++)
                setUpDatum(0.0,x-1,y-1,z-1,true);
        x=0;
        for (y=1; y<sizey+1; y++)
            for (z=1; z<sizez+1; z++)
                setUpDatum(0.0,x-1,y-1,z-1,true);
        x=sizex+1;
        for (y=1; y<sizey+1; y++)
            for (z=1; z<sizez+1; z++)
                setUpDatum(0.0,x-1,y-1,z-1,true);
    }

    public void readDATFile(String filename)
        throws IOException,NumberFormatException
    {
        java.util.StringTokenizer st;
	TokenReader in;
	String word;
	int x,y,z;
	try
	{
	    in=new TokenReader(new FileReader(filename));
	}
	catch (FileNotFoundException e)
	{
	    throw new IOException(e.toString());
	}

        //Read in dimensions
	word=in.readToken();
	if (word==null) throw new IOException("EOF reached prematurely");
	this.sizex=Integer.parseInt(word);
	word=in.readToken();
	if (word==null) throw new IOException("EOF reached prematurely");
	this.sizey=Integer.parseInt(word);
	word=in.readToken();
	if (word==null) throw new IOException("EOF reached prematurely");
	this.sizez=Integer.parseInt(word);

        //Read in data
	initDataArrays();
	for (x=0; x<sizex; x++)
	    for (y=0; y<sizey; y++)
		for (z=0; z<sizez; z++)
		{
		    word=in.readToken();
		    if (word==null) 
			throw new IOException("EOF reached prematurely");
		    setUpDatum((new Double(word)).doubleValue(),x,y,z);
		}
    }

    public void readVFile(String filename)
        throws IOException
    {
        DataInputStream in;
        try
        {
            in=new DataInputStream(new BufferedInputStream(new FileInputStream(filename)));
        }
        catch (FileNotFoundException e)
        {
            throw new IOException(e.toString());
        }

        int dimx,dimy,dimz;
        //dimx=readSwappedInt(in);
        //dimy=readSwappedInt(in);
        //dimz=readSwappedInt(in);
        dimx=in.readInt();
        dimy=in.readInt();
        dimz=in.readInt();
        
        int rate=1;
        this.sizex=dimx/rate;
        this.sizey=dimy/rate;
        this.sizez=dimz/rate;

        int x,y,z;
        float datum,min=Float.POSITIVE_INFINITY,max=Float.NEGATIVE_INFINITY;
        int numVoxels=0;
        System.out.print("("+dimx+","+dimy+","+dimz+")");
        initDataArrays();
        for (z=0; z<dimz; z++)
        {
            System.out.print(" "+z);
            for (y=0; y<dimy; y++)
                for (x=0; x<dimx; x++)
                {
                    if ((z%rate==rate-1) && (y%rate==rate-1) && (x%rate==rate-1))
                    {
                        //Isosurface algorithm assumes voxel data are densities.
                        //Since the .v files are using distance from surface,
                        //we use the negative so the smallest are towards the outside.
                        //datum = -readSwappedFloat(in);
                        datum = -in.readFloat();
                        setUpDatum((double)datum,x/rate,y/rate,sizez-z/rate-1);
                        numVoxels++;
                        if (datum<min) min=datum;
                        if (datum>max) max=datum;
                    }
                    else
                    {
                        in.skipBytes(4);
                    }
                }
        }
        System.out.println("("+numVoxels+")["+min+","+max+"]");

    }

    public int readSwappedInt(DataInput in)
        throws IOException
    {
        int a,b,c,d;

        a=(int)in.readByte();
        b=(int)in.readByte();
        c=(int)in.readByte();
        d=(int)in.readByte();
        
        return (((d & 0xff) << 24) | ((c & 0xff) << 16) |
                ((b & 0xff) << 8) | (a & 0xff));
    }

    public float readSwappedFloat(DataInput in)
        throws IOException
    {
        return Float.intBitsToFloat(readSwappedInt(in));
    }
        
    public void readANALYZEFile(String filename)
        throws IOException
    {
        int i;
        String name,hdrFilename,imgFilename;
        if ((filename.endsWith(".hdr")) || (filename.endsWith(".img")))
            name=filename.substring(0,filename.lastIndexOf("."));
        else
            name=filename;
        hdrFilename=name+".hdr";
        imgFilename=name+".img";

        DataInputStream in;
        try
        {
            in=new DataInputStream(new BufferedInputStream(new FileInputStream(hdrFilename)));
        }
        catch (FileNotFoundException e)
        {
            throw new IOException(e.toString());
        }

        int sizeof_hdr;
        byte[] data_type=new byte[10];
        byte[] db_name=new byte[18];
        int extents;
        short session_error;
        byte regular;
        byte hkey_un0;
        short[] dim=new short[8];
        short[] unused=new short[7];
        short datatype;
        short bitpix;
        short dim_un0;

        sizeof_hdr=in.readInt();
        in.readFully(data_type,0,10);
        in.readFully(db_name,0,18);
        extents=in.readInt();
        session_error=in.readShort();
        regular=in.readByte();
        hkey_un0=in.readByte();
        for (i=0; i<8; i++)
            dim[i]=in.readShort();
        for (i=0; i<7; i++)
            unused[i]=in.readShort();
        datatype=in.readShort();
        bitpix=in.readShort();
        dim_un0=in.readShort();

        int rate=1;
        this.sizex=dim[1]/rate;
        this.sizey=dim[2]/rate;
        this.sizez=dim[3]/rate;

        int x,y,z;
        int datum;
        try
        {
            in=new DataInputStream(new BufferedInputStream(new FileInputStream(imgFilename)));
        }
        catch (FileNotFoundException e)
        {
            throw new IOException(e.toString());
        }

        int numVoxels=0;
        System.out.print("("+dim[1]+","+dim[2]+","+dim[3]+")");
        initDataArrays();
        for (z=0; z<dim[3]; z++)
        {
            System.out.print(" "+z);
            for (y=0; y<dim[2]; y++)
                for (x=0; x<dim[1]; x++)
                {
                    if ((z%rate==rate-1) && (y%rate==rate-1) && (x%rate==rate-1))
                    {
                        datum=in.readUnsignedByte();
                        setUpDatum((double)datum,x/rate,y/rate,z/rate);
                        numVoxels++;
                    }
                    else
                    {
                        in.skipBytes(1);
                    }
                }
        }
        System.out.println("("+numVoxels+")");
    }

    public void writeVFile(String filename,double p_thresh)
        throws IOException
    {
        DataOutputStream out;
        try
        {
            out=new DataOutputStream(new BufferedOutputStream(new FileOutputStream(filename)));
        }
        catch (FileNotFoundException e)
        {
            throw new IOException(e.toString());
        }

        //writeSwappedInt(sizex,out);
        //writeSwappedInt(sizey,out);
        //writeSwappedInt(sizez,out);
        out.writeInt(sizex);
        out.writeInt(sizey);
        out.writeInt(sizez);
        
        int x,y,z;
        float datum;
        float thresh=(float)p_thresh;
        for (z=0; z<sizez; z++)
            for (y=0; y<sizey; y++)
                for (x=0; x<sizex; x++)
                {
                    if (getFlag(x,y,sizez-z-1,OUTSIDE))
                        datum=thresh-values[x+1][y+1][sizez-z];
                    else
                    {
                        datum=thresh-values[x+1][y+1][sizez-z];
                        if (datum>-0.1f)
                            datum=-0.1f;
                    }
                    //writeSwappedFloat(datum,out);
                    out.writeFloat(datum);
                }
        out.close();

        System.out.println("Wrote data to file "+filename);
    }

    public void writeSwappedInt(int v,DataOutput out)
        throws IOException
    {
        byte a,b,c,d;

        a=(byte)(0xff & (v >> 24));
        b=(byte)(0xff & (v >> 16));
        c=(byte)(0xff & (v >> 8));
        d=(byte)(0xff & v);

        out.writeByte(d);
        out.writeByte(c);
        out.writeByte(b);
        out.writeByte(a);
    }

    public void writeSwappedFloat(float v,DataOutput out)
        throws IOException
    {
        writeSwappedInt(Float.floatToIntBits(v),out);
    }

    public boolean hasData()
    {
        return ((sizex+sizey+sizez)>0);
    }
    
    public void unmarkAll()
    {
	int x,y,z;
	for (x=0; x<sizex; x++)
            for (y=0; y<sizey; y++)
                for (z=0; z<sizez; z++)
                {
                    setFlag(x,y,z,OUTSIDE,false);
                    setFlag(x,y,z,BOUNDARY,false);
                }
    }

    public void markBelowThresh(double thresh)
    {
        int x,y,z;
        for (x=0; x<sizex; x++)
            for (y=0; y<sizey; y++)
                for (z=0; z<sizez; z++)
                {
                    if (values[x+1][y+1][z+1]<=thresh)
                        setFlag(x,y,z,OUTSIDE,true);
                }
                    
        int n;
        Voxel[] neighbors;
        boolean mk,bd;
        int iterations=0;
        for (x=0; x<sizex; x++)
            for (y=0; y<sizey; y++)
                for (z=0; z<sizez; z++)
                {
                    bd=false;
                    neighbors=this.getNeighbors(getVoxel(x,y,z));
                    mk=getFlag(x,y,z,OUTSIDE);
                    for (n=0; n<neighbors.length; n++)
                    {
                        if (neighbors[n].isMarked() != mk) bd=true;
                    }
                    if (bd)
                        setFlag(x,y,z,BOUNDARY,true);
                    iterations++;
                    if (iterations%10000==0) System.out.print(".");
                    if (iterations%10000000==0) System.out.println(iterations/10000000);
                }
        if (iterations>10000) System.out.println();
    }
        
    public int getConnectivity()
    {
        return connectivity;
    }
    
    public int getSize()
    {
        return Math.min(Math.min(sizex,sizey),sizez);
    }

    public int getMaxSize()
    {
        return Math.max(Math.max(sizex,sizey),sizez);
    }

    public int getSizeX()
    {
        return sizex;
    }

    public int getSizeY()
    {
        return sizey;
    }

    public int getSizeZ()
    {
        return sizez;
    }

    public int[] getDimensions()
    {
        int[] dims=new int[3];
	dims[0]=sizex;
	dims[1]=sizey;
	dims[2]=sizez;
	return dims;
    }

    public void printDimensions(PrintStream out)
    {
        out.print("("+sizex+","+sizey+","+sizez+")");
    }
        
    public double getDataPoint(int x,int y,int z)
    {
        return values[x+1][y+1][z+1];
    }
    
    public Voxel getVoxel(int x,int y,int z)
    {
        Voxel r=new Voxel(values[x+1][y+1][z+1],x,y,z,getFlag(x,y,z,OUTSIDE),getFlag(x,y,z,BOUNDARY));
        /*if ((x!=r.getX()) || (y!=r.getY()) || (z!=r.getZ()))
        {
            System.out.print("Error:");
            System.out.print("("+x+","+y+","+z+")-");
            System.out.print("("+((x<<20) | (y<<10) | z)+")-");
            System.out.println(r.coordsToString());
        }*/
        return r;
    }

    public Voxel[] getNeighbors(Voxel v)
    {
        Voxel[] neighbors;
        switch (connectivity)
        {
            case VoxelData.VERTEX_CONN:
                neighbors=new Voxel[6];
                break;
            case VoxelData.FACE_CONN:
                neighbors=new Voxel[26];
                break;
            default:
                System.err.println("Connectivity type not supported.");
                neighbors=new Voxel[26];
        }
        int numNeighbors=getNeighbors(v,neighbors);
        
        Voxel[] r=new Voxel[numNeighbors];
        for (int t=0; t<numNeighbors; t++)
            r[t]=neighbors[t];
        return r;        
    }

    //Assumes neighbors already allocated (for speed purposes)
    public int getNeighbors(Voxel v,Voxel[] neighbors)
    {
        switch (connectivity)
        {
            case VoxelData.VERTEX_CONN:
                return getNeighbors1(v,neighbors);
            case VoxelData.FACE_CONN:
                return getNeighbors3(v,neighbors);
            default:
                System.err.println("Connectivity type not supported.");
                return getNeighbors3(v,neighbors);
        }
    }
    
    public int getNeighbors1(Voxel v,Voxel[] neighbors)
    {
        int x=v.getX(),y=v.getY(),z=v.getZ();
        neighbors[0]=getVoxel(x-1,y,z);
        neighbors[1]=getVoxel(x+1,y,z);
        neighbors[2]=getVoxel(x,y-1,z);
        neighbors[3]=getVoxel(x,y+1,z);
        neighbors[4]=getVoxel(x,y,z-1);
        neighbors[5]=getVoxel(x,y,z+1);
        return 6;
    }

    public int getNeighbors3(Voxel v,Voxel[] neighbors)
    {
        int x=v.getX(),y=v.getY(),z=v.getZ();
        neighbors[0]=getVoxel(x-1,y-1,z-1);
        neighbors[1]=getVoxel(x,y-1,z-1);
        neighbors[2]=getVoxel(x+1,y-1,z-1);
        neighbors[3]=getVoxel(x-1,y,z-1);
        neighbors[4]=getVoxel(x,y,z-1);
        neighbors[5]=getVoxel(x+1,y,z-1);
        neighbors[6]=getVoxel(x-1,y+1,z-1);
        neighbors[7]=getVoxel(x,y+1,z-1);
        neighbors[8]=getVoxel(x+1,y+1,z-1);
        neighbors[9]=getVoxel(x-1,y-1,z);
        neighbors[10]=getVoxel(x,y-1,z);
        neighbors[11]=getVoxel(x+1,y-1,z);
        neighbors[12]=getVoxel(x-1,y,z);
        //neighbors[0]=getVoxel(x,y,z); //I'm not my own neighbor
        neighbors[13]=getVoxel(x+1,y,z);
        neighbors[14]=getVoxel(x-1,y+1,z);
        neighbors[15]=getVoxel(x,y+1,z);
        neighbors[16]=getVoxel(x+1,y+1,z);
        neighbors[17]=getVoxel(x-1,y-1,z+1);
        neighbors[18]=getVoxel(x,y-1,z+1);
        neighbors[19]=getVoxel(x+1,y-1,z+1);
        neighbors[20]=getVoxel(x-1,y,z+1);
        neighbors[21]=getVoxel(x,y,z+1);
        neighbors[22]=getVoxel(x+1,y,z+1);
        neighbors[23]=getVoxel(x-1,y+1,z+1);
        neighbors[24]=getVoxel(x,y+1,z+1);
        neighbors[25]=getVoxel(x+1,y+1,z+1);
        return 26;
    }


    //array-boundary safe version
    public int getNeighbors1safe(Voxel v,Voxel[] neighbors)
    {
        int x=v.getX(),y=v.getY(),z=v.getZ();
	int numNeighbors=0;
	if (x>-1)
	    neighbors[numNeighbors++]=getVoxel(x-1,y,z);
	if (x<sizex)
	    neighbors[numNeighbors++]=getVoxel(x+1,y,z);
	if (y>-1)
	    neighbors[numNeighbors++]=getVoxel(x,y-1,z);
	if (y<sizey)
	    neighbors[numNeighbors++]=getVoxel(x,y+1,z);
	if (z>-1)
	    neighbors[numNeighbors++]=getVoxel(x,y,z-1);
	if (z<sizez)
	    neighbors[numNeighbors++]=getVoxel(x,y,z+1);

	return numNeighbors;
    }

    //array-boundary safe version
    public int getNeighbors3safe(Voxel v,Voxel[] neighbors)
    {
        int x=v.getX(),y=v.getY(),z=v.getZ();
	int numNeighbors=0;
	if (z>-1)
	{
	    //front face OK
	    if (y>-1)
	    {
		//left col OK
		if (x>-1)
		{
		    //top row OK
		    neighbors[numNeighbors++]=getVoxel(x-1,y-1,z-1);
		}
		//middle row always OK
		neighbors[numNeighbors++]=getVoxel(x,y-1,z-1);
		if (x<sizex)
		{
		    //bottom row OK
		    neighbors[numNeighbors++]=getVoxel(x+1,y-1,z-1);
		}
	    }
	    //middle col always OK
	    if (x>-1)
	    {
		//top row OK
		neighbors[numNeighbors++]=getVoxel(x-1,y,z-1);
	    }
	    //middle row always OK
	    neighbors[numNeighbors++]=getVoxel(x,y,z-1);
	    if (x<sizex)
	    {
		//bottom row OK
		neighbors[numNeighbors++]=getVoxel(x+1,y,z-1);
	    }
	    if (y<sizey)
	    {
		//right col OK
		if (x>-1)
		{
		    //top row OK
		    neighbors[numNeighbors++]=getVoxel(x-1,y+1,z-1);
		}
		//middle row always OK
		neighbors[numNeighbors++]=getVoxel(x,y+1,z-1);
		if (x<sizex)
		{
		    //bottom row OK
		    neighbors[numNeighbors++]=getVoxel(x+1,y+1,z-1);
		}
	    }
	}
	//middle face always OK
	if (y>-1)
	{
	    //left col OK
	    if (x>-1)
	    {
		//top row OK
		neighbors[numNeighbors++]=getVoxel(x-1,y-1,z);
	    }
	    //middle row always OK
	    neighbors[numNeighbors++]=getVoxel(x,y-1,z);
	    if (x<sizex)
	    {
		//bottom row OK
		neighbors[numNeighbors++]=getVoxel(x+1,y-1,z);
	    }
	}
	//middle col always OK
	if (x>-1)
	{
	    //top row OK
	    neighbors[numNeighbors++]=getVoxel(x-1,y,z);
	}
	//middle row always OK
	//I'm not my own neighbor
	//neighbors[numNeighbors++]=getVoxel(x,y,z);
	if (x<sizex)
	{
	    //bottom row OK
	    neighbors[numNeighbors++]=getVoxel(x+1,y,z);
	}
	if (y<sizey)
	{
	    //right col OK
	    if (x>-1)
	    {
		//top row OK
		neighbors[numNeighbors++]=getVoxel(x-1,y+1,z);
	    }
	    //middle row always OK
	    neighbors[numNeighbors++]=getVoxel(x,y+1,z);
	    if (x<sizex)
	    {
		//bottom row OK
		neighbors[numNeighbors++]=getVoxel(x+1,y+1,z);
	    }
	}
	if (z<sizez)
	{
	    //back face OK
	    if (y>-1)
	    {
		//left col OK
		if (x>-1)
		{
		    //top row OK
		    neighbors[numNeighbors++]=getVoxel(x-1,y-1,z+1);
		}
		//middle row always OK
		neighbors[numNeighbors++]=getVoxel(x,y-1,z+1);
		if (x<sizex)
		{
		    //bottom row OK
		    neighbors[numNeighbors++]=getVoxel(x+1,y-1,z+1);
		}
	    }
	    //middle col always OK
	    if (x>-1)
	    {
		//top row OK
		neighbors[numNeighbors++]=getVoxel(x-1,y,z+1);
	    }
	    //middle row always OK
	    neighbors[numNeighbors++]=getVoxel(x,y,z+1);
	    if (x<sizex)
	    {
		//bottom row OK
		neighbors[numNeighbors++]=getVoxel(x+1,y,z+1);
	    }
	    if (y<sizey)
	    {
		//right col OK
		if (x>-1)
		{
		    //top row OK
		    neighbors[numNeighbors++]=getVoxel(x-1,y+1,z+1);
		}
		//middle row always OK
		neighbors[numNeighbors++]=getVoxel(x,y+1,z+1);
		if (x<sizex)
		{
		    //bottom row OK
		    neighbors[numNeighbors++]=getVoxel(x+1,y+1,z+1);
		}
	    }
	}
	
	return numNeighbors;
    }

    public void setFlag(int x,int y,int z,int flag,boolean value)
    {
        if (value)
            flags[x+1][y+1][z+1]=(byte)(flags[x+1][y+1][z+1] | flag);
        else
            flags[x+1][y+1][z+1]=(byte)(flags[x+1][y+1][z+1] & ~flag);
    }

    public boolean getFlag(int x,int y,int z,int flag)
    {
        return ((flags[x+1][y+1][z+1] & flag) == flag);
    }

    public void setMark(Voxel v,boolean mark)
    {
        v.setMark(mark);
        setFlag(v.getX(),v.getY(),v.getZ(),OUTSIDE,mark);
    }

    public boolean getMark(int x,int y,int z)
    {
        return getFlag(x,y,z,OUTSIDE);
    }

    public void setBoundaryFlag(Voxel v,boolean flag)
    {
        v.setBoundaryFlag(flag);
        setFlag(v.getX(),v.getY(),v.getZ(),BOUNDARY,flag);
    }

    public void printNeighborhood(Voxel v,PrintStream out)
    {
        int vx=v.getX(),vy=v.getY(),vz=v.getZ();
        int x,y,z;
        out.println(""+getNeighbors(v).length+" neighbors");
        for (x=vx-1; x<=vx+1; x++)
        {
            for (y=vy-1; y<=vy+1; y++)
            {
                for (z=vz-1; z<=vz+1; z++)
                {
                    if (getFlag(x,y,z,OUTSIDE))
                        out.print("x.");
                    else
                        out.print(""+(int)values[x+1][y+1][z+1]+".");
                    out.print(" ");
                }
                out.println();
            }
            out.println();
        }
        
    }
    
    public void printData(PrintStream out)
    {
	int x,y,z;
	for (x=0; x<sizex; x++)
	{
	    for (y=0; y<sizey; y++)
	    {
		for (z=0; z<sizez; z++)
		{
		    out.print(values[x+1][y+1][z+1]);
		    out.print(" ");
		}
		out.println();
	    }
	    out.println();
	}
    }
    
    public void printMarkedData(PrintStream out)
    {
        int x,y,z;
        for (x=0; x<sizex; x++)
        {
            for (y=0; y<sizey; y++)
            {
                for (z=0; z<sizez; z++)
                {
                    if (getFlag(x,y,z,OUTSIDE))
			out.print("X.X");
		    else
			out.print(values[x+1][y+1][z+1]);
		    out.print(" ");
		}
		out.println();
	    }
	    out.println();
	}
    }
    
    public static void main(String[] args)
    {
        String filename="sample.dat";
        if (args.length>0) filename=args[0];
        int conn=VoxelData.FACE_CONN;
        if (args.length>1)
            if (args[1].equalsIgnoreCase("VERTEX"))
                conn=VoxelData.VERTEX_CONN;
        double thresh=1.5;
        if (args.length>2)
        {
            try
            {
                thresh=(new Double(args[2])).doubleValue();
            }
            catch (NumberFormatException e)
            {
                System.err.println(e);
            }
        }
        String ofilename=filename+"2.v";
        if (args.length>3)
        {
            ofilename=args[3];
        }

        if (args.length==0)
        {
            System.out.println("Usage:");
            System.out.println("java VoxelData \n"+
                               "<voxel file> <VERTEX|FACE> <threshold> <output .v file>");
            System.exit(0);
        }

        VoxelData vd=new VoxelData(conn);
        vd.readFile(filename);
        vd.unmarkAll();
        vd.markBelowThresh(thresh);
        try
        {
            vd.writeVFile(ofilename,thresh);
        }
        catch (java.io.IOException e)
        {
            System.err.println(e);
        }
    }
    
}
