import javax.swing.JFrame;


public class Main
{
	static QueryCollection query = new QueryCollection();
	static ChemistryPanel panel = new ChemistryPanel ();
	
	public static void main(String[] args)
	{
		query.moleculeList();
		panel.setContentQuery(query.queryString);
		panel.setContentReturn(query.returnString);
	}
}
