import javax.swing.JFrame;



public class Main
{
	static QueryCollection query = new QueryCollection();
	static ChemistryPanel panel = new ChemistryPanel ();
	
	public static void main(String[] args)
	{

		panel.setContentQuery(query.getQueryString(query.MOLECULE_LIST));
		panel.setContentReturn(query.getQueryResult(query.MOLECULE_LIST));
	}
}
