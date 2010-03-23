
public class HTMLShow
{	
	static String arrow = " &rarr; ";
	static String add = " + ";
	static String equ = " = ";	
	
	public static String symbol (String s)
	{
		if (s.length() == 1) s = "" + Character.toUpperCase(s.charAt(0));
		else s = "" + Character.toUpperCase(s.charAt(0)) + Character.toLowerCase(s.charAt(1));
		return "<font color=\"blue\" size=\"5\" face=\"Tahoma\"><b>" + s + "</b></font>";
	}
	
	public static String subNumber (String s) {return subNumber(s, 4);}	
	public static String subNumber (String s, int size)
	{
		return "<font size=\""+ size +"\" face=\"Tahoma\"><sub>" + s + "</sub></font>";
	}
	
	public static String number (String s)
	{
		return "<font color=\"red\" size=\"6\" face=\"Times New Roman\">" + s + "</font>";
	}
}
