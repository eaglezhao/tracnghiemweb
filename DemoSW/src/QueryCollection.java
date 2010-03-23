import java.util.Vector;

import org.mindswap.pellet.jena.PelletReasonerFactory;

import com.hp.hpl.jena.ontology.OntModel;
import com.hp.hpl.jena.ontology.OntModelSpec;
import com.hp.hpl.jena.query.Query;
import com.hp.hpl.jena.query.QueryExecution;
import com.hp.hpl.jena.query.QueryExecutionFactory;
import com.hp.hpl.jena.query.QueryFactory;
import com.hp.hpl.jena.query.QuerySolution;
import com.hp.hpl.jena.query.ResultSet;
import com.hp.hpl.jena.rdf.model.Model;
import com.hp.hpl.jena.rdf.model.ModelFactory;
import com.hp.hpl.jena.rdf.model.RDFNode;
import com.hp.hpl.jena.reasoner.Reasoner;

public class QueryCollection {
	Reasoner reasoner;
	Model infModel;
	OntModel m;
	String prefix = "PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#> " +
					"PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> " +
					"PREFIX owl: <http://www.w3.org/2002/07/owl#> " +
					"PREFIX chemistry: <http://www.ea3888.univ-rennes1.fr/dameron/ontology/chemistry.owl#> ";
	
	public QueryCollection()
	{
		// TODO Auto-generated constructor stub
		reasoner =	PelletReasonerFactory.theInstance().create();
		infModel = ModelFactory.createInfModel(reasoner, ModelFactory.createDefaultModel());			
		m = ModelFactory.createOntologyModel (OntModelSpec.OWL_MEM, infModel);
        
        m.getDocumentManager().addAltEntry(
        	   "file:Ontologies/chemistryv4.owl",
               "file:Ontologies/chemistryv4.owl");
        m.read("file:Ontologies/chemistryv4.owl",null, "RDF/XML");
	}
	
	public static String queryString = "";
	public static Vector<String> returnString = new Vector<String>();
	
	public void moleculeList()
	{
		String strQuery = 	"SELECT ?molecule " +
							"WHERE { " +
							"?molecule rdfs:subClassOf chemistry:Molecule. " +
							"}"; 
		
		StringBuffer queryStr = new StringBuffer();
		queryStr.append(prefix);
		queryStr.append(strQuery);
		
		Query query = QueryFactory.create(queryStr.toString());
		QueryExecution qexec = QueryExecutionFactory.create(query, m);
		
		queryString = queryStr.toString();
		returnString.clear();
		try
		{
			ResultSet response = qexec.execSelect();
			System.out.println("\nQuery done");
			Vector<RDFNode> molecule = new Vector<RDFNode>();
			
			while( response.hasNext())
			{
				QuerySolution soln = response.nextSolution();
				RDFNode name1 = soln.get("?molecule");

				if(name1!=null) molecule.add(name1);
			}
			//System.out.println ("Molecule :");
			for (RDFNode node : molecule) returnString.add(node.asNode().getLocalName());
			//System.out.println(node.asNode().getLocalName());			
			//System.out.println("");
		}
		finally { qexec.close();}		
		System.out.println("Done");		
	}
}