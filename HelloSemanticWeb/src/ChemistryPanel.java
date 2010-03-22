import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JTextArea;
import javax.swing.JTextField;
import javax.swing.SpringLayout;


public class ChemistryPanel extends JFrame
{
	private JTextArea httpTextArea;
	private JTextField textField_1;
	private JTextField textField;
	private SpringLayout springLayout;
	public ChemistryPanel() {
		super();
		springLayout = new SpringLayout();
		getContentPane().setLayout(springLayout);
		setSize(369, 352);

		final JLabel rectantLabel = new JLabel();
		rectantLabel.setText("Rectant");
		getContentPane().add(rectantLabel);
		springLayout.putConstraint(SpringLayout.SOUTH, rectantLabel, 25, SpringLayout.NORTH, getContentPane());
		springLayout.putConstraint(SpringLayout.NORTH, rectantLabel, 9, SpringLayout.NORTH, getContentPane());
		springLayout.putConstraint(SpringLayout.WEST, rectantLabel, 5, SpringLayout.WEST, getContentPane());

		textField = new JTextField();
		getContentPane().add(textField);
		springLayout.putConstraint(SpringLayout.EAST, textField, 356, SpringLayout.WEST, rectantLabel);
		springLayout.putConstraint(SpringLayout.WEST, textField, 0, SpringLayout.WEST, rectantLabel);
		springLayout.putConstraint(SpringLayout.SOUTH, textField, 20, SpringLayout.SOUTH, rectantLabel);
		springLayout.putConstraint(SpringLayout.NORTH, textField, 0, SpringLayout.SOUTH, rectantLabel);

		final JLabel productionLabel = new JLabel();
		productionLabel.setText("Production");
		getContentPane().add(productionLabel);
		springLayout.putConstraint(SpringLayout.SOUTH, productionLabel, 70, SpringLayout.NORTH, getContentPane());
		springLayout.putConstraint(SpringLayout.WEST, productionLabel, 5, SpringLayout.WEST, getContentPane());

		textField_1 = new JTextField();
		getContentPane().add(textField_1);
		springLayout.putConstraint(SpringLayout.EAST, textField_1, 356, SpringLayout.WEST, productionLabel);
		springLayout.putConstraint(SpringLayout.WEST, textField_1, 0, SpringLayout.WEST, productionLabel);
		springLayout.putConstraint(SpringLayout.SOUTH, textField_1, 20, SpringLayout.SOUTH, productionLabel);
		springLayout.putConstraint(SpringLayout.NORTH, textField_1, 0, SpringLayout.SOUTH, productionLabel);

		httpTextArea = new JTextArea();
		httpTextArea.setColumns(20);
		getContentPane().add(httpTextArea);
		springLayout.putConstraint(SpringLayout.SOUTH, httpTextArea, 286, SpringLayout.NORTH, getContentPane());
		springLayout.putConstraint(SpringLayout.NORTH, httpTextArea, 120, SpringLayout.NORTH, getContentPane());
		springLayout.putConstraint(SpringLayout.EAST, httpTextArea, 0, SpringLayout.EAST, textField_1);
		springLayout.putConstraint(SpringLayout.WEST, httpTextArea, 5, SpringLayout.WEST, getContentPane());

		final JLabel rectantionLabel = new JLabel();
		rectantionLabel.setText("Rectantion");
		getContentPane().add(rectantionLabel);
		springLayout.putConstraint(SpringLayout.SOUTH, rectantionLabel, -5, SpringLayout.NORTH, httpTextArea);
		springLayout.putConstraint(SpringLayout.WEST, rectantionLabel, 5, SpringLayout.WEST, getContentPane());

		final JButton queryButton = new JButton();
		queryButton.setText("Query");
		getContentPane().add(queryButton);
		springLayout.putConstraint(SpringLayout.EAST, queryButton, 0, SpringLayout.EAST, httpTextArea);
		springLayout.putConstraint(SpringLayout.WEST, queryButton, 5, SpringLayout.WEST, getContentPane());
		springLayout.putConstraint(SpringLayout.SOUTH, queryButton, 31, SpringLayout.SOUTH, httpTextArea);
		springLayout.putConstraint(SpringLayout.NORTH, queryButton, 5, SpringLayout.SOUTH, httpTextArea);
	}
}