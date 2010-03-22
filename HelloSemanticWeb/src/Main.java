import javax.swing.JFrame;


public class Main
{
	public static void main(String[] args)
	{
		ChemistryPanel panel = new ChemistryPanel ();
		panel.setVisible(true);
		panel.setEnabled(true);
		panel.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
	}
}
