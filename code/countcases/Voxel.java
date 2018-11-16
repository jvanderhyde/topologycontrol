//Voxel--a class for storing value and labels of a voxel data point
//James Vanderhyde, 16 May 2002

public class Voxel
    implements Comparable
{
    protected float value;
    protected boolean mark;
    protected boolean boundaryFlag;
    //protected int x,y,z;
    protected int coords;
    
    public Voxel(int p_x,int p_y,int p_z)
    {
        this(0.0,p_x,p_y,p_z,false,false);
    }
    
    public Voxel(double p_val,int p_x,int p_y,int p_z)
    {
        this(p_val,p_x,p_y,p_z,false,false);
    }

    public Voxel(double p_val,int p_x,int p_y,int p_z,boolean p_mark)
    {
        this(p_val,p_x,p_y,p_z,p_mark,false);
    }

    public Voxel(double p_val,int p_x,int p_y,int p_z,boolean p_mark,boolean p_bdFlag)
    {
        value=(float)p_val;
        mark=p_mark;
        boundaryFlag=p_bdFlag;
        //x=p_x;
        //y=p_y;
        //z=p_z;
        setCoords(p_x,p_y,p_z);
    }

    public double getValue()
    {
        return (double)value;
    }
    
    public void setValue(double p_val)
    {
	value=(float)p_val;
    }

    public boolean getBoundaryFlag()
    {
        return boundaryFlag;
    }

    public void setBoundaryFlag(boolean flag)
    {
        boundaryFlag=flag;
    }

    public void mark()
    {
        mark=true;
    }
    
    public void unmark()
    {
        mark=false;
    }
    
    public void setMark(boolean p_mark)
    {
        mark=p_mark;
    }
    
    public boolean isMarked()
    {
        return mark;
    }
    
    public boolean getMark()
    {
        return mark;
    }

    //don't use
    public int[] getCoords()
    {
	int[] r=new int[3];
	r[0]=getX();
	r[1]=getY();
	r[2]=getZ();
	return r;
    }

    public void setCoords(int p_x,int p_y,int p_z)
    {
        coords = ((p_x+1)<<20) | ((p_y+1)<<10) | (p_z+1);
    }

    public int getX()
    {
        return (coords>>20)-1;
    }

    public int getY()
    {
        return ((coords>>10) & 0x00003ff)-1;
    }

    public int getZ()
    {
        return (coords & 0x00003ff)-1;
    }

    public String coordsToString()
    {
	return "("+getX()+","+getY()+","+getZ()+")";
    }
    
    public String toString()
    {
        return ""+value;
    }

    //not used anywhere
    public static Voxel[] setIntersection(Voxel[] s1,Voxel[] s2)
    {
	int i,j,n=0;
	Voxel[] intersection=new Voxel[Math.min(s1.length,s2.length)];
	for (i=0; i<s1.length; i++)
	    for (j=0; j<s2.length; j++)
		if ((s1[i].getX()==s2[j].getX()) &&
                    (s1[i].getY()==s2[j].getY()) &&
                    (s1[i].getZ()==s2[j].getZ()))
		    intersection[n++]=s1[i];
	Voxel[] r=new Voxel[n];
	for (i=0; i<n; i++)
	    r[i]=intersection[i];
	return r;
    }
    
    public double getPriority()
    {
        return value;
    }

    public int compareTo(Object o)
    {
        Voxel v=(Voxel)o;
        double val=v.getValue();
        if (value<val) return -1;
        if (value>val) return 1;
        if (v.getZ()<getZ()) return -1;
        if (v.getZ()>getZ()) return 1;
        if (v.getY()<getY()) return -1;
        if (v.getY()>getY()) return 1;
        if (v.getX()<getX()) return -1;
        if (v.getX()>getX()) return 1;
        return 0;
    }

    public boolean equals(Object o)
    {
        if (!(o instanceof Voxel)) return false;
        Voxel v=(Voxel)o;
        return ((getX()==v.getX()) && (getY()==v.getY()) && (getZ()==v.getZ()));
    }
}
