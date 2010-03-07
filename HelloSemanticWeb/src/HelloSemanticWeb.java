

import java.io.IOException;
import java.io.InputStream;
import java.util.Iterator;
import java.util.Vector;

import org.mindswap.pellet.jena.PelletReasonerFactory;


import com.hp.hpl.jena.Jena;
import com.hp.hpl.jena.ontology.OntDocumentManager;
import com.hp.hpl.jena.ontology.OntModel;
import com.hp.hpl.jena.ontology.OntModelSpec;
import com.hp.hpl.jena.ontology.Ontology;
import com.hp.hpl.jena.query.Query;
import com.hp.hpl.jena.query.QueryExecution;
import com.hp.hpl.jena.query.QueryExecutionFactory;
import com.hp.hpl.jena.query.QueryFactory;
import com.hp.hpl.jena.query.QuerySolution;
import com.hp.hpl.jena.query.ResultSet;
import com.hp.hpl.jena.rdf.model.*;
import com.hp.hpl.jena.reasoner.Reasoner;
import com.hp.hpl.jena.reasoner.ReasonerRegistry;
import com.hp.hpl.jena.reasoner.ValidityReport;
import com.hp.hpl.jena.reasoner.rulesys.GenericRuleReasoner;
import com.hp.hpl.jena.reasoner.rulesys.Rule;
import com.hp.hpl.jena.util.FileManager;


public class HelloSemanticWeb {
	static String defaultNameSpace = "http://org.semwebprogramming/chapter2/people#";
	
	Model _friends = null;
	Model schema = null;
	InfModel inferredFriends = null;

	public static void main(String[] args) throws IOException {
		
		Reasoner reasoner =	PelletReasonerFactory.theInstance().create();
		Model infModel = ModelFactory.createInfModel(reasoner, ModelFactory.createDefaultModel());
				
		OntModel m = ModelFactory.createOntologyModel( OntModelSpec.OWL_DL_MEM, infModel );
		//OntModel m = ModelFactory.createOntologyModel( OntModelSpec.OWL_DL_MEM_RDFS_INF);
        // we have a local copy of the wine ontology
        
        m.getDocumentManager().addAltEntry( "file:Ontologies/chemistryv2.owl",
                                            "file:Ontologies/chemistryv2.owl" );
        

        m.read( "file:Ontologies/chemistryv2.owl",null, "RDF/XML" );
        
       
        
        HelloSemanticWeb hello = new HelloSemanticWeb();
        
        System.out.println("\nSay Hello to Myself");
		//hello.mySelf(m);
        hello.produceElement(m);
        
	}
	
	
	private void mySelf(Model model){
		//Hello to Me - focused search
		

		runQuery("PREFIX chemistry: <http://www.ea3888.univ-rennes1.fr/dameron/ontology/chemistry.owl#> " +
				"PREFIX me: <http://localhost/chemistryv2.owl#> " +
				"SELECT ?reaction ?formulaReactant ?formulaProduct " +
				"WHERE { " +
				"{?reaction rdf:type chemistry:Reaction. " +
				"?reaction chemistry:hasReactant ?reactant." +
				"?reaction chemistry:hasProduct ?product." +
				"?reactant chemistry:hasFormula ?formulaReactant." +
				"?product chemistry:hasFormula ?formulaProduct. }" +
				
				//"?product rdfs:subClassOf chemistry:MoleculeContainingHydrogen. " +
				//"?productType rdfs:subClassOf chemistry:MoleculeContainingHydrogen. " +
				//"?reactant rdf:type chemistry:Oxygen. " +
				//"?reactantType rdf:subClassOf chemistry:MoleculeContainingOxygen. " +
				"{" +
				"?reaction chemistry:hasProduct ?product1." +
				"?product1 rdfs:subClassOf chemistry:MoleculeContainingHydrogen. " +
				"} " +
				"}", model);  //add the query string" +
						//""  " +
				//" 
//hopchat SubClassOf MoleculeContainingHydrogen 
	}
	
	private void produceElement(Model model)
	{
		runQuery("PREFIX chemistry: <http://www.ea3888.univ-rennes1.fr/dameron/ontology/chemistry.owl#> " +
				"SELECT ?formulaReactant ?formulaProduct " +
				"WHERE { " +
				"?reactant chemistry:produces ?product." +
				"?reactant rdf:type chemistry:Oxygen. " +
				
				"?reactant chemistry:hasFormula ?formulaReactant." +
				"?product chemistry:hasFormula ?formulaProduct." +
				
				"}", model);  //add the query string" +
	}
	

	private void runQuery(String queryRequest, Model model){
		
		StringBuffer queryStr = new StringBuffer();
		// Establish Prefixes
		//Set default Name space first
		queryStr.append("PREFIX people" + ": <" + defaultNameSpace + "> ");
		queryStr.append("PREFIX rdfs" + ": <" + "http://www.w3.org/2000/01/rdf-schema#" + "> ");
		queryStr.append("PREFIX rdf" + ": <" + "http://www.w3.org/1999/02/22-rdf-syntax-ns#" + "> ");
		queryStr.append("PREFIX foaf" + ": <" + "http://xmlns.com/foaf/0.1/" + "> ");
		queryStr.append("PREFIX owl" + ": <" + "http://www.w3.org/2002/07/owl#" + "> ");
		
		//Now add query
		queryStr.append(queryRequest);
		Query query = QueryFactory.create(queryStr.toString());
		QueryExecution qexec = QueryExecutionFactory.create(query, model);
		try {
		ResultSet response = qexec.execSelect();
		
		Vector<RDFNode> product = new Vector<RDFNode>();
		Vector<RDFNode> reactant = new Vector<RDFNode>();
		Vector<RDFNode> reaction = new Vector<RDFNode>(); 
		
		//RDFNode reaction, reactant, product;
		
		while( response.hasNext()){
			QuerySolution soln = response.nextSolution();
			System.out.println(soln.hashCode() + "\n");
			RDFNode name = soln.get("?formulaProduct");
			RDFNode name1 = soln.get("?formulaReactant");
			//RDFNode name2 = soln.get("?reaction");

			System.out.println("P: " + name.toString());
			if(!product.contains(name))
				product.add(name);

			System.out.println("R: " + name1.toString());
			if(!reactant.contains(name1))
			{
				reactant.add(name1);
			}
			
			/*System.out.println("R: " + name2.toString());
			if(!reaction.contains(name2))
			{
				reaction.add(name2);
			}*/
		}
		/*
		System.out.print("Reaction : ");
		for (RDFNode node : reaction) {
			System.out.println(node.toString() + " ");
		}*/
		
		System.out.print("Reactant : ");
		for (RDFNode node : reactant) {
			System.out.println(node.toString() + " ");
		}
		
		System.out.println("");
		
		System.out.print("Product : ");
		for (RDFNode node : product) {
			System.out.println(node.toString() + " ");
		}
		
		
		
		System.out.println("");
		
		} finally { qexec.close();}				
	}
		
}