import java.awt.BorderLayout;
import java.awt.GridLayout;
import java.awt.ItemSelectable;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.util.Collections;
import java.util.Vector;

import javax.swing.BoxLayout;
import javax.swing.DefaultListModel;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JList;
import javax.swing.JPanel;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;

public class ChemistryPanel extends JFrame
{
	private JTextArea textQuery = new JTextArea();
	private JComboBox comboListQuery = new JComboBox();
	private JList listReturn = new JList();
	private JButton runButton = new JButton("Run");
	
	static QueryCollection query = new QueryCollection();
	
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
		
		ActionListener actionListener = new ActionListener() {
		      public void actionPerformed(ActionEvent actionEvent) {
		        //System.out.println("Command: " + actionEvent.getActionCommand());
		        ItemSelectable is = (ItemSelectable)actionEvent.getSource();
		        
		        System.out.println(", Selected: " + selectedString(is));

		      }
		      
		      private String selectedString(ItemSelectable is) {
		  	    Object selected[] = is.getSelectedObjects();
		  	    MyItem a = (MyItem)selected[0];
		  	    setContentQuery(query.getQueryString(a.id));
		  	    return ((selected.length == 0) ? "null" : selected[0].toString());
		  	 }
		      
		       
		    };
		comboListQuery.addActionListener(actionListener);

		
		int i = 1;
		for(String s : query.queryName)
			comboListQuery.addItem(new MyItem(s, i++));
		
		
		runButton.addActionListener(new ActionListener() {
		      public void actionPerformed(ActionEvent actionEvent) {
		        //System.out.println("Command: " + actionEvent.getActionCommand());
		    	MyItem i = (MyItem) comboListQuery.getSelectedItem();
		        //ItemSelectable is = (ItemSelectable)actionEvent.getSource();
		        setContentReturn(query.getQueryResult(i.id), 1);

		      }

		    });

		panel_2.add(new JScrollPane(textQuery));
		panel_2.add(panel_3, BorderLayout.NORTH);
		
		panelQuery.add(new JLabel("Query String"), BorderLayout.NORTH);
		panelQuery.add(panel_2, BorderLayout.CENTER);

		panelRetun.add(new JLabel("Return"), BorderLayout.NORTH);
		panelRetun.add(new JScrollPane(listReturn));		

		getContentPane().setLayout(new GridLayout(1, 2, 5, 2));
		getContentPane().add(panelQuery);
		getContentPane().add(panelRetun);
		
		this.setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
		this.setTitle("Chemistry Query - Demo");		
		this.setSize(600, 400);
		
		this.setVisible(true);
		this.setEnabled(true);
		this.setResizable(false);
	}
	
	public void setContentQuery (String query) {this.textQuery.setText(query);}	
	public void setContentReturn (Vector<String> vector, int type)
	{
		Collections.sort(vector);
		//Vector<String> names = new Vector<String>();
		
		//if (type == 1) for (String string : vector) names.add(HTMLShow.formatFormula(string));
		//else if (type == 2) for (String string : vector) names.add(HTMLShow.formatName(string));
		
		this.listReturn.setListData(vector);
	}
	
	
	
	class MyItem
	{
		String name;
		public int id;
		public MyItem(String name, int id) {
			// TODO Auto-generated constructor stub
			this.name = name;this.id = id;
		}
		@Override
		public String toString() {
			// TODO Auto-generated method stub
			return name;
		}
	}
}
