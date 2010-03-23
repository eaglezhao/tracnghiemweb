import java.awt.BorderLayout;
import java.awt.GridLayout;
import javax.swing.BoxLayout;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JTextArea;

public class ChemistryPanel extends JFrame
{
	private JTextArea textQuery = new JTextArea();
	private JComboBox comboListQuery = new JComboBox();
	private JList listReturn = new JList();
	private JButton runButton = new JButton("Run");
	
	public ChemistryPanel()
	{
		super();
		
		final JPanel panelQuery = new JPanel(new BorderLayout(0, 3));
		final JPanel panelRetun = new JPanel(new BorderLayout(0, 4));
		final JPanel panel_2 = new JPanel(new BorderLayout(0, 3));
		final JPanel panel_3 = new JPanel();
		
		panel_3.setLayout(new BoxLayout(panel_3, BoxLayout.X_AXIS));
		panel_3.add(new JLabel("List Query: "));
		panel_3.add(comboListQuery);
		panel_3.add(runButton);

		panel_2.add(panel_3, BorderLayout.NORTH);
		panel_2.add(textQuery, BorderLayout.CENTER);
		
		panelQuery.add(new JLabel("Query String"), BorderLayout.NORTH);
		panelQuery.add(panel_2, BorderLayout.CENTER);
		
		panelRetun.add(new JLabel("Return"), BorderLayout.NORTH);
		panelRetun.add(listReturn, BorderLayout.CENTER);

		getContentPane().setLayout(new GridLayout(1, 2, 5, 2));
		getContentPane().add(panelQuery);
		getContentPane().add(panelRetun);
		
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setTitle("Chemistry Query - Demo");
		this.setSize(800, 600);
		
		this.setVisible(true);
		this.setEnabled(true);
		this.setResizable(false);
	}
}
