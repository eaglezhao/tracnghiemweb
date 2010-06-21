/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.example.hellojni;

import android.app.Activity;
import android.widget.TextView;
import android.os.Bundle;
import java.io.*;
import java.net.*;


public class HelloJni extends Activity
{
    /** Called when the activity is first created. */
	TextView  tv;
    @Override
    /* (non-Javadoc)
     * @see android.app.Activity#onCreate(android.os.Bundle)
     */
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        /* Create a TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        tv = new TextView(this);
        tv.setText( stringFromJNI() );
        setContentView(tv);
        
        String FromServer = null;
        String ToServer = null;
        
        Socket clientSocket = null;
		try {
			
			//clientSocket = new Socket("127.0.0.1", 5000);
			InetAddress serverAddr = InetAddress.getByName("10.218.9.212");//10.66.3.44 is my pc' IP 
			//Log.d("TCP", "C: Connecting..."); 
			clientSocket = new Socket(serverAddr, 4115);
			//ServerSocket svSocket = new ServerSocket(4500);
		} catch (UnknownHostException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
        
        //BufferedReader inFromUser =
        //               new BufferedReader(new InputStreamReader(System.in));
        
       
           
        PrintWriter outToServer = null;
        BufferedReader inFromServer = null;
		try {
			outToServer = new PrintWriter(
			   clientSocket.getOutputStream(),true);
			inFromServer = new BufferedReader(new InputStreamReader(
			           clientSocket.getInputStream()));
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
           
        
           
		while (true)
		{
			try {
				//Thread.sleep(1000);
				FromServer = inFromServer.readLine();

				if ( FromServer.equals("q") || FromServer.equals("Q"))
				{
					clientSocket.close();
					break;
				}
				else

				{
					cout("RECIEVED:" + FromServer);
					cout("SEND(Type Q or q to Quit):");

					//ToServer = inFromUser.readLine();
					ToServer = "Welcome from client";

					if (ToServer.equals("Q") || ToServer.equals("q"))
					{
						outToServer.println (ToServer) ;
						clientSocket.close();
						break;
					}

					else
					{
						outToServer.println(ToServer);
					}
				}
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}




		}   
        
//        String fromclient;
//        String toclient;
//         
//        ServerSocket Server = null;
//		try {
//			Server = new ServerSocket (4500);
//		} catch (IOException e) {
//			// TODO Auto-generated catch block
//			e.printStackTrace();
//		}
//        
//        System.out.println ("TCPServer Waiting for client on port 4500");
//
//		while (true) {
//			Socket connected;
//			try {
//				connected = Server.accept();
//				cout(" THE CLIENT" + " "
//						+ connected.getInetAddress() + ":"
//						+ connected.getPort() + " IS CONNECTED ");
//
//				//BufferedReader inFromUser = new BufferedReader(
//				//		new InputStreamReader(System.in));
//
//				BufferedReader inFromClient = new BufferedReader(
//						new InputStreamReader(connected.getInputStream()));
//
//				PrintWriter outToClient = new PrintWriter(connected
//						.getOutputStream(), true);
//
//				while (true) {
//
//					System.out.println("Server: SEND(Type Q or q to Quit):");
//					//toclient = inFromUser.readLine();
//					toclient =  "Welcome to server";
//
//					if (toclient.equals("q") || toclient.equals("Q")) {
//						outToClient.println(toclient);
//						connected.close();
//						break;
//					} else {
//						outToClient.println(toclient);
//					}
//
//					fromclient = inFromClient.readLine();
//
//					if (fromclient.equals("q") || fromclient.equals("Q")) {
//						connected.close();
//						break;
//					}
//
//					else {
//						System.out.println("Server: RECIEVED:" + fromclient);
//					}
//
//				}
//			} catch (IOException e) {
//				// TODO Auto-generated catch block
//				e.printStackTrace();
//			}
//
//		}
    }

    /* A native method that is implemented by the
     * 'hello-jni' native library, which is packaged
     * with this application.
     */
    public void cout(String s)
    {
    	tv.setText(tv.getText() + "\n" + s);
    }
    public native String  stringFromJNI();

    /* This is another native method declaration that is *not*
     * implemented by 'hello-jni'. This is simply to show that
     * you can declare as many native methods in your Java code
     * as you want, their implementation is searched in the
     * currently loaded native libraries only the first time
     * you call them.
     *
     * Trying to call this function will result in a
     * java.lang.UnsatisfiedLinkError exception !
     */
    public native String  unimplementedStringFromJNI();

    /* this is used to load the 'hello-jni' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.example.HelloJni/lib/libhello-jni.so at
     * installation time by the package manager.
     */
    static {
        System.loadLibrary("hello-jni");
    }
}

// The contents of this file are subject to the Mozilla Public
// License Version 1.1 (the "License"); you may not use this file
// except in compliance with the License. You may obtain a copy of
// the License at http://www.mozilla.org/MPL/
// 
// Software distributed under the License is distributed on an "AS
// IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
// implied. See the License for the specific language governing
// rights and limitations under the License.
// 
// The Original Code is eBus.
// 
// The Initial Developer of the Original Code is Charles W. Rapp.
// Portions created by Charles W. Rapp are
// Copyright (C) 2001 Charles W. Rapp.
// All Rights Reserved.
// 
// Contributor(s): 
//
// RCS ID
// $Id: AsyncSocket.java,v 1.0 2003/11/20 02:02:06 charlesr Exp $
//
// Change Log
// $Log: AsyncSocket.java,v $
// Revision 1.0  2003/11/20 02:02:06  charlesr
// Initial revision
//

