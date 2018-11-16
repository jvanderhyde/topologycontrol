//Stack--a simple stack (LIFO) of elements
//James Vanderhyde, 31 May 2002

public class Stack extends java.util.Vector
{
    public void push(Object o)
    {
	addElement(o);
    }
    
    public Object pop()
    {
	Object r=this.lastElement();
	removeElementAt(elementCount-1);
	return r;
    }
    
}
