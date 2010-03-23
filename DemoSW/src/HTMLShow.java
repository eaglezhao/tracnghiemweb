import java.util.Vector;


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
	
static String numberRome [] = new String [] {"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X"}; 
	
	public static String formatFormula (String formula)
	{
		Vector<String> s = new Vector<String>();
		String tempNumber = "";
		String tempName = "";
		char c [] = formula.toCharArray();
		
		for (char d : c)
		{
			if (Character.isDigit(d))
			{
				if (tempName != ""){ s.add(tempName); tempName = "";}
				tempNumber += d;
			}
			else if (Character.isLetter(d))
			{
				if (tempNumber != "") {s.add(tempNumber); tempNumber = "";}
				if (Character.isUpperCase(d)) {s.add(tempName); tempName = "";}
				tempName += d;
			}
			else
			{
				if (tempName != "") {s.add(tempName); tempName = "";}
				else if (tempNumber != "") {s.add(tempNumber); tempNumber = "";}
				s.add("" + d);
			}
		}
		if (tempName != "") s.add(tempName);
		if (tempNumber != "") s.add(tempNumber);
		
		String html = "<html> <font size=\"6\">";
		
		for (String string : s)
		{
			if (string == "") continue;
			if (Character.isDigit(string.charAt(0))) html += subNumber(string);
			else if (Character.isLetter(string.charAt(0))) html += symbol(string);
			else html += string;
			
		}
		html += "</font> </html>";
		
		return html;
	}
	
	public static String formatName (String name)
	{
		String s = "";
		String tempNumber = "";
		String tempName = "";
		char c [] = name.toCharArray();
		
		for (char d : c)
		{
			if (Character.isDigit(d))
			{
				if (tempName != ""){ s += tempName; tempName = "";}
				tempNumber += d;
			}
			else if (Character.isLetter(d))
			{
				if (tempNumber != "") {s += "(" + numberRome [new Integer (tempNumber)] + ")"; tempNumber = "";}
				if (tempName == "")
				{
					if (s != "") s += " ";
					tempName += Character.toUpperCase(d);
				}
				else
				{
					if (Character.isUpperCase(d))
					{
						if (s != "") s += " ";
						else {s += tempName + " "; tempName = "";}
					}
					tempName += d;
				}
			}
			else if (tempName != "") {s += tempName; tempName = "";}
		}
		if (tempName != "") s += tempName;
		if (tempNumber != "") s += tempNumber;
		return s;
	}
}
