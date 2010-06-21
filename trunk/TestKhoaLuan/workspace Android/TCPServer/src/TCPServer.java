//TCPServer.java

import java.io.*;
import java.net.*;

class TCPServer 
{
   public static void main(String argv[]) throws Exception
      {
         String fromclient;
         String toclient;
          
         ServerSocket Server = new ServerSocket (4115);
         
         System.out.println ("TCPServer Waiting for client on port 4115");

         while(true) 
         {
         	Socket connected = Server.accept();
            System.out.println( " THE CLIENT"+" "+
            connected.getInetAddress() +":"+connected.getPort()+" IS CONNECTED ");
            
            BufferedReader inFromUser = 
            new BufferedReader(new InputStreamReader(System.in));    
     
            BufferedReader inFromClient =
               new BufferedReader(new InputStreamReader (connected.getInputStream()));
                  
            PrintWriter outToClient =
               new PrintWriter(
                  connected.getOutputStream(),true);
            
            while ( true )
            {
            	
            	System.out.println("Server: SEND(Type Q or q to Quit):");
            	toclient = inFromUser.readLine();
            	
            	if ( toclient.equals ("q") || toclient.equals("Q") )
            	{
            		outToClient.println(toclient);
            		connected.close();
            		break;
            	}
            	else
            	{
            	outToClient.println(toclient);
                }
            	
            	fromclient = inFromClient.readLine();
            	
                if ( fromclient.equals("q") || fromclient.equals("Q") )
                {
                	connected.close();
                	break;
                }
                	
		        else
		        {
		         System.out.println( "Server: RECIEVED:" + fromclient );
		        } 
			    
			}  
			
          }
      }
}