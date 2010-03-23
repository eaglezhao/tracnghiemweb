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
	
	static public final int  MOLECULE_LIST = 1;

	
	Reasoner reasoner;
	Model infModel;
	OntModel m;
	String prefix = "PREFIX rdfs: <http://www.w3.org/2000/01/rdf-schema#> \n" +
					"PREFIX rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n" +
					"PREFIX owl: <http://www.w3.org/2002/07/owl#> \n" +
					"PREFIX chemistry: <http://www.ea3888.univ-rennes1.fr/dameron/ontology/chemistry.owl#> \n";
	
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
	

	
	public String getQueryString(int queryNum)
	{
		switch(queryNum)
		{
		case MOLECULE_LIST:
			return getMoleculeListString();
		}
		return null;
	}
	
	public String getMoleculeListString()
	{
		String strQuery = 	"SELECT ?molecule \n" +
		"WHERE { \n" +
		"?molecule rdfs:subClassOf chemistry:Molecule. \n" +
		"}"; 
		return strQuery; 
	}
	
	public Vector<String> queryString(int queryNum)
	{
		switch(queryNum)
		{
			case MOLECULE_LIST:
				return moleculeList();
		}
		return null;
	}
	
	
	
	public Vector<String> moleculeList()
	{
		
		
		StringBuffer queryStr = new StringBuffer();
		queryStr.append(prefix);
		queryStr.append(getQueryString(MOLECULE_LIST));
		
		
		Query query = QueryFactory.create(queryStr.toString());
		QueryExecution qexec = QueryExecutionFactory.create(query, m);
		
		Vector<String> result = new Vector<String>();
		result.clear();
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
			for (RDFNode node : molecule) result.add(node.asNode().getLocalName());
			//System.out.println(node.asNode().getLocalName());			
			//System.out.println("");
		}
		finally { qexec.close();}
		System.out.println("Done");	
		return result;  
			
	}
}