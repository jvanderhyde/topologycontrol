//TokenReader--a class for reading tokens separated by whitespace from a reader
//James Vanderhyde, 15 May 2002

import java.io.*;
import java.util.StringTokenizer;

public class TokenReader extends BufferedReader
{
    String line;
    StringTokenizer st;
    
    public TokenReader(Reader in)
    {
	super(in);
	st=null;
	line=null;
    }
    
    public String readToken() throws IOException
    {
	if (st==null)
        {
	    line=this.readLine();
	    if (line==null) return null;
	    st=new StringTokenizer(line);
        }
    	while (!st.hasMoreTokens())
	{
	    line=this.readLine();
	    if (line==null) return null;
	    st=new StringTokenizer(line);
	}
	return st.nextToken();
    }
    
    public boolean markSupported()
    {
        return false;
    }
    
    public void mark()
    {
    }
    
    public void reset()
    {
    }
    
    
}
